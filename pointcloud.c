#include "pointcloud.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include "bmp.h"
#include <math.h>



void stat1() {
    //initialize all variables needed
    float x, y, z;
    float max = -1e6;
    float min = 1e6;
    int count = 0;
    float sum = 0;
    float average;

    //loop to get all tuples from stream
    while (scanf("%f %f %f", &x, &y, &z) == 3) {
        count++;
        sum += z;

        //calculate min and max
        if (z > max) {
            max = z;
        }
        if (z < min) {
            min = z;
        }
    }

    if (count > 0) {
        average = sum / count;
        printf("Max height: %f\n", max);
        printf("Min height: %f\n", min);
        printf("Average height: %f\n", average);
    } else {
        printf("No valid point cloud data provided\n");
    }
}

pointcloud_t* readPointCloudData(FILE *stream) {
    //initialize variables
    int numColumns;
    float x, y, z;
    pcd_t point;

    //allocate memory for the pointcloud structure
    pointcloud_t* pointcloud = (pointcloud_t*)malloc(sizeof(pointcloud_t));
    if (!pointcloud) {
        fprintf(stderr, "malloc failed\n");
        return NULL;
    }

    //create the expanding list for points
    if (listInit(&pointcloud->points, sizeof(pcd_t)) != 0) {
        free(pointcloud);
        fprintf(stderr, "list initialization failed\n");
        return NULL;
    }

    //read the number of columns from the file
    if (fscanf(stream, "%d", &numColumns) != 1) {
        free(pointcloud->points.data);
        free(pointcloud);
        fprintf(stderr, "unable to read number of columns\n");
        return NULL;
    }

    pointcloud->cols = numColumns;
    pointcloud->rows = 0;

    //read the point data from the file
    while (fscanf(stream, "%f %f %f", &x, &y, &z) == 3) {
        point.x = x;
        point.y = y;
        point.z = z;
        point.wd = 0; 
        point.north = point.south = point.east = point.west = NULL;
        listAddEnd(&pointcloud->points, &point);
        pointcloud->rows++;
    }

    //calculate the actual number of rows based on the total points read
    if (pointcloud->rows % pointcloud->cols != 0) {
        fprintf(stderr, "mismatch between points read and number of columns\n");
        free(pointcloud->points.data);
        free(pointcloud);
        return NULL;
    }

    //update rows to reflect correct number of rows calculated from total points and columns
    pointcloud->rows = pointcloud->rows / pointcloud->cols;

    return pointcloud;
}

//not used anymore (replaced with imagePointCloudWater)
void imagePointCloud(pointcloud_t* pc, char* filename) {
    if (!pc || !filename || filename[0] == '\0') {
        fprintf(stderr, "Null pointer\n");
        return;
    }

    //determine the width and height of the point cloud
    int width = pc->cols;
    int height = pc->rows;

    if (width <= 0 || height <= 0) {
        fprintf(stderr, "Invalid point cloud dimensions\n");
        return;
    }

    //create new bitmap image
    Bitmap* image = bm_create(width, height);
    if (!image) {
        fprintf(stderr, "Unable to create bitmap image\n");
        return;
    }

    //determine min and max height
    double minZ = ((pcd_t*)listGet(&pc->points, 0))->z;
    double maxZ = ((pcd_t*)listGet(&pc->points, 0))->z;
    for (int i = 1; i < pc->rows * pc->cols; i++) {
        pcd_t* point = (pcd_t*)listGet(&pc->points, i);
        if (point) {
            if (point->z < minZ) {
                minZ = point->z;
            }
            if (point->z > maxZ) {
                maxZ = point->z;
            }
        }
    }

    if (minZ == maxZ) {
        maxZ += 1.0; //avoid divide by 0
    }

    //assign values based on height
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            int index = row * width + col;
            pcd_t* point = (pcd_t*)listGet(&pc->points, row);

            double zRange = maxZ - minZ;
            double normalizedZ = (zRange > 0) ? (point->z - minZ) / zRange : 0;
            unsigned char intensity = (unsigned char)(normalizedZ * 255);

            bm_set(image, col, row, bm_rgb(intensity, intensity, intensity));
        }
    }

    //save image to file
    if (bm_save(image, filename) == 0) {
        fprintf(stderr, "Unable to save bitmap image %s.\n", filename);
    }

    //free the image
    bm_free(image);
}

//helper function to assign neighbors
void assignPointNeighbors(pointcloud_t* pc, int row, int col, pcd_t* point) {
    point->north = (row > 0) ? (pcd_t*)listGet(&pc->points, INDEX_2D(row - 1, col, pc->cols)) : NULL;
    point->south = (row < pc->rows - 1) ? (pcd_t*)listGet(&pc->points, INDEX_2D(row + 1, col, pc->cols)) : NULL;
    point->west = (col > 0) ? (pcd_t*)listGet(&pc->points, INDEX_2D(row, col - 1, pc->cols)) : NULL;
    point->east = (col < pc->cols - 1) ? (pcd_t*)listGet(&pc->points, INDEX_2D(row, col + 1, pc->cols)) : NULL;
}

int initializeWatershed(pointcloud_t* pc) {
    if (!pc) {
        fprintf(stderr, "error null pointer\n");
        return -1;
    }

    //2d array for water values
    double** waterDepthArray = allocateArray(pc->rows, pc->cols);
    if (!waterDepthArray) {
        fprintf(stderr, "malloc fail\n");
        return -1;
    }

    //set the water depth
    for (int row = 0; row < pc->rows; row++) {
        for (int col = 0; col < pc->cols; col++) {
            int index = INDEX_2D(row, col, pc->cols);
            pcd_t* point = (pcd_t*)listGet(&pc->points, index);
            if (point) {
                point->wd = waterDepthArray[row][col];
            }
        }
    }

    //free the array
    freeArray(waterDepthArray, pc->rows);

    //set north, south, east, and west pointers
    for (int row = 0; row < pc->rows; row++) {
        for (int col = 0; col < pc->cols; col++) {
            int index = INDEX_2D(row, col, pc->cols);
            pcd_t* point = (pcd_t*)listGet(&pc->points, index);
            if (point) {
                assignPointNeighbors(pc, row, col, point);
            }
        }
    }

    return 0;
}

