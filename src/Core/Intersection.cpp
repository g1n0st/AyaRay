#include "Intersection.h"

#include "../Lights/AreaLight.h"

namespace Aya {
	Spectrum SurfaceIntersection::emit(const Vector3 & dir) const {
		return arealight ? arealight->emit(dir, gn) : Spectrum(0.f);
	}

	void SurfaceIntersection::computeDifferentials(const RayDifferential& ray) const {
		do {
			if (!ray.m_has_differentials) break;

			float d = -n.dot(p);
			float tx = -(n.dot(ray.m_rx_ori) + d) / n.dot(ray.m_rx_dir);
			if (std::isnan(tx)) break;
			float ty = -(n.dot(ray.m_ry_ori) + d) / n.dot(ray.m_ry_dir);
			if (std::isnan(ty)) break;

			Vector3 v_px = ray.m_rx_ori + ray.m_rx_dir * tx;
			Vector3 v_py = ray.m_ry_ori + ray.m_ry_dir * ty;

			dpdx = v_px - p;
			dpdy = v_py - p;

			auto SolveLinearSystem2x2 = [](const float A[2][2], const float B[2], float *x0, float *x1) -> bool
			{
				float det = A[0][0] * A[1][1] - A[0][1] * A[1][0];
				if (Abs(det) < 1e-10f)
					return false;
				float inv = 1.f / det;
				*x0 = (A[1][1] * B[0] - A[0][1] * B[1]) * inv;
				*x1 = (A[0][0] * B[1] - A[1][0] * B[0]) * inv;
				if (std::isnan(*x0) || std::isnan(*x1))
					return false;
				return true;
			};

			float A[2][2], Bx[2], By[2];
			int axes[2];
			if (Abs(n.x()) > Abs(n.y()) && Abs(n.x()) > Abs(n.z())) {
				axes[0] = 1;
				axes[1] = 2;
			}
			else if (Abs(n.y()) > Abs(n.z())) {
				axes[0] = 0;
				axes[1] = 2;
			}
			else {
				axes[0] = 0;
				axes[1] = 1;
			}

			A[0][0] = dpdu[axes[0]];
			A[0][1] = dpdv[axes[0]];
			A[1][0] = dpdu[axes[1]];
			A[1][1] = dpdv[axes[1]];
			Bx[0] = v_px[axes[0]] - p[axes[0]];
			Bx[1] = v_px[axes[1]] - p[axes[1]];
			By[0] = v_py[axes[0]] - p[axes[0]];
			By[1] = v_py[axes[1]] - p[axes[1]];
			if (!SolveLinearSystem2x2(A, Bx, &dudx, &dvdx))
				dudx = dvdx = 0.f;
			if (!SolveLinearSystem2x2(A, By, &dudy, &dvdy))
				dudy = dvdy = 0.f;

			return;

		} while (0);

		dpdx = dpdy = Vector3(0.f, 0.f, 0.f);
		dudx = dudy = dvdx = dvdy = 0.f;
	}
}