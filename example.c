#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <math.h>

#define FBG_IMPLEMENTATION
#include "fbg.h"

#define FRAMES 1000

void cleanup_and_exit() {
	fbg_set_tty_text();
	exit(0);
}

int main (int argc, char **argv) {
	signal(SIGINT, cleanup_and_exit);
	atexit(cleanup_and_exit);
	
	if(fbg_set_tty_graphics() < 0)
		return -1;

	fbg_Screen s;

	if(fbg_init_screen(&s) < 0)
		return -1;

	int t1 = time (NULL);

	for (int t = 0; t < FRAMES; t++) {
		fbg_clear(s, (fbg_Color){20, 20, 20});

		// fbg_draw_triangle_fill(s, 100, 100, 400, 200, 100, 300, (fbg_Color){255, 0, 0});

		fbg_draw_triangle_fill(s, 700, 500, 700, 600, 100, 550, (fbg_Color){0, 255, 0});
		
		/*
		for (int y = 0; y < s.height; y++) {
			for (int x = 0; x < s.width; x++) {
				fbg_draw_pixel(s, x, y, (fbg_Color){x * 255 / s.width, y * 255 / s.height, t});
			}
		}
		*/

		fbg_display(s);
	}

	int t2 = time(NULL);

	fbg_free_screen(&s);
	fbg_set_tty_text();

	printf("fps: %f\n", (float)FRAMES / (t2 - t1));

	return 0;
}
