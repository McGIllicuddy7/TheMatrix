#include "feyutils.h"
#include <stdbool.h>
typedef struct{
	bool error_type;
	double * data;
	size_t num_rows;
} matrix_row_t;
typedef struct{
	bool error_type;
	matrix_row_t * data;
	size_t num_rows;
	size_t num_collumns;
}matrix_t;
matrix_t matrix_new_from_args(fey_arena_t * arena,int height, int width,double * args);
matrix_row_t matrix_row_add(matrix_row_t v1, matrix_row_t v2, fey_arena_t * arena);
matrix_row_t matrix_row_sub(matrix_row_t v1, matrix_row_t v2, fey_arena_t * arena);
matrix_row_t matrix_row_mlt(matrix_row_t v1, double v2, fey_arena_t * arena);
matrix_row_t matrix_row_div(matrix_row_t v1, double v2, fey_arena_t * arena);
void matrix_print(matrix_t matrix);
void matrix_insert(matrix_t* matrix, matrix_row_t row, size_t index);

