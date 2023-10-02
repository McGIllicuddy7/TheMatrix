#include "feyutils.h"
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include "matrix.h"
bool nearly_eq(double a, double b){
	return fabs(a-b)<=.01;
}
//syntax r1(operator)(some value)->r2
typedef enum{
	error_type,matrix,add,subtract, multiply, divide, value, insert, swap
}operator;
typedef struct{
	operator op;
	long value;
} lexum;
bool str_eq(char * a, char *b){
	int al = strlen(a);
	int bl = strlen(b);
	int l = bl;
	if(al <= bl){
		l = al;
	}
	for(int i = 0; i<l; i++){
		if(a[i] != b[i]){
			return false;
		}
	}
	return true;
}
EnableArrayType(lexum);
void lexum_array_print(lexumArray_t v){
	for(int i =0 ; i<v.len; i++){
		//	error_type,matrix,add,subtract, multiply, divide, value, insert
		if(v.arr[i].op ==error_type){
			printf("erroy_type, ");
		}
		else if(v.arr[i].op ==matrix){
			printf("matrix %ld, ", v.arr[i].value);
		}
		else if(v.arr[i].op == add){
			printf("add, ");
		}
		else if(v.arr[i].op == subtract){
			printf("subtract, ");
		}
		else if(v.arr[i].op == multiply){
			printf("multiply %f ", *(double *)(&v.arr[i].value));
		}
		else if(v.arr[i].op == divide){
			printf("divide %f ", *(double *)(&v.arr[i].value));
		}
		else if(v.arr[i].op == value){
			printf("value %f ", *(double *)(&v.arr[i].value));
		}
		else if(v.arr[i].op == insert){
			printf("insert, ");
		}
	}
	printf("\n");
}
void mat_cleanup(matrix_t * mat){
	for(int i =0 ; i<mat->num_collumns; i++){
		for(int j =0 ; j<mat->num_rows; j++){
			if(nearly_eq(mat->data[i].data[j], -0)){
				mat->data[i].data[j] = 0;
			}
		}
	}
}
lexumArray_t parse_input(fey_arena_t * arena, char * buffer){
	fey_arena_init();
	fstrArray_t v = parse_fstr(local,buffer, "r -> * + - /");
	lexumArray_t out = lexumArray_New(arena);
	for(int i =0; i<v.len; i++){
		fstr s = v.arr[i];
		if(str_eq(s.data,"r")){
			if(strlen(s.data)>1){
				int e = atoi(s.data+1);
				lexumArray_Push(arena, &out,(lexum){matrix, e});
			}
			else{			
				int a = atoi(v.arr[i+1].data);
				lexumArray_Push(arena, &out,(lexum){matrix, a});
				i++;
			}
		}
		if(s.data[0] == '-' && s.data[1] == '>'){
			lexumArray_Push(arena, &out,(lexum){insert, 0});
		}
		if(str_eq(s.data, "swap")){
			lexumArray_Push(arena, &out,(lexum){swap, 0});
		}
		if(str_eq(s.data, "*")){
			double a = atof(v.arr[i+1].data);
			lexumArray_Push(arena, &out,(lexum){multiply, *(long*)(&a)});
			i++;
		}		
		if(str_eq(s.data, "/")){
			double a = atof(v.arr[i+1].data);
			lexumArray_Push(arena, &out,(lexum){divide, *(long*)(&a)});
			i++;
		}
		if(str_eq(s.data, "+")){
			lexumArray_Push(arena, &out,(lexum){add,0});
		}
		if(str_eq(s.data, "-")){
			lexumArray_Push(arena, &out,(lexum){subtract,0});
		}
	}
	return out;
}
void repl(void){
	fey_arena_init();
	printf("enter height: ");
	char buff[100];
	fgets(buff, 100, stdin);
	char buff2[100];
	printf("enter width: ");
	fgets(buff2, 100, stdin);
	int height = atoi(buff);
	int width = atoi(buff2);
	double * arr = fey_arena_alloc(local, sizeof(double)*width*height);
	if(!arr){
		printf("failed");
		exit(1);
	}
	for(int i =0 ; i<height; i++){
restart_entry:
		printf("enter row %d: ", i);
		char buffer[3000];
		fgets(buffer, 3000, stdin);
		fstrArray_t s = parse_fstr(local, buffer,"");
		if(s.len != width){
			printf("wrong number of arguments\n");
			goto restart_entry;
		}
		for(int j =0 ; j<width; j++){
			if(s.arr[j].data[0] != ' ' && s.arr[j].data[0] != ','){
				arr[i*width+j] = atof(s.arr[j].data);
			}
		}
		fey_arena_free(local, s.arr);
	}
	matrix_t mat = matrix_new_from_args(GLOBAL_ARENA, height, width,arr);
	matrix_print(mat);
	fey_arena_hard_reset(local);
	while(true){
	restart_repl:
		fey_arena_hard_reset(local);
		printf("enter operation: ");
		char buffer[100];
		fgets(buffer, 100, stdin);
		if(str_eq(buffer, "exit")){
			break;
		}
		lexumArray_t lex = parse_input(local,buffer);
		//lexum_array_print(lex);
		matrix_row_t r1;
		bool r1loaded = false;
		for(int i =0 ; i<lex.len; i++){
			//	error_type,matrix,add,subtract, multiply, divide, value, insert
			if(lex.arr[i].op == error_type){
				printf("error\n");
				goto restart_repl;
			}
			else if(lex.arr[i].op == matrix){
				if(lex.arr[i+1].op == swap){
					if(lex.arr[i+2].op == matrix){
						int r1ref = lex.arr[i].value;
						int r2ref = lex.arr[i+2].value;
						matrix_row_t temp;
						temp.data = fey_arena_alloc(local, 8*mat.num_rows);
						temp.error_type = false;
						temp.num_rows = mat.num_rows;
						for(int j = 0; j<temp.num_rows; j++){
							temp.data[j] = mat.data[r1ref].data[j];
						}
						for(int j = 0; j<temp.num_rows; j++){
							mat.data[r1ref].data[j] = mat.data[r2ref].data[j];
						}
						for(int j = 0; j<temp.num_rows; j++){
							mat.data[r2ref].data[j] = temp.data[j];
						}
						i+= 2;
					}
					else{
						printf("error");
						goto restart_repl;
					}
				}
				else{
					r1 = mat.data[lex.arr[i].value];
					r1.data = fey_arena_alloc(local,mat.data[lex.arr[i].value].num_rows*sizeof(double));
					for(int j =0; j<mat.num_rows; j++){
						r1.data[j] =  mat.data[lex.arr[i].value].data[j];
					}
					r1loaded = true;
				}
			}
			else if(lex.arr[i].op == add){
				if(!r1loaded){
					printf("error\n");
					goto restart_repl;
				}
				else{
					r1 = matrix_row_add(r1, mat.data[lex.arr[i+1].value],local);
					i++;
				}
			}
			else if(lex.arr[i].op == subtract){
				if(!r1loaded){
					printf("error\n");
					goto restart_repl;
				}
				else{
					r1 = matrix_row_sub(r1, mat.data[lex.arr[i+1].value],local);
					i++;
				}
			}
			else if(lex.arr[i].op == multiply){
				if(!r1loaded){
					printf("error\n");
					goto restart_repl;
				}
				else{
					r1 = matrix_row_mlt(r1, *(double *)(&lex.arr[i].value), local);
					if(r1.error_type){
						printf("error: %f\n", *(double *)(&lex.arr[i].value));
						goto restart_repl;
					}
					printf("%f\n", r1.data[0]);
				}
			}
			else if(lex.arr[i].op == divide){
				if(!r1loaded){
					printf("error\n");
					goto restart_repl;
				}
				else{
					r1 = matrix_row_div(r1, *(double *)(&lex.arr[i].value), local);
					if(r1.error_type){
						printf("error\n");
						goto restart_repl;
					}
					printf("%f\n", r1.data[0]);
				}
			}
			else if(lex.arr[i].op == insert){
				//goto restart_repl;
				if(!r1loaded){
					printf("error\n");
					goto restart_repl;
				}
				i++;
				long v = lex.arr[i+1].value;
				for(int j =0 ; j<mat.num_rows; j++){
					mat.data[v].data[j] = r1.data[j];
				}
			}
		}
		mat_cleanup(&mat);
		form_t frm = analyze_matrix(mat);
		if(frm == not){
			printf("\n not in row echelon form\n");
		}
		else if(frm == row_echelon_form){
			printf("\n in row echelon form\n");
		}
		else if(frm == reduced_row_echelon_form){
			printf("\n in reduced row echelon form\n");
		}
		matrix_print(mat);
	}
	return;
error:
	printf("usage; rn(operator)(some value)->rm");
}

int main(void){
	fey_arena_init();
	repl();
	return 0;
}
