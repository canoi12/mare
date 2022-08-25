# mare (Mini Agnostic REnder)

A simple software based render (all render operations done in a array).

```c
#include "mare.h"
#include <SDL2/SDL.h>
#include <stdio.h>

int main(void) {
	SDL_Init(SDL_INIT_VIDEO);
	SDL_Window *window;
	SDL_Renderer *render;

	mare_t *m = mare_create(0);

	SDL_CreateWindowAndRenderer(640, 380, SDL_WINDOW_SHOWN, &window, &render);
	SDL_SetWindowTitle(window, "mare");

	SDL_Event event;
	SDL_Texture *tex = SDL_CreateTexture(render, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
	float x;
	float time = 0.f;
	while (event.type != SDL_QUIT) {
		SDL_PollEvent(&event);
		mare_begin(m);
		mare_clear(m, 0xff000000);
		mare_line(m, 0, 0, 32, 32);
		mare_rect(m, 32, 32, 16, 16);
		mare_rectfill(m, 32, 48, 16, 16);
		time += 0.01f;
		x = sinf(time) * 16;

		vertex_t v[6] = {
			{ x+64, 16, 0xff0000ff, 0, 0 },
			{ x+48, 32, 0xff00ff00, 0, 0 },
			{ x+80, 32, 0xffff0000, 0, 0 },
			{ x+64, 48, 0xff0000ff, 0, 0 },
			{ x+48, 32, 0xff00ff00, 0, 0 },
			{ x+80, 32, 0xffff0000, 0, 0 }
		};
		mare_polygon(m, MARE_TRIANGLES, 2, v);

		const char* px = mare_end(m);
		SDL_SetRenderDrawColor(render, 0, 255, 0, 255);
		SDL_RenderClear(render);
		SDL_SetRenderDrawColor(render, 255, 255, 255, 255);
		void *data;
		int pitch;
		SDL_LockTexture(tex, NULL, &data, &pitch);
		memcpy(data, px, sizeof(color_t) * SCREEN_SIZE);
		SDL_UnlockTexture(tex);
		SDL_RenderCopy(render, tex, NULL, NULL);
		SDL_RenderPresent(render);
	}
	
	mare_destroy(m);
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(render);
	SDL_Quit();

	return 0;
}
```
