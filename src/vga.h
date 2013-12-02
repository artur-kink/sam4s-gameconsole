#ifndef _VGA_
#define _VGA_
#include "asf.h"
#include "controllers/controller.h"

/** 
 * VGA Output on SAM4S, for xplained-pro demo board.
 *
 * VGA timing is generated mainly by using interrupts.
 * The main timing interrupt is the vsync interrupt. It is called
 * ~60 times per second(see specific timing information in code).
 * Whenever a vsync interrupt occurs, a second timer is started to
 * time the length of a vsync pulse. Once the vsync pulse timer 
 * has elapsed, a new timer is started for the hsync signals.
 * For every hsync interrupt we time the pulse manually in clock cycles
 * (since it is so short), then we blit the pixel buffer to the screen and
 * wait for the next hsync interrupt to occur. This happens ~600 times
 * in one vsync, the last hsync is timed to happen exactly before a new
 * vsync interrupt should occur.
 *
 * Color output. 8 bit color is used, 3 bits for Red, 3 bits for Green and
 * 2 bits for Blue. To minimize the amount of cycles needed to change 
 * colors for every pixel, we set all pins in one command using a port
 * mask. For this reason all color pins have to be on the same port,
 * and no other pins should be relied or written to on that port.
 * Pins have to be positioned sequentially as they are positioned
 * in memory, so a 8 bit value can be set by simply writing that value
 * to the port, (On the xplained-pro the lowest 8 sequential pins
 * we have access to start at pin 17, which is the explanation for
 * why the color byte is bit shifted by 17 when setting the pin
 * values in pixel blitting). Since VGA takes analog values for
 * RGB, DACS are needed to convert the 3,3 and 2 bit values to
 * corresponding 0.0 to 1.0 values. A simple DAC can be created
 * using resistor ladders.
 *
 * Output resolution is designed for 640x480 at 60hz. However since
 * we do not have enough cycles for that resolution, each pixel is stretched
 * by two resulting in a resolution of 320x240.
 */


//Macro to sleep 1 clock cycle.
#define SLEEP1 asm("nop");
//Sleep 5 clock cycles.
#define SLEEP5 SLEEP1 SLEEP1 SLEEP1 SLEEP1 SLEEP1
//Sleep 10 clock cycles.
#define SLEEP10 SLEEP5 SLEEP5
//Sleep 20 clock cycles.
#define SLEEP20 SLEEP10 SLEEP10
//Sleep 50 clock cycles.
#define SLEEP50 SLEEP20 SLEEP20 SLEEP10
//Sleep 100 clock cycles.
#define SLEEP100 SLEEP50 SLEEP50
//Sleep 200 clock cycles.
#define SLEEP200 SLEEP100 SLEEP100

//VGA Info
//640x480 60hz
//Field Rate: 60.04hz
//System Clock: 120mhz - 120,000,000hz
//Vsync Timer Speed: 120mhz/128 = 937,500hz
//Vsync pulse: 60.04hz = 937,000/60.04 = 15,614.59
#define VSYNC_PULSE 15614

//Vsync pulse length: 63.57mu
//Vsync Timer Speed: 120mhz/2 = 60mhz = 60hz/mu
//Vsync pulse length: 60x63.57 = 3,814
#define VSYNC_PULSE_LEN 3814

//Hsync pulse: 31.46khz
//Hsync Timer Speed: 120mhz/2 = 60mhz = 60,000khz
//Hsync pulse length: 60,000/31.46 = 1,907.18
#define HSYNC_PULSE 1907//1907

//Hsync pulse length: 3.81mu
//Hsync Timer Speed: 120mhz/2 = 60mhz = 60hz/mu
//Vsync pulse length: 60x 3.81 = 228.6
#define HSYNC_PULSE_LEN 228

//VGA Timer
#define TC_VGA TC0
//Vertical Sync Channel
#define TCC_VSYNC 0
//Vertical Sync Pulse Length Channel
#define TCC_VSYNC_PULSE 1
//Horizontal Sync Channel
#define TCC_HSYNC 2

//VSync Pin
#define PIN_VSYNC EXT3_PIN_3

//HSync Pin
#define PIN_HSYNC EXT3_PIN_5
//Hsync port
static Pio* hsync_port;
//Hsync pin mask in port
static unsigned int hsync_mask;

//RGB port, The port where the rgb pins are.
static Pio* rgb_port;
static unsigned int rgb_mask;

//Red Pins
#define PIN_RED_1 EXT1_PIN_5
static unsigned int red_mask_1;
#define PIN_RED_2 EXT1_PIN_7
static unsigned int red_mask_2;
#define PIN_RED_3 EXT2_PIN_14
static unsigned int red_mask_3;

//Green Pins
#define PIN_GREEN_1 EXT2_PIN_13
static unsigned int green_mask_1;
#define PIN_GREEN_2 EXT2_PIN_8
static unsigned int green_mask_2;
#define PIN_GREEN_3 EXT1_PIN_8
static unsigned int green_mask_3;

//Blue Pins
#define PIN_BLUE_1 EXT1_PIN_4
static unsigned int blue_mask_1;
#define PIN_BLUE_2 EXT1_PIN_3
static unsigned int blue_mask_2;

//Bitmask for Red
#define COLOR_RED 32+64+128
//Bitmask for Green
#define COLOR_GREEN 4+8+16
//Bitmask for Blue
#define COLOR_BLUE 1+2

//Bitmasks for additive colors.
#define COLOR_MAGENTA (COLOR_RED + COLOR_BLUE)
#define COLOR_TEAL (COLOR_GREEN + COLOR_BLUE)
#define COLOR_YELLOW (COLOR_RED + COLOR_GREEN)
#define COLOR_WHITE (COLOR_RED + COLOR_GREEN + COLOR_BLUE)
#define COLOR_BLACK 0

//Height of screen, the number of lines.
#define SCREEN_HEIGHT 240
//Width of screen, number of pixels per line.
#define SCREEN_WIDTH 320

//Screen pixel buffer.
unsigned char pixel_buffer[SCREEN_HEIGHT][SCREEN_WIDTH];

//Vsync porch counts
#define VSYNC_FRONT_PORCH 12
#define VSYNC_BACK_PORCH (524-32)

//Horizontal sync counter.
static int hsync_counter = 0;

//Debug LED state.
static unsigned long active = LED_0_ACTIVE;
//Debug second counter.
static unsigned int sec_counter = 0;

void vga_vsync_Handler(void);
void vga_vsync_pulse_Handler(void);
void vga_hsync_Handler(void);
void vga_hsync(void);

inline void vga_clear_screen(unsigned char color);
inline void vga_set_pixel(unsigned int x, unsigned int y, unsigned char color);
inline unsigned char vga_get_pixel(unsigned int x, unsigned int y);

void vga_start(void);
void vga_stop(void);
void vga_init(void);

inline int vga_get_sec_frame(void);

#endif