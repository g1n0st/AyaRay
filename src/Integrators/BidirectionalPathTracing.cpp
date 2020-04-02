#include "BidirectionalPathTracing.h"

namespace Aya {
	Spectrum BidirectionalPathTracingIntegrator::li(const RayDifferential &ray, 
		const Scene *scene, Sampler *sampler, RNG &rng, MemoryPool &memory) const {
		// Randomly select a light source, and generate the light path
		PathVertex *light_path = memory.alloc<PathVertex>(m_max_depth);
		int light_vertex_cnt;
		int light_path_len = generateLightPath(scene, sampler, rng,
			mp_cam, mp_film, m_max_depth,
			light_path, &light_vertex_cnt);

		// Initialize camera path with eye ray
		PathState cam_path;
		sampleCamera(scene, mp_cam, mp_film, ray, cam_path);

		// Iterate camera path with Path Tracing, and connect it with the light path
		Spectrum L(0.f);
		while (true) {
			RayDifferential path_ray(cam_path.ori, cam_path.dir);
			SurfaceIntersection local_isect;

			if (!scene->intersect(path_ray, &local_isect)) {
				// miss intersecting, but directly hitting IBL
				if (scene->getEnviromentLight()) {
					++cam_path.path_len;
					L += cam_path.throughput * 
						hittingLightSource(scene, rng, path_ray, local_isect, scene->getEnviromentLight(), cam_path);
				}
				break;
			}
			scene->postIntersect(ray, &local_isect);

			// Update MIS quantities from iteration (34) (35)
			// Divide by g_i-> factor, Forward pdf conversion factor from solid angle measure to area measure (4) (8)
			float cos_in = Abs(local_isect.n.dot(-path_ray.m_dir));
			cam_path.dvcm *= MIS(local_isect.dist * local_isect.dist);
			cam_path.dvcm /= MIS(cos_in);
			cam_path.dvc /= MIS(cos_in);

			// Add contribution when directly hitting a light source
			if (local_isect.arealight != nullptr) {
				++cam_path.path_len;
				L += cam_path.throughput *
					hittingLightSource(scene, rng, path_ray, local_isect, (Light*)local_isect.arealight, cam_path);
				break;
			}
			if (++cam_path.path_len >= m_max_depth + 2)
				break;

			const BSDF *bsdf = local_isect.bsdf;
			// Excute vertex connection between light and camera on Diffuse Surface
			if (!bsdf->isSpecular()) {
				// Connect to light source
				L += cam_path.throughput *
					connectToLight(scene, sampler, rng, path_ray, local_isect, cam_path);
				
				// Connect to light vertices
				for (auto i = 0; i < light_vertex_cnt; ++i) {
					const PathVertex &light_vertex = light_path[i];
					if (light_vertex.path_len + cam_path.path_len > m_max_depth + 2)
						break;

					L += cam_path.throughput * 
						connectVertex(scene, rng, local_isect, light_vertex, cam_path);
				}
			}

			// Extend camera path with importance sampling on BSDF
			if (!sampleScattering(scene, rng, path_ray, local_isect, sampler->getSample(), cam_path))
				break;
		}

		return L;
	}

	BidirectionalPathTracingIntegrator::PathState 
		BidirectionalPathTracingIntegrator::sampleLightSource(const Scene *scene,
			Sampler *sampler, RNG &rng) {
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
		path.dvcm = MIS(direct_pdf / emit_pdf);

		// Such light sources cannot be hit by random rays, as their emission is defined via a delta distribution, 
		// i.e. we have pVC,0 = 0. (48) (49)
		if (light->isDelta())
			path.dvc = 0.f;
		// formula (31) (32)
		else if (light->isFinite())
			path.dvc = MIS(emit_cos / emit_pdf);
		// handling infinite lights via solid anglg integration, 
		// and derive the corresponding path pdfs for use in MIS.
		else
			path.dvc = MIS(1.f / emit_pdf);

		return path;
	}

	int BidirectionalPathTracingIntegrator::generateLightPath(const Scene *scene, Sampler *sampler, RNG &rng,
		const Camera *camera, Film *film, const uint32_t max_depth,
		PathVertex *path, int *vertex_count,
		const bool connect_to_cam, const int RR_depth) {
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
			
			// Update MIS quantities from iteration (34) (35)
			// Divide by g_i-> factor, Forward pdf conversion factor from solid angle measure to area measure (4) (8)
			float cos_in = Abs(intersection.n.dot(-path_ray.m_dir));
			// Special Case at Infinite Light Source (5.1)
			if (light_path.is_finite_light || light_path.path_len > 1)
				light_path.dvcm *= MIS(intersection.dist * intersection.dist);
			light_path.dvcm /= MIS(cos_in);
			light_path.dvc /= MIS(cos_in);

			// Store the current vertex and connect the light vertex with camera
			// Unless hitting specular surface
			const BSDF *bsdf = intersection.bsdf;
			if (!bsdf->isSpecular()) {
				// Store the light vertex
				PathVertex &light_vertex = path[(*vertex_count)++];
				light_vertex.throughput = light_path.throughput;
				light_vertex.path_len = light_path.path_len + 1;
				light_vertex.isect = intersection;
				light_vertex.v_in = -light_path.dir;
				light_vertex.dvc = light_path.dvc;
				light_vertex.dvcm = light_path.dvcm;

				// Connect to camera
				if (connect_to_cam) {
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
			if (!sampleScattering(scene, rng, path_ray, intersection, sampler->getSample(), light_path))
				break;
		}

		return light_path.path_len;
	}

	Spectrum BidirectionalPathTracingIntegrator::connectToCamera(const Scene *scene, Sampler *sampler, RNG &rng,
		const Camera *camera, Film *film,
		const SurfaceIntersection &intersection, const PathVertex &path_vertex, Point3 *raster_pos) {
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
			dir2cam = camera->view(eye_ray.m_ori) - intersection.p;
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

		float cos_cam2gn = intersection.gn.dot(dir2cam);
		float cos_cam2dir = cam_dir.dot(-dir2cam);
		float raster2cam_dist = camera->getImagePlaneDistance() / cos_cam2dir;
		float raster2SA_fac = raster2cam_dist * raster2cam_dist / cos_cam2dir;	// Camera solid angle to area pdf factor
		float cam_pdfA = raster2SA_fac * Abs(cos_cam2gn) / (dist2cam * dist2cam); // Intersection solid angle to area pdf factor
		// nlight is the total number of light sub-paths
		float n_lights = float(film->getPixelCount());
		// Vertex connection (t = 1). formula (46)
		// The light sub-path vertex y_s-1 is connected to vertex z0 on the eye lens (a.k.a. eye/camera projectin)
		float weight_light = MIS(cam_pdfA / n_lights) * (path_vertex.dvcm + path_vertex.dvc * MIS(pdf)); 
		float MIS_weight = 1.f / (weight_light + 1.f);
		Spectrum L = MIS_weight * path_vertex.throughput * bsdf_fac * cam_pdfA / n_lights;

		Ray ray2cam(intersection.p, dir2cam, intersection.m_medium_interface.getMedium(dir2cam, intersection.n), dist2cam);
		if (L.isBlack() || scene->occluded(ray2cam))
			return Spectrum(0.f);

		return L;
	}
}