#ifndef AYA_INTEGRATORS_VERTEXCM_H
#define AYA_INTEGRATORS_VERTEXCM_H

#include <Core/Integrator.h>

#include <thread>
#include <mutex>

namespace Aya {
	class VertexCMIntegrator : public Integrator {
	public:
		// The sole point of this structure is to make carrying around the ray baggage easier.
		struct PathState {
			Point3 ori;					// Path origin
			Vector3 dir;					// Where to go next
			Spectrum throughput;			// Path throughput

			// Shared a 32-bit field
			uint32_t path_len	 : 30;	// Number of path segments, including this
			bool is_finite_light : 1;	// Just generate by finite light
			bool specular_path	 : 1;	// All scattering events so far were specular

			float dvcm;					// MIS quantity used for vertex connection and merging
			float dvc;					// MIS quantity used for vertex connection
			float dvm;					// MIS quantity used for vertex merging
		};

		// Path vertex, used for merging and connection
		struct PathVertex {
			Spectrum throughput;			// Path throughput (including emission)
			uint32_t path_len;			// Number of segments between source and vertex

			// Stores all required local information, including incoming direction.
			SurfaceIntersection isect;
			Vector3 v_in;

			float dvcm;					// MIS quantity used for vertex connection and merging
			float dvc;					// MIS quantity used for vertex connection
			float dvm;					// MIS quantity used for vertex merging

			// Used by HashGrid
			const Point3& getPosition() const {
				return isect.p;
			}
		};

		class RangeQuery {
		private:
			const VertexCMIntegrator &m_VCM;
			const SurfaceIntersection &m_cam_isect;
			const BSDF *m_cam_bsdf;
			const PathState &m_cam_state;

			Spectrum m_contrib;

		public:
			RangeQuery(
				const VertexCMIntegrator &vcm,
				const SurfaceIntersection &intersection,
				const BSDF *cam_bsdf,
				const PathState &cam_state
			) :
				m_VCM(vcm),
				m_cam_isect(intersection),
				m_cam_bsdf(cam_bsdf),
				m_cam_state(cam_state),
				m_contrib(0.f) {
			}

			const Point3& getPostion() const {
				return m_cam_isect.p;
			}
			const Spectrum& getContrib() const {
				return m_contrib;
			}

			void process(const PathVertex &light_vertex) {
				// Reject if full path length below/above min/max path length
				if ((light_vertex.path_len + m_cam_state.path_len > m_VCM.m_maxDepth) ||
					(light_vertex.path_len + m_cam_state.path_len < m_VCM.m_minDepth))
					return;

				const Vector3 light_dir = light_vertex.isect.n;
				const BSDF *cam_bsdf = m_cam_isect.bsdf;
				Vector3 v_out_cam = -m_cam_state.dir;
				Spectrum cam_bsdf_fac = cam_bsdf->f(v_out_cam, light_dir, m_cam_isect);
				float cam_dir_pdfW = cam_bsdf->pdf(v_out_cam, light_dir, m_cam_isect);
				float rev_cam_dir_pdfW = cam_bsdf->pdf(light_dir, v_out_cam, m_cam_isect);

				if (cam_bsdf_fac.isBlack() || cam_dir_pdfW == 0.f || rev_cam_dir_pdfW == 0.f)
					return;

				// Partial light sub-path MIS weight [tech. rep. (38)]
				const float weight_light = light_vertex.dvcm * m_VCM.m_MIS_VC_weight +
					light_vertex.dvm * m_VCM.MIS(cam_dir_pdfW);

				// Partial eye sub-path MIS weight [tech. rep. (39)]
				const float weight_camera = m_cam_state.dvcm * m_VCM.m_MIS_VC_weight +
					m_cam_state.dvm * m_VCM.MIS(rev_cam_dir_pdfW);

				// Full path MIS weight [tech. rep. (37)]. No MIS for PPM
				const float MIS_weight = m_VCM.m_PPM ?
					1.f :
					1.f / (weight_light + 1.f + weight_camera);

				m_contrib += MIS_weight * cam_bsdf_fac * light_vertex.throughput;
			}
		};

		class HashGrid {
		private:
			BBox m_bbox;
			std::vector<int> m_indices;
			std::vector<int> m_cell_ends;

