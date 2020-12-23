#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <math.h>

#define FBG_IMPLEMENTATION
#include "fbg.h"

void cleanup_and_exit() {
	fbg_set_tty_text();
	exit(0);
}

void test_pixel(fbg_Screen s, int frame) {
	for (int y = 0; y < s.height; y++) {
		for (int x = 0; x < s.width; x++) {
			fbg_draw_pixel(s, x, y, (fbg_Color){x * 255 / s.width, y * 255 / s.height, frame});
		}
	}
}

#define CIRCLE_RESOLUTION 100
#define CIRCLE_RADIUS 100
#define CIRCLE_CENTER_X 500
#define CIRCLE_CENTER_Y 500
#define CIRCLE_CENTER_X_2 200
#define CIRCLE_CENTER_Y_2 200

void test_line(fbg_Screen s) {
	for(int i = 0; i < CIRCLE_RESOLUTION; i++) {
		float angle = 2 * M_PI / CIRCLE_RESOLUTION * i;
		int endX = CIRCLE_CENTER_X + CIRCLE_RADIUS * cos(angle);
		int endY = CIRCLE_CENTER_Y + CIRCLE_RADIUS * sin(angle);

		fbg_draw_line(s, CIRCLE_CENTER_X, CIRCLE_CENTER_Y, endX, endY, (fbg_Color){0, 0, 255});
	}
}

void test_triangle(fbg_Screen s) {

}

void test_box(fbg_Screen s) {
	fbg_draw_box_fill(s, 100, 100, 500, 200, (fbg_Color){255, 0, 0});
	fbg_draw_box(s, 100, 100, 500, 200, (fbg_Color){255, 255, 255});
	
}

#define FRAMES 1000

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
		fbg_clear_screen(s, (fbg_Color){20, 20, 20});

		test_box(s);

		fbg_display(s);
	}

	int t2 = time(NULL);

	fbg_free_screen(&s);
	fbg_set_tty_text();

	printf("fps: %f\n", (float)FRAMES / (t2 - t1));

	return 0;
}
