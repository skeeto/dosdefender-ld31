/* http://dosgraphicseditor.blogspot.com/p/mouse-events-tutorial.html */
#include "int.h"

#define MOUSE_XMAX 639
#define MOUSE_YMAX 199

struct mouse {
    uint16_t x, y;
    bool left, right, middle;
};

static void mouse_init()
{
    asm ("mov   $0x00, %ax\n"
         "int   $0x33");
}

static void mouse_show()
{
    asm ("mov   $0x01, %ax\n"
         "int   $0x33");
}

static void mouse_hide()
{
    asm ("mov   $0x02, %ax\n"
         "int   $0x33");
}

static struct mouse mouse_read()
{
    struct mouse mouse;
    uint16_t buttons;
    asm volatile ("mov  $0x03, %%ax\n"
                  "int  $0x33\n"
                  : "=b"(buttons), "=c"(mouse.x), "=d"(mouse.y)
                  : /* no input */
                  : "ax");
    mouse.left =   !!(buttons & 0x1);
    mouse.right =  !!(buttons & 0x2);
    mouse.middle = !!(buttons & 0x4);
    return mouse;
}
