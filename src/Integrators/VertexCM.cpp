#include "VertexCM.h"

namespace Aya {
	VertexCMIntegrator::VertexCMIntegrator(const TaskSynchronizer &task, const uint32_t &spp,
		uint32_t min_depth, uint32_t max_depth,
		const Camera *camera, Film *film,
		AlgorithmType algorithm_type,
		const float radius_factor,
		const float radius_alpha) :
		Integrator(task, spp),
		m_min_depth(min_depth),
		m_max_depth(max_depth),
		mp_cam(camera),
		mp_film(film),
		m_useVC(false),
		m_useVM(false),
		m_PPM(false) {
		switch (algorithm_type) {
		case kPpm:
			m_PPM   = true;
			m_useVM = true;
			break;
		case kBpm:
			m_useVM = true;
			break;
		case kBpt:
			m_useVC = true;
			break;
		case kVcm:
			m_useVC = true;
			m_useVM = true;
			break;
		default:
			assert((void)(printf("Unknown algorithm requested\n")));
			break;
		}
		
		m_base_radius = radius_factor;
		m_radius_alpha = radius_alpha;

		// While we have the same number of pixels (camera paths)
		// and light paths, we do keep them separate for clarity reasons
		m_light_path_count = float(film->getPixelCount());
		m_screen_pixel_count = float(film->getPixelCount());
	}

