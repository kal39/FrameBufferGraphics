#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <signal.h>

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

	int time_start = time (NULL);

	for (int t = 0; t < 255; t++) {
		for (int y = 0; y < s.height; y++) {
			for (int x = 0; x < s.width; x++) {
				fbg_draw_pixel(s, x, y, x * 255 / s.width, y * 255 / s.height, t);
			}
		}
	}

	int time_end = time(NULL);

	fbg_free_screen(&s);
	fbg_set_tty_text();

	printf("%f fps\n", (float)255 / (time_end - time_start));

	return 0;
}
