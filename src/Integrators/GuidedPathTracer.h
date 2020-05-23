#ifndef AYA_INTEGRATORS_GUIDEDPATHTRACER_H
#define AYA_INTEGRATORS_GUIDEDPATHTRACER_H

#include "../Core/Integrator.h"

#include <array>
#include <atomic>
#include <stack>
#include <chrono>
#include <fstream>
#include <functional>
#include <iomanip>
#include <sstream>

namespace Aya {
	enum class SampleCombination {
		Discard,
		DiscardWithAutomaticBudget,
		InverseVariance,
	};

	enum class BsdfSamplingFractionLoss {
		None,
		KL,
		Variance,
	};

	enum class SpatialFilter {
		Nearest,
		StochasticBox,
		Box,
	};

	enum class DirectionalFilter {
		Nearest,
		Box,
	};

	static void addToAtomicFloat(std::atomic<float> &var, float val) {
		auto current = var.load();
		while (!var.compare_exchange_weak(current, current + val));
	}

	inline float logistic(float x) {
		return 1.f / (1.f + std::expf(-x));
	}

	// Implements the stochastic-gradient-based Adam optimizer [Kingma and Ba 2014]
	class AdamOptimizer {
	private:
		struct State {
			int iter = 0;
			float first_moment = 0.f;
			float second_moment = 0.f;
			float variable = 0.f;

			float batch_accumulation = 0.f;
			float batch_gradient = 0.f;
		} m_state;

		struct Hyperparameters {
			float learning_rate;
			int batch_size;
			float epsilon;
			float beta1;
			float beta2;
		} m_hparams;

	public:
		AdamOptimizer(float learning_rate, int batch_size = 1, float epsilon = 1e-08f, float beta1 = 0.9f, float beta2 = 0.999f) {
			m_hparams = { learning_rate ,batch_size , epsilon ,beta1 ,beta2 };
		}
		AdamOptimizer& operator = (const AdamOptimizer &arg) {
			m_state = arg.m_state;
			m_hparams = arg.m_hparams;
			return *this;
		}
		AdamOptimizer(const AdamOptimizer &arg) {
			*this = arg;
		}

		void append(float gradient, float statistical_weight) {
			m_state.batch_gradient += gradient * statistical_weight;
			m_state.batch_accumulation += statistical_weight;

			if (m_state.batch_accumulation > m_hparams.batch_size) {
				step(m_state.batch_gradient / m_state.batch_accumulation);

				m_state.batch_gradient     = 0.f;
				m_state.batch_accumulation = 0.f;
			}

		}

		void step(float gradient) {
			++m_state.iter;

			++m_state.iter;

			float actual_learning_rate = m_hparams.learning_rate * 
				std::sqrtf(1.f - std::powf(m_hparams.beta2, m_state.iter)) / (1.f - std::powf(m_hparams.beta1, m_state.iter));
			m_state.first_moment = m_hparams.beta1 * m_state.first_moment + (1.f - m_hparams.beta1) * gradient;
			m_state.second_moment = m_hparams.beta2 * m_state.second_moment + (1.f - m_hparams.beta2) * gradient * gradient;
			m_state.variable -= actual_learning_rate * m_state.first_moment / (std::sqrtf(m_state.second_moment) + m_hparams.epsilon);

			// Clamp the variable to the range [-20, 20] as a safeguard to avoid numerical instability:
			// since the sigmoid involves the exponential of the variable, value of -20 or 20 already yield
			// in *extremely* small and large results that are pretty much never necessary in practice.
			m_state.variable = Clamp(m_state.variable, -20.f, 20.f);
		}
		float variable() const {
			return m_state.variable;
		}
	};

	class QuadTreeNode {
	private:
		std::array<std::atomic<float>, 4> m_sum;
		std::array<uint16_t, 4> m_children;

	public:
		QuadTreeNode() {
			m_children = {};
			for (size_t i = 0; i < m_sum.size(); ++i) {
				m_sum[i].store(0.f, std::memory_order::memory_order_relaxed);
			}
		}

		void setSum(int idx, float val) {
			m_sum[idx].store(val, std::memory_order::memory_order_relaxed);
		}

		void setSum(float val) {
			for (int i = 0; i < 4; ++i) {
				setSum(i, val);
			}
		}

		float sum(int idx) const {
			return m_sum[idx].load(std::memory_order::memory_order_relaxed);
		}

