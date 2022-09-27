#ifndef MARE_H
#define MARE_H

#define MARE_VERSION "0.1.0"
#define MARE_API

#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 95
#define SCREEN_SIZE SCREEN_WIDTH * SCREEN_HEIGHT

typedef float vec2_t[2];
typedef float vec3_t[3];
typedef float vec4_t[4];

typedef struct mare_t mare_t;
typedef unsigned int color_t;
typedef struct vertex_t vertex_t;

typedef struct texture_t texture_t;

enum {
	MARE_POINTS = 0,
	MARE_LINES,
	MARE_TRIANGLES
};

enum {
	MARE_ALPHA = 0,
	MARE_RED,
	MARE_RG,
	MARE_RGB,
	MARE_RGBA
};

struct vertex_t {
    vec2_t position;
    vec4_t color;
    vec2_t texcoord;
#if 0
	float x, y;
	color_t color;
	float u, v;
#endif
};

MARE_API mare_t *mare_create(int flags);
MARE_API void mare_destroy(mare_t *mare);

MARE_API void* mare_get_pixels(mare_t *mare);

MARE_API void mare_clear(mare_t *m, vec4_t color);
MARE_API void mare_color(mare_t* m, vec4_t color);

MARE_API void mare_begin(mare_t* m);
MARE_API const void* mare_end(mare_t* m);

MARE_API void mare_pixel(mare_t* m, float x, float y);
MARE_API void mare_line(mare_t* m, float x0, float y0, float x1, float y1);
MARE_API void mare_rect(mare_t* m, float x, float y, float w, float h);
MARE_API void mare_rectfill(mare_t* m, float x, float y, float w, float h);
MARE_API void mare_circ(mare_t* m, float x, float y, float r);
MARE_API void mare_circfill(mare_t* m, float x, float y, float r);
MARE_API void mare_tria(mare_t* m, float x0, float y0, float x1, float y1, float x2, float y2);
MARE_API void mare_triafill(mare_t* m, float x0, float y0, float x1, float y1, float x2, float y2);
MARE_API void mare_polygon(mare_t *m, int mode, int count, vertex_t* vertices);

MARE_API texture_t* mare_texture(int width, int height, int format, unsigned char *pixels);
MARE_API void mare_bind_texture(mare_t *m, texture_t *t);

#endif /* MARE_H */
