#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mare.h"

struct screen_t {
	color_t *pixels;
	int width, height;
};

struct mare_t {
	struct screen_t screen;
	vertex_t *data;
	int index, size;

	int draw_mode;
	vec4_t color;
};

static mare_t *s_bind_ctx;

static float s_lerp(float v0, float v1, float t);
static color_t s_lerp_color(color_t c1, color_t c2, float t);

static void s_copy_vec2(vec2_t dest, vec2_t src);
static void s_copy_vec4(vec4_t dest, vec4_t src);

static void s_lerp_vec2(vec2_t out, vec2_t v0, vec2_t v1, float t);
static void s_lerp_vec4(vec4_t out, vec4_t v0, vec4_t v1, float t);

static int s_setup_screen(struct screen_t *s, int w, int h);
static void s_screen_draw_pixel(struct screen_t *s, int x, int y, vec4_t c);
static void s_screen_draw_line(struct screen_t *s, vertex_t v0, vertex_t v1);
static void s_screen_draw_triangle(struct screen_t *s,  vertex_t v[3]);
// static void s_screen_draw_quad(struct screen_t* s, vertex_t v[4]);

mare_t *mare_create(int flags) {
	mare_t *m = (mare_t*)malloc(sizeof(*m));
	memset(m, 0, sizeof(*m));

	m->size = 1000 * sizeof(struct vertex_t);
	m->data = malloc(m->size);
	m->draw_mode = MARE_POINTS;
    vec4_t white = {1.f, 1.f, 1.f, 1.f};
    s_copy_vec4(m->color, white);
	s_setup_screen(&m->screen, SCREEN_WIDTH, SCREEN_HEIGHT);

	s_bind_ctx = NULL;

	return m;
}

void mare_destroy(mare_t *m) {
	if (!m) return;
	if (m->data) free(m->data);
}

void* mare_get_pixels(mare_t *mare) {
	return mare->screen.pixels;
}

void mare_clear(mare_t *m, vec4_t color) {
	int ww, hh;
	ww = m->screen.width;
	hh = m->screen.height;
    unsigned int c = 0x0;
    for (int i = 3; i >= 0; i--) {
        c <<= 8;
        c |= (unsigned int)(color[i] * 255);
    }
	for (int i = 0; i < ww*hh; i++) {
		m->screen.pixels[i] = c;
	}
}

void mare_color(mare_t* m, vec4_t c) {
    s_copy_vec4(m->color, c);
}

void mare_begin(mare_t *m) {
	s_bind_ctx = m;
	m->index = 0;
}

const void* mare_end(mare_t *m) {
	s_bind_ctx = NULL;
	// s_draw_functions[m->draw_mode](m, m->index, m->data);
	return mare_get_pixels(m);
}

void mare_pixel(mare_t* m, float x, float y) {
	s_screen_draw_pixel(&m->screen, x, y, m->color);
}

void mare_line(mare_t* m, float x0, float y0, float x1, float y1) {
	vertex_t v0, v1;
    v0.position[0] = x0;
    v0.position[1] = y0;
    v1.position[0] = x1;
    v1.position[1] = y1;
    s_copy_vec4(v0.color, m->color);
    s_copy_vec4(v1.color, m->color);
	s_screen_draw_line(&m->screen, v0, v1);
}

void mare_rect(mare_t * m, float x, float y, float w, float h) {
    vec4_t c;
    s_copy_vec4(c, m->color);
	vertex_t v0 = { {  x,   y}, {c[0], c[1], c[2], c[3]}, {0, 0} };
	vertex_t v1 = { {x+w,   y}, {c[0], c[1], c[2], c[3]}, {0, 0} };
	vertex_t v2 = { {x+w, y+h}, {c[0], c[1], c[2], c[3]}, {0, 0} };
	vertex_t v3 = { {  x, y+h}, {c[0], c[1], c[2], c[3]}, {0, 0} };

	s_screen_draw_line(&m->screen, v0, v1);
	s_screen_draw_line(&m->screen, v1, v2);
	s_screen_draw_line(&m->screen, v2, v3);
	s_screen_draw_line(&m->screen, v3, v0);
}

