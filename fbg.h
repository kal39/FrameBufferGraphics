//============================================================================//
// FrameBufferGraphics
//============================================================================//

//----------------------------------------------------------------------------//
// todo
//----------------------------------------------------------------------------//

/*
 * > draw_polygon
 *   > wrapper for triangles and squares
 * > fonts
 * > input
 * > check if what is being drawn is inside the screen
 * > image
 */

//============================================================================//
// interface
//============================================================================//

#ifndef FBG_H
#define FBG_H

typedef struct fbg_Screen {
	char *onscreenBuffer;
	char *offscreenBuffer;
	int size;

	int width;
	int height;

	int bytesPerPixel;
	int bytesPerLine;
	int offsetRed;
	int offsetGreen;
	int offsetBlue;

} fbg_Screen;

typedef struct fbg_Color {
	int r;
	int g;
	int b;

} fbg_Color;

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
 * Initialises fbg_Screen.
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

/*
 * Clear screen with some color.
 * 
 * ARGUMENTS:
 *   screen:
 *     Screen to draw to.
 *   color:
 *     Color to clear with.
 * 
 * RETURN VALUE:
 *   (NONE)
 */

void fbg_clear_screen(fbg_Screen screen, fbg_Color color);

/*
 * Copies data from the offscreen buffer to the onscreen buffer.
 * 
 * ARGUMENTS:
 *   screen:
 *     Screen to draw to.
 * 
 * RETURN VALUE:
 *   (NONE)
 */

void fbg_display(fbg_Screen screen);

/*
 * Draws pixel to screen. Very slow at drawing a large number of pixels.
 * 
 * ARGUMENTS:
 *   screen:
 *     Screen to draw to.
 *   x, y:
 *     Position of pixel.
 *   color:
 *     Color of pixel.
 * 
 * RETURN VALUE:
 *   0:
 *     Success.
 *   -1:
 *     Pixel was out of bounds.
 */

int fbg_draw_pixel(fbg_Screen screen, int x, int y, fbg_Color color);

/*
 * Draws line to screen.
 * 
 * ARGUMENTS:
 *   screen:
 *     Screen to draw to.
 *   x1, y1:
 *     Start position.
 *  x2, y2:
 *     End position.
 *   color:
 *     Color of line.
 */

void fbg_draw_line(fbg_Screen screen, int x1, int y1, int x2, int y2, fbg_Color color);

void fbg_draw_triangle(fbg_Screen screen, int x1, int y1, int x2, int y2, int x3, int y3, fbg_Color color);

void fbg_draw_triangle_fill(fbg_Screen screen, int x1, int y1, int x2, int y2, int x3, int y3, fbg_Color color);

void fbg_draw_box(fbg_Screen screen, int x, int y, int width, int height, fbg_Color color);

void fbg_draw_box_fill(fbg_Screen screen, int x, int y, int width, int height, fbg_Color color);
//============================================================================//
// implementation
//============================================================================//

#ifdef FBG_IMPLEMENTATION

#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <math.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <linux/fb.h>
#include <linux/kd.h>

//----------------------------------------------------------------------------//
// private functions
//----------------------------------------------------------------------------//

static void _flip(int *a, int *b) {
	int i = *a;
	*a = *b;
	*b = i;
}

static void _draw_top_flat_triangle(fbg_Screen screen, int x1, int y1, int x2, int y2, int x3, int y3, fbg_Color color) {
	int min = x1;
	if(x2 < min)
		min = x2;
	if(x3 < min)
		min = x3;
	
	int max = x1;
	if(x2 > max)
		max = x2;
	if(x3 > max)
		max = x3;
	
	int width = max - min;

	// same as fbg_clear_screen(): copy each line instead of drawing each pixel
	char *buff = malloc(width * screen.bytesPerPixel);

	for(int i = 0; i < width; i++) {
		int offset = i * screen.bytesPerPixel;

		buff[offset + screen.offsetRed] = color.r;
		buff[offset + screen.offsetGreen] = color.g;
		buff[offset + screen.offsetBlue] = color.b;
	}

	float angle2 = (float)(x1 - x2) / (float)(y1 - y2);
	float angle3 = (float)(x1 - x3) / (float)(y1 - y3);

	float currentX2 = x1;
	float currentX3 = x1;

	float *left, *right;

	if(angle3 < angle2) {
		left = &currentX2;
		right = &currentX3;
	} else {
		left = &currentX3;
		right = &currentX2;
	}

	for(int currentY = y1; currentY > y3; currentY--) {
		currentX2 -= angle2;
		currentX3 -= angle3;
		
		int offset = (int)*left * screen.bytesPerPixel + currentY * screen.bytesPerLine;

		memcpy(screen.offscreenBuffer + offset, buff, (*right - *left) * screen.bytesPerPixel);
	}

	free(buff);
}

