#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <unistd.h>
#include <getopt.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_image.h>

#define ARRAY_SIZE(x)	(sizeof(x) / sizeof(*(x)))

#define BUFFER_WIDTH	800
#define BUFFER_HEIGHT	800

#define DISPLAY_SCALE 	1
#define DISPLAY_WIDTH 	(BUFFER_WIDTH  * DISPLAY_SCALE)
#define DISPLAY_HEIGHT 	(BUFFER_HEIGHT * DISPLAY_SCALE)

#define KEY_SEEN      1
#define KEY_RELEASED  2

#define ASTEROID_VERTICES_COUNT 6

#define BULLETS_COUNT   128
#define ASTEROIDS_COUNT 40


// DISPLAY
ALLEGRO_DISPLAY* display;
ALLEGRO_BITMAP* buffer;

// KEYBOARD
unsigned char key[ALLEGRO_KEY_MAX];

// SAMPLES
ALLEGRO_SAMPLE* sample_fire;

// FONTS
ALLEGRO_FONT* font;

// ENTITY TEMPLATE TRANSFORMS
ALLEGRO_VERTEX ship_v[4];
ALLEGRO_VERTEX bullet_v[4];

// COLOUR VARIABLES
ALLEGRO_COLOR ERROR_COLOUR = { .r = 1.0, .g = 0.0, .b = 1.0, .a = 1.0 };

ALLEGRO_COLOR DEBUG_COLLIDER_COLOUR = { .r = 0.0, .g = 1.0, .b = 0.0, .a = 1.0};

ALLEGRO_COLOR U_NAC_BOARD_COLOUR = { .r = 1.0, .g = 1.0, .b = 1.0, .a = 1.0};
ALLEGRO_COLOR NAC_BOARD_COLOUR = { .r = 0.7, .g = 0.7, .b = 0.7, .a = 1.0};

ALLEGRO_COLOR P1_COLOUR = { .r = 0.0, .g = 0.0, .b = 1.0, .a = 1.0 };
ALLEGRO_COLOR P2_COLOUR = { .r = 1.0, .g = 0.0, .b = 0.0, .a = 1.0 };
ALLEGRO_COLOR ASTEROID_COLOUR = { .r = 0.3, .g = 0.3, .b = 0.3, .a = 1.0 };

// DEBUG VARIABLES
int DEBUG_VIEW_COLLIDERS = false;

// MISC VARIABLES
const int BORDER_LENGTH  = 750;
const int BORDER_PADDING =  25;


// STRUCTS
typedef struct SHIP
{
    double x, y, r, dx, dy;
    double thrust;
    double rot_speed;
    double fire_delay;
    int lives;
    int id;

    ALLEGRO_COLOR colour;

    ALLEGRO_VERTEX transformed_v[4];
} SHIP;


typedef struct BULLET
{
    double x, y, r, dx, dy, max_time, timer;
    bool used;

    ALLEGRO_VERTEX transformed_v[4];
} BULLET;

typedef struct CHARGE
{
    double x, y, radius, dx, dy, timer;
    int state;
} CHARGE;


typedef struct ASTEROID
{
    double x, y, r, dx, dy, dr;

    ALLEGRO_VERTEX template_v[ASTEROID_VERTICES_COUNT];
    ALLEGRO_VERTEX transformed_v[ASTEROID_VERTICES_COUNT];
} ASTEROID;


typedef struct U_NAC_BOARD
{
    double x_0, y_0, length;
    int winner;
    int padding;
} U_NAC_BOARD;


typedef struct NAC_BOARD
{
    double x_0, y_0, length;
    int squares[3][3];
    int winner;
    double padding;
} NAC_BOARD;

// STRUCTS VARIABLES
SHIP ships[2];

BULLET bullets[BULLETS_COUNT];

CHARGE charge;

ASTEROID asteroids[ASTEROIDS_COUNT];

U_NAC_BOARD u_nac_board;

NAC_BOARD nac_boards[3][3];


// FUNCTION PROTOTYPES

// UTILITY FUNCTIONS

void flag_reader(int argc, char* argv[]);

// 2D Rotation Helper Function
void rotate2D(ALLEGRO_VERTEX* v, double r);

int rand_int(int lo, int hi);
float rand_double(double lo, double hi);
bool circular_collision(double x_0, double y_0, double x_1, double y_1);
//bool within_cell_boundaries(double x, double y)

// INIT FUNCTIONS
void must_init(bool test, const char *description);
void display_init();
void display_deinit();
void display_pre_draw();
void display_post_draw();

void audio_init();
void audio_deinit();

void keyboard_init();

void gui_init();
void gui_deinit();

void ship_init();
void bullets_init();
void charge_init();
void asteroids_init();
void nac_boards_init();

void bullets_add(SHIP* ship);
void charge_set(SHIP* ship);

bool bullet_collision(double x, double y);
bool charge_collision(double x, double y);
bool asteroid_collision(double x, double y);

void keyboard_update(ALLEGRO_EVENT* event);
void gui_update();
void input_update();
void ship_update(SHIP* ship);
void bullets_update();
void charge_update();
void asteroids_update();

bool game_end_update();

void gui_draw();
void nac_boards_draw();
void nac_board_mark(int i, int j);
void x_draw(double x_0, double y_0, int i, int j);
void o_draw(double x_0, double y_0, int i, int j);
void ship_draw();
void bullets_draw();
void charge_draw();
void asteroids_draw();
void border_draw();
