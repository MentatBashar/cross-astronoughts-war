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

#define DISPLAY_SCALE 	2
#define DISPLAY_WIDTH 	(BUFFER_WIDTH  * DISPLAY_SCALE)
#define DISPLAY_HEIGHT 	(BUFFER_HEIGHT * DISPLAY_SCALE)

#define KEY_SEEN			1
#define KEY_RELEASED	2

#define ASTEROID_VERTICES_COUNT 6

#define BULLETS_COUNT	  128
#define ASTEROIDS_COUNT 20


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

ALLEGRO_COLOR DEBUG_COLLISION_COLOUR = { .r = 0.0, .g = 1.0, .b = 0.0, .a = 1.0};

ALLEGRO_COLOR P1_COLOUR = { .r = 0.0, .g = 0.0, .b = 1.0, .a = 1.0 };
ALLEGRO_COLOR P2_COLOUR = { .r = 1.0, .g = 0.0, .b = 0.0, .a = 1.0 };
ALLEGRO_COLOR ASTEROID_COLOUR = { .r = 0.3, .g = 0.3, .b = 0.3, .a = 1.0 };

// DEBUG VARIABLES
int DEBUG_VIEW_COLLIDERS = false;

// MISC VARIABLES
const int BORDER_LENGTH  = 360;
const int BORDER_PADDING =  12;


typedef struct SHIP
{
  double x, y, r, dx, dy;
  double thrust;
  double rot_speed;
  double fire_delay;
  int lives;

  ALLEGRO_COLOR colour;

  ALLEGRO_VERTEX transformed_v[4];
} SHIP;

SHIP ships[2];

typedef struct BULLET
{
  double x, y, r, dx, dy, max_time, timer;
  bool used;

  ALLEGRO_VERTEX transformed_v[4];
} BULLET;

BULLET bullets[BULLETS_COUNT];

typedef struct ASTEROID
{
  double x, y, r, dx, dy, dr;

  ALLEGRO_VERTEX template_v[ASTEROID_VERTICES_COUNT];
  ALLEGRO_VERTEX transformed_v[ASTEROID_VERTICES_COUNT];
} ASTEROID;

ASTEROID asteroids[ASTEROIDS_COUNT];

// 2D Rotation Helper Function
ALLEGRO_VERTEX rotate2D();

int rand_int(int lo, int hi)
{
  return lo + (rand() % (hi - lo));
}

float rand_double(double lo, double hi)
{
  return lo + ((double)rand() / (double)RAND_MAX) * (hi - lo);
}

bool circular_collision(double x_0, double y_0, double x_1, double y_1)
{
  if (x_0 == x_1 && y_0 == y_1)
    return false;
  if ( sqrt(pow((x_0 - x_1), 2) + pow((y_0 - y_1), 2)) <= 7.0)
    return true;
  return false;
}

void must_init(bool test, const char *description)
{
  if(test) return;

  printf("Couldn't initialise %s\n", description);
  exit(1);
}

void display_init()
{
  al_set_new_window_title("Space Wars II");
  display = al_create_display(DISPLAY_WIDTH, DISPLAY_HEIGHT);
  must_init(display, "display");

  buffer = al_create_bitmap(BUFFER_WIDTH, BUFFER_HEIGHT);
  must_init(buffer, "bitmap buffer");
}

void display_deinit()
{
  al_destroy_bitmap(buffer);
  al_destroy_display(display);
}

void display_pre_draw()
{
  al_set_target_bitmap(buffer);
}

void display_post_draw()
{
  al_set_target_backbuffer(display);
  al_draw_scaled_bitmap(buffer, 0, 0, BUFFER_WIDTH, BUFFER_HEIGHT, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, 0);

  al_flip_display();
}

void audio_init()
{
  must_init(al_install_audio(), "audio");
  must_init(al_init_acodec_addon(), "audio codecs");
  must_init(al_reserve_samples(128), "reserve samples");

  sample_fire = al_load_sample("fire.flac");
  must_init(sample_fire, "fire sample");
}