void mare_rectfill(mare_t* m, float x, float y, float w, float h) {
	vertex_t v0, v1;
	for (int yy = y; yy < y+h; yy++) {
		v0.position[0] = x;
		v0.position[1] = yy;
		v1.position[0] = x + w;
		v1.position[1] = yy;
        s_copy_vec4(v0.color, m->color);
        s_copy_vec4(v1.color, m->color);
		s_screen_draw_line(&m->screen, v0, v1);
	}
}

void mare_tria(mare_t* m, float x0, float y0, float x1, float y1, float x2, float y2) {
    vec4_t c;
    s_copy_vec4(c, m->color);
	vertex_t v[3] = {
		{ {x0, y0}, {c[0], c[1], c[2], c[3]}, {0, 0} },
		{ {x1, y1}, {c[0], c[1], c[2], c[3]}, {0, 0} },
		{ {x2, y2}, {c[0], c[1], c[2], c[3]}, {0, 0} },
	};
	s_screen_draw_line(&m->screen, v[0], v[1]);
	s_screen_draw_line(&m->screen, v[1], v[2]);
	s_screen_draw_line(&m->screen, v[2], v[0]);
}

void mare_triafill(mare_t* m, float x0, float y0, float x1, float y1, float x2, float y2) {
    vec4_t c;
    s_copy_vec4(c, m->color);
	vertex_t v[3] = {
		{ {x0, y0}, {c[0], c[1], c[2], c[3]}, {0, 0} },
		{ {x1, y1}, {c[0], c[1], c[2], c[3]}, {0, 0} },
		{ {x2, y2}, {c[0], c[1], c[2], c[3]}, {0, 0} },
	};
	s_screen_draw_triangle(&m->screen, v);
}

typedef void(*DrawFunction)(mare_t*, int, vertex_t*);
static void s_draw_points(mare_t *m, int n, vertex_t *v) {
	if (n < 1) return;
	for (int i = 0; i < n; i++)
		s_screen_draw_pixel(&m->screen, v[i].position[0], v[i].position[1], v[i].color); 
}
static void s_draw_lines(mare_t *m, int n, vertex_t *v) {
	if (n < 1) return;

	for (int i = 0; i < n; i += 2)
		s_screen_draw_line(&m->screen, v[i], v[i + 1]);
}
static void s_draw_triangles(mare_t *m, int n, vertex_t *v) {
	if (n < 1) return;
	for (int i = 0; i < n; i += 3)
		s_screen_draw_triangle(&m->screen, &v[i]);
}

static DrawFunction s_draw_functions[] = {
	[MARE_POINTS] = s_draw_points,
	[MARE_LINES] = s_draw_lines,
	[MARE_TRIANGLES] = s_draw_triangles
};

void mare_polygon(mare_t* m, int mode, int count, vertex_t* v) {
	if (!m) return;
	if (mode < 0 || mode > MARE_TRIANGLES) return;
	int n = count * (mode+1);
	s_draw_functions[mode](m, n, v);
}


void mare_vertex(int x, int y) {
	mare_t *m = s_bind_ctx;
	vertex_t *cv = m->data + m->index;
	cv->position[0] = x;
	cv->position[1] = y;
    s_copy_vec4(cv->color, m->color);
	m->index++;
}

int s_setup_screen(struct screen_t *s, int w, int h) {
	s->width = w;
	s->height = h;
	s->pixels = malloc(w * h * sizeof(color_t));
	memset(s->pixels, 0, w * h * sizeof(color_t));
	return 1;
}

void s_screen_draw_pixel(struct screen_t *s, int x, int y, vec4_t c) {
	if (x < 0 || x >= s->width) return;
	if (y < 0 || y >= s->height) return;
    unsigned int color = 0x0;
    for (int i = 3; i >= 0; i--) {
        color <<= 8;
        color |= (int)(c[i] * 255);
    }
	s->pixels[x + (y * s->width)] = color;
}

