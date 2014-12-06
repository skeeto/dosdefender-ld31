#include "init.h"
#include "print.h"
#include "joystick.h"
#include "vga.h"
#include "rand.h"
#include "time.h"
#include "alloc.h"

#define SCALE              1000
#define BACKGROUND         17
#define PLAYER             14
#define BULLET_SPEED       3
#define PARTICLE_MAX_AGE   50

typedef unsigned int tick_t;

static tick_t ticks;

struct ship {
    int32_t x, y, dx, dy;
    tick_t last_fire;
    uint8_t fire_delay;
    uint8_t color_a, color_b;
};

struct bullet {
    int32_t x, y, dx, dy;
    tick_t birthtick;
    uint8_t color;
    bool alive;
};

struct particle {
    int32_t x, y;
    tick_t birthtick;
    bool alive;
};

static struct bullet *bullets;
static size_t bullets_max = 128;

static struct particle *particles;
static size_t particles_max = 128;

static void ship_draw(struct ship *ship, bool clear)
{
    struct point c = {ship->x / SCALE, ship->y / SCALE};
    for (int i = -1; i <= 1; i++) {
        struct point ha = {ship->x / SCALE - i, ship->y / SCALE - 2};
        struct point hb = {ship->x / SCALE - i, ship->y / SCALE + 2};
        struct point va = {ship->x / SCALE - 2, ship->y / SCALE - i};
        struct point vb = {ship->x / SCALE + 2, ship->y / SCALE - i};
        vga_pixel(ha, clear ? BACKGROUND : ship->color_a);
        vga_pixel(va, clear ? BACKGROUND : ship->color_a);
        vga_pixel(hb, clear ? BACKGROUND : ship->color_a);
        vga_pixel(vb, clear ? BACKGROUND : ship->color_a);
    }
    vga_pixel(c, clear ? BACKGROUND : ship->color_b);
    struct point d = {c.x + ship->dx / 10, c.y + ship->dy / 10};
    vga_pixel(d, clear ? BACKGROUND : WHITE);
}

static void bullet_draw(int i, bool clear)
{
    struct point c = {bullets[i].x / SCALE, bullets[i].y / SCALE};
    vga_pixel(c, clear ? BACKGROUND : bullets[i].color);
}

static void bullet_step(int i)
{
    bullets[i].x += bullets[i].dx;
    bullets[i].y += bullets[i].dy;
    if (bullets[i].x < 0 || bullets[i].x > VGA_PWIDTH * SCALE ||
        bullets[i].y < 0 || bullets[i].y > VGA_PHEIGHT * SCALE)
        bullets[i].alive = false;
}

static void ship_step(struct ship *ship)
{
    ship->x += ship->dx;
    ship->y += ship->dy;
    ship->dx = (ship->dx * 99) / 100;
    ship->dy = (ship->dy * 99) / 100;
}

static int ship_fire(struct ship *source)
{
    if (source->last_fire + source->fire_delay > ticks)
        return -1;
    source->last_fire = ticks;
    int choice = 0;
    for (int i = 0; i < bullets_max; i++) {
        if (!bullets[i].alive) {
            choice = i;
            break;
        } else if (bullets[i].birthtick < bullets[choice].birthtick) {
            choice = i;
        }
    }
    if (bullets[choice].alive)
        bullet_draw(choice, true);
    bullets[choice].x = source->x + source->dx / 100;
    bullets[choice].y = source->y + source->dy / 100;
    bullets[choice].dx = source->dx * BULLET_SPEED;
    bullets[choice].dy = source->dy * BULLET_SPEED;
    bullets[choice].color = source->color_b;
    bullets[choice].birthtick = ticks;
    bullets[choice].alive = true;
    return choice;
}

static void particle_draw(int i, bool clear)
{
    struct point c = {particles[i].x / SCALE, particles[i].y / SCALE};
    if (clear) {
        vga_pixel(c, BACKGROUND);
    } else {
        int age = ticks - particles[i].birthtick;
        vga_pixel(c, age > PARTICLE_MAX_AGE * 3 / 4
                  ? ((rand() % 5) + 24)    // smoke
                  : ((rand() % 5) + 40));  // fire
    }
}

static void particle_step(int i)
{
    if (ticks - particles[i].birthtick > PARTICLE_MAX_AGE) {
        particles[i].alive = false;
        particle_draw(i, true);
    } else {
        int speed = 2;
        particles[i].x += (rand() % (SCALE * speed)) - (SCALE * speed / 2);
        particles[i].y += (rand() % (SCALE * speed)) - (SCALE * speed / 2);
    }
}

static void burn(int32_t x, int32_t y)
{
    int choice = 0;
    for (int i = 0; i < particles_max; i++) {
        if (!particles[i].alive) {
            choice = i;
            break;
        } else if (particles[i].birthtick < particles[choice].birthtick) {
            choice = i;
        }
    }
    if (particles[choice].alive)
        particle_draw(choice, true);
    particles[choice].alive = true;
    particles[choice].x = x;
    particles[choice].y = y;
    particles[choice].birthtick = ticks;
}

static bool joystick_detected()
{
    struct joystick joy;
    joystick_read(&joy);
    return joy.x != 0 || joy.y != 0;
}

int _main(void)
{
    rand_seed += get_tick();
    if (!joystick_detected()) {
        print("DOS Defender requires a joystick!$");
        return -1;
    }

    /* Allocate memory. */
    bullets = sbrk(bullets_max * sizeof(bullets[0]));
    particles = sbrk(particles_max * sizeof(particles[0]));

    vga_on();
    // TODO: this is temporary
    joystick_config[0].xmax = joystick_config[0].ymax = 254;
    joystick_config[0].xmin = joystick_config[0].ymin = 1;
    joystick_config[0].xcenter = joystick_config[0].ycenter = 128;
    //joystick_calibrate();

    vga_clear(BACKGROUND);
    struct joystick_config jconf = joystick_config[0];
    struct ship player = {
        .x = VGA_PWIDTH / 2 * SCALE,
        .y = VGA_PHEIGHT / 2 * SCALE,
        .color_a = YELLOW,
        .color_b = LIGHT_BLUE,
        .fire_delay = 10,
    };
    int xrange = 2 * (jconf.xmax - jconf.xmin);
    int yrange = 2 * (jconf.ymax - jconf.ymin);
    for (;;) {
        ship_draw(&player, true);
        struct joystick joy;
        joystick_read(&joy);
        player.dx += ((joy.x - jconf.xcenter) * 100) / xrange;
        player.dy += ((joy.y - jconf.xcenter) * 100) / yrange;
        ship_step(&player);
        ship_draw(&player, false);
        if (joy.b) // TODO: temporary
            burn(player.x, player.y);
        if (joy.a)
            ship_fire(&player);
        for (int i = 0; i < bullets_max; i++) {
            bullet_draw(i, true);
            if (bullets[i].alive) {
                bullet_step(i);
                bullet_draw(i, false);
            }
        }
        for (int i = 0; i < particles_max; i++) {
            particle_draw(i, true);
            if (particles[i].alive) {
                particle_step(i);
                if (particles[i].alive)
                    particle_draw(i, false);
            }
        }
        vga_vsync();
        ticks++;
    }
    vga_off();
    return 0;
}