			float m_radius;
			float m_radius_sqr;
			float m_cell_size;
			float m_cell_size_inv;

		public:
			void reserve(int num) {
				m_cell_ends.resize(num);
			}
			void build(const std::vector<PathVertex> &particles, float radius) {
				m_radius = radius;
				m_radius_sqr = radius * radius;
				m_cell_size = m_radius * 2.f;
				m_cell_size_inv = 1.f / m_cell_size;

				m_bbox = BBox();
				for (size_t i = 0; i < particles.size(); i++) {
					const Point3 &pos = particles[i].getPosition();
					m_bbox.unity(pos);
				}

				m_indices.resize(particles.size());
				memset(&m_cell_ends[0], 0, m_cell_ends.size() * sizeof(int));
				
				// set mCellEnds[x] to number of particles within x
				for (size_t i = 0; i < particles.size(); i++) {
					const Point3 &pos = particles[i].getPosition();
					m_cell_ends[getCellIndex(pos)]++;
				}

				// run exclusive prefix sum to really get the cell starts
				// mCellEnds[x] is now where the cell starts
				int sum = 0;
				for (size_t i = 0; i < m_cell_ends.size(); i++) {
					int temp = m_cell_ends[i];
					m_cell_ends[i] = sum;
					sum += temp;
				}
				for (size_t i = 0; i < particles.size(); i++) {
					const Point3 &pos = particles[i].getPosition();
					const int target_idx = m_cell_ends[getCellIndex(pos)]++;
					m_indices[target_idx] = int(i);
				}
			}
			void process(const std::vector<PathVertex> &particles, RangeQuery &query) const {
				const Point3 query_pos = query.getPostion();
				
				const Vector3 dist_min = query_pos - m_bbox.m_pmin;
				const Vector3 dist_max = m_bbox.m_pmax - query_pos;
				for (int i = 0; i < 3; i++) {
					if (dist_min[i] < 0.f) return;
					if (dist_max[i] < 0.f) return;
				}

				const Vector3 cell_pt = m_cell_size_inv * dist_min;
				const Vector3 coord_f(
					std::floorf(cell_pt.x),
					std::floorf(cell_pt.y),
					std::floorf(cell_pt.z));

				const int px = int(coord_f.x);
				const int py = int(coord_f.y);
				const int pz = int(coord_f.z);

				const Vector3 fract_coord = cell_pt - coord_f;

				const int pxo = px + (fract_coord.x < 0.5f ? -1 : +1);
				const int pyo = py + (fract_coord.y < 0.5f ? -1 : +1);
				const int pzo = pz + (fract_coord.z < 0.5f ? -1 : +1);

				int found = 0;

				for (int j = 0; j < 8; j++) {
					Vector2i active_range;
					switch (j) {
					case 0: active_range = getCellRange(getCellIndex(px, py, pz)); break;
					case 1: active_range = getCellRange(getCellIndex(px, py, pzo)); break;
					case 2: active_range = getCellRange(getCellIndex(px, pyo, pz)); break;
					case 3: active_range = getCellRange(getCellIndex(px, pyo, pzo)); break;
					case 4: active_range = getCellRange(getCellIndex(pxo, py, pz)); break;
					case 5: active_range = getCellRange(getCellIndex(pxo, py, pzo)); break;
					case 6: active_range = getCellRange(getCellIndex(pxo, pyo, pz)); break;
					case 7: active_range = getCellRange(getCellIndex(pxo, pyo, pzo)); break;
					}

					for (; active_range.x < active_range.y; ++active_range.x) {
						const int particle_idx = m_indices[active_range.x];
						const PathVertex &particle = particles[particle_idx];

						const float dist_sqr =
							(query.getPostion() - particle.getPosition()).length2();

						if (dist_sqr <= m_radius_sqr)
							query.process(particle);
					}
				}
			}
			