		QuadTreeNode(const QuadTreeNode &arg) {
			for (int i = 0; i < 4; i++) {
				setSum(i, arg.sum(i));
				m_children[i] = arg.m_children[i];
			}
		}
		QuadTreeNode& operator = (const QuadTreeNode &arg) {
			for (int i = 0; i < 4; i++) {
				setSum(i, arg.sum(i));
				m_children[i] = arg.m_children[i];
			}

			return *this;
		}

		void setChild(int idx, uint16_t val) {
			m_children[idx] = val;
		}

		uint16_t child(int idx) const {
			return m_children[idx];
		}

		int childIndex(Point2f &p) const {
			int res = 0;
			for (int i = 0; i < 2; ++i) {
				if (p[i] < .5f) {
					p[i] *= 2.f;
				}
				else {
					p[i] = (p[i] - .5f) * 2.f;
					res |= 1 << i;
				}
			}

			return res;
		}

		bool isLeaf(int idx) const {
			return child(idx) == 0;
		}


		// Evaluates the directional irradiance *sum density* (i.e. sum / area) at a given location p.
		// To obtain radiance, the sum density (result of this function) must be divided
		// by the total statistical weight of the estimates that were summed up.
		float eval(Point2f &p, const std::vector<QuadTreeNode> &nodes) const {
			assert(p.x >= 0.f && p.x <= 1.f && p.y >= 0.f && p.y <= 1.f);
			const int idx = childIndex(p);
			if (isLeaf(idx)) {
				return 4.f * sum(idx);
			}
			else {
				return 4.f * nodes[child(idx)].eval(p, nodes);
			}
		}

		float pdf(Point2f &p, const std::vector<QuadTreeNode> &nodes) const {
			assert(p.x >= 0.f && p.x <= 1.f && p.y >= 0.f && p.y <= 1.f);
			const int idx = childIndex(p);
			if (!(sum(idx) > 0.f)) {
				return 0.f;
			}

			const float factor = 4.f * sum(idx) / (sum(0) + sum(1) + sum(2) + sum(3));
			if (isLeaf(idx)) {
				return factor;
			}
			else {
				return factor * nodes[child(idx)].pdf(p, nodes);
			}
		}

		int depthAt(Point2f &p, const std::vector<QuadTreeNode> &nodes) const {
			assert(p.x >= 0.f && p.x <= 1.f && p.y >= 0.f && p.y <= 1.f);
			const int idx = childIndex(p);
			if (isLeaf(idx)) {
				return 1;
			}
			else {
				return nodes[child(idx)].depthAt(p, nodes) + 1;
			}
		}

		Point2f sample(Sampler *sampler, const std::vector<QuadTreeNode> &nodes) const {
			int idx = 0;

			float top_left = sum(0);
			float top_right = sum(1);
			float partial = top_left + sum(2);
			float total = partial + top_right + sum(3);

			// Should only happen when there are numerical instabilities.
			if (!(total > 0.f)) {
				return sampler->get2D();
			}

			float boundary = partial / total;
			Point2f origin = Point2f{ 0.f, 0.f };

			float sample = sampler->get1D();

			if (sample < boundary) {
				assert(partial > 0.f);
				sample /= boundary;
				boundary = top_left / partial;
			}
			else {
				partial = total - partial;
				assert(partial > 0.f);
				origin.x = .5f;
				sample = (sample - boundary) / (1.f - boundary);
				boundary = top_right / partial;
				idx |= 1 << 0;
			}

			if (sample < boundary) {
				sample /= boundary;
			}
			else {
				origin.y = .5f;
				sample = (sample - boundary) / (1.f - boundary);
				idx |= 1 << 1;
			}

			if (isLeaf(idx)) {
				return origin + .5f * sampler->get2D();
			}
			else {
				return origin + .5f * nodes[child(idx)].sample(sampler, nodes);
			}
		}

		void record(Point2f &p, float irradiance, std::vector<QuadTreeNode> &nodes) {
			assert(p.x >= 0.f && p.x <= 1.f && p.y >= 0.f && p.y <= 1.f);
			const int idx = childIndex(p);

			if (isLeaf(idx)) {
				addToAtomicFloat(m_sum[idx], irradiance);
			}
			else {
				nodes[child(idx)].record(p, irradiance, nodes);
			}
		}

