#include "init.h"
#include "print.h"
#include "joystick.h"
#include "vga.h"
#include "rand.h"
#include "time.h"
#include "alloc.h"
#include "keyboard.h"

#define SCALE              1000
#define BACKGROUND         17
#define PLAYER             14
#define BULLET_SPEED       3
#define PARTICLE_MAX_AGE   50

typedef unsigned int tick_t;
typedef void (*ai_t)(int id);

static tick_t ticks;
static unsigned score;

struct ship {
    int32_t x, y, dx, dy;
    tick_t last_fire;
    ai_t ai;
    uint16_t score;
    uint8_t fire_delay;
    uint8_t color_a, color_b;
    uint8_t hp, hp_max;
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
static size_t bullets_max = 64;

static struct particle *particles;
static size_t particles_max = 64;

static struct ship *ships;
static size_t ships_max = 16;

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

static int ship_fire(int i)
{
    if (ships[i].last_fire + ships[i].fire_delay > ticks)
        return -1;
    ships[i].last_fire = ticks;
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
    bullets[choice].x = ships[i].x + ships[i].dx / 100;
    bullets[choice].y = ships[i].y + ships[i].dy / 100;
    bullets[choice].dx = ships[i].dx * BULLET_SPEED;
    bullets[choice].dy = ships[i].dy * BULLET_SPEED;
    bullets[choice].color = ships[i].color_b;
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
                  ? (randn(5) + 24)    // smoke
                  : (randn(5) + 40));  // fire
    }
}

static void particle_step(int i)
{
    if (ticks - particles[i].birthtick > PARTICLE_MAX_AGE) {
        particles[i].alive = false;
        particle_draw(i, true);
    } else {
        int speed = 2;
        particles[i].x += randn(SCALE * speed) - (SCALE * speed / 2);
        particles[i].y += randn(SCALE * speed) - (SCALE * speed / 2);
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

static void ship_draw(int id, bool clear)
{
    struct point c = {ships[id].x / SCALE, ships[id].y / SCALE};
    for (int i = -1; i <= 1; i++) {
        struct point ha = {ships[id].x / SCALE - i, ships[id].y / SCALE - 2};
        struct point hb = {ships[id].x / SCALE - i, ships[id].y / SCALE + 2};
        struct point va = {ships[id].x / SCALE - 2, ships[id].y / SCALE - i};
        struct point vb = {ships[id].x / SCALE + 2, ships[id].y / SCALE - i};
        vga_pixel(ha, clear ? BACKGROUND : ships[id].color_a);
        vga_pixel(va, clear ? BACKGROUND : ships[id].color_a);
        vga_pixel(hb, clear ? BACKGROUND : ships[id].color_a);
        vga_pixel(vb, clear ? BACKGROUND : ships[id].color_a);
    }
    struct point d = {c.x + ships[id].dx / 10, c.y + ships[id].dy / 10};
    if (id == 0)
        vga_pixel(d, clear ? BACKGROUND : WHITE);
}

static void ship_step(int i)
{
    ships[i].x += ships[i].dx;
    ships[i].y += ships[i].dy;
    ships[i].dx = (ships[i].dx * 99) / 100;
    ships[i].dy = (ships[i].dy * 99) / 100;
    if (ships[i].hp < ships[i].hp_max / 2) {
        if ((randn(ships[i].hp < 1 ? 1 : ships[i].hp) == 0))
            burn(ships[i].x, ships[i].y);
    }
}

static bool joystick_detected()
{
    struct joystick joy;
    joystick_read(&joy);
    return joy.x != 0 || joy.y != 0;
}

static void print_game_over()
{
    vga_print((struct point){133, 97}, WHITE, "GAME OVER");
}

static void ship_check_bounds(int i)
{
    int32_t xlim = VGA_PWIDTH * SCALE;
    int32_t ylim = VGA_PHEIGHT * SCALE;
    if (ships[i].x < 0 || ships[i].x > xlim ||
        ships[i].y < 0 || ships[i].y > ylim) {
        ships[i].dx = 0;
        ships[i].dy = 0;
        ships[i].hp = 0;
    }
}

int spawn(int hp, ai_t ai)
{
    int choice = -1;
    for (int i = 1; i < ships_max; i++) {
        if (!ships[i].hp > 0) {
            choice = i;
            break;
        }
    }
    if (choice > -1) {
        ships[choice].hp = hp;
        ships[choice].dx = ships[choice].dy = 0;
        if (randn(2)) {
            ships[choice].x = randn(2) * VGA_PWIDTH * SCALE;
            ships[choice].y = randn(VGA_PHEIGHT * SCALE);
        } else {
            ships[choice].x = randn(VGA_PWIDTH * SCALE);
            ships[choice].y = randn(2) * VGA_PHEIGHT * SCALE;
        }
        ships[choice].ai = ai;
    }
    return choice;
}

static void ai_player(int i)
{
    int xrange = 2 * (joystick_config[0].xmax - joystick_config[0].xmin);
    int yrange = 2 * (joystick_config[0].ymax - joystick_config[0].ymin);
    if (ships[i].hp > 0) {
        struct joystick joy;
        joystick_read(&joy);
        ships[i].dx += ((joy.x - joystick_config[0].xcenter) * 100) / xrange;
        ships[i].dy += ((joy.y - joystick_config[0].xcenter) * 100) / yrange;
        if (joy.a)
            ship_fire(i);
    }
}

static void ai_seeker(int i)
{
    int noise = 400;
    int dx = ships[0].x - ships[i].x;
    int dy = ships[0].y - ships[i].y;
    ships[i].dx = dx / 250 + randn(noise) - noise / 2;
    ships[i].dy = dy / 250 + randn(noise) - noise / 2;
    ship_fire(i);
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
    ships = sbrk(ships_max * sizeof(ships[0]));

    /* Initialize Interface */
    vga_on();
    // TODO: hardcoded calibration is temporary
    joystick_config[0].xmax = joystick_config[0].ymax = 254;
    joystick_config[0].xmin = joystick_config[0].ymin = 1;
    joystick_config[0].xcenter = joystick_config[0].ycenter = 128;
    //joystick_calibrate();

    /* Initialize Ships */
    ships[0] = (struct ship) {
        .x = VGA_PWIDTH / 2 * SCALE,
        .y = VGA_PHEIGHT / 2 * SCALE,
        .color_a = YELLOW,
        .color_b = LIGHT_BLUE,
        .fire_delay = 10,
        .hp = 100,
        .hp_max = 100,
        .ai = ai_player
    };

    /* Main Loop */
    vga_clear(BACKGROUND);
    for (;;) {
        if (randn(100) == 0) {
            int id = spawn(2, ai_seeker);
            if (id >= 0) {
                ships[id].color_a = BROWN;
                ships[id].color_b = LIGHT_RED;
                ships[id].fire_delay = 100;
                ships[id].hp_max = 1;
                ships[id].score = 100;
            }
        }

        if (ships[0].hp == 0) {
            print_game_over();
            if (kbhit())
                break;
        }
        for (int i = 0; i < ships_max; i++) {
            if (ships[i].hp > 0 || i == 0) {
                ship_draw(i, true);
                ship_step(i);
                ships[i].ai(i);
                if (ships[i].hp > 0)
                    ship_draw(i, false);
            }
        }
        ship_check_bounds(0);
        for (int i = 0; i < bullets_max; i++) {
            bullet_draw(i, true);
            if (bullets[i].alive) {
                bullet_step(i);
                if (bullets[i].alive)
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
    print("score: $");
    printl(score);
    return 0;
}