void s_screen_draw_line(struct screen_t *s, vertex_t v0, vertex_t v1) {
	int dx = v1.position[0] - v0.position[0];
	int dy = v1.position[1] - v0.position[1];
	int adx = abs(dx);
	int ady = abs(dy);
	int steps = adx > ady ? adx : ady;

	float xinc = dx / (float)steps;
	float yinc = dy / (float)steps;
	
	s_screen_draw_pixel(s, v0.position[0], v0.position[1], v0.color);
	float x, y;
	x = v0.position[0];
	y = v0.position[1];
	for (int v = 0; v < steps; v++) {
		x += xinc;
		y += yinc;
		float t = v / (float)steps;
        vec4_t c;
        s_lerp_vec4(c, v0.color, v1.color, t);
		s_screen_draw_pixel(s, x, y, c);
	}
}

static void s_vertices_swap(vertex_t* v0, vertex_t* v1) {
	vertex_t aux = *v0;
	*v0 = *v1;
	*v1 = aux;
}

static void s_vertices_y_sort(vertex_t v[3]) {
	if (v[1].position[1] < v[0].position[1]) s_vertices_swap(&v[0], &v[1]);
	if (v[2].position[1] < v[1].position[1]) s_vertices_swap(&v[1], &v[2]);
	if (v[2].position[1] < v[0].position[1]) s_vertices_swap(&v[0], &v[2]);
}

static void s_bottom_flat_triangle(struct screen_t *s, vertex_t v[3]) {
	float invslope1 = (v[1].position[0] - v[0].position[0]) / (v[1].position[1] - v[0].position[1]);
	float invslope2 = (v[2].position[0] - v[0].position[0]) / (v[2].position[1] - v[0].position[1]);
	
	float curx1, curx2;
	curx1 = curx2 = v[0].position[0];

	int scanY;
	vec4_t c1, c2;
    s_copy_vec4(c1, v[0].color);
    s_copy_vec4(c2, v[0].color);
	for (scanY = v[0].position[1]; scanY <= v[1].position[1]; scanY++) {
		vertex_t v0, v1;
		v0.position[0] = curx1;
		v0.position[1] = scanY;
		v1.position[0] = curx2;
		v1.position[1] = scanY;
        s_copy_vec4(v0.color, c1);
        s_copy_vec4(v1.color, c2);
		s_screen_draw_line(s, v0, v1);
		curx1 += invslope1;
		curx2 += invslope2;
		float t = (scanY - v[0].position[1]) / (v[1].position[1] - v[0].position[1]);
        s_lerp_vec4(c1, v[0].color, v[1].color, t);
        s_lerp_vec4(c2, v[0].color, v[2].color, t);
	}
}

static void s_top_flat_triangle(struct screen_t *s, vertex_t v[3]) {
	float invslope1 = (v[2].position[0] - v[0].position[0]) / (v[2].position[1] - v[0].position[1]);
	float invslope2 = (v[2].position[0] - v[1].position[0]) / (v[2].position[1] - v[1].position[1]);
	
	float curx1, curx2;
	curx1 = curx2 = v[2].position[0];

	int scanY;
#if 0
	color_t c1, c2;
	c1 = c2 = v[2].color;
#endif
    vec4_t c1, c2;
    s_copy_vec4(c1, v[2].color);
    s_copy_vec4(c2, v[2].color);
	for (scanY = v[2].position[1]; scanY >= v[0].position[1]; scanY--) {
		vertex_t v0, v1;
        s_copy_vec2(v0.position, ((vec2_t){curx1, scanY}));
        s_copy_vec2(v1.position, ((vec2_t){curx2, scanY}));
        s_copy_vec4(v0.color, c1);
        s_copy_vec4(v1.color, c2);
		s_screen_draw_line(s, v0, v1);
		curx1 -= invslope1;
		curx2 -= invslope2;

		float step = (scanY - v[0].position[1]) / (v[2].position[1] - v[0].position[1]);
#if 0
		c1 = s_lerp_color(v[0].color, v[2].color, step);
		c2 = s_lerp_color(v[1].color, v[2].color, step);
#endif
        s_lerp_vec4(c1, v[0].color, v[2].color, step);
        s_lerp_vec4(c2, v[1].color, v[2].color, step);
	}
}