static void _draw_bottom_flat_triangle(fbg_Screen screen, int x1, int y1, int x2, int y2, int x3, int y3, fbg_Color color) {
	int min = x1;
	if(x2 < min)
		min = x2;
	if(x3 < min)
		min = x3;
	
	int max = x1;
	if(x2 > max)
		max = x2;
	if(x3 > max)
		max = x3;
	
	int width = max - min;

	char *buff = malloc(width * screen.bytesPerPixel);

	for(int i = 0; i < width; i++) {
		int offset = i * screen.bytesPerPixel;

		buff[offset + screen.offsetRed] = color.r;
		buff[offset + screen.offsetGreen] = color.g;
		buff[offset + screen.offsetBlue] = color.b;
	}

	float angle1 = (float)(x3 - x1) / (float)(y3 - y1);
	float angle2 = (float)(x3 - x2) / (float)(y3 - y2);

	float currentX1 = x3;
	float currentX2 = x3;

	float *left, *right;

	if(angle2 > angle1) {
		left = &currentX1;
		right = &currentX2;
	} else {
		left = &currentX2;
		right = &currentX1;
	}

	for(int currentY = y3; currentY < y1; currentY++) {
		currentX1 += angle1;
		currentX2 += angle2;

		int offset = (int)*left * screen.bytesPerPixel + currentY * screen.bytesPerLine;

		memcpy(screen.offscreenBuffer + offset, buff, (*right - *left) * screen.bytesPerPixel);
	}

	free(buff);
}

//----------------------------------------------------------------------------//
// public functions
//----------------------------------------------------------------------------//

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
	screen->bytesPerPixel = variableInfo.bits_per_pixel / 8;
	screen->bytesPerLine = fixedInfo.line_length;
	screen->offsetRed = variableInfo.red.offset / 8;
	screen->offsetGreen = variableInfo.green.offset / 8;
	screen->offsetBlue = variableInfo.blue.offset / 8;

	screen->onscreenBuffer = mmap(0, screen->size, PROT_READ | PROT_WRITE, MAP_SHARED, fbFile, 0);
	if(screen->onscreenBuffer < 0)
		return -3;
	
	screen->offscreenBuffer = malloc(screen->size);
	if(screen->offscreenBuffer == NULL)
		return -4;

	close(fbFile);
	return 0;
}

void fbg_free_screen(fbg_Screen *screen) {
	munmap(screen->onscreenBuffer, screen->size);
	free(screen->offscreenBuffer);
}

void fbg_clear_screen(fbg_Screen screen, fbg_Color color) {
	// draw top line
	fbg_draw_line(screen, 0, 0, screen.width, 0, color);

	// copy the line down (much faster the drawing individual lines)
	for(int i = 1; i < screen.height; i++) {
		int offset = i * screen.bytesPerLine;
		memcpy(&screen.offscreenBuffer[offset], &screen.offscreenBuffer[0], screen.bytesPerLine);
	}
}

void fbg_display(fbg_Screen screen) {
	// copy contents of the offscreenBuffer to the onscreenBuffer
	memcpy(screen.onscreenBuffer, screen.offscreenBuffer, screen.size);
}

int fbg_draw_pixel(fbg_Screen screen, int x, int y, fbg_Color color) {
	if(x < 0 || x > screen.width || y < 0 || y > screen.height)
		return -1;

	int offset = x * screen.bytesPerPixel + y * screen.bytesPerLine;
				
	screen.offscreenBuffer[offset + screen.offsetRed] = color.r;
	screen.offscreenBuffer[offset + screen.offsetGreen] = color.g;
	screen.offscreenBuffer[offset + screen.offsetBlue] = color.b;

	return 0;
}

