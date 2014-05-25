#ifndef GPU_POISSON_SOLVER_H
#define GPU_POISSON_SOLVER_H

#ifdef  GPU_POISSON_SOLVER_EXPORTS
#define GPU_POISSON_SOLVER_API __declspec(dllexport)
#else
#define GPU_POISSON_SOLVER_API __declspec(dllimport)
#endif

GPU_POISSON_SOLVER_API void gpuJacobiSolver(char* h_matrix,
                                            int* h_col_idx,
                                            int* h_row_ptr,
                                            float* h_unk_vect_red,
                                            float* h_unk_vect_green,
                                            float* h_unk_vect_blue,
                                            float* h_rhs_vect_red,
                                            float* h_rhs_vect_green,
                                            float* h_rhs_vect_blue,
                                            int matrix_dim,
                                            int matrix_size,
                                            int iters);

#endif