void audio_deinit()
{
  al_destroy_sample(sample_fire);
}

void keyboard_init()
{
  memset(key, 0, sizeof(key));
}

void gui_init()
{ 
  font = al_create_builtin_font();
  must_init(font, "font");
}

void gui_deinit()
{
  al_destroy_font(font);
}

void ship_init()
{
  ships[0].x = 2*BORDER_PADDING ; ships[0].y = BORDER_LENGTH/2 ; ships[0].r = 0;
  ships[0].dx = 0 ; ships[0].dy = 0;
  ships[0].thrust = 0.1;
  ships[0].rot_speed = 0.1;
  ships[0].fire_delay = 0.0;
  ships[0].lives = 3;
  ships[0].colour = P1_COLOUR;

  ships[1].x = BORDER_LENGTH ; ships[1].y = BORDER_LENGTH/2 ; ships[1].r = ALLEGRO_PI;
  ships[1].dx = 0 ; ships[1].dy = 0;
  ships[1].thrust = 0.1;
  ships[1].rot_speed = 0.1;
  ships[1].fire_delay = 0.0;
  ships[1].lives = 3;
  ships[1].colour = P2_COLOUR;

  ship_v[0] = (ALLEGRO_VERTEX) { .x =  8, .y =  0, .z = 0, .color = DEBUG_COLLISION_COLOUR };
  ship_v[1] = (ALLEGRO_VERTEX) { .x = -6, .y = -4, .z = 0, .color = DEBUG_COLLISION_COLOUR };
  ship_v[2] = (ALLEGRO_VERTEX) { .x = -8, .y =  0, .z = 0, .color = DEBUG_COLLISION_COLOUR };
  ship_v[3] = (ALLEGRO_VERTEX) { .x = -6, .y =  4, .z = 0, .color = DEBUG_COLLISION_COLOUR };

  memcpy(ships[0].transformed_v, ship_v, sizeof(ALLEGRO_VERTEX) * ARRAY_SIZE(ship_v));
  for (int i = 0; i < ARRAY_SIZE(ships[0].transformed_v); i++)
  {
    ships[0].transformed_v[i].color = P1_COLOUR;
  }

  memcpy(ships[1].transformed_v, ship_v, sizeof(ALLEGRO_VERTEX) * ARRAY_SIZE(ship_v));
  for (int i = 0; i < ARRAY_SIZE(ships[1].transformed_v); i++)
  {
    ships[1].transformed_v[i].color = P2_COLOUR;
  }
}

void bullets_init()
{
  bullet_v[0] = (ALLEGRO_VERTEX) { .x =  3, .y =  0, .z = 0, .color = ERROR_COLOUR };
  bullet_v[1] = (ALLEGRO_VERTEX) { .x = -3, .y = -2, .z = 0, .color = ERROR_COLOUR };
  bullet_v[2] = (ALLEGRO_VERTEX) { .x = -1, .y =  0, .z = 0, .color = ERROR_COLOUR };
  bullet_v[3] = (ALLEGRO_VERTEX) { .x = -3, .y =  2, .z = 0, .color = ERROR_COLOUR };

  for (int i = 0; i < BULLETS_COUNT; i++)
  {
    bullets[i].used = false;
    bullets[i].max_time = 19.0;
    bullets[i].timer = 0.0;

    memcpy(bullets[i].transformed_v, bullet_v, sizeof(ALLEGRO_VERTEX) * ARRAY_SIZE(bullet_v));
  }
}

