#include "../include/triangle.h"
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

// Function declaration for our wireframe line drawing helper
void drawLine(Canvas* canvas,
             int x0, int y0, int x1, int y1,
             Color color);

void drawLine(Canvas* canvas,
             int x0, int y0, int x1, int y1,
             Color color)
{
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;
    while (true) {
        Canvas_PutPixel(canvas, x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2*err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void drawTriangle(Canvas* canvas, const Triangle* t)
{
    // local base-triangle pointing up
    float bx[3] = { 0,  t->size, -t->size };
    float by[3] = { -t->size,  t->size,  t->size };
    float c = cosf(t->angle), s = sinf(t->angle);

    int vx[3], vy[3];
    for (int i = 0; i < 3; i++) {
        float rx = bx[i]*c - by[i]*s;
        float ry = bx[i]*s + by[i]*c;
        vx[i] = (int)(t->cx + rx);
        vy[i] = (int)(t->cy + ry);
    }

    // draw the three edges
    drawLine(canvas, vx[0], vy[0], vx[1], vy[1], t->color);
    drawLine(canvas, vx[1], vy[1], vx[2], vy[2], t->color);
    drawLine(canvas, vx[2], vy[2], vx[0], vy[0], t->color);
}
