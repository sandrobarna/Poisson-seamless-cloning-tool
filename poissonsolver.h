#ifndef POISSONSOLVER_H
#define POISSONSOLVER_H

typedef unsigned char uchar;

class PoissonSolver
{
public:
    void blend(int *mask,
               uchar* source_red,
               uchar* source_green,
               uchar* source_blue,
               uchar* dest_red,
               uchar* dest_green,
               uchar* dest_blue,
               size_t width, size_t height,
               int iters,
			   bool mix);
private:
    char* _matrix_val;
    int*  _matrix_col_idx;
    int*  _matrix_row_ptr;

    float* _unknown_vect_red;
    float* _unknown_vect_green;
    float* _unknown_vect_blue;

    float* _guidance_vect_red;
    float* _guidance_vect_green;
    float* _guidance_vect_blue;
};

#endif // POISSONSOLVER_H
