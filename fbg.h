//============================================================================//
// framebuffer graphics
//============================================================================//

//----------------------------------------------------------------------------//
// interface
//----------------------------------------------------------------------------//

#ifndef FBG_H
#define FBG_H

typedef struct {
	char *buffer;
	int size;

	int width;
	int height;

	int pixelBytes;
	int lineBytes;
	int offsetRed;
	int offsetGreen;
	int offsetBlue;

} fbg_Screen;

int fbg_set_tty_graphics();
int fbg_set_tty_text();

int fbg_init_screen(fbg_Screen *screen);
void fbg_free_screen(fbg_Screen *screen);

int fbg_draw_pixel(fbg_Screen screen, int x, int y, int r, int g, int b);

//----------------------------------------------------------------------------//
// implementation
//----------------------------------------------------------------------------//

#ifdef FBG_IMPLEMENTATION

#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <linux/fb.h>
#include <linux/kd.h>

int fbg_set_tty_graphics() {
	int tty_file = open("/dev/tty", O_RDWR);
	if(tty_file < 0)
		return -1;

	if(ioctl(tty_file, KDSETMODE, KD_GRAPHICS) < 0)
		return -1;

	close(tty_file);
	return 0;
}

int fbg_set_tty_text() {
	int tty_file = open("/dev/tty", O_RDWR);
	if(tty_file < 0)
		return -1;

	if(ioctl(tty_file, KDSETMODE, KD_TEXT) < 0)
		return -1;

	close(tty_file);
	return 0;
}

int fbg_init_screen(fbg_Screen *screen) {
	int fb_file = open("/dev/fb0", O_RDWR);
	if(fb_file < 0)
		return -1;

	struct fb_var_screeninfo variable_info;
	struct fb_fix_screeninfo fixed_info;

	if(ioctl(fb_file, FBIOGET_FSCREENINFO, &fixed_info) < 0)
		return -1;

	if(ioctl(fb_file, FBIOGET_VSCREENINFO, &variable_info) < 0)
		return -1;

	screen->size = fixed_info.line_length * variable_info.yres;
	screen->width = variable_info.xres;
	screen->height = variable_info.yres;
	screen->pixelBytes = variable_info.bits_per_pixel / 8;
	screen->lineBytes = fixed_info.line_length;
	screen->offsetRed = variable_info.red.offset / 8;
	screen->offsetGreen = variable_info.green.offset / 8;
	screen->offsetBlue = variable_info.blue.offset / 8;

	screen->buffer = (char*)mmap(0, screen->size, PROT_READ | PROT_WRITE, MAP_SHARED, fb_file, 0);
	if(screen->buffer < 0)
		return -1;

	close(fb_file);
	return 0;
}

void fbg_free_screen(fbg_Screen *screen) {
	munmap(screen->buffer, screen->size);
}

int fbg_draw_pixel(fbg_Screen screen, int x, int y, int r, int g, int b) {
	int returnValue = 0;

	if(x < 0) {
		x = 0;
		returnValue = -1;
	} else if(x > screen.width) {
		x = screen.width;
		returnValue = -1;
	}

	if(y < 0) {
		y = 0;
		returnValue = -1;
	} else if(y > screen.height) {
		y = screen.height;
		returnValue = -1;
	}

	int offset = x * screen.pixelBytes + y * screen.lineBytes;
				
	screen.buffer[offset + screen.offsetRed] = r;
	screen.buffer[offset + screen.offsetGreen] = g;
	screen.buffer[offset + screen.offsetBlue] = b;

	return returnValue;
};

#endif
#endif