#ifndef AYA_SAMPLER_SOBOLMATRICES_H
#define AYA_SAMPLER_SOBOLMATRICES_H

#include <Core/Config.h>

namespace Aya {
		// data source: https://web.maths.unsw.edu.au/~fkuo/sobol/
		// Sobol Matrix Declarations
		static const int num_sobol_dimensions = 1024;
		static const int sobol_matrix_size = 52;
		extern const uint32_t sobol_matrices32[num_sobol_dimensions * sobol_matrix_size];
		extern const uint64_t sobol_matrices64[num_sobol_dimensions * sobol_matrix_size];
		extern const uint64_t VdC_sobol_matrices[][sobol_matrix_size];
		extern const uint64_t VdC_sobol_matrices_inv[][sobol_matrix_size];
}

#endif