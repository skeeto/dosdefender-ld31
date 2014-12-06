#include "math.h"
#include "int.h"

struct joystick {
    uint16_t x, y;
    bool a, b;
};

struct joystick_config {
    uint16_t xmin, xmax;
    uint16_t ymin, ymax;
    uint16_t xcenter, ycenter;
};

static struct joystick_config joystick_config[2] = {
    {0xffff, 0x0000, 0xffff, 0x0000, 0xffff, 0xffff},
    {0xffff, 0x0000, 0xffff, 0x0000, 0xffff, 0xffff}
};

static void joystick_read2(struct joystick *a, struct joystick *b)
{
    asm volatile ("mov  $0x84, %%ah\n"
                  "mov  $1, %%dx\n"
                  "int  $0x15\n"
                  : "=a"(a->x), "=b"(a->y), "=c"(b->x), "=d"(b->y));
    joystick_config[0].xmin = min(joystick_config[0].xmin, a->x);
    joystick_config[0].xmax = max(joystick_config[0].xmax, a->x);
    joystick_config[0].ymin = min(joystick_config[0].ymin, a->y);
    joystick_config[0].ymax = max(joystick_config[0].ymax, a->y);
    joystick_config[1].xmin = min(joystick_config[1].xmin, b->x);
    joystick_config[1].xmax = max(joystick_config[1].xmax, b->x);
    joystick_config[1].ymin = min(joystick_config[1].ymin, b->y);
    joystick_config[1].ymax = max(joystick_config[1].ymax, b->y);
    uint16_t buttons = 0;
    asm volatile ("mov  $0x84, %%ah\n"
                  "mov  $0, %%dx\n"
                  "int  $0x15\n"
                  : "=a"(buttons)
                  : /**/
                  : "bx", "cx", "dx", "flags");
    a->a = !(buttons & (1 << 4));
    a->b = !(buttons & (1 << 5));
    b->a = !(buttons & (1 << 6));
    b->b = !(buttons & (1 << 7));
}

static void joystick_read(struct joystick *joystick)
{
    struct joystick dummy;
    joystick_read2(joystick, &dummy);
}

#include "vga.h"
#include "vga_font.h"

static void joystick_crosshair(struct point c, uint8_t color)
{
    vga_line((struct point){c.x - 2, c.y},
             (struct point){c.x + 2, c.y}, color);
    vga_line((struct point){c.x, c.y - 2},
             (struct point){c.x, c.y + 2}, color);
}

static void joystick_calibrate()
{
    vga_clear(BLUE);
    struct point cursor = {0, 0};
    struct joystick joy;
    do {
        joystick_read(&joy);
        int xmin = joystick_config[0].xmin;
        int xmax = joystick_config[0].xmax;
        int ymin = joystick_config[0].ymin;
        int ymax = joystick_config[0].ymax;
        vga_vsync();
        joystick_crosshair(cursor, BLUE);
        cursor.x = ((joy.x - xmin) * VGA_PWIDTH) / xmax;
        cursor.y = ((joy.y - ymin) * VGA_PHEIGHT) / ymax;
        joystick_crosshair(cursor, WHITE);
        vga_print((struct point){55, 80}, YELLOW,
                  "MOVE JOYSTICK AROUND ITS FULL RANGE");
        vga_print((struct point){43, 113}, YELLOW,
                  "THEN CENTER JOYSTICK AND PRESS BUTTON A");
        vga_pixel((struct point){VGA_PWIDTH / 2, VGA_PHEIGHT / 2}, BLACK);
    } while (!joy.a);
    joystick_config[0].xcenter = joy.x;
    joystick_config[0].ycenter = joy.y;
    do
        joystick_read(&joy);
    while (joy.a);
}
