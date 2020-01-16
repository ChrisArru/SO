#include "base.h" 

struct Matrix {
    int rows; // number of rows
    int cols; // number of columns
    double** data; // a pointer to an array of n_rows pointers to rows
};
typedef struct Matrix Matrix;

Matrix* make_matrix(int n_rows, int n_cols) {
	Matrix* m = malloc( sizeof( Matrix ) );
    //struct Matrix matrix;
    m->rows = n_rows;
    m->cols = n_cols;
    m->data = (double**)malloc(sizeof(double*) * n_rows);
    for(int x = 0; x < n_rows; x++){
        m->data[x] = (double*)calloc(n_cols, sizeof(double));
    }
    //struct Matrix *m;
    //m = &matrix;
    return m;
}

Matrix* copy_matrix(double* data, int n_rows, int n_cols) {
    struct Matrix *matrix = make_matrix(n_rows, n_cols);
    for(int x = 0; x < n_rows; x++) {
        for(int y = 0; y < n_cols; y++) {
            matrix->data[x][y] = data[x+y];
        }
    }
    return matrix;
}

void print_matrix(Matrix* m) {
    for(int x = 0; x < m->rows; x++) {
        for(int y = 0; y < m->cols; y++) {
            printf("%f", m->data[x][y]);
        }
    }
}

void matrix_test(void) {

    double a[] = { 
        1, 2, 3, 
        4, 5, 6, 
        7, 8, 9 };
    Matrix* m1 = copy_matrix(a, 3, 3);
    print_matrix(m1);
}

int main(void) {
    base_init();
    base_set_memory_check(true);
    matrix_test();
    return 0;
}