		float computeOverlappingArea(const Point2f &min1, const Point2f &max1, const Point2f &min2, const Point2f &max2) const {
			float lengths[2];
			for (int i = 0; i < 2; i++) {
				lengths[i] = Max(Min(max1[i], max2[i]) - Max(min1[i], min2[i]), 0.f);
			}

			return lengths[0] * lengths[1];
		}

		void record(const Point2f &origin, float size, Point2f node_origin, float node_size, float value, std::vector<QuadTreeNode> &nodes) {
			float child_size = node_size / 2.f;
			for (int i = 0; i < 4; i++) {
				Point2f child_origin = node_origin;
				if (i & 1) { child_origin[0] += child_size; }
				if (i & 2) { child_origin[1] += child_size; }

				float w = computeOverlappingArea(origin, origin + Point2f(size, size), child_origin, child_origin + Point2f(child_size, child_size));
				if (w > 0.f) {
					if (isLeaf(i)) {
						addToAtomicFloat(m_sum[i], value * w);
					}
					else {
						nodes[child(i)].record(origin, size, child_origin, child_size, value, nodes);
					}
				}
			}
		}

		// Ensure that each quadtree node's sum of irradiance estimates
		// equals that of all its children.
		void build(std::vector<QuadTreeNode>& nodes) {
			for (int i = 0; i < 4; ++i) {
				// During sampling, all irradiance estimates are accumulated in
				// the leaves, so the leaves are built by definition.
				if (isLeaf(i)) {
					continue;
				}

				QuadTreeNode& c = nodes[child(i)];

				// Recursively build each child such that their sum becomes valid...
				c.build(nodes);

				// ...then sum up the children's sums.
				float sum = 0;
				for (int j = 0; j < 4; ++j) {
					sum += c.sum(j);
				}
				setSum(i, sum);
			}
		}
	};

	class DTree {
	private:
		std::vector<QuadTreeNode> m_nodes;

		struct Atomic {
			std::atomic<float> sum;
			std::atomic<float> statistical_weight;
			Atomic() {
				sum.store(0.f, std::memory_order::memory_order_relaxed);
				statistical_weight.store(0.f, std::memory_order::memory_order_relaxed);
			}

			Atomic(const Atomic &arg) {
				*this = arg;
			}

			Atomic& operator = (const Atomic &arg) {
				sum.store(arg.sum.load(std::memory_order::memory_order_relaxed), std::memory_order::memory_order_relaxed);
				statistical_weight.store(arg.statistical_weight.load(std::memory_order::memory_order_relaxed), std::memory_order::memory_order_relaxed);
				return *this;
			}
		} m_atomic;

		int m_maxdepth;

	public:
		DTree() {
			m_atomic.sum.store(0.f, std::memory_order::memory_order_relaxed);
			m_atomic.statistical_weight.store(0.f, std::memory_order::memory_order_relaxed);
			m_maxdepth = 0;
			m_nodes.emplace_back();
			m_nodes.front().setSum(0.f);
		}

		const QuadTreeNode& node(size_t i) const {
			return m_nodes[i];
		}

		float mean() const {
			if (m_atomic.statistical_weight == 0.f) {
				return 0.f;
			}
			const float factor = 1.f / (float(M_PI) * 4.f * m_atomic.statistical_weight);
			return factor * m_atomic.sum;
		}

		void recordIrradiance(Point2f p, float irradiance, float statistical_weight, DirectionalFilter directional_filter) {
			if (std::isfinite(statistical_weight) && statistical_weight > 0.f) {
				addToAtomicFloat(m_atomic.statistical_weight, statistical_weight);

				if (std::isfinite(irradiance) && irradiance > 0.f) {
					if (directional_filter == DirectionalFilter::Nearest) {
						m_nodes[0].record(p, irradiance * statistical_weight, m_nodes);
					}
					else {
						int depth = depthAt(p);
						float size = std::powf(.5f, depth);

						Point2f origin = p;
						origin.x -= size / 2.f;
						origin.y -= size / 2.f;
						m_nodes[0].record(origin, size, Point2f(0.f, 0.f), 1.f, irradiance * statistical_weight / (size * size), m_nodes);
					}
				}
			}
		}

		float pdf(Point2f p) const {
			if (!(mean() > 0.f)) {
				return 1.f / (4.f * float(M_PI));
			}

			return m_nodes[0].pdf(p, m_nodes) / (4.f * float(M_PI));
		}

