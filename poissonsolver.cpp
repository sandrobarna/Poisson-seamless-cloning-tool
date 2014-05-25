#include <stdlib.h>
#include "poissonsolver.h"
#include "GpuPoissonSolver.h"

void PoissonSolver::blend(int *mask,
                          uchar *source_red,
                          uchar *source_green,
                          uchar *source_blue,
                          uchar *dest_red,
                          uchar *dest_green,
                          uchar *dest_blue,
                          size_t width, size_t height,
                          int iters,
						  bool mix)
{
    int mask_pix_cnt = 0;
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int* p = &mask[i * width + j];

            if (!i || !j || i == height - 1 || j == width - 1) {
                *p = -1;
            } else if (*p == 1) {
                *p = mask_pix_cnt++;
            }
        }
    }

    _matrix_val          = (char*)  malloc(mask_pix_cnt * sizeof(char) * 5);
    _matrix_col_idx      = (int*)   malloc(mask_pix_cnt * sizeof(int) * 5);
    _matrix_row_ptr      = (int*)   malloc(mask_pix_cnt * sizeof(int));

    _unknown_vect_red    = (float*) malloc(mask_pix_cnt * sizeof(float));
    _unknown_vect_green  = (float*) malloc(mask_pix_cnt * sizeof(float));
    _unknown_vect_blue   = (float*) malloc(mask_pix_cnt * sizeof(float));

    _guidance_vect_red   = (float*) malloc(mask_pix_cnt * sizeof(float));
    _guidance_vect_green = (float*) malloc(mask_pix_cnt * sizeof(float));
    _guidance_vect_blue  = (float*) malloc(mask_pix_cnt * sizeof(float));

    int val_idx = 0;
    int col_idx = 0;
    int row_idx = 0;
    int gui_idx = 0;
    int del[] = { -(int)width, (int)width, -1, 1 };

    for (int i = 1; i < height - 1; i++) {
        for (int j = 1; j < width - 1; j++) {
            int idx = i * (int)width + j;

            if (mask[idx] != -1) {
                int top    = mask[idx - width];
                int left   = mask[idx - 1];
                int right  = mask[idx + 1];
                int bottom = mask[idx + width];

				_guidance_vect_red[gui_idx]   = 0;
				_guidance_vect_green[gui_idx] = 0;
				_guidance_vect_blue[gui_idx]  = 0;
				for (int k = 0; k < 4; k++) {
					if (mix && abs((int)dest_red[idx + del[k]] - (int)dest_red[idx]) > abs((int)source_red[idx + del[k]] - (int)source_red[idx])) {
						_guidance_vect_red[gui_idx] += (int)dest_red[idx + del[k]] - (int)dest_red[idx];
					} else {
						_guidance_vect_red[gui_idx] += (int)source_red[idx + del[k]] - (int)source_red[idx];
					}

					if (mix && abs((int)dest_green[idx + del[k]] - (int)dest_green[idx]) > abs((int)source_green[idx + del[k]] - (int)source_green[idx])) {
						_guidance_vect_green[gui_idx] += (int)dest_green[idx + del[k]] - (int)dest_green[idx];
					} else {
						_guidance_vect_green[gui_idx] += (int)source_green[idx + del[k]] - (int)source_green[idx];
					}

					if (mix && abs((int)dest_blue[idx + del[k]] - (int)dest_blue[idx]) > abs((int)source_blue[idx + del[k]] - (int)source_blue[idx])) {
						_guidance_vect_blue[gui_idx] += (int)dest_blue[idx + del[k]] - (int)dest_blue[idx];
					} else {
						_guidance_vect_blue[gui_idx] += (int)source_blue[idx + del[k]] - (int)source_blue[idx];
					}
				}

                if (top != -1) {
                    _matrix_row_ptr[row_idx++] = val_idx;
                    _matrix_val[val_idx++] = 1;
                    _matrix_col_idx[col_idx++] = top;
                } else {
                    _guidance_vect_red[gui_idx]   -= dest_red[idx - width];
                    _guidance_vect_green[gui_idx] -= dest_green[idx - width];
                    _guidance_vect_blue[gui_idx]  -= dest_blue[idx - width];
                }

                if (left != -1) {
                    if (top == -1)
                        _matrix_row_ptr[row_idx++] = val_idx;

                    _matrix_val[val_idx++] = 1;
                    _matrix_col_idx[col_idx++] = left;
                } else {
                    _guidance_vect_red[gui_idx]   -= dest_red[idx - 1];
                    _guidance_vect_green[gui_idx] -= dest_green[idx - 1];
                    _guidance_vect_blue[gui_idx]  -= dest_blue[idx - 1];
                }

                if (left == -1 && top == -1)
                    _matrix_row_ptr[row_idx++] = val_idx;

                _matrix_val[val_idx++] = -4;
                _matrix_col_idx[col_idx++] = mask[idx];

                if (right != -1) {
                    _matrix_val[val_idx++] = 1;
                    _matrix_col_idx[col_idx++] = right;
                } else {
                    _guidance_vect_red[gui_idx]   -= dest_red[idx + 1];
                    _guidance_vect_green[gui_idx] -= dest_green[idx + 1];
                    _guidance_vect_blue[gui_idx]  -= dest_blue[idx + 1];
                }

                if (bottom != -1) {
                    _matrix_val[val_idx++] = 1;
                    _matrix_col_idx[col_idx++] = bottom;
                } else {
                    _guidance_vect_red[gui_idx]   -= dest_red[idx + width];
                    _guidance_vect_green[gui_idx] -= dest_green[idx + width];
                    _guidance_vect_blue[gui_idx]  -= dest_blue[idx + width];
                }

                _unknown_vect_red[gui_idx]   = source_red[idx];
                _unknown_vect_green[gui_idx] = source_green[idx];
                _unknown_vect_blue[gui_idx]  = source_blue[idx];

                gui_idx++;
            }
        }
    }

    gpuJacobiSolver(_matrix_val,
                    _matrix_col_idx,
                    _matrix_row_ptr,
                    _unknown_vect_red,
                    _unknown_vect_green,
                    _unknown_vect_blue,
                    _guidance_vect_red,
                    _guidance_vect_green,
                    _guidance_vect_blue,
                    mask_pix_cnt,
                    val_idx,
                    iters);

    gui_idx = 0;
    for (int i = 1; i < height - 1; i++) {
        for (int j = 1; j < width - 1; j++) {
            int idx = i * (int)width + j;

            int curr = mask[idx];

            if (curr != -1) {
                dest_red[idx]   = (uchar)_unknown_vect_red[gui_idx];
                dest_green[idx] = (uchar)_unknown_vect_green[gui_idx];
                dest_blue[idx]  = (uchar)_unknown_vect_blue[gui_idx];
                gui_idx++;
            }
        }
    }

    free(_matrix_val);
    free(_matrix_col_idx);
    free(_matrix_row_ptr);
    free(_unknown_vect_red);
    free(_unknown_vect_green);
    free(_unknown_vect_blue);
    free(_guidance_vect_red);
    free(_guidance_vect_green);
    free(_guidance_vect_blue);
}
