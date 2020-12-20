//============================================================================//
// FrameBufferGraphics
//============================================================================//

//----------------------------------------------------------------------------//
// interface
//----------------------------------------------------------------------------//

#ifndef FBG_H
#define FBG_H

typedef struct {
	char *onscreenBuffer;
	char *offscreenBuffer;
	int size;

	int width;
	int height;

	int pixelBytes;
	int lineBytes;
	int offsetRed;
	int offsetGreen;
	int offsetBlue;

} fbg_Screen;

/*
 * Set tty to graphics mode. This stops the terminal overwriting the
 * framebuffer.
 * 
 * ARGUMENTS:
 *   (NONE)
 * 
 * RETURN VALUE:
 *   0:
 *     Success.
 *   -1:
 *     Could not open fb file.
 *   -2:
 *     ioctl() failed.
 *   -3:
 *     mmap() failed.
 *   -4:
 *     malloc() failed.
 */

int fbg_set_tty_graphics();

/* 
 * Set tty to text mode. If fbg_set_tty_graphics() has been used, this function
 * has to be called to be able to use the terminal again.
 * 
 * ARGUMENTS:
 *   (NONE)
 * 
 * RETURN VALUE:
 *   0:
 *     Success.
 *   -1:
 *     Could not open tty file.
 *   -2:
 *     ioctl() failed.
 */

int fbg_set_tty_text();

/*
 * Initlialises fbg_Screen.
 * 
 * ARGUMENTS:
 *   *screen:
 *     fbg_Screen to initialise.
 * 
 * RETURN VALUE:
 *   0:
 *     Success.
 *   -1:
 *     Could not open tty file.
 *   -2:
 *     ioctl() failed.
 */

int fbg_init_screen(fbg_Screen *screen);

/*
 * Unmaps buffer in fbg_Screen.
 * 
 * ARGUMENTS:
 *   *screen:
 *     fbg_Screen to free.
 * 
 * RETURN VALUE:
 *   (NONE)
 */

void fbg_free_screen(fbg_Screen *screen);

void fbg_clear(fbg_Screen screen, int r, int g, int b);

void fbg_display(fbg_Screen screen);

/*
 * Draws pixel to screen.
 * 
 * ARGUMENTS:
 *   screen:
 *     Screen to draw to.
 *   x, y:
 *     Position of pixel.
 *   r, g, b:
 *     Color of pixel.
 * 
 * RETURN VALUE:
 *   0:
 *     Success.
 *   -1:
 *     Pixel was out of bounds.
 */

int fbg_draw_pixel(fbg_Screen screen, int x, int y, int r, int g, int b);

//----------------------------------------------------------------------------//
// implementation
//----------------------------------------------------------------------------//

#ifdef FBG_IMPLEMENTATION

#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <linux/fb.h>
#include <linux/kd.h>

int fbg_set_tty_graphics() {
	int ttyFile = open("/dev/tty", O_RDWR);
	if(ttyFile < 0)
		return -1;

	if(ioctl(ttyFile, KDSETMODE, KD_GRAPHICS) < 0)
		return -2;

	close(ttyFile);
	return 0;
}

int fbg_set_tty_text() {
	int ttyFile = open("/dev/tty", O_RDWR);
	if(ttyFile < 0)
		return -1;

	if(ioctl(ttyFile, KDSETMODE, KD_TEXT) < 0)
		return -1;

	close(ttyFile);
	return 0;
}

int fbg_init_screen(fbg_Screen *screen) {
	int fbFile = open("/dev/fb0", O_RDWR);
	if(fbFile < 0)
		return -1;

	struct fb_var_screeninfo variableInfo;
	struct fb_fix_screeninfo fixedInfo;

	if(ioctl(fbFile, FBIOGET_FSCREENINFO, &fixedInfo) < 0)
		return -2;

	if(ioctl(fbFile, FBIOGET_VSCREENINFO, &variableInfo) < 0)
		return -2;

	screen->size = fixedInfo.line_length * variableInfo.yres;
	screen->width = variableInfo.xres;
	screen->height = variableInfo.yres;
	screen->pixelBytes = variableInfo.bits_per_pixel / 8;
	screen->lineBytes = fixedInfo.line_length;
	screen->offsetRed = variableInfo.red.offset / 8;
	screen->offsetGreen = variableInfo.green.offset / 8;
	screen->offsetBlue = variableInfo.blue.offset / 8;

	screen->onscreenBuffer = mmap(0, screen->size, PROT_READ | PROT_WRITE, MAP_SHARED, fbFile, 0);
	if(screen->onscreenBuffer < 0)
		return -3;
	
	screen->offscreenBuffer = malloc(screen->size);
	if(screen->offscreenBuffer < 0)
		return -4;

	close(fbFile);
	return 0;
}

void fbg_free_screen(fbg_Screen *screen) {
	munmap(screen->onscreenBuffer, screen->size);
	free(screen->offscreenBuffer);
}

void fbg_clear(fbg_Screen screen, int r, int g, int b) {
	for(int i = 0; i < screen.width; i++) {
		int offset = i * screen.pixelBytes;

		screen.offscreenBuffer[offset + screen.offsetRed] = r;
		screen.offscreenBuffer[offset + screen.offsetGreen] = g;
		screen.offscreenBuffer[offset + screen.offsetBlue] = b;
	}

	for(int i = 1; i < screen.height; i++) {
		int offset = i * screen.lineBytes;
		memcpy(&screen.offscreenBuffer[offset], &screen.offscreenBuffer[0], screen.lineBytes);
	}
}

void fbg_display(fbg_Screen screen) {
	memcpy(screen.onscreenBuffer, screen.offscreenBuffer, screen.size);
}

int fbg_draw_pixel(fbg_Screen screen, int x, int y, int r, int g, int b) {
	if(x < 0 || x > screen.width || y < 0 || y > screen.height)
		return -1;

	int offset = x * screen.pixelBytes + y * screen.lineBytes;
				
	screen.offscreenBuffer[offset + screen.offsetRed] = r;
	screen.offscreenBuffer[offset + screen.offsetGreen] = g;
	screen.offscreenBuffer[offset + screen.offsetBlue] = b;

	return 0;
};

#endif
#endif