		int depthAt(Point2f p) const {
			return m_nodes[0].depthAt(p, m_nodes);
		}

		int depth() const {
			return m_maxdepth;
		}

		Point2f sample(Sampler *sampler) const {
			if (!(mean() > 0.f)) {
				return sampler->get2D();
			}

			Point2f res = m_nodes[0].sample(sampler, m_nodes);

			res.x = Clamp(res.x, 0.f, 1.f);
			res.y = Clamp(res.y, 0.f, 1.f);

			return res;
		}

		size_t numNodes() const {
			return m_nodes.size();
		}

		float statisticalWeight() const {
			return m_atomic.statistical_weight;
		}

		float setStatisticalWeight(float statistical_weight) {
			m_atomic.statistical_weight = statistical_weight;
		}

		void reset(const DTree &previous_DTree, int maxdepth, float subdivision_threshold) {
			m_atomic = Atomic{};
			m_maxdepth = 0;
			m_nodes.clear();
			m_nodes.emplace_back();

			struct StackNode {
				size_t node_index;
				size_t other_index;
				const DTree *other_DTree;
				int depth;
			};

			std::stack<StackNode> node_indices;
			node_indices.push({ 0, 0, &previous_DTree, 1 });

			const float total = previous_DTree.m_atomic.sum;

			// Create the topology of the new DTree to be the refined version
			// of the previous DTree. Subdivision is recursive if enough energy is there.
			while (!node_indices.empty()) {
				StackNode node = node_indices.top();
				node_indices.pop();

				m_maxdepth = Max(m_maxdepth, node.depth);

				for (int i = 0; i < 4; i++) {
					const QuadTreeNode &other_node = node.other_DTree->m_nodes[node.other_index];
					const float fraction = total > 0.f ? (other_node.sum(i) / total) : std::powf(.25f, node.depth);
					assert(fraction <= 1.f + float(AYA_EPSILON));

					if (node.depth < maxdepth && fraction > subdivision_threshold) {
						if (!other_node.isLeaf(i)) {
							assert(node.other_DTree == &previous_DTree);
							node_indices.push({ m_nodes.size(), other_node.child(i), &previous_DTree, node.depth + 1 });
						}
						else {
							node_indices.push({ m_nodes.size(), m_nodes.size(), this, node.depth + 1 });
						}

						m_nodes[node.node_index].setChild(i, static_cast<uint16_t>(m_nodes.size()));
						m_nodes.emplace_back();
						m_nodes.back().setSum(other_node.sum(i) / 4.f);

						if (m_nodes.size() > (std::numeric_limits<uint16_t>::max)()) {
							std::exception("DTreeWrapper hit maximum children count.");
							node_indices = std::stack<StackNode>();
							break;
						}
					}
				}
			}

			// Uncomment once memory becomes an issue.
			//m_nodes.shrink_to_fit();

			for (auto& node : m_nodes) {
				node.setSum(0.f);
			}
		}

		size_t approxMemoryFootprint() const {
			return m_nodes.capacity() * sizeof(QuadTreeNode) + sizeof(*this);
		}

		void build() {
			auto& root = m_nodes[0];

			// Build the quadtree recursively, starting from its root.
			root.build(m_nodes);

			// Ensure that the overall sum of irradiance estimates equals
			// the sum of irradiance estimates found in the quadtree.
			float sum = 0.f;
			for (int i = 0; i < 4; ++i) {
				sum += root.sum(i);
			}
			m_atomic.sum.store(sum);
		}
	};

	struct DTreeRecord {
		Vector3 d;
		float radiance, product;
		float wo_pdf, bsdf_pdf, DTree_pdf;
		float statistical_weight;
		bool is_delta;
	};

	struct DTreeWrapper {
	private:
		DTree building;
		DTree sampling;

		AdamOptimizer bsdfSamplingFractionOptimizer{ 0.01f };

		class SpinLock {
		private:
			std::atomic_flag m_mutex;

		public:
			SpinLock() {
				m_mutex.clear(std::memory_order::memory_order_release);
			}

			SpinLock(const SpinLock &other) { m_mutex.clear(std::memory_order::memory_order_release); }
			SpinLock& operator = (const SpinLock &other) { return *this; }

			void lock() {
				while (m_mutex.test_and_set(std::memory_order::memory_order_acquire)) {}
			}
			void unlock() {
				m_mutex.clear(std::memory_order::memory_order_release);
			}
		} m_lock;

