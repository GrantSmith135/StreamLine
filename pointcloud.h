#ifndef POINTCLOUD_H
#define POINTCLOUD_H

#include <stdio.h> 
#include "util.h"



//structure representing each point in the point cloud
typedef struct pointcloud {
    double x;
    double y;
    double z;
    double wd;

    struct pointcloud* north;
    struct pointcloud* south;
    struct pointcloud* east;
    struct pointcloud* west;
} pcd_t;

//structure representing the point cloud and its associated data
typedef struct pointcloud_data {
    List points;
    int rows;
    int cols;
} pointcloud_t;

//function prototypes
void stat1();

pointcloud_t* readPointCloudData(FILE *stream);

void assignPointNeighbors(pointcloud_t* pc, int row, int col, pcd_t* point);

void imagePointCloud(pointcloud_t *pc, char* filename);

int initializeWatershed(pointcloud_t *pc);

void watershedAddUniformWater(pointcloud_t* pc, double amount);

void watershedStep(pointcloud_t* pc, double wcoef, double ecoef);

void imagePointCloudWater(pointcloud_t* pc, double maxwd, char* filename);

double compute_f(double t1, double w1, double t2, double w2, double wcoef);

#endif