void s_screen_draw_triangle(struct screen_t *s, vertex_t v[3]) {
	s_vertices_y_sort(v);
	if (v[1].position[1] == v[2].position[1]) s_bottom_flat_triangle(s, v);
	else if (v[0].position[1] == v[1].position[1]) s_top_flat_triangle(s, v);
	else {
		vertex_t v3;
		v3.position[0] = v[0].position[0] + ((v[1].position[1] - v[0].position[1]) / (v[2].position[1] - v[0].position[1]) * (v[2].position[0] - v[1].position[0]));
		v3.position[1] = v[1].position[1];
        v3.color[0] = v3.color[1] =
        v3.color[2] = v3.color[3] = 1.f;
		vertex_t va[3] = {
			v[0], v[1], v3
		};
		vertex_t vb[3] = {
			v[1], v3, v[3]
		};
		s_bottom_flat_triangle(s, va);
		s_top_flat_triangle(s, vb);
	}
}

#if 0
void s_screen_draw_triangle(struct screen_t *s,  vertex_t v[3]) {
	int xmin, ymin;
	int xmax, ymax;
	if (v[0].x < v[1].x) {
		xmin = v[0].x;
		xmax = v[1].x;
	} else {
		xmax = v[0].x;
		xmin = v[1].x;
	}

	if (v[2].x < xmin) xmin = v[2].x;
	else if (v[2].x > xmax) xmax = v[2].x;

	if (v[0].y < v[1].y) {
		ymin = v[0].y;
		ymax = v[1].y;
	} else {
		ymax = v[0].y;
		ymin = v[1].y;
	}

	if (v[2].y < ymin) ymin = v[2].y;
	else if (v[2].y > ymax) ymax = v[2].y;

	s_screen_draw_line(s, v[0], v[1]);
	s_screen_draw_line(s, v[1], v[2]);
	s_screen_draw_line(s, v[2], v[0]);

	for (int y = ymin; y < ymax; y++) {
		vertex_t vs, ve; // start, end
		vs.color = 0x0;
		ve.color = 0x0;
		for (int x = xmin; x < xmax; x++) {
			color_t c = s->pixels[x + (y * s->width)];
			if ((c & 0xff000000) != 0) {
				if (vs.color == 0x0) {
					vs.color = c;
					vs.x = x;
					vs.y = y;
				} else {
					ve.color = c;
					ve.x = x;
					ve.y = y;
				}
			}
		}
		if (vs.color != 0x0 && ve.color != 0x0) s_screen_draw_line(s, vs, ve);
	}
}
#endif
/* Utils */

float s_lerp(float v0, float v1, float t) {
	return (1 - t) * v0 + t * v1;
}

color_t s_lerp_color(color_t c1, color_t c2, float step) {
	float r0 = (c1 & 0xff) / 255.f;
	float g0 = ((c1 >> 8) & 0xff) / 255.f;
	float b0 = ((c1 >> 16) & 0xff) / 255.f;

	float r1 = (c2 & 0xff) / 255.f;
	float g1 = ((c2 >> 8) & 0xff) / 255.f;
	float b1 = ((c2 >> 16) & 0xff) / 255.f;

	int rx = (s_lerp(r0, r1, step) * 255.f);
	int gx = (s_lerp(g0, g1, step) * 255.f);
	int bx = (s_lerp(b0, b1, step) * 255.f);

	color_t c = 0xff000000 | (bx << 16) |
				(gx << 8) | rx;
	return c;
}

void s_copy_vec2(vec2_t dest, vec2_t src) {
    dest[0] = src[0];
    dest[1] = src[1];
}

void s_copy_vec4(vec4_t dest, vec4_t src) {
    dest[0] = src[0];
    dest[1] = src[1];
    dest[2] = src[2];
    dest[3] = src[3];
}

void s_lerp_vec2(vec2_t out, vec2_t v0, vec2_t v1, float t) {
    out[0] = s_lerp(v0[0], v1[0], t);
    out[1] = s_lerp(v0[1], v1[1], t);
}

void s_lerp_vec4(vec4_t out, vec4_t v0, vec4_t v1, float t) {
    out[0] = s_lerp(v0[0], v1[0], t);
    out[1] = s_lerp(v0[1], v1[1], t);
    out[2] = s_lerp(v0[2], v1[2], t);
    out[3] = s_lerp(v0[3], v1[3], t);
}
