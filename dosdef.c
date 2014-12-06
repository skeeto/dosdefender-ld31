#include "init.h"
#include "print.h"
#include "joystick.h"

int _main(void)
{
    for (;;) {
        struct joystick joy;
        joystick_read(&joy);
        printl(joy.x);
        print("\n$");
    }
    return 0;
}
