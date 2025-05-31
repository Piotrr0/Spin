#include <stdio.h>
#include <math.h>

#define WIDTH 100
#define HEIGHT 60
#define DIST_FROM_CAM 50
#define SCALE 10
#define CUBE_SIZE 20

double A = 0, B = 0, C = 0;
char buffer[WIDTH*HEIGHT];
double zBuffer[WIDTH*HEIGHT];

const char shades[] = "@$#*!=;:~-,.";

const double cubeVertices[8][3] = {
    {  1,  1,  1 },
    {  1,  1, -1 },
    {  1, -1, -1 },
    {  1, -1,  1 },
    { -1,  1,  1 },
    { -1,  1, -1 },
    { -1, -1, -1 },
    { -1, -1,  1 }
};

const int cubeFaces[12][3] = {
    {0,1,2}, {0,2,3},
    {4,5,7}, {4,7,6},
    {0,1,5}, {0,5,4},
    {2,3,6}, {2,6,7},
    {0,3,6}, {0,6,4},
    {1,2,7}, {1,7,5}
};

double calculateX(const double i, const double j, const double k)
{
    return j * sin(A) * sin(B) * cos(C)
        - k * cos(A) * sin(B) * cos(C)
        + j * cos(A) * sin(C)
        + k * sin(A) * sin(C)
        + i * cos(B) * cos(C);
}

double calculateY(const double i, const double j, const double k)
{
    return j * cos(A) * cos(C)
    + k * sin(A) * cos(C)
    - j * sin(A) * sin(B) * sin(C)
    + k * cos(A) * sin(B) * sin(C)
    - i * cos(B) * sin(C);
}

double calculateZ(const double i, const double j, const double k)
{
    return k * cos(A) * cos(B)
        - j * sin(A) * cos(B)
        + i * sin(B);
}

void rotate(double x, double y, double z, double* xp, double* yp, double* zp)
{
    *xp = calculateX(x,y,z);
    *yp = calculateY(x,y,z);
    *zp = calculateZ(x,y,z) + DIST_FROM_CAM;
}

void project(double x, double y, double z, int* xp, int* yp)
{
    if (z == 0) z = 0.0001;
    *xp = (int)(WIDTH / 2 + SCALE * x / z);
    *yp = (int)(HEIGHT / 2 - SCALE * y / z);
}

int main()
{

}