void fbg_draw_line(fbg_Screen screen, int x1, int y1, int x2, int y2, fbg_Color color) {
	// check if the line is steeper than PI/4 (travels more int the y axis)
	int steep = 0;

	if(abs(x2 - x1) < abs(y2 - y1)) {
		_flip(&x1, &y1);
		_flip(&x2, &y2);

		steep = 1;
	}

	// make sure x1 is always on the right
	if(x1 > x2) {
		_flip(&x1, &x2);
		_flip(&y1, &y2);	
	}

	float angle = (float)(y2 - y1) / (float)(x2 - x1);

	float y = y1;

	if(steep) {
		// draw first pixel
		fbg_draw_pixel(screen, x1, y1, color);
		char *src = &screen.offscreenBuffer[x1 * screen.bytesPerPixel + y1 * screen.bytesPerLine];

		// copy all other pixels from first pixel
		for(int x = x1 + 1; x < x2; x++) {
			y += angle;
			int offset = (int)y * screen.bytesPerPixel + x * screen.bytesPerLine;

			memcpy(screen.offscreenBuffer + offset, src, screen.bytesPerPixel);
		}
	} else {
		fbg_draw_pixel(screen, y1, x1, color);
		char *src = &screen.offscreenBuffer[y1 * screen.bytesPerPixel + x1 * screen.bytesPerLine];

		for(int x = x1 + 1; x < x2; x++) {
			y += angle;
			int offset = x * screen.bytesPerPixel + (int)y * screen.bytesPerLine;

			memcpy(screen.offscreenBuffer + offset, src, screen.bytesPerPixel);
		}
	}
}

void fbg_draw_triangle(fbg_Screen screen, int x1, int y1, int x2, int y2, int x3, int y3, fbg_Color color) {
	fbg_draw_line(screen, x1, y1, x2, y2, color);
	fbg_draw_line(screen, x2, y2, x3, y3, color);
	fbg_draw_line(screen, x3, y3, x1, y1, color);
}

void fbg_draw_triangle_fill(fbg_Screen screen, int x1, int y1, int x2, int y2, int x3, int y3, fbg_Color color) {
	// sort points by y-ascending order
	if(y1 < y3) {
		_flip(&x1, &x3);
		_flip(&y1, &y3);
	}
	if(y1 < y2) {
		_flip(&x1, &x2);
		_flip(&y1, &y2);
	}
	if(y2 < y3) {
		_flip(&x2, &x3);
		_flip(&y2, &y3);
	}

	if(y1 == y2) {
		// bottom flat
		_draw_bottom_flat_triangle(screen, x1, y1, x2, y2, x3, y3, color);

	} else if(y2 == y3) {
		// top flat
		_draw_top_flat_triangle(screen, x1, y1, x2, y2, x3, y3, color);

	} else {
		// split triangle into top flat and bottom flat
		int x4 = (float)x1 + (float)(y2 - y1) / (float)(y3 - y1) * (float)(x3 - x1);
		int y4 = y2;

		_draw_top_flat_triangle(screen, x1, y1, x2, y2, x4, y4, color);
		_draw_bottom_flat_triangle(screen, x2, y2 + 1, x4, y4 + 1, x3, y3, color);
	}

}

void fbg_draw_box(fbg_Screen screen, int x, int y, int width, int height, fbg_Color color) {
	fbg_draw_line(screen, x, y, x + width, y, color);
	fbg_draw_line(screen, x + width, y, x + width, y + height, color);
	fbg_draw_line(screen, x + width, y + height, x, y + height, color);
	fbg_draw_line(screen, x, y + height, x, y, color);
}

void fbg_draw_box_fill(fbg_Screen screen, int x, int y, int width, int height, fbg_Color color) {
	if(height < 0) {
		height = -height;
		y = y - height;
	}

	if(width < 0) {
		width = -width;
		x = x - width;
	}

	fbg_draw_line(screen, x, y, x + width, y, color);

	char *src = &screen.offscreenBuffer[x * screen.bytesPerPixel + y * screen.bytesPerLine];
	int offset = x * screen.bytesPerPixel + (y + 1) * screen.bytesPerLine;

	for(int i = 1; i < height; i++) {
		offset += screen.bytesPerLine;
		memcpy(screen.offscreenBuffer + offset, src, width * screen.bytesPerPixel);
	}
}

#endif
#endif