void asteroids_init()
{
  for (int i = 0; i < ASTEROIDS_COUNT; i++)
  {
    asteroids[i].x = rand_double(BORDER_PADDING*2, BORDER_LENGTH);
    asteroids[i].y = rand_double(BORDER_PADDING*2, BORDER_LENGTH);
    asteroids[i].r = 0;

    asteroids[i].dx = rand_double(-1.0, 1.0);
    asteroids[i].dy = rand_double(-1.0, 1.0);
    asteroids[i].dr = rand_double(-0.1, 0.1);

    // Creating the asteroid's unique vertex array
    for (int j = 0; j < ASTEROID_VERTICES_COUNT; j++)
    {
      double start_x = 0.0 ; double start_y = rand_double(3.0, 12.0);

      double rot_x = start_x*cos(j * (ALLEGRO_PI/3)) - start_y*sin(j * (ALLEGRO_PI/3));
      double rot_y = start_x*sin(j * (ALLEGRO_PI/3)) + start_y*cos(j * (ALLEGRO_PI/3));

      asteroids[i].template_v[j] = 
        (ALLEGRO_VERTEX) { .x = rot_x, .y = rot_y, .z = 0, .color = ERROR_COLOUR };

      double final_x = rot_x + asteroids[i].x ; double final_y = rot_y + asteroids[i].y;

      asteroids[i].transformed_v[j] = 
        (ALLEGRO_VERTEX) { .x = final_x, .y = final_y, .z = 0, .color = ASTEROID_COLOUR };
    }
  }
}


void bullets_add(SHIP* ship)
{
  for (int i = 0; i < BULLETS_COUNT; i++)
  {
    if (bullets[i].used == true)
    {
      continue;
    }

    bullets[i].used = true;
    bullets[i].timer = 0.0;
    bullets[i].x = ship->transformed_v[0].x ; bullets[i].y = ship->transformed_v[0].y;
    bullets[i].r = ship->r;
    bullets[i].dx = ship->dx + 5.0*cos(ship->r) ; bullets[i].dy = ship->dy + 5.0*sin(ship->r);

    for (int j = 0; j < ARRAY_SIZE(bullet_v); j++)
    {
      bullets[i].transformed_v[j].color = ship->transformed_v[0].color;
    }

    ship->dx -= 5.0 * 0.1 * cos(ship->r) ; ship->dy -= 5.0 * 0.1 * sin(ship->r);
    break;
  }

  al_play_sample(sample_fire, 
      0.6,
      0.0,
      1.5,
      ALLEGRO_PLAYMODE_ONCE,
      NULL
      );
}

bool bullet_collision(double x, double y)
{
  for (int i = 0; i < BULLETS_COUNT; i++)
  {
    if (bullets[i].used == false)
    {
      continue;
    }

    if (circular_collision(bullets[i].x, bullets[i].y, x, y))
      return true;
  }

  return false;
}

bool asteroid_collision(double x, double y)
{
  for (int i = 0; i < ASTEROIDS_COUNT; i++)
  {
    if (circular_collision(asteroids[i].x, asteroids[i].y, x, y))
      return true;
  }

  return false;
}


void keyboard_update(ALLEGRO_EVENT* event)
{
  switch(event->type)
  {
    case ALLEGRO_EVENT_TIMER:
      for (int i = 0; i < ALLEGRO_KEY_MAX; i++)
        key[i] &= KEY_SEEN;
      break;

    case ALLEGRO_EVENT_KEY_DOWN:
      key[event->keyboard.keycode] = KEY_SEEN | KEY_RELEASED;
      break;
    case ALLEGRO_EVENT_KEY_UP:
      key[event->keyboard.keycode] &= KEY_RELEASED;
      break;
  }
}

void gui_update()
{
  ;
}

