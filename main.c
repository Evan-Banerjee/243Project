#include <stdio.h>
#include <math.h>

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
    printf("hello world this is evan");
    return 0;
}
