# README for Project1

#Features Implemented
- This program takes in 
    - A file of cordinate points
    - number of iterations to run
    - intiional water amount (reccomend around 300)
    - the water flow coefficent (must be between 0.0 - 0.2)
    - the evaporation coefficent (must be between 0.9 - 1.0)
    - the base file name of the output file (if no seq parameter is given the output file is simply <ofilebase>.gif)
    - the max water depth
    - and the final number seq, which is the amount of iterations to run before printing an output image
- This program simulates water flow over an area and outputs images with blue shading to indicate water level.

# Known issues
- no known issues
- edge casses are handled

# How to complie and run
- To compile the program run make in terminal from top directory
- this will produce executable named watershed
- an example of command to run is
    - /watershed data.txt 5 300.0 0.03 0.9 output/watershed 20 1
- where data.txt is the location of your file
- to clean run make clean command