void input_update()
{
  if (key[ALLEGRO_KEY_A])
  {
    ships[0].r -= ships[0].rot_speed;
  }
  if (key[ALLEGRO_KEY_D])
  {
    ships[0].r += ships[0].rot_speed;
  }
  if (key[ALLEGRO_KEY_W])
  {
    ships[0].dx += cos(ships[0].r) * ships[0].thrust;
    ships[0].dy += sin(ships[0].r) * ships[0].thrust;
  }
  if (key[ALLEGRO_KEY_SPACE])
  {
    if (ships[0].fire_delay <= 0)
    {
      bullets_add(&ships[0]);
      ships[0].fire_delay = 3.0;
    }
  }

  if (key[ALLEGRO_KEY_LEFT])
  {
    ships[1].r -= ships[1].rot_speed;
  }
  if (key[ALLEGRO_KEY_RIGHT])
  {
    ships[1].r += ships[1].rot_speed;
  }
  if (key[ALLEGRO_KEY_UP])
  {
    ships[1].dx += cos(ships[1].r) * ships[1].thrust;
    ships[1].dy += sin(ships[1].r) * ships[1].thrust;
  }
  if (key[ALLEGRO_KEY_FULLSTOP])
  {
    if (ships[1].fire_delay <= 0)
    {
      bullets_add(&ships[1]);
      ships[1].fire_delay = 3.0;
    }
  }
}

void ship_update(SHIP* ship)
{
  if (ship->lives <= 0)
  {
    return;
  }

  if (bullet_collision(ship->x, ship->y) ||
      asteroid_collision(ship->x, ship->y))
  {
    ship->lives -= 1;

    ship->x = rand_int(BORDER_PADDING, BORDER_LENGTH);
    ship->y = rand_int(BORDER_PADDING, BORDER_PADDING + BORDER_LENGTH);

    ship->dx = 0; ship->dy = 0;
  }

  ship->x += ship->dx ; ship->y += ship->dy;

  if (ship->r < 0)
    ship->r = 2*ALLEGRO_PI - ship->r;
  else if(ship->r > 2*ALLEGRO_PI)
    ship->r = 0 + (ship->r - 2*ALLEGRO_PI);

  if (ship->x <= BORDER_PADDING)
    ship->x =  BORDER_LENGTH + BORDER_PADDING - 1;
  if (ship->x >= BORDER_LENGTH + BORDER_PADDING)
    ship->x =  BORDER_PADDING + 1;

  if (ship->y <= BORDER_PADDING)
    ship->y =  BORDER_LENGTH + BORDER_PADDING - 1;
  if (ship->y >= BORDER_LENGTH + BORDER_PADDING)
    ship->y =  BORDER_PADDING + 1;

  ship->fire_delay -= 0.1;


  for (int i = 0; i < ARRAY_SIZE(ship_v); i++)
  {
    ship->transformed_v[i].x = ship_v[i].x*cos(ship->r) - ship_v[i].y*sin(ship->r);
    ship->transformed_v[i].y = ship_v[i].x*sin(ship->r) + ship_v[i].y*cos(ship->r);
  }

  for (int i = 0; i < ARRAY_SIZE(ship_v); i++)
  {
    ship->transformed_v[i].x += ship->x ; ship->transformed_v[i].y += ship->y;
  }
}

void bullets_update()
{
  for (int i = 0; i < BULLETS_COUNT; i++)
  {
    if (bullets[i].used == false)
    {
      continue;
    }

    bullets[i].x += bullets[i].dx; bullets[i].y += bullets[i].dy;

    if (bullets[i].x <= BORDER_PADDING)
      bullets[i].x =  BORDER_LENGTH + BORDER_PADDING - 1;
    if (bullets[i].x >= BORDER_LENGTH + BORDER_PADDING)
      bullets[i].x =  BORDER_PADDING + 1;

    if (bullets[i].y <= BORDER_PADDING)
      bullets[i].y =  BORDER_LENGTH + BORDER_PADDING - 1;
    if (bullets[i].y >= BORDER_LENGTH + BORDER_PADDING)
      bullets[i].y =  BORDER_PADDING + 1;

    bullets[i].timer += 0.1;

    if (bullets[i].timer >= bullets[i].max_time)
    {
      bullets[i].used = false;
      continue;
    }

    if (bullet_collision(bullets[i].x, bullets[i].y) || 
        asteroid_collision(bullets[i].x, bullets[i].y))
    {
      bullets[i].used = false;
      continue;
    }


    for (int j = 0; j < ARRAY_SIZE(bullet_v); j++)
    {
      bullets[i].transformed_v[j].x = bullet_v[j].x*cos(bullets[i].r) - bullet_v[j].y*sin(bullets[i].r);
      bullets[i].transformed_v[j].y = bullet_v[j].x*sin(bullets[i].r) + bullet_v[j].y*cos(bullets[i].r);
    }

    for (int j = 0; j < ARRAY_SIZE(bullet_v); j++)
    {
      bullets[i].transformed_v[j].x += bullets[i].x ; bullets[i].transformed_v[j].y += bullets[i].y;
    }
  }
}

