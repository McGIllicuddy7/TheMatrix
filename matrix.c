#include "matrix.h"
#include <stdio.h>
#include <stdarg.h>
matrix_row_t matrix_row_add(matrix_row_t v1, matrix_row_t v2, fey_arena_t * arena){
    matrix_row_t out;
    out.error_type = v1.num_rows != v2.num_rows;
    if(out.error_type){
        out.data = NULL;
        out.num_rows = 0;
        return out;
    }
    out.data = fey_arena_alloc(arena, sizeof(double)*v1.num_rows);
    out.num_rows = v1.num_rows;
    for(int i = 0; i<v1.num_rows; i++){
        out.data[i] = v1.data[i]+v2.data[i];
    }
    return out;
}
matrix_row_t matrix_row_sub(matrix_row_t v1, matrix_row_t v2, fey_arena_t * arena){
    matrix_row_t out;
    out.error_type = v1.num_rows != v2.num_rows;
    if(out.error_type){
        out.data = NULL;
        out.num_rows = 0;
        return out;
    }
    out.data = fey_arena_alloc(arena, sizeof(double)*v1.num_rows);
    out.num_rows = v1.num_rows;
    for(int i = 0; i<v1.num_rows; i++){
        out.data[i] = v1.data[i]-v2.data[i];
    }
    return out;
}
matrix_row_t matrix_row_mlt(matrix_row_t v1, double v2, fey_arena_t * arena){
    matrix_row_t out;
    out.error_type = false;
    if(out.error_type){
        out.data = NULL;
        out.num_rows = 0;
        return out;
    }
    out.data = fey_arena_alloc(arena, sizeof(double)*v1.num_rows);
    out.num_rows = v1.num_rows;
    for(int i = 0; i<v1.num_rows; i++){
        out.data[i] = v1.data[i]*v2;
    }
    return out;
}
matrix_row_t matrix_row_div(matrix_row_t v1, double v2, fey_arena_t * arena){
    matrix_row_t out;
    out.error_type = v2 == 0;
    if(out.error_type){
        out.data = NULL;
        out.num_rows = 0;
        return out;
    }
    out.data = fey_arena_alloc(arena, sizeof(double)*v1.num_rows);
    out.num_rows = v1.num_rows;
    for(int i = 0; i<v1.num_rows; i++){
        out.data[i] = v1.data[i]/v2;
    }
    return out;
}
void matrix_print(matrix_t matrix){
    printf("--");
    for(int i = 0; i<matrix.num_rows*10-1; i++){
        printf(" ");
    }
    printf("--\n");
    for(int i = 0; i<matrix.num_collumns; i++){
        printf("| ");
        for(int j = 0; j<matrix.num_rows; j++){
            printf("%f, ", (matrix.data[i]).data[j]);
        }
        printf("|\n");
    }
    printf("--");
    for(int i = 0; i<matrix.num_rows*10-1; i++){
        printf(" ");
    }
    printf("--\n");
}
void matrix_insert(matrix_t* matrix, matrix_row_t row, size_t index){
    if(index<matrix->num_collumns){
        matrix->data[index] = row;
    }
}
matrix_t matrix_new_from_args(fey_arena_t * arena,int height, int width, double * args){
    matrix_t out;
    out.data = fey_arena_alloc(arena, sizeof(matrix_row_t)*height);
    out.error_type = false;
    out.num_collumns = height;
    out.num_rows = width;
    for(int i = 0; i<height; i++){
        out.data[i].data = fey_arena_alloc(arena, width*sizeof(double));
        out.data[i].error_type = false;
        out.data[i].num_rows = width;
        for(int j = 0; j<width; j++){
          //  printf("<i:%d, j:%d> ", i, j);
            out.data[i].data[j] = args[i*width+j];
        }
    }
    return out;
}
form_t analyze_matrix(matrix_t mat){
	int last_index = -1;
	bool reduced = true;
	for(int i = 0; i<mat.num_collumns; i++){
		int j = 0;
		while(mat.data[i].data[j] == 0 && j<mat.num_rows){
			j++;
		}
		double f = mat.data[i].data[j];
		if(last_index>j){
			return not;
		}
		else{
			if(mat.data[i].data[j] != 1){
				reduced = false;
			}
			else{
				j = j+1;
				while (j<mat.num_rows){
					if(mat.data[i].data[j] != 0){
						reduced = false;
					}
					j++;
				}
			}
			last_index = j;
		}
	}
	if(reduced){
		return reduced_row_echelon_form;
	}
	return row_echelon_form;
}