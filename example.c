#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <math.h>

#define FBG_IMPLEMENTATION
#include "fbg.h"

void cleanup_and_exit() {
	printf("cleanup\n");
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

	for (int t = 0; t < 255; t++) {
		fbg_clear(s, 0, 0, 0);

		/*
		for (int y = 0; y < s.height; y++) {
			for (int x = 0; x < s.width; x++) {
				fbg_draw_pixel(s, x, y, x * 255 / s.width, y * 255 / s.height, t);
			}
		}
		*/

		int x0 = 300;
		int y0 = 300;
		int count = 1000;
		int r = 200;

		for(int i = 0; i < count; i++) {
			float angle = 2 * M_PI / count * i;
			fbg_draw_pixel(s, x0 + cos(angle) * r, y0 + sin(angle) * r, 255, 0, 0);
			fbg_draw_line(s, x0, y0, x0 + cos(angle) * r, y0 + sin(angle) * r, 0, 255, 0);
		}
		
		/*
		fbg_draw_line(s, x0, y0, x0 + 200, y0 + 100, 255, 0, 0);
		fbg_draw_line(s, x0, y0, x0 + 100, y0 + 200, 255, 0, 0);
		fbg_draw_line(s, x0, y0, x0 + 100, y0 - 100, 255, 0, 0);
		fbg_draw_line(s, x0, y0, x0 + 100, y0 - 200, 255, 0, 0);
		fbg_draw_line(s, x0, y0, x0 - 200, y0 + 100, 0, 255, 0);
		fbg_draw_line(s, x0, y0, x0 - 100, y0 + 200, 0, 255, 0);
		fbg_draw_line(s, x0, y0, x0 - 100, y0 - 100, 0, 255, 0);
		fbg_draw_line(s, x0, y0, x0 - 100, y0 - 200, 0, 255, 0);
		*/

		// fbg_draw_line(s, 100, 100, 300, 100, 255, 0, 0);
		// fbg_draw_line(s, 100, 100, 100, 600, 255, 0, 0);

		fbg_display(s);
	}

	int t2 = time(NULL);

	fbg_free_screen(&s);
	fbg_set_tty_text();

	printf("fps: %f\n", (float)255 / (t2 - t1));

	return 0;
}
