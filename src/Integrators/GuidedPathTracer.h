#ifndef AYA_INTEGRATORS_GUIDEDPATHTRACER_H
#define AYA_INTEGRATORS_GUIDEDPATHTRACER_H

#include "../Core/Integrator.h"

#include <array>
#include <atomic>
#include <chrono>
#include <fstream>
#include <functional>
#include <iomanip>
#include <sstream>

namespace Aya {
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
}

#endif