void watershedAddUniformWater(pointcloud_t* pc, double amount) {
    if (!pc) {
        fprintf(stderr, "null pointer provided\n");
        return; //null pointer
    }

    for (int i = 0; i < pc->rows * pc->cols; i++) {
        pcd_t* point = (pcd_t*)listGet(&pc->points, i);
        if (point) {
            point->wd += amount;
        }
    }
}

void watershedStep(pointcloud_t* pc, double wcoef, double ecoef) {
    if (!pc) {
        fprintf(stderr, "null pointer provided\n");
        return; //null pointer
    }

    //create an array to store the new water values for each point
    double** newWater = allocateArray(pc->rows, pc->cols);
    if (!newWater) {
        fprintf(stderr, "malloc fail\n");
        return;
    }

    //calculate new water values for each point
    for (int row = 0; row < pc->rows; row++) {
        for (int col = 0; col < pc->cols; col++) {
            int index = INDEX_2D(row, col, pc->cols);
            pcd_t* point = (pcd_t*)listGet(&pc->points, index);
            if (!point) {
                newWater[index] = 0;
                continue;
            }

            double cw = point->wd;
            double ce = point->z;
            double newValue = 0;

            //compute contributions from neighboring cells
            if (point->west) {
                double We = point->west->z;
                double Ww = point->west->wd;
                double flow = compute_f(ce, cw, We, Ww, wcoef);
                newValue += flow;
            }

            if (point->east) {
                double ee = point->east->z;
                double ew = point->east->wd;
                double flow = compute_f(ce, cw, ee, ew, wcoef);
                newValue += flow;
            }

            if (point->north) {
                double ne = point->north->z;
                double nw = point->north->wd;
                double flow = compute_f(ce, cw, ne, nw, wcoef);
                newValue += flow;
            }

            if (point->south) {
                double se = point->south->z;
                double sw = point->south->wd;
                double flow = compute_f(ce, cw, se, sw, wcoef);
                newValue += flow;
            }

            //apply evaporation and calculate new water value for point
            newValue -= cw * ecoef;
            newWater[row][col] = cw + newValue;
        }
    }

    //update water values for each point
    for (int row = 0; row < pc->rows; row++) {
        for (int col = 0; col < pc->cols; col++) {
            int index = INDEX_2D(row, col, pc->cols);
            pcd_t* point = (pcd_t*)listGet(&pc->points, index);
            if (point) {
                point->wd = newWater[row][col];
            }
        }
    }

    //free the memory allocated for new water values
    free(newWater);
}

//function to compute the f function
double compute_f(double t1, double w1, double t2, double w2, double wcoef) {
    return ((t2 + w2) - (t1 + w1)) * wcoef;
}


void imagePointCloudWater(pointcloud_t* pc, double maxwd, char* filename) {
    if (!pc || !filename) {
        fprintf(stderr, "Error Null pointer\n");
        return;
    }

    //determine width and height of the point cloud
    int width = pc->cols;
    int height = pc->rows;

    //create new bitmap image
    Bitmap* image = bm_create(width, height);
    if (!image) {
        fprintf(stderr, "Unable to create bitmap image\n");
        return;
    }

    //determine min and max height of the points for shading purposes
    double minZ = ((pcd_t*)listGet(&pc->points, 0))->z;
    double maxZ = ((pcd_t*)listGet(&pc->points, 0))->z;
    double actualMaxwd = 0.0;
    for (int i = 0; i < pc->rows * pc->cols; i++) {
        pcd_t* point = (pcd_t*)listGet(&pc->points, i);
        if (point) {
            if (point->z < minZ) {
                minZ = point->z;
            }
            if (point->z > maxZ) {
                maxZ = point->z;
            }
            if (point->wd > actualMaxwd) {
                actualMaxwd = point->wd;
            }
        }
    }

    if (actualMaxwd < maxwd) {
        maxwd = actualMaxwd;
    }

    //assign pixel values based on water depth and height
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            int index = INDEX_2D(row, col, width);
            pcd_t* point = (pcd_t*)listGet(&pc->points, index);
            if (point) {
                //calculate grayscale intensity based on height for terrain
                double heightRatio = (point->z - minZ) / (maxZ - minZ);
                heightRatio = pow(heightRatio, 1.2);
                int grayIntensity = (int)(heightRatio * 255);
                if (grayIntensity > 255) grayIntensity = 255;
                if (grayIntensity < 0) grayIntensity = 0;
                int red = grayIntensity;
                int green = grayIntensity;
                int blue = grayIntensity;

                //calculate water depth intensity for blue overlay
                double intensityRatio = point->wd / maxwd;
                intensityRatio = pow(intensityRatio, 0.5);
                int waterIntensity = (int)(intensityRatio * 255);
                if (waterIntensity > 255) waterIntensity = 255;
                if (waterIntensity < 0) waterIntensity = 0;

                //apply water as blue overlay
                blue += waterIntensity;
                if (blue > 255) blue = 255;

                //set pixel color
                bm_set(image, col, row, bm_rgb(red, green, blue));
            }
        }
    }

    //save the image to file
    if (!bm_save(image, filename)) {
        fprintf(stderr, "Unable to save bitmap image: %s\n", filename);
    }

    //free the bitmap image
    bm_free(image);
}

