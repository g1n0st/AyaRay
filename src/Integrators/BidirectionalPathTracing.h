#ifndef AYA_BIDIRECTIONALINTEGRATORS_PATHTRACING_H
#define AYA_BIDIRECTIONALINTEGRATORS_PATHTRACING_H

#include <Core/Integrator.h>

namespace Aya {
	class BidirectionalPathTracingIntegrator : public TiledIntegrator {
	public:
		struct PathState {
			Point3 ori;
			Vector3 dir;
			Spectrum throughput;

			// Shared a 32-bit field
			uint32_t path_len		: 30;
			bool is_finite_light		: 1;
			bool specular_path		: 1;

			// Multiple Importance Sampling
			// Implementing Vertex Connection and Merging
			float dvcm;
			float dvc;
		};
		struct PathVertex {
			Spectrum throughput; // Path throughput including emission
			uint32_t path_len;

			// Diffuse surface Intersection local infomation
			SurfaceIntersection isect;
			Vector3 v_in;

			float dvcm;
			float dvc;
		};
	private:
		uint32_t m_maxDepth;
		const Camera *mp_cam;
		Film *mp_film;

	public:
		BidirectionalPathTracingIntegrator(const TaskSynchronizer &task, const uint32_t &spp, uint32_t max_depth,
			const Camera *camera, Film *film) :
			TiledIntegrator(task, spp), m_maxDepth(max_depth),
			mp_cam(camera), mp_film(film) {
		}
		~BidirectionalPathTracingIntegrator() {
		}

		Spectrum li(const RayDifferential &ray, const Scene *scene, Sampler *sampler, RNG &rng, MemoryPool &memory) const override;

		static PathState sampleLightSource(const Scene *scene, Sampler *sampler, RNG &rng);
		static int generateLightPath(const Scene *scene, Sampler *sampler, RNG &rng,
			const Camera *camera, Film *film, const uint32_t max_depth,
			PathVertex *path, int *vertex_count,
			const bool connect_to_cam = true, const int RR_depth = 3);
		static Spectrum connectToCamera(const Scene *scene, Sampler *sampler, RNG &rng,
			const Camera *camera, Film *film,
			const SurfaceIntersection &intersection, const PathVertex &path_vertex, Point3 *raster_pos);
		static void sampleCamera(const Scene *scene,
			const Camera *camera, Film *film,
			const RayDifferential &ray, PathState &init_path);
		static Spectrum connectToLight(const Scene *scene, Sampler *sampler, RNG &rng,
			const RayDifferential &ray, const SurfaceIntersection &intersection, PathState &cam_path);
		static Spectrum hittingLightSource(const Scene *scene, RNG &rng,
			const RayDifferential &ray, const SurfaceIntersection &intersection, const Light *light, PathState &cam_path);

		static Spectrum connectVertex(const Scene *scene, RNG &rng,
			const SurfaceIntersection &intersection, const PathVertex &light_vertex, const PathState &cam_path);
		static bool sampleScattering(const Scene *scene, RNG &rng,
			const RayDifferential &ray, const SurfaceIntersection &intersection, const Sample& bsdf_sample, PathState &path_state,
			const int RR_depth = -1);

		inline static float MIS(const float val) {
			// Power Heuristic Method
			return val * val;
		}
	};
}

#endif