void asteroids_update()
{
  for (int i = 0; i < ASTEROIDS_COUNT; i++)
  {
    asteroids[i].x += asteroids[i].dx; asteroids[i].y += asteroids[i].dy ; asteroids[i].r += asteroids[i].dr;

    if (asteroids[i].x <= BORDER_PADDING)
      asteroids[i].x =  BORDER_LENGTH + BORDER_PADDING - 1;
    if (asteroids[i].x >= BORDER_LENGTH + BORDER_PADDING)
      asteroids[i].x =  BORDER_PADDING + 1;

    if (asteroids[i].y <= BORDER_PADDING)
      asteroids[i].y =  BORDER_LENGTH + BORDER_PADDING - 1;
    if (asteroids[i].y >= BORDER_LENGTH + BORDER_PADDING)
      asteroids[i].y =  BORDER_PADDING + 1;

    if (bullet_collision(asteroids[i].x, asteroids[i].y) ||
        asteroid_collision(asteroids[i].x, asteroids[i].y))
    {
      ;
    }


    for (int j = 0; j < ASTEROID_VERTICES_COUNT; j++)
    {
      asteroids[i].transformed_v[j].x = asteroids[i].template_v[j].x*cos(asteroids[i].r) - 
        asteroids[i].template_v[j].y*sin(asteroids[i].r);
      asteroids[i].transformed_v[j].y = asteroids[i].template_v[j].x*sin(asteroids[i].r) + 
        asteroids[i].template_v[j].y*cos(asteroids[i].r);
    }

    for (int j = 0; j < ASTEROID_VERTICES_COUNT; j++)
    {
      asteroids[i].transformed_v[j].x += asteroids[i].x ; asteroids[i].transformed_v[j].y += asteroids[i].y;
    }
  }
}

bool game_end_update()
{
  if (ships[0].lives <= 0)
  {
    al_draw_text(font, 
        al_map_rgb(255, 0, 0), 
        BORDER_PADDING + BORDER_LENGTH/2, BORDER_PADDING + BORDER_LENGTH/2, ALLEGRO_ALIGN_CENTRE, 
        "P2 WINS!");
    return true;
  }

  if (ships[1].lives <= 0)
  {
    al_draw_text(font, 
        al_map_rgb(0, 0, 255), 
        BORDER_PADDING + BORDER_LENGTH/2, BORDER_PADDING + BORDER_LENGTH/2, ALLEGRO_ALIGN_CENTRE, 
        "P1 WINS!");
    return true;
  }

  return false;
}


void gui_draw()
{
  al_draw_textf(font, 
      al_map_rgb(255, 255, 255), 
      BORDER_LENGTH + BORDER_PADDING*2, 20, ALLEGRO_ALIGN_LEFT, 
      "P1 LIVES: %d", ships[0].lives);

  al_draw_textf(font, 
      al_map_rgb(255, 255, 255), 
      BORDER_LENGTH + BORDER_PADDING*2, 40, ALLEGRO_ALIGN_LEFT, 
      "P2 LIVES: %d", ships[1].lives);
}