	public:
		DTreeWrapper() {
		}

		void record(const DTreeRecord &rec, DirectionalFilter directional_filter, BsdfSamplingFractionLoss sampling_loss) {
			if (!rec.is_delta) {
				float irradiance = rec.radiance / rec.wo_pdf;
				building.recordIrradiance(dirToCanonical(rec.d), irradiance, rec.statistical_weight, directional_filter);
			}

			if (sampling_loss != BsdfSamplingFractionLoss::None && rec.product > 0.f) {
			}
		}

		static Vector3 canonicalToDir(const Point2f &p) {
			const float cos_theta = 2.f * p.x - 1.f;
			const float phi = 2.f * float(M_PI) * p.y;

			const float sin_theta = std::sqrtf(1.f - cos_theta * cos_theta);
			const float sin_phi = std::sinf(phi);
			const float cos_phi = std::cosf(phi);
			
			return Vector3(sin_theta * cos_phi, sin_theta * sin_phi, cos_theta);
		}

		static Point2f dirToCanonical(const Vector3 &d) {
			if (!std::isfinite(d.x) || !std::isfinite(d.y) || !std::isfinite(d.z)) {
				return Point2f(0, 0);
			}

			const float cos_theta = Clamp(d.z, -1.f, 1.f);
			float phi = std::atan2f(d.y, d.x);
			while (phi < 0.f)
				phi += 2.f * float(M_PI);

			return Point2f((cos_theta + 1.f) / 2.f, phi / (2.f * float(M_PI)));
		}

		void build() {
			building.build();
			sampling = building;
		}

		void reset(int maxdepth, float subdivision_threshold) {
			building.reset(sampling, maxdepth, subdivision_threshold);
		}

		Vector3 sample(Sampler *sampler) const {
			return canonicalToDir(sampling.sample(sampler));
		}

		float pdf(const Vector3 &dir) const {
			return sampling.pdf(dirToCanonical(dir));
		}

		int depth() const {
			return sampling.depth();
		}

		size_t numNodes() const {
			return sampling.numNodes();
		}

		float meanRadiance() const {
			return sampling.mean();
		}

		float statisticalWeight() const {
			return sampling.statisticalWeight();
		}

		float statisticalWeightBuilding() const {
			return building.statisticalWeight();
		}

		float setStatisticalWeightBuilding(float statistical_weight) {
			return building.setStatisticalWeight(statistical_weight);
		}

		size_t approxMemoryFootprint() const {
			return building.approxMemoryFootprint() + sampling.approxMemoryFootprint();
		}

		inline float bsdfSamplingFraction(float variable) const {
			return logistic(variable);
		}

		inline float dBsdfSamplingFraction_dVariable(float variable) const {
			float fraction = bsdfSamplingFraction(variable);
			return fraction * (1.f - fraction);
		}

		inline float bsdfSamplingFraction() const {
			return bsdfSamplingFraction(bsdfSamplingFractionOptimizer.variable());
		}

		void optimizeBsdfSamplingFraction(const DTreeRecord& rec, float ratio_power) {
			m_lock.lock();

			// GRADIENT COMPUTATION
			float variable = bsdfSamplingFractionOptimizer.variable();
			float sampling_fraction = bsdfSamplingFraction(variable);

			// Loss gradient w.r.t. sampling fraction
			float mix_pdf = sampling_fraction * rec.bsdf_pdf + (1.f - sampling_fraction) * rec.DTree_pdf;
			float ratio = std::powf(rec.product / mix_pdf, ratio_power);
			float dLoss_dSamplingFraction = -ratio / rec.wo_pdf * (rec.bsdf_pdf - rec.DTree_pdf);

			// Chain rule to get loss gradient w.r.t. trainable variable
			float dLoss_dVariable = dLoss_dSamplingFraction * dBsdfSamplingFraction_dVariable(variable);

			// We want some regularization such that our parameter does not become too big.
			// We use l2 regularization, resulting in the following linear gradient.
			float l2Reg_gradient = .01f * variable;

			float loss_gradient = l2Reg_gradient + dLoss_dVariable;

			// ADAM GRADIENT DESCENT
			bsdfSamplingFractionOptimizer.append(loss_gradient, rec.statistical_weight);

			m_lock.unlock();
		}
	};
}

#endif