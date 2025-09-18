#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pointcloud.h"
#include "util.h"
#include "bmp.h"

//runProject prototype
void runProject(pointcloud_t *pc, int iter, double iwater, double wcoef, double ecoef, char *ofilebase, double maxwdepth, int seq);

int main(int argc, char *argv[]) {
    //check number of arguments
    if (argc < 7 || argc > 9) {
        fprintf(stderr, "Usage: ./watershed <ifile> <iter> <iwater> <wcoef> <ecoef> <ofilebase> [<seq>] [<maxwd>]\n");
        return 1;
    }

    // parse in command line qrguments
    char *ifile = argv[1];
    int iter;
    double iwater, wcoef, ecoef, maxwdepth;
    char *ofilebase = argv[6];
    int seq = 0;

    //check if number of iterations is valid
    if (sscanf(argv[2], "%d", &iter) != 1 || iter < 1) {
        fprintf(stderr, "invalid number of iterations\n");
        return 1;
    }
    
    //check if water amount is valid
    if (sscanf(argv[3], "%lf", &iwater) < 0) {
        fprintf(stderr, "invalid initial water amount\n");
        return 1;
    }

    //check if water flow coefficient is between 0 and .2
    if (sscanf(argv[4], "%lf", &wcoef) != 1 || wcoef < 0.0 || wcoef > 0.2) {
        fprintf(stderr, "water flow coefficient must be between 0.0 and 0.2\n");
        return 1;
    }

    //check if evaporation coefficient is betwwen .9 and 1
    if (sscanf(argv[5], "%lf", &ecoef) != 1 || ecoef < 0.9 || ecoef > 1.0) {
        fprintf(stderr, "evaporation coefficient must be between 0.9 and 1.0\n");
        return 1;
    }

    if (sscanf(argv[7], "%lf", &maxwdepth) <= 0) {
        fprintf(stderr, "max water depth must be above 1.0\n");
        return 1;
    }

    if (sscanf(argv[8], "%d", &seq) != 1 || seq < 1) {
        fprintf(stderr, "seq must be greater than 0\n");
        return 1;
    }


    //read point cloud data
    FILE *input_file = fopen(ifile, "r");
    if (!input_file) {
        fprintf(stderr, "failed to open input file: %s\n", ifile);
        return 1;
    }

    //put pointcloud data into pointcloud_t
    pointcloud_t *pc = readPointCloudData(input_file);
    fclose(input_file);

    if (!pc) {
        fprintf(stderr, "failed to read point cloud data\n");
        return 1;
    }

    //initialize water level on the point cloud
    for (int i = 0; i < pc->rows * pc->cols; i++) {
        pcd_t *point = (pcd_t *)listGet(&pc->points, i);
        point->wd = iwater;
    }

    //run the project
    runProject(pc, iter, iwater, wcoef, ecoef, ofilebase, maxwdepth, seq);


    //clean up
    free(pc->points.data);
    free(pc);

    return 0;
}

void runProject(pointcloud_t *pc, int iter, double iwater, double wcoef, double ecoef, char *ofilebase, double maxwdepth, int seq) {
    //initialize the watershed by setting up initial water levels
    initializeWatershed(pc);
    watershedAddUniformWater(pc, iwater);

    //create 2d array
    double** waterDepthArray = allocateArray(pc->rows, pc->cols);
    if (!waterDepthArray) {
        fprintf(stderr, "Error: Failed to allocate water depth array.\n");
        return;
    }

      //loop through iterations
    for (int i = 0; i < iter; i++) {
        watershedStep(pc, wcoef, ecoef);

        //store current water depths in array
        for (int row = 0; row < pc->rows; row++) {
            for (int col = 0; col < pc->cols; col++) {
                int index = INDEX_2D(row, col, pc->cols);
                pcd_t* point = (pcd_t*)listGet(&pc->points, index);
                if (point) {
                    waterDepthArray[row][col] = point->wd;
                }
            }
        }

        // Output images if required
        if (seq > 0 && (i + 1) % seq == 0) {
            char filename[256];
            sprintf(filename, "%s%d.gif", ofilebase, i + 1);
            imagePointCloudWater(pc, iwater, filename);
        }
    }

    //output final result
    char final_filename[256];
    sprintf(final_filename, "%s.gif", ofilebase);
    imagePointCloudWater(pc, iwater, final_filename);

    freeArray(waterDepthArray, pc->rows);
}
