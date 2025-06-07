#include <stdio.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

#define WIDTH 120
#define HEIGHT 60
#define DIST_FROM_CAM 100
#define CUBE_SIZE 40

double A = 0, B = 0, C = 0;
double lightX = 0.2, lightY = 0.5, lightZ = 0.7;
double zoom = 20.f;
char buffer[WIDTH*HEIGHT];
double zBuffer[WIDTH*HEIGHT];

const char shades[] = "@$#*!=;:~-,.";
bool isRunning = true;

pthread_mutex_t renderMutex = PTHREAD_MUTEX_INITIALIZER;

const double cubeVertices[8][3] = {
    {  1,  1,  1 }, /* Front-top-right corner */
    {  1,  1, -1 },
    {  1, -1, -1 },
    {  1, -1,  1 },
    { -1,  1,  1 },
    { -1,  1, -1 },
    { -1, -1, -1 },
    { -1, -1,  1 }
};

const int cubeFaces[12][3] = {
    {0,3,1}, {1,3,2},
    {4,5,7}, {5,6,7},
    {0,1,4}, {1,5,4},
    {3,7,2}, {2,7,6},
    {0,4,3}, {3,4,7},
    {1,2,5}, {2,6,5}
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

void rotate(const double x, const double y, const double z, double* xp, double* yp, double* zp)
{
    *xp = calculateX(x,y,z);
    *yp = calculateY(x,y,z);
    *zp = calculateZ(x,y,z) + DIST_FROM_CAM;
}

void project(const double x, const double y, double z, int* xp, int* yp)
{
    if (z == 0) z = 0.0001;
    *xp = (int)(WIDTH / 2 + zoom * x / z);
    *yp = (int)(HEIGHT / 2 - zoom * y / z);
}

void drawTriangle(const double x1, const double y1, const double z1,
                  const double x2, const double y2, const double z2,
                  const double x3, const double y3, const double z3,
                  const char shade)
{
    // Project to screen
    int px1, py1, px2, py2, px3, py3;
    project(x1, y1, z1, &px1, &py1);
    project(x2, y2, z2, &px2, &py2);
    project(x3, y3, z3, &px3, &py3);

    const int minX = px1 < px2 ? (px1 < px3 ? px1 : px3) : (px2 < px3 ? px2 : px3);
    const int maxX = px1 > px2 ? (px1 > px3 ? px1 : px3) : (px2 > px3 ? px2 : px3);
    const int minY = py1 < py2 ? (py1 < py3 ? py1 : py3) : (py2 < py3 ? py2 : py3);
    const int maxY = py1 > py2 ? (py1 > py3 ? py1 : py3) : (py2 > py3 ? py2 : py3);

    for (int y = minY; y <= maxY; y++)
    {
        for (int x = minX; x <= maxX; x++)
        {
            if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT)
            {
                /* https://courses.cms.caltech.edu/cs171/assignments/hw2/hw2-notes/notes-hw2.html */
                const double denom = (py2 - py3) * (px1 - px3) + (px3 - px2) * (py1 - py3);
                if (fabs(denom) < 0.001) continue;

                const double a = ((py2 - py3) * (x - px3) + (px3 - px2) * (y - py3)) / denom;
                const double b = ((py3 - py1) * (x - px3) + (px1 - px3) * (y - py3)) / denom;
                const double c = 1 - a - b;

                if (a >= 0 && b >= 0 && c >= 0)
                {
                    const double z = a * z1 + b * z2 + c * z3;
                    const int idx = y * WIDTH + x;

                    const double invZ = 1.0 / z;
                    if (invZ > zBuffer[idx]) {
                        zBuffer[idx] = invZ;
                        buffer[idx] = shade;
                    }
                }
            }
        }
    }
}

void clearBuffers()
{
    memset(buffer, ' ', WIDTH * HEIGHT);
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        zBuffer[i] = 0;
    }
}

