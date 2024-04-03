#include <stdio.h>
#include <math.h>

int height = 10;
int width = 10;
double theta = 45;

typedef struct {
    int xCoordinate;
    int yCoordinate;
    int zCoordinate;
} ThreeDimensionalPoint;

typedef struct {
    ThreeDimensionalPoint points[3];
} Triangle;

typedef struct {
    int xCoordinate;
    int yCoordinate;
} TwoDimensionalPoint;


int main(){
    double aspectRatio = height / width;
    printf("hello world this is evan");
    return 0;
}