	void VertexCMIntegrator::render(const Scene *scene, const Camera *camera, Sampler *sampler, Film *film) const {
		Point3 scene_center;
		float scene_radius;
		scene->worldBound().boundingSphere(&scene_center, &scene_radius);

		// global Lock
		std::mutex mutex;

		std::vector<PathVertex> light_vertices;	// Stored light vertices

		// For light path belonging to pixel index [x] it stores
		// where it's light vertices end (begin is at [x-1])
		BlockedArray<Vector2i> ranges;
		HashGrid grid;

		ranges.init(m_task.getX(), m_task.getY());

		for (uint32_t spp = 0; spp < m_spp; spp++) {
			printf("Rendering %d spp(s)\n", spp);

			int tiles_count = m_task.getTilesCount();
			int height = m_task.getX();
			int width = m_task.getY();

			// Setup our radius, 1st iteration has aIteration == 0, thus offset
			float radius = m_base_radius * scene_radius;
			radius /= std::powf(float(spp + 1), .5f * (1.f - m_radius_alpha));
			// Purely for numeric stability
			radius = Max(radius, 1e-7f);
			const float radius_sqr = radius * radius;

			// Factor used to normalise vertex merging contribution.
			// We divide the summed up energy by disk radius and number of light paths
			m_VM_normalization = 1.f / (radius_sqr * float(M_PI) * m_light_path_count);

			// MIS weight constant [tech. rep. (20)], with n_VC = 1 and n_VM = mLightPathCount
			const float etaVCM = (radius_sqr * float(M_PI)) * m_light_path_count;
			m_MIS_VM_weight = m_useVM ? MIS(etaVCM)			: 0.f;
			m_MIS_VC_weight = m_useVC ? MIS(1.f / etaVCM)	: 0.f;

			// Remove all light vertices and reserve space for some
			light_vertices.reserve(m_task.getX() * m_task.getY());
			light_vertices.clear();

			concurrency::parallel_for(0, tiles_count, [&](int i) {
				//for (int i = 0; i < tiles_count; i++) {
				const RenderTile& tile = m_task.getTile(i);

				UniquePtr<Sampler> tile_sampler(sampler->clone(spp * tiles_count + i));

				RNG rng;
				MemoryPool memory;

				for (int y = tile.min_y; y < tile.max_y; ++y) {
					for (int x = tile.min_x; x < tile.max_x; ++x) {
						if (m_task.aborted())
							return;

						// Randomly select a light source, and generate the light path
						PathVertex *light_path = memory.alloc<PathVertex>(m_max_depth);
						int light_vertex_cnt;
						int light_path_len = generateLightPath(scene, tile_sampler.get(), rng,
							mp_cam, mp_film, m_max_depth + 1,
							light_path, &light_vertex_cnt);
						{
							std::lock_guard<std::mutex> lck(mutex);
							int vertex_start = int(light_vertices.size());
							for (int i = 0; i < light_vertex_cnt; i++) {
								const PathVertex &vert = light_path[i];
								light_vertices.push_back(vert);
							}
							int vertex_end = int(light_vertices.size());

							ranges(x, y) = Vector2i(vertex_start, vertex_end);
						}
					}
				}
				//}
			});

			// Only build grid when merging (VCM, BPM, and PPM)
			if (m_useVM) {
				grid.reserve(m_task.getX() * m_task.getY());
				grid.build(light_vertices, radius);
			}

			concurrency::parallel_for(0, tiles_count, [&](int i) {
				//for (int i = 0; i < tiles_count; i++) {
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

						RayDifferential ray;
						Spectrum L(0.f);
						if (camera->generateRayDifferential(cam_sample, &ray)) {
							// Initialize camera path with eye ray
							PathState cam_path;
							sampleCamera(scene, mp_cam, mp_film, ray, cam_path);

							// Iterate camera path with Path Tracing, and connect it with the light path
							while (true) {
								RayDifferential path_ray(cam_path.ori, cam_path.dir);
								SurfaceIntersection local_isect;

								if (!scene->intersect(path_ray, &local_isect)) {
									// miss intersecting, but directly hitting IBL
									if (scene->getEnviromentLight()) {
										if (cam_path.path_len >= m_min_depth)
											L += cam_path.throughput *
												hittingLightSource(scene, rng, path_ray, local_isect, scene->getEnviromentLight(), cam_path);
									}
									break;
								}
								scene->postIntersect(ray, &local_isect);

								// Update the MIS quantities, following the initialization in
								// GenerateLightSample() or SampleScattering(). Implement equations
								// [tech. rep. (31)-(33)] or [tech. rep. (34)-(36)], respectively.
								{
									float cos_in = Abs(local_isect.n.dot(-path_ray.m_dir));
									cam_path.dvcm *= MIS(local_isect.dist * local_isect.dist);
									cam_path.dvcm /= MIS(cos_in);
									cam_path.dvc /= MIS(cos_in);
									cam_path.dvm /= MIS(cos_in);
								}

								// Add contribution when directly hitting a light source
								if (local_isect.arealight != nullptr) {
									if (cam_path.path_len >= m_min_depth)
										L += cam_path.throughput *
										hittingLightSource(scene, rng, path_ray, local_isect, (Light*)local_isect.arealight, cam_path);
									break;
								}

								if (++cam_path.path_len >= m_max_depth + 1)
									break;

								const BSDF *bsdf = local_isect.bsdf;
								// Vertex connection
								if (!bsdf->isSpecular() && m_useVC) {
									// Connect to light source
									if (cam_path.path_len >= m_min_depth)
										L += cam_path.throughput *
											connectToLight(scene, tile_sampler.get(), rng, path_ray, local_isect, cam_path);

									// Connect to light vertices
									Vector2i range = ranges(x, y);
									for (auto i = range.x; i < range.y; ++i) {
										const PathVertex &light_vertex = light_vertices[i];
										
										if (light_vertex.path_len + cam_path.path_len < m_min_depth)
											continue;

										// Light vertices are stored in increasing path length
										// order; once we go above the max path length, we can
										// skip the rest
										if (light_vertex.path_len + cam_path.path_len > m_max_depth)
											break;

										L += cam_path.throughput * light_vertex.throughput *
											connectVertex(scene, rng, local_isect, light_vertex, cam_path);
									}
								}

								// Vertex merging
								if (!bsdf->isSpecular() && m_useVM) {
									RangeQuery query(*this, local_isect, bsdf, cam_path);
									grid.process(light_vertices, query);
									L += cam_path.throughput * m_VM_normalization * query.getContrib();

									// PPM merges only at the first non-specular surface from camera
									if (m_PPM)
										break;
								}

								// Extend camera path with importance sampling on BSDF
								if (!sampleScattering(scene, rng, path_ray, local_isect, tile_sampler->getSample(), cam_path))
									break;
							}
						}

						film->addSample(cam_sample.image_x, cam_sample.image_y, L);
						memory.freeAll();
					}
				}
				//}
			});

			sampler->advanceSampleIndex();

			film->addSampleCount();
			film->updateDisplay();

