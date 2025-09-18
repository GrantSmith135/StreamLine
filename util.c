#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "util.h"

//allocates a 2D array of doubles and initializes all elements to zero
double** allocateArray(int rows, int columns) {
    double** arr = (double **)malloc(rows * sizeof(double *));
    if (arr == NULL) {
        printf("malloc fail\n");
        return NULL;
    }

    //allocate memory for each row and check for allocation failure
    for (int i = 0; i < rows; i++) {
        arr[i] = (double *)malloc(columns * sizeof(double));
        if (arr[i] == NULL) {
            printf("Memory allocation failed on row %d\n", i);
            //free previously allocated rows to prevent memory leaks
            for (int j = 0; j < i; j++) {
                free(arr[j]);
            }
            free(arr);
            return NULL;
        }
    }

    //initialize all elements to 0.0
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            arr[i][j] = 0.0;
        }
    }

    return arr;
}

//frees allocated 2D array
void freeArray(double** arr, int rows) {
    if (arr == NULL) {
        return;
    }
    for (int i = 0; i < rows; i++) {
        free(arr[i]);
    }
    free(arr);
}

//initializes the expanding list
int listInit(List* l, int max_elmt_size) {
    if (l == NULL || max_elmt_size <= 0) {
        return -1;
    }
    
    l->max_size = 10;
    l->max_element_size = max_elmt_size;
    l->size = 0;

    l->data = malloc((size_t)(l->max_size * l->max_element_size));
    if (l->data == NULL) {
        return -1; //malloc fail
    }

    return 0;
}


//add element to end of list
void listAddEnd(List* l, void* elmt) {
    if (l && elmt) {
        if (l->size == l->max_size) {
            size_t new_max_size = l->max_size * 2;
            void* new_data = malloc(new_max_size * l->max_element_size);
            if (!new_data) {
                return; //malloc fail
            }
            memcpy(new_data, l->data, l->size * l->max_element_size);
            free(l->data);
            l->data = new_data;
            l->max_size = new_max_size;
        }
        void* target = (char*)l->data + (l->size * l->max_element_size);
        memcpy(target, elmt, l->max_element_size);
        l->size++;
    }
}

//gets an element from the list at the specified index
void* listGet(List* l, int index) {
    if (!l || index < 0 || index >= l->size) {
        return NULL; 
    }
    return (char*)l->data + (l->max_element_size * index);
}
