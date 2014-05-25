#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include "GpuPoissonSolver.h"

__global__ 
void jacobiIter(char* matrix, int* col_idx, int* row_ptr, float* unk_vect, float* rhs_vect, int matrix_dim, int matrix_size)
{
	int idx = blockIdx.x * blockDim.x + threadIdx.x;
	
	if (idx >= matrix_dim) return;

	int n = idx + 1 < matrix_dim ? row_ptr[idx + 1] : matrix_size;

	float sum = 0.f;
	for (int i = row_ptr[idx]; i < n; i++) {
		int j = col_idx[i];
		if (idx != j) {
			sum += matrix[i] * unk_vect[j];
		}
	}

	unk_vect[idx] = (rhs_vect[idx] - sum) / -4.f;
}

__global__
void fitRange(float* red, float* green, float* blue, int len)
{
	int idx = blockIdx.x * blockDim.x + threadIdx.x;
	
	if (idx >= len) return;

	if (red[idx] < 0.f)     red[idx] = 0.f;
	if (red[idx] > 255.f)   red[idx] = 255.f;

	if (green[idx] < 0.f)   green[idx] = 0.f;
	if (green[idx] > 255.f) green[idx] = 255.f;

	if (blue[idx] < 0.f)    blue[idx] = 0.f;
	if (blue[idx] > 255.f)  blue[idx] = 255.f;
}

void gpuJacobiSolver(char* h_matrix, 
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
					 int iters)
{
	char* d_matrix;
	int* d_col_idx;
	int* d_row_ptr;

	float* d_unk_vect_red;
	float* d_unk_vect_green;
	float* d_unk_vect_blue;

	float* d_rhs_vect_red;
	float* d_rhs_vect_green;
	float* d_rhs_vect_blue;


	cudaMalloc((void**) &d_matrix, matrix_size * sizeof(char));
	cudaMalloc((void**) &d_col_idx, matrix_size * sizeof(int));
	cudaMalloc((void**) &d_row_ptr, matrix_dim * sizeof(int));

	cudaMalloc((void**) &d_unk_vect_red, matrix_dim * sizeof(float));
	cudaMalloc((void**) &d_unk_vect_green, matrix_dim * sizeof(float));
	cudaMalloc((void**) &d_unk_vect_blue, matrix_dim * sizeof(float));

	cudaMalloc((void**) &d_rhs_vect_red, matrix_dim * sizeof(float));
	cudaMalloc((void**) &d_rhs_vect_green, matrix_dim * sizeof(float));
	cudaMalloc((void**) &d_rhs_vect_blue, matrix_dim * sizeof(float));


	cudaMemcpy(d_matrix, h_matrix, matrix_size * sizeof(char), cudaMemcpyHostToDevice);
	cudaMemcpy(d_col_idx, h_col_idx, matrix_size * sizeof(int), cudaMemcpyHostToDevice);
	cudaMemcpy(d_row_ptr, h_row_ptr, matrix_dim * sizeof(int), cudaMemcpyHostToDevice);

	cudaMemcpy(d_unk_vect_red, h_unk_vect_red, matrix_dim * sizeof(float), cudaMemcpyHostToDevice);
	cudaMemcpy(d_unk_vect_green, h_unk_vect_green, matrix_dim * sizeof(float), cudaMemcpyHostToDevice);
	cudaMemcpy(d_unk_vect_blue, h_unk_vect_blue, matrix_dim * sizeof(float), cudaMemcpyHostToDevice);
	
	cudaMemcpy(d_rhs_vect_red, h_rhs_vect_red, matrix_dim * sizeof(float), cudaMemcpyHostToDevice);
	cudaMemcpy(d_rhs_vect_green, h_rhs_vect_green, matrix_dim * sizeof(float), cudaMemcpyHostToDevice);
	cudaMemcpy(d_rhs_vect_blue, h_rhs_vect_blue, matrix_dim * sizeof(float), cudaMemcpyHostToDevice);

	int threadsPerBlock = 128;
	int blocks = (matrix_dim / threadsPerBlock) + !!(matrix_dim % threadsPerBlock);
	
	for (int i = 0; i < iters; i++) {
		jacobiIter<<<blocks, threadsPerBlock>>>(d_matrix, d_col_idx, d_row_ptr, d_unk_vect_red, d_rhs_vect_red, matrix_dim, matrix_size);
		jacobiIter<<<blocks, threadsPerBlock>>>(d_matrix, d_col_idx, d_row_ptr, d_unk_vect_green, d_rhs_vect_green, matrix_dim, matrix_size);
		jacobiIter<<<blocks, threadsPerBlock>>>(d_matrix, d_col_idx, d_row_ptr, d_unk_vect_blue, d_rhs_vect_blue, matrix_dim, matrix_size);
	}

	fitRange<<<blocks, threadsPerBlock>>>(d_unk_vect_red, d_unk_vect_green, d_unk_vect_blue, matrix_dim);

	cudaMemcpy(h_unk_vect_red, d_unk_vect_red, matrix_dim * sizeof(float), cudaMemcpyDeviceToHost);
	cudaMemcpy(h_unk_vect_green, d_unk_vect_green, matrix_dim * sizeof(float), cudaMemcpyDeviceToHost);
	cudaMemcpy(h_unk_vect_blue, d_unk_vect_blue, matrix_dim * sizeof(float), cudaMemcpyDeviceToHost);

	cudaFree(d_matrix);
	cudaFree(d_col_idx);
	cudaFree(d_row_ptr);
	cudaFree(d_unk_vect_red);
	cudaFree(d_unk_vect_green);
	cudaFree(d_unk_vect_blue);
	cudaFree(d_rhs_vect_red);
	cudaFree(d_rhs_vect_green);
	cudaFree(d_rhs_vect_blue);
}