#ifndef UTILS_H
#define UTILS_H

#define SET_FULLSCREEN(x) \
    do { \
        if (x) { \
            int display = GetCurrentMonitor(); \
            SetWindowSize(GetMonitorWidth(display), GetMonitorHeight(display)); \
            ToggleFullscreen(); \
        } \
    } while (0)

#define SQUARE(size) (Vector2) {size, size}
#define VARGS(vector) vector.x, vector.y
#define VPRINT(vector) printf("Vector2{ x=%f, y=%f }\n", vector.x, vector.y)
#define V(x, y) (Vector2) {x, y}
#define R(x, y, w, h) (Rectangle) {x, y, w, h}
#define VCMP(v1, v2) (v1.x == v2.x && v1.y == v2.y)
#define VEMPTY V(-1, -1)

#define ColorFromRGB(r, g, b) (Color){r, g, b, 255}

#endif // UTILS_H
