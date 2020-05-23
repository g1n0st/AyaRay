#ifndef AYA_INTEGRATORS_MULTIPLEXMLT_H
#define AYA_INTEGRATORS_MULTIPLEXMLT_H

#include <Core/Integrator.h>

#include <thread>
#include <mutex>

namespace Aya {
	class MetropolisSampler : public Sampler {
	private:
		struct PrimarySample {
			float value				= 0.f;
			uint64_t modify			= 0u;	// When did the sample last mutate

			// If mutation rejected, use  backup infomation to restore
			float backup_value		= 0.f;
			uint64_t backup_modify	= 0u;

			void backup() {
				backup_value = value;
				backup_modify = modify;
			}
			void restore() {
				value = backup_value;
				modify = backup_modify;
			}
		};

	private:
		std::vector<PrimarySample> m_samples;

		const float m_sigma;						// Parameter used to control the pace of mutation
		const float m_largeStepProb;

		int m_sampleIdx;
		int m_streamIdx;
		const int stream_count		= 3;

		uint64_t m_largeStepTime	= 0u;		// Number of accepted mutations after last large step
		uint64_t m_time				= 0u;		// Current number of accepted mutations
		bool m_largeStep			= true;

		RNG m_rng;

	public:
		MetropolisSampler(const float sigma,
			const float large_step_prob,
			int sample_idx, int stream_idx,
			uint64_t large_step_time, uint64_t time, bool large_step, RNG rng,
			std::vector<PrimarySample> samples) 
			: m_sigma(sigma), m_largeStepProb(large_step_prob), m_rng(rng),
			m_sampleIdx(sample_idx), m_streamIdx(stream_idx), 
			m_largeStepTime(large_step_time), m_time(time), m_largeStep(large_step),
			m_samples(samples) {
		}
		MetropolisSampler(const float sigma,
			const float large_step_prob,
			const int seed) : m_sigma(sigma), m_largeStepProb(large_step_prob), m_rng(seed) {
		}

		float get1D() override;
		Vector2f get2D() override;
		Sample getSample() override;
		void generateSamples(const int pixel_x, const int pixel_y, CameraSample *samples, RNG &rng) override;

		UniquePtr<Sampler> clone(const int seed) const override;
		UniquePtr<Sampler> deepClone() const override;

		void startIteration();
		void accept();
		void reject();
		void mutate(const int idx);

		void startStream(const int idx) {
			assert(idx < stream_count);
			m_streamIdx = idx;
			m_sampleIdx = 0;
		}

	private:
		int getNextIndex() {
			return m_streamIdx + stream_count * m_sampleIdx++;
		}
	};

	class MultiplexMLTIntegrator : public Integrator {
	private:
		const Camera *mp_camera;
		Film *mp_film;
		uint32_t m_maxDepth;

		int m_numBootstrap;
		int m_numChains;
		float m_sigma;
		float m_largeStepProb;

		mutable std::mutex m_mutex;

	public:
		MultiplexMLTIntegrator(const TaskSynchronizer &task, const uint32_t &spp,
			uint32_t max_depth, const Camera *camera, Film *film,
			int num_bootstrap = 1 << 17, int num_chains = 1 << 11, float sigma = .01f, float large_step_prob = .3f)
			: Integrator(task, spp),
			m_maxDepth(max_depth), mp_camera(camera), mp_film(film),
			m_numBootstrap(num_bootstrap), m_numChains(num_chains),
			m_sigma(sigma), m_largeStepProb(large_step_prob) {
		}
		~MultiplexMLTIntegrator() = default;

		void render(const Scene *scene, const Camera *camera, Sampler *sampler, Film *film) const override;

	public:
		Spectrum evalSample(const Scene *scene, MetropolisSampler *sampler,
			const int connect_depth, Vector2f *raster_pos,
			RNG &rng, MemoryPool &memory) const;
	};
}

#endif