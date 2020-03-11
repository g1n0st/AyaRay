#include "integrator.h"

void Aya::TiledIntegrator::render(const Scene *scene, const Camera *camera, Sampler *sampler, Film *film) const {
	for (uint32_t spp = 0; spp < m_spp; spp++) {
		int tiles_count = m_task.getTilesCount();

		concurrency::parallel_for(0, tiles_count, [&](int i) {
			const RenderTile& tile = m_task.getTile(i);

			UniquePtr<Sampler> tile_sampler(sampler->clone(spp * tiles_count + i));

			RNG rng;
			MemoryPool memory;

			for (int y = tile.min_y; y < tile.max_y; ++y) {
				for (int x = tile.min_x; x < tile.max_x; ++x) {
					if (m_task.aborted())
						return;

					tile_sampler->startPixel(x, y);
					CameraSample cam_sample;
					tile_sampler->generateSamples(x, y, &cam_sample, rng);
					cam_sample.image_x += x;
					cam_sample.image_y += y;

					// When Camera built, there should to modify
					RayDifferential ray = camera->getRay(cam_sample.image_x, cam_sample.image_y);
					Spectrum L = li(ray, scene, tile_sampler.get(), rng, memory);

					film->addSample(cam_sample.image_x, cam_sample.image_y, L);
					memory.freeAll();
				}
			}
		});

		sampler->advanceSampleIndex();

		film->addSampleCount();
		film->updateDisplay();

		if (m_task.aborted())
			break;
	}
}
