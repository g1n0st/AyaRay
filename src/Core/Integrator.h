#ifndef AYA_CORE_INTEGRATOR_H
#define AYA_CORE_INTEGRATOR_H

#include "../Core/Config.h"
#include "../Core/Camera.h"
#include "../Core/Scene.h"
#include "../Core/Sampler.h"
#include "../Core/Ray.h"
#include "../Core/BSDF.h"

#include <vector>
#include <ppl.h>

namespace Aya {
	struct RenderTile {
		int min_x, min_y, max_x, max_y;
		static const int TILE_SIZE = 32;

		RenderTile(int minx, int miny, int maxx, int maxy)
			: min_x(minx), min_y(miny), max_x(maxx), max_y(maxy) {}
	};

	class TaskSynchronizer {
	private:
		std::vector<RenderTile> m_tiles;

		bool m_abort;

	public:
		TaskSynchronizer(const int x, const int y) {
			init(x, y);
		}

		void init(const int x, const int y) {
			m_tiles.clear();
			m_abort = false;

			for (int i = 0; i < y; i += RenderTile::TILE_SIZE) {
				for (int j = 0; j < x; j += RenderTile::TILE_SIZE) {
					int min_x = j;
					int min_y = i;
					int max_x = Min(min_x + RenderTile::TILE_SIZE, x);
					int max_y = Min(min_y + RenderTile::TILE_SIZE, y);

					m_tiles.emplace_back(min_x, min_y, max_x, max_y);
				}
			}
		}

		inline const RenderTile& getTile(const int idx) const {
			assert(idx < (int)m_tiles.size());
			return m_tiles[idx];
		}
		inline int getTilesCount() const {
			return (int)m_tiles.size();
		}
		inline void setAbort(const bool ab) {
			m_abort = ab;
		}
		inline const bool aborted() const {
			return m_abort;
		}
	};

	class Integrator {
	protected:
		const TaskSynchronizer&m_task;
		const uint32_t &m_spp;

	public:
		Integrator(const TaskSynchronizer &task, const uint32_t &spp)
			: m_task(task), m_spp(spp) {}

		virtual void render(const Scene *scene, const Camera *camera, Sampler *sampler, Film *film) const = 0;
		virtual ~Integrator() {}
	};

	class TiledIntegrator : public Integrator {
	protected:

	public:
		TiledIntegrator(const TaskSynchronizer &task, const uint32_t &spp)
			: Integrator(task, spp) {
		}

		virtual void render(const Scene *scene, const Camera *camera, Sampler *sampler, Film *film) const override;
		virtual Spectrum li(const RayDifferential &ray, const Scene *scene, Sampler *sampler, RNG& rng, MemoryPool &memory) const {
			return Spectrum::fromRGB(0.1, 0.2, 0.3);
		}
		virtual ~TiledIntegrator() {}
	};
}

#endif