void renderFrame()
{
    clearBuffers();

    // Draw each face of the cube
    for (int face = 0; face < 12; face++)
    {
        const int v1 = cubeFaces[face][0];
        const int v2 = cubeFaces[face][1];
        const int v3 = cubeFaces[face][2];

        // Calculate face normal for lighting
        const double x1 = cubeVertices[v1][0] * CUBE_SIZE;
        const double y1 = cubeVertices[v1][1] * CUBE_SIZE;
        const double z1 = cubeVertices[v1][2] * CUBE_SIZE;

        const double x2 = cubeVertices[v2][0] * CUBE_SIZE;
        const double y2 = cubeVertices[v2][1] * CUBE_SIZE;
        const double z2 = cubeVertices[v2][2] * CUBE_SIZE;

        const double x3 = cubeVertices[v3][0] * CUBE_SIZE;
        const double y3 = cubeVertices[v3][1] * CUBE_SIZE;
        const double z3 = cubeVertices[v3][2] * CUBE_SIZE;

        double rx1, ry1, rz1, rx2, ry2, rz2, rx3, ry3, rz3;
        rotate(x1, y1, z1, &rx1, &ry1, &rz1);
        rotate(x2, y2, z2, &rx2, &ry2, &rz2);
        rotate(x3, y3, z3, &rx3, &ry3, &rz3);

        /*
        *   v1 = P2 - P1 = (rx2 - rx1, ry2 - ry1, rz2 - rz1)
        *   v2 = P3 - P1 = (rx3 - rx1, ry3 - ry1, rz3 - rz1)
        */

        // Calculate face normal
        double nx = (ry2-ry1)*(rz3-rz1) - (rz2-rz1)*(ry3-ry1);
        double ny = (rz2-rz1)*(rx3-rx1) - (rx2-rx1)*(rz3-rz1);
        double nz = (rx2-rx1)*(ry3-ry1) - (ry2-ry1)*(rx3-rx1);
        const double len = sqrt(nx*nx + ny*ny + nz*nz);
        if (len > 0) {
            nx /= len;
            ny /= len;
            nz /= len;
        }

        //Simple backface culling - disable for now to see all faces
        //if (nz <= 0) continue;

        double dot = nx*lightX + ny*lightY + nz*lightZ;
        if (dot < 0) dot = 0;
        if (dot > 1) dot = 1;

        int shadeIdx = (int)(dot * (sizeof(shades) - 2));
        shadeIdx = (shadeIdx + face/2) % (sizeof(shades) - 1);
        char shade = shades[shadeIdx];

        drawTriangle(rx1, ry1, rz1, rx2, ry2, rz2, rx3, ry3, rz3, shade);
    }
}

void display()
{
    printf("\033[2J\033[H"); // Clear screen and move cursor to top

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            putchar(buffer[y * WIDTH + x]);
        }
        putchar('\n');
    }
}

void* renderCube()
{
    while (isRunning) {
        pthread_mutex_lock(&renderMutex);

        renderFrame();
        display();

        A += 0.03;
        B += 0.03;
        C += 0.03;

        pthread_mutex_unlock(&renderMutex);
        usleep(50000);
    }
    return NULL;
}

int main()
{
    char input = '\0';
    pthread_t renderThread;
    pthread_create(&renderThread, NULL, renderCube, NULL);

    while (isRunning)
    {
        scanf(" %c", &input);

        pthread_mutex_lock(&renderMutex);
        switch (input)
        {
            case 'q':
                isRunning = false;
                break;
            case 'r':
                A = B = C = 0;
                break;
            case '+':
                zoom *= 1.1;
                break;
            case '-':
                zoom /= 1.1;
                break;
            default:
                break;
        }
        pthread_mutex_unlock(&renderMutex);
    }
    pthread_join(renderThread, NULL);
    pthread_mutex_destroy(&renderMutex);
    return 0;
}