			if (m_task.aborted())
				break;
		}
	}

	VertexCMIntegrator::PathState
		VertexCMIntegrator::sampleLightSource(const Scene *scene,
			Sampler *sampler, RNG &rng) const {
		float light_sample = sampler->get1D();
		float light_pick_pdf;
		const Light *light = scene->chooseLightSource(light_sample, &light_pick_pdf);

		Ray light_ray;
		Normal3 emit_dir;
		float emit_pdf, direct_pdf;

		PathState path;
		path.throughput =
			light->sample(sampler->getSample(), sampler->getSample(), &light_ray, &emit_dir, &emit_pdf, &direct_pdf);
		if (emit_pdf == 0.f)
			return path;

		direct_pdf *= light_pick_pdf;
		emit_pdf *= light_pick_pdf;
		path.throughput /= emit_pdf;
		path.is_finite_light = light->isFinite();
		path.specular_path = true;
		path.path_len = 1;
		path.dir = light_ray.m_dir;
		path.ori = light_ray.m_ori;

		float emit_cos = emit_dir.dot(light_ray.m_dir);

		// Light sub-path MIS quantities. Implements [tech. rep. (31)-(33)] partially.
		// The evaluation is completed after tracing the emission ray in the light sub-path loop.
		// Delta lights are handled as well [tech. rep. (48)-(50)].
		path.dvcm = MIS(direct_pdf / emit_pdf);

		if (light->isDelta())
			path.dvc = 0.f;
		else if (light->isFinite())
			path.dvc = MIS(emit_cos / emit_pdf);
		else
			path.dvc = MIS(1.f / emit_pdf);

		path.dvm = path.dvc * m_MIS_VC_weight;

		return path;
	}
	int VertexCMIntegrator::generateLightPath(const Scene *scene, Sampler *sampler, RNG &rng,
		const Camera *camera, Film *film, const uint32_t max_depth,
		PathVertex *path, int *vertex_count,
		const bool connect_to_cam, const int RR_depth) const {
		if (max_depth == 0) {
			*vertex_count = 0;
			return 0;
		}

		// Select a light source, and generate the light path
		PathState light_path = sampleLightSource(scene, sampler, rng);
		if (light_path.throughput.isBlack())
			return 0;
		if (light_path.path_len >= max_depth) {
			*vertex_count = 0;
			return light_path.path_len;
		}

		// Light Tracing
		*vertex_count = 0;
		while (true) {
			Ray path_ray(light_path.ori, light_path.dir);
			SurfaceIntersection intersection;
			if (!scene->intersect(path_ray, &intersection))
				break;
			scene->postIntersect(path_ray, &intersection);

			// Update the MIS quantities before storing them at the vertex.
			// These updates follow the initialization in GenerateLightSample() or
			// SampleScattering(), and together implement equations [tech. rep. (31)-(33)]
			// or [tech. rep. (34)-(36)], respectively.
			float cos_in = Abs(intersection.n.dot(-path_ray.m_dir));
			// Special Case at Infinite Light Source (5.1)
			if (light_path.is_finite_light || light_path.path_len > 1)
				light_path.dvcm *= MIS(intersection.dist * intersection.dist);
			light_path.dvcm /= MIS(cos_in);
			light_path.dvc /= MIS(cos_in);
			light_path.dvm /= MIS(cos_in);

			// Store vertex, unless BSDF is purely specular, which prevents
			// vertex connections and merging
			const BSDF *bsdf = intersection.bsdf;
			if (!bsdf->isSpecular() && (m_useVC || m_useVM)) {
				// Store the light vertex
				PathVertex &light_vertex = path[(*vertex_count)++];
				light_vertex.throughput = light_path.throughput;
				light_vertex.path_len = light_path.path_len;
				light_vertex.isect = intersection;
				light_vertex.v_in = -light_path.dir;
				
				light_vertex.dvc = light_path.dvc;
				light_vertex.dvm = light_path.dvm;
				light_vertex.dvcm = light_path.dvcm;

				// Connect to camera
				if (connect_to_cam && light_path.path_len + 1 >= m_min_depth) {
					Point3 raster;
					Spectrum L =
						connectToCamera(scene, sampler, rng, camera, film, intersection, light_vertex, &raster);
					film->splat(raster.x(), raster.y(), L);
				}
			}

			// Terminate with hard limitation
			if (++light_path.path_len >= max_depth)
				break;

			// Extend light path with importance sampling on BSDF
			if (!sampleScattering(scene, rng, path_ray, intersection, sampler->getSample(), light_path, RR_depth))
				break;
		}

		return light_path.path_len;
	}

	Spectrum VertexCMIntegrator::connectToCamera(const Scene *scene, Sampler *sampler, RNG &rng,
		const Camera *camera, Film *film,
		const SurfaceIntersection &intersection, const PathVertex &path_vertex, Point3 *raster_pos) const {
		Point3 cam_pos = camera->m_pos;
		Vector3 cam_dir = camera->m_dir;

		// Check if the point lies within the raster range
		Vector3 dir2cam;

		if (camera->getCircleOfConfusionRadius() == 0.f) { // Pin Hole Model
			*raster_pos = camera->worldToRaster(intersection.p);
			dir2cam = cam_pos - intersection.p;
		}
		else { // Thin Lens Model
			*raster_pos = camera->worldToRaster(intersection.p);
			Vector2f scr_coord(2.f * float(raster_pos->x()) / float(camera->getResolusionX()) - 1.f,
				2.f * float(raster_pos->y()) / float(camera->getResolusionY()) - 1.f);
			scr_coord.x *= camera->m_ratio;
			scr_coord.y *= -1.f;

			// Sampling on disk lens
			float U, V;
			ConcentricSampleDisk(sampler->get1D(), sampler->get1D(), &U, &V);
			if (Vector2f(scr_coord.x + U, scr_coord.v + V).length() > camera->m_vignette_factor)
				return Spectrum(0.f);
			U *= camera->getCircleOfConfusionRadius();
			V *= camera->getCircleOfConfusionRadius();

			Ray eye_ray;
			eye_ray.m_ori = Point3(U, V, 0.f);
			eye_ray.m_dir = (camera->view(intersection.p) - eye_ray.m_ori).normalize();
			Point3 focal = eye_ray(camera->getFocusDistance() / eye_ray.m_dir.z());

			*raster_pos = camera->cameraToRaster(focal);
			dir2cam = camera->viewInv(eye_ray.m_ori) - intersection.p;
		}

		// Check whether the ray is in front of camera
		if (cam_dir.dot(-dir2cam) <= 0.f || !camera->checkRaster(*raster_pos))
			return Spectrum(0.f);

		float dist2cam = dir2cam.length();
		dir2cam.normalized();

		const BSDF *bsdf = intersection.bsdf;
		Spectrum bsdf_fac = bsdf->f(dir2cam, path_vertex.v_in, intersection, ScatterType(BSDF_ALL & ~BSDF_SPECULAR))
			* Abs(path_vertex.v_in.dot(intersection.n))
			/ Abs(path_vertex.v_in.dot(intersection.gn));
		if (bsdf_fac.isBlack())
			return Spectrum(0.f);

		// Forward and reverse solid angle pdfs for sub-path vertex
		float pdf = bsdf->pdf(path_vertex.v_in, dir2cam, intersection, ScatterType(BSDF_ALL & ~BSDF_SPECULAR));
		float rev_pdf = bsdf->pdf(dir2cam, path_vertex.v_in, intersection, ScatterType(BSDF_ALL & ~BSDF_SPECULAR));
		if (pdf == 0.f || rev_pdf == 0.f)
			return Spectrum(0.f);

		// Compute pdf conversion factor from image plane area to surface area
		float cos_cam2gn = intersection.gn.dot(dir2cam);
		float cos_cam2dir = cam_dir.dot(-dir2cam);
		float raster2cam_dist = camera->getImagePlaneDistance() / cos_cam2dir;
		float raster2SA_fac = raster2cam_dist * raster2cam_dist / cos_cam2dir;
		
		// We put the virtual image plane at such a distance from the camera origin
		// that the pixel area is one and thus the image plane sampling pdf is 1.
		// The area pdf of aHitpoint as sampled from the camera is then equal to
		// the conversion factor from image plane area density to surface area density
		float cam_pdfA = raster2SA_fac * Abs(cos_cam2gn) / (dist2cam * dist2cam);
		
		// nlight is the total number of light sub-paths
		float n_lights = float(film->getPixelCount());
		
		// Partial light sub-path weight [tech. rep. (46)]. Note the division by
		// mLightPathCount, which is the number of samples this technique uses.
		// This division also appears a few lines below in the framebuffer accumulation.
		float weight_light = MIS(cam_pdfA / n_lights) * 
			(m_MIS_VM_weight + path_vertex.dvcm + MIS(rev_pdf) * path_vertex.dvc);

		// Partial eye sub-path weight is 0 [tech. rep. (47)]

		// Full path MIS weight [tech. rep. (37)].
		float MIS_weight = 1.f / (weight_light + 1.f);

		// We divide the contribution by surfaceToImageFactor to convert the (already
		// divided) pdf from surface area to image plane area, w.r.t. which the
		// pixel integral is actually defined. We also divide by the number of samples
		// this technique makes, which is equal to the number of light sub-paths
		Spectrum L = MIS_weight * path_vertex.throughput * bsdf_fac * cam_pdfA / n_lights;

		Ray ray2cam(intersection.p, dir2cam, intersection.m_medium_interface.getMedium(dir2cam, intersection.n), 0.f, dist2cam);
		if (L.isBlack() || scene->occluded(ray2cam))
			return Spectrum(0.f);

		return L;
	}

	void VertexCMIntegrator::sampleCamera(const Scene *scene,
		const Camera *camera, Film *film,
		const RayDifferential &ray, PathState &init_path) const {
		// Compute pdf conversion factor from area on image plane to solid angle on ray
		float cos_cam2dir = camera->m_dir.dot(ray.m_dir);
		float raster2cam_dist = camera->getImagePlaneDistance() / cos_cam2dir;
		
		// We put the virtual image plane at such a distance from the camera origin
		// that the pixel area is one and thus the image plane sampling pdf is 1.
		// The solid angle ray pdf is then equal to the conversion factor from
		// image plane area density to ray solid angle density// We put the virtual image plane at such a distance from the camera origin
        // that the pixel area is one and thus the image plane sampling pdf is 1.
        // The solid angle ray pdf is then equal to the conversion factor from
        // image plane area density to ray solid angle density
		float camera_pdfW = raster2cam_dist * raster2cam_dist / cos_cam2dir;
		float n_lights = float(film->getPixelCount());

		init_path.ori = ray.m_ori;
		init_path.dir = ray.m_dir;
		init_path.throughput = Spectrum(1.f);
		init_path.path_len = 1;
		init_path.specular_path = true;

		// Eye sub-path MIS quantities. Implements [tech. rep. (31)-(33)] partially.
		// The evaluation is completed after tracing the camera ray in the eye sub-path loop.
		init_path.dvcm = MIS(n_lights / camera_pdfW);
		init_path.dvc = 0.f;
		init_path.dvm = 0.f;
	}

	Spectrum VertexCMIntegrator::connectToLight(const Scene *scene, Sampler *sampler, RNG &rng,
		const RayDifferential &ray, const SurfaceIntersection &intersection, PathState &cam_path) const {
		// Sample light source and get radiance
		float light_sample = sampler->get1D();
		float light_pick_pdf;
		const Light *light =
			scene->chooseLightSource(light_sample, &light_pick_pdf);

		const Point3 &pos = intersection.p;
		Vector3 v_in;
		VisibilityTester tester;
		float light_pdfW, emit_pdfW, cos_at_light;
		Spectrum radiance = light->illuminate(intersection, sampler->getSample(),
			&v_in, &tester, &light_pdfW, &cos_at_light, &emit_pdfW);
		if (radiance.isBlack() || light_pdfW == 0.f)
			return Spectrum(0.f);

		const BSDF *bsdf = intersection.bsdf;
		Vector3 v_out = -ray.m_dir;
		Spectrum bsdf_fac = bsdf->f(v_out, v_in, intersection);
		if (bsdf_fac.isBlack())
			return Spectrum(0.f);

		float bsdf_pdfW = bsdf->pdf(v_out, v_in, intersection);
		if (bsdf_pdfW == 0.f)
			return Spectrum(0.f);
		
		// If the light is delta light, we can never hit it
		// by BSDF sampling, so the probability of this path is 0
		if (light->isDelta())
			bsdf_pdfW = 0.f;

		float rev_bsdf_pdfW = bsdf->pdf(v_in, v_out, intersection);
		
		// Partial light sub-path MIS weight [tech. rep. (44)].
		// Note that wLight is a ratio of area pdfs. But since both are on the
		// light source, their distance^2 and cosine terms cancel out.
		// Therefore we can write wLight as a ratio of solid angle pdfs,
		// both expressed w.r.t. the same shading point.
		float weight_light = MIS(bsdf_pdfW / (light_pdfW * light_pick_pdf));
		
		float cos2light = Abs(intersection.n.dot(v_in));

		// Partial eye sub-path MIS weight [tech. rep. (45)].
		//
		// In front of the sum in the parenthesis we have Mis(ratio), where
		//    ratio = emissionPdfA / directPdfA,
		// with emissionPdfA being the product of the pdfs for choosing the
		// point on the light source and sampling the outgoing direction.
		// What we are given by the light source instead are emissionPdfW
		// and directPdfW. Converting to area pdfs and plugging into ratio:
		//    emissionPdfA = emissionPdfW * cosToLight / dist^2
		//    directPdfA   = directPdfW * cosAtLight / dist^2
		//    ratio = (emissionPdfW * cosToLight / dist^2) / (directPdfW * cosAtLight / dist^2)
		//    ratio = (emissionPdfW * cosToLight) / (directPdfW * cosAtLight)
		//
		// Also note that both emissionPdfW and directPdfW should be
		// multiplied by lightPickProb, so it cancels out.
		float weight_camera = MIS(emit_pdfW * cos2light / (light_pdfW * cos_at_light))
			* (m_MIS_VM_weight + cam_path.dvcm + cam_path.dvc * MIS(rev_bsdf_pdfW));
		
		// Full path MIS weight [tech. rep. (37)]
		float MIS_weight = 1.f / (weight_light + 1.f + weight_camera);
		Spectrum L = (MIS_weight * cos2light / (light_pdfW * light_pick_pdf)) * bsdf_fac * radiance;

		if (L.isBlack() || !tester.unoccluded(scene))
			return Spectrum(0.f);

		return L;
	}

	Spectrum VertexCMIntegrator::hittingLightSource(const Scene *scene, RNG &rng,
		const RayDifferential &ray, const SurfaceIntersection &intersection, const Light *light, PathState &cam_path) const {
		float light_pick_pdf = scene->lightPdf(light);
		Vector3 v_out = -ray.m_dir;
		float direct_pdfA;
		float emit_pdfW;
		Spectrum radiance = light->emit(v_out, intersection.gn, &emit_pdfW, &direct_pdfA);

		if (radiance.isBlack())
			return Spectrum(0.f);
		
		// If we see light source directly from camera, no weighting is required
		if (cam_path.path_len == 1)
			return radiance;

		// When using only vertex merging, we want purely specular paths
		// to give radiance (cannot get it otherwise). Rest is handled
		// by merging and we should return 0.
		if (m_useVM && !m_useVC)
			return cam_path.specular_path ? radiance : Spectrum(0.f);

		direct_pdfA *= light_pick_pdf;
		emit_pdfW *= light_pick_pdf;

		// Partial eye sub-path MIS weight [tech. rep. (43)].
		// If the last hit was specular, then dVCM == 0.
		float weight_camera = MIS(direct_pdfA) * cam_path.dvcm + MIS(emit_pdfW) * cam_path.dvc;

		// Partial light sub-path weight is 0 [tech. rep. (42)].

		// Full path MIS weight [tech. rep. (37)].
		float MIS_weight = 1.f / (1.f + weight_camera);

		return MIS_weight * radiance;
	}

	Spectrum VertexCMIntegrator::connectVertex(const Scene *scene, RNG &rng,
		const SurfaceIntersection &intersection, const PathVertex &light_vertex, const PathState &cam_path) const {
		const Point3 &cam_pos = intersection.p;

		// Get the connection
		Vector3 dir2light = light_vertex.isect.p - cam_pos;
		float dist2light2 = dir2light.length2();
		float dist2light = Sqrt(dist2light2);
		dir2light.normalized();

		// Evaluate BSDF at camera vertex
		const BSDF *cam_bsdf = intersection.bsdf;
		const Normal3 &cam_n = intersection.n;
		Vector3 v_out_cam = -cam_path.dir;
		Spectrum cam_bsdf_fac = cam_bsdf->f(v_out_cam, dir2light, intersection);
		float cos_at_cam = cam_n.dot(dir2light);
		float cam_dir_pdfW = cam_bsdf->pdf(v_out_cam, dir2light, intersection);
		float rev_cam_dir_pdfW = cam_bsdf->pdf(dir2light, v_out_cam, intersection);

		if (cam_bsdf_fac.isBlack() || cam_dir_pdfW == 0.f || rev_cam_dir_pdfW == 0.f)
			return Spectrum(0.f);

		// Evaluate BSDF at light vertex
		const BSDF *light_bsdf = light_vertex.isect.bsdf;
		Vector3 dir2cam = -dir2light;
		Spectrum light_bsdf_fac = light_bsdf->f(light_vertex.v_in, dir2cam, light_vertex.isect);
		float cos_at_light = light_vertex.isect.n.dot(dir2cam);
		float light_dir_pdfW = light_bsdf->pdf(light_vertex.v_in, dir2cam, light_vertex.isect);
		float rev_light_dir_pdfW = light_bsdf->pdf(dir2cam, light_vertex.v_in, light_vertex.isect);

		if (light_bsdf_fac.isBlack() || light_dir_pdfW == 0.f || rev_light_dir_pdfW == 0.f)
			return Spectrum(0.f);

		// Compute geometry term
		float g = cos_at_light * cos_at_cam / dist2light2;
		if (g < 0.f) {
			return Spectrum(0.f);
		}

		// Convert pdfs to area pdf
		float cam_dir_pdfA = PdfWToA(cam_dir_pdfW, dist2light, cos_at_light);
		float light_dir_pdfA = PdfWToA(light_dir_pdfW, dist2light, cos_at_cam);

		// Partial light sub-path MIS weight [tech. rep. (40)]
		float weight_light = MIS(cam_dir_pdfA) * 
			(m_MIS_VM_weight + light_vertex.dvcm + MIS(rev_light_dir_pdfW) * light_vertex.dvc);
		
		// Partial eye sub-path MIS weight [tech. rep. (41)]
		float weight_camera = MIS(light_dir_pdfA) * 
			(m_MIS_VM_weight + cam_path.dvcm + MIS(rev_cam_dir_pdfW) * cam_path.dvc);
		
		// Full path MIS weight [tech. rep. (37)]
		float MIS_weight = 1.f / (weight_light + 1.f + weight_camera);
		
		Spectrum L = (MIS_weight * g) * light_bsdf_fac * cam_bsdf_fac;

		Ray ray2light(cam_pos, dir2light, intersection.m_medium_interface.getMedium(dir2light, cam_n), 0.f, dist2light);
		if (L.isBlack() || scene->occluded(ray2light))
			return Spectrum(0.f);

		return L;
	}

	bool VertexCMIntegrator::sampleScattering(const Scene *scene, RNG &rng,
		const RayDifferential &ray, const SurfaceIntersection &intersection, const Sample& bsdf_sample, PathState &path_state,
		const int RR_depth) const {
		// Sample the scattered direction
		const BSDF *bsdf = intersection.bsdf;
		Vector3 v_in;
		float pdf;
		ScatterType sampled_type;
		Spectrum L = bsdf->sample_f(-ray.m_dir, bsdf_sample, intersection, &v_in, &pdf, BSDF_ALL, &sampled_type);

		// Update Path state before walking to the next vertex
		if (L.isBlack() || pdf == 0.f)
			return false;

		bool non_specular = (sampled_type & BSDF_SPECULAR) == 0;

		// For specular bounce, reverse pdf equals to forward pdf
		float rev_pdf = non_specular ? bsdf->pdf(v_in, -ray.m_dir, intersection) : pdf;

		// Apply Russian Roulette if non-specular surface was hit
		if (non_specular && RR_depth != -1 && int(path_state.path_len) > RR_depth) {
			float RR_prob = Min(1.f, path_state.throughput.y());
			if (rng.drand48() < RR_prob) {
				pdf *= RR_prob;
				rev_pdf *= RR_prob;
			}
			else {
				return false;
			}
		}

		path_state.ori = intersection.p;
		path_state.dir = v_in;

		float cos_out = Abs(intersection.n.dot(v_in));
		if (non_specular) {
			path_state.specular_path &= 0;

			// Implements [tech. rep. (34)-(36)] (partially, as noted above)
			path_state.dvcm = MIS(1.f / pdf);
			path_state.dvc = MIS(cos_out / pdf) * 
				(MIS(rev_pdf) * path_state.dvc + path_state.dvcm + m_MIS_VM_weight);
			path_state.dvm = MIS(cos_out / pdf) * 
				(MIS(rev_pdf) * path_state.dvm + path_state.dvcm * m_MIS_VC_weight + 1.f);
		}
		else {
			path_state.specular_path &= 1;

			// Specular scattering case [tech. rep. (53)-(55)] (partially, as noted above)
			path_state.dvcm = 0.f;
			path_state.dvc *= MIS(cos_out);	// rev_pdf / pdf equals to 1
			path_state.dvm *= MIS(cos_out);	// rev_pdf / pdf equals to 1
		}
		path_state.throughput *= L * cos_out / pdf;

		return true;
	}
}