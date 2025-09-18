#ifndef UTIL_H
#define UTIL_H

//cacro to calculate the 1D index for a 2D array
#define INDEX_2D(row, col, num_cols) ((row) * (num_cols) + (col))



//structure representing a list
typedef struct {
    int max_size;
    int max_element_size;
    void* data;
    int size;
} List;


//function prototypes
double** allocateArray(int rows, int cols);

void freeArray(double** arr, int rows);

int listInit(List* l, int max_elmt_size);

void listAddEnd(List* l, void* elmt);

void* listGet(List* l, int index);

#endif