		private:
			Vector2i getCellRange(int cell_index) const {
				if (!cell_index) {
					return Vector2i(0, m_cell_ends[0]);
				}
				return Vector2i(m_cell_ends[cell_index - 1], m_cell_ends[cell_index]);
			}
			int getCellIndex(const int _x,
				const int _y,
				const int _z) const {
				uint32_t x = uint32_t(_x);
				uint32_t y = uint32_t(_y);
				uint32_t z = uint32_t(_z);

				return int(((x * 73856093) ^ (y * 19349663) ^
					(z * 83492791)) % uint32_t(m_cell_ends.size()));
			}
			int getCellIndex(const Point3 &pos) const {
				Vector3 dis_min = pos - m_bbox.m_pmin;
				return getCellIndex(FloorToInt(dis_min.x * m_cell_size_inv),
					FloorToInt(dis_min.y * m_cell_size_inv),
					FloorToInt(dis_min.z * m_cell_size_inv));
			}
		};

	public:

		enum AlgorithmType {
			// Camera and light vertices merged on first non-specular surface from camera.
			// Cannot handle mixed specular + non-specular materials.
			// No MIS weights (dVCM, dVM, dVC all ignored)
			kPpm,

			// Camera and light vertices merged on along full path.
			// dVCM and dVM used for MIS
			kBpm,

			// Standard bidirectional path tracing
			// dVCM and dVC used for MIS
			kBpt,

			// Vertex connection and mering
			// dVCM, dVM, and dVC used for MIS
			kVcm
		};

	public:
		VertexCMIntegrator(const TaskSynchronizer &task, const uint32_t &spp,
			uint32_t min_depth, uint32_t max_depth,
			const Camera *camera, Film *film,
			AlgorithmType algorithm_type = AlgorithmType::kVcm,
			const float radius_factor = .003f,
			const float radius_alpha = .75f);
		~VertexCMIntegrator() = default;

		void render(const Scene *scene, const Camera *camera, Sampler *sampler, Film *film) override;

		PathState sampleLightSource(const Scene *scene, Sampler *sampler, RNG &rng) const;
		int generateLightPath(const Scene *scene, Sampler *sampler, RNG &rng,
			const Camera *camera, Film *film, const uint32_t max_depth,
			PathVertex *path, int *vertex_count,
			const bool connect_to_cam = true, const int RR_depth = 3) const;
		Spectrum connectToCamera(const Scene *scene, Sampler *sampler, RNG &rng,
			const Camera *camera, Film *film,
			const SurfaceIntersection &intersection, const PathVertex &path_vertex, Point3 *raster_pos) const;
		void sampleCamera(const Scene *scene,
			const Camera *camera, Film *film,
			const RayDifferential &ray, PathState &init_path) const;
		Spectrum connectToLight(const Scene *scene, Sampler *sampler, RNG &rng,
			const RayDifferential &ray, const SurfaceIntersection &intersection, PathState &cam_path) const;
		Spectrum hittingLightSource(const Scene *scene, RNG &rng,
			const RayDifferential &ray, const SurfaceIntersection &intersection, const Light *light, PathState &cam_path) const;

		Spectrum connectVertex(const Scene *scene, RNG &rng,
			const SurfaceIntersection &intersection, const PathVertex &light_vertex, const PathState &cam_path) const;
		bool sampleScattering(const Scene *scene, RNG &rng,
			const RayDifferential &ray, const SurfaceIntersection &intersection, const Sample& bsdf_sample, PathState &path_state,
			const int RR_depth = -1) const;


	private:
		bool m_useVM;				// Vertex merging (of some form) is used
		bool m_useVC;				// Vertex connection (BPT) is used
		bool m_PPM;					// Do PPM, same terminates camera after first merge

		float m_radiusAlpha;		// Radius reduction rate parameter
		float m_baseRadius;			// Initial merging radius
		float m_lightPathCount;	// Number of light sub-paths
		float m_screenPixelCount;	// Number of pixels

		mutable float m_MIS_VC_weight;		// Weight of vertex merging (used in VC)
		mutable float m_MIS_VM_weight;		// Weight of vertex connection (used in VM)
		mutable float m_VM_normalization;	// 1 / (Pi * radius^2 * light_path_count)

		uint32_t m_maxDepth;
		uint32_t m_minDepth;
		const Camera *mp_cam;
		Film *mp_film;

		inline static float MIS(const float val) {
			// Power Heuristic Method
			return val * val;
		}
	};
}

#endif