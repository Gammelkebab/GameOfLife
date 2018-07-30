#include <stdlib.h>
#include <stdio.h>
#include <algorithm>
#include <time.h>
#include <sys/time.h>
#include <math.h>

#define SMAL //uses small resolution, less iterations for testing

#ifndef SMALL
#define GRIDSIZE_X 1920
#define GRIDSIZE_Y 1080
#define FRAMES 900
#else
#define GRIDSIZE_X 202
#define GRIDSIZE_Y 200
#define FRAMES 100
#endif

using namespace std;


//rules for Game of Life
//you can change this function, but the rules have to remain the same
int isAlive(int neighbours, unsigned char cell) {
    switch(neighbours) {
        case(2): return cell;
        case(3): return 1;
        default: return 0;
    }
}

//count neighbours that are alive in a 2D grid
//there are 9 neighbours for each cell
//the grid has no real border, neighbours above top row is the bottom row
//alive = 1, dead = 0
int getNeighbours(unsigned char** grid, int x, int y) {
    int sum = 0;
    for(int i=0; i<3; ++i) {
        sum += grid[(y+1)%GRIDSIZE_Y][(x-1+i+GRIDSIZE_X)%GRIDSIZE_X];
        sum += grid[(y-1+GRIDSIZE_Y)%GRIDSIZE_Y][(x-1+i+GRIDSIZE_X)%GRIDSIZE_X];
    }
    sum += grid[y][(x-1+GRIDSIZE_X)%GRIDSIZE_X];
    sum += grid[y][(x+1)%GRIDSIZE_X];
    return sum;
}

//next evolution
//iterate over each cell, count neighbours and apply the rules
//if a cell is alive/dies depends only on the previous step
void step(unsigned char** grid, unsigned char** nextGrid) {
    int neighbours;
    for(int y=0; y<GRIDSIZE_Y; ++y) {
        for(int x=0; x<GRIDSIZE_X; ++x) {
            neighbours = getNeighbours(grid, x, y);
            nextGrid[y][x] = isAlive(neighbours, grid[y][x]);
        }
    }
}


//console output, small grid is preferable
void printGrid(unsigned char** grid) {
    for(int y=0; y<GRIDSIZE_Y; ++y) {
        for(int x=0; x<GRIDSIZE_X; ++x) {
            if(grid[y][x]) printf("X ");
            else printf("- ");
        }
        printf("\n");
    }
    printf("\n\n");
}

//allocate a 2D array
unsigned char** array2D(int x, int y) {
    unsigned char** array = new unsigned char*[y]();
    for(int i=0; i<y; ++i) {
        array[i] = new unsigned char[x]();
    }
    return array;
}

/**
the figures are ignoring the size of the grid, offset has to be checked manually
**/

//figure with a 2-step period, stationary
void blinker(unsigned char** grid, int offset_X=0, int offset_Y=0) {
    grid[3+offset_Y][4+offset_X] = 1;
    grid[4+offset_Y][4+offset_X] = 1;
    grid[5+offset_Y][4+offset_X] = 1;
}


//figure with a 8-step period, moves
void glider(unsigned char** grid, int offset_X=0, int offset_Y=0) {
    grid[4+offset_Y][4+offset_X] = 1;
    grid[4+offset_Y][5+offset_X] = 1;
    grid[4+offset_Y][6+offset_X] = 1;
    grid[3+offset_Y][6+offset_X] = 1;
    grid[2+offset_Y][5+offset_X] = 1;
}

//figure
void pentadecathlon(unsigned char** grid, int offset_X=0, int offset_Y=0) {
    for(int x=0; x<8; ++x) {
        for(int y=0; y<3; ++y) {
            grid[y+offset_Y][x+offset_X] = 1;
        }
    }
    grid[1+offset_Y][1+offset_X] = 0;
    grid[1+offset_Y][6+offset_X] = 0;
}

//fill the grid randomly with ~35% alive
void random(unsigned char **grid) {
    srand(time(NULL));
    for(int x=0; x<GRIDSIZE_X; ++x) {
        for(int y=0; y<GRIDSIZE_Y; ++y) {
            grid[y][x] = (int) rand()%100 < 35? 1 : 0;
        }
    }
}

//creates a .pbm file displaying the current grid
//pbm in mode P4 reads bitwise, 1 black, 0 white
void writeFrame(unsigned char** grid, int i) {
    char *filename = new char[100];
    sprintf(filename, "./images/frame_%03d.pbm", i);
    FILE *outfile = fopen(filename, "w");
    //Create first lines of .pbm layout:
    //mode: P4 (bitwise black white)
    //number of pixels in a row
    //number of rows
    fprintf(outfile, "P4\n%d %d\n",GRIDSIZE_X,GRIDSIZE_Y);

    //determine number of bytes in a row
    int gridsize_x_byte = GRIDSIZE_X/8;
    //if not all bits of a full byte are used (e.g. 10x16 grid)
    //the last bits are ignored
    if (GRIDSIZE_X % 8 != 0) {
        gridsize_x_byte++;
    }
    //array holding all bit-information packed in bytes
    char out[gridsize_x_byte*GRIDSIZE_Y];
    memset(out, 0, gridsize_x_byte * GRIDSIZE_Y * sizeof(char));

    for (int y=0; y<GRIDSIZE_Y; ++y) {
        for(int x=0; x<gridsize_x_byte; ++x) {
            //information about 8 cells are packed in one byte
            for(int k=0; k<8; ++k) {
                if (x*8+k >= GRIDSIZE_X) break;
                if (grid[y][x*8+k] == 1) {
                    out[y*gridsize_x_byte+x] += pow(2,7-k);
                }
            }
        }

    }
    fwrite(&out[0], sizeof(char), gridsize_x_byte*GRIDSIZE_Y, outfile);
    fclose(outfile);
}

int main() {
    //time measurement
    double elapsed = 0;
    struct timeval begin, end;

    //create 2 2D arrays, current step and next step
    unsigned char** grid = array2D(GRIDSIZE_X, GRIDSIZE_Y);
    unsigned char** nextGrid = array2D(GRIDSIZE_X, GRIDSIZE_Y);
    unsigned char** swap;

    //fill grid randomly and create the first frame
    random(grid);
    writeFrame(grid, 0);

    gettimeofday(&begin, NULL);
    for(int i=0; i<FRAMES; ++i) {
        //calculate next step
        step(grid, nextGrid);

        //swap pointers
        swap = grid;
        grid = nextGrid;
        nextGrid = swap;

        //add frame to video
        writeFrame(grid, i+1);
    }
    gettimeofday(&end, NULL);
    elapsed += (end.tv_sec - begin.tv_sec) + ((end.tv_usec - begin.tv_usec)/1000000.0);
    printf("Runtime: %.5fs\n",elapsed);

    delete[] grid;
    delete[] nextGrid;
}