void ship_draw()
{
  if (ships[0].lives > 0)
  {
    al_draw_prim(ships[0].transformed_v, NULL, NULL, 0, ARRAY_SIZE(ship_v), ALLEGRO_PRIM_LINE_LOOP);
  }

  if (ships[1].lives > 0)
  {
    al_draw_prim(ships[1].transformed_v, NULL, NULL, 0, ARRAY_SIZE(ship_v), ALLEGRO_PRIM_LINE_LOOP);
  }
}

void bullets_draw()
{
  for (int i = 0; i < BULLETS_COUNT; i++)
  {
    if (bullets[i].used == false)
    {
      continue;
    }

    al_draw_prim(bullets[i].transformed_v, NULL, NULL, 0, ARRAY_SIZE(bullet_v), ALLEGRO_PRIM_LINE_LOOP);
  }
}

void asteroids_draw()
{
  for (int i = 0; i < ASTEROIDS_COUNT; i++)
  {
    al_draw_prim(asteroids[i].transformed_v, NULL, NULL, 0, ASTEROID_VERTICES_COUNT, ALLEGRO_PRIM_LINE_LOOP);
  }
}

void border_draw()
{
  double x_0 = BORDER_PADDING			, y_0 = BORDER_PADDING;
  double x_1 = BORDER_LENGTH + BORDER_PADDING	, y_1 = BORDER_LENGTH + BORDER_PADDING;

  al_draw_rectangle(x_0, y_0, x_1, y_1, al_map_rgb_f(1, 1, 1), 1);
}



int main(int argc, char *argv[])
{
  must_init(al_init(), "Allegro");

  ALLEGRO_TIMER* timer = al_create_timer(1.0 / 30.0);
  must_init(timer, "timer");

  ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
  must_init(queue, "queue");

  must_init(al_install_keyboard(), "keyboard");

  must_init(al_init_primitives_addon(), "primitives");

  must_init(al_init_image_addon(), "images");

  // Temporary place for debug inits
  if (argc > 1)
  {
    for (int i = 1; i < argc; i++)
    {
      int opt = getopt(argc, argv, "d");
      if (opt == -1)
      {
        break;
      }

      switch (opt)
      {
        case 'd':
          DEBUG_VIEW_COLLIDERS++;
          break;
        default:
          return 1;
      }
    }
  }

  display_init();
  audio_init();
  gui_init();

  al_register_event_source(queue, al_get_keyboard_event_source());
  al_register_event_source(queue, al_get_display_event_source(display));
  al_register_event_source(queue, al_get_timer_event_source(timer));

  srand(time(NULL));

  ship_init();
  bullets_init();
  asteroids_init();

  bool done = false;
  bool redraw = true;
  ALLEGRO_EVENT event;

  al_start_timer(timer);
  while(1)
  {
    al_wait_for_event(queue, &event);

    switch(event.type)
    {
      case ALLEGRO_EVENT_TIMER:
        input_update();
        ship_update(&ships[0]);
        ship_update(&ships[1]);
        bullets_update();
        asteroids_update();
        gui_update();

        if (key[ALLEGRO_KEY_ESCAPE])
          done = true;

        redraw = true;
        break;

      case ALLEGRO_EVENT_DISPLAY_CLOSE:
        done = true;
        break;
    }

    if (done)
      break;

    keyboard_update(&event);

    if(redraw && al_is_event_queue_empty(queue))
    {
      display_pre_draw();

      al_clear_to_color(al_map_rgb(0, 0, 0));

      ship_draw();
      bullets_draw();
      asteroids_draw();

      border_draw();

      gui_draw();

      done = game_end_update();

      display_post_draw();
      redraw = false;

      if(done)
      {
        sleep(3);
        break;
      }
    }
  }

  display_deinit();
  audio_deinit();
  gui_deinit();

  al_destroy_timer(timer);
  al_destroy_event_queue(queue);

  return 0;
}
