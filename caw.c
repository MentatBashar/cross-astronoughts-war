/*

                           _______  ______  _____  _______ _______
                           |       |_____/ |     | |______ |______
                           |_____  |    \_ |_____| ______| ______|
 _______ _______ _______  ______  _____  __   _  _____  _     _  ______ _     _ _______ _______
 |_____| |______    |    |_____/ |     | | \  | |     | |     | |  ____ |_____|    |    |______
 |     | ______|    |    |    \_ |_____| |  \_| |_____| |_____| |_____| |     |    |    ______|
                                   _  _  _ _______  ______
                                   |  |  | |_____| |_____/
                                   |__|__| |     | |    \_

*/

#include "caw.h"


void flag_reader(int argc, char* argv[])
{
    int opt;

    while((opt = getopt(argc, argv, "ds:")) != -1)
    {
      switch (opt)
      {
        case 'd':
          DEBUG_VIEW_COLLIDERS++;
          DEBUG_NO_ASTEROID_COLLISION--;
          break;
        case 's':
          display_scale = strtol(optarg, (char**) NULL, 10);

          if (display_scale <= 0)
            printf("Cannot have a negative or zero-value display scale\n");

          break;
        default:
          exit(1);
      }
    }
}

void rotate2D(ALLEGRO_VERTEX* v, double r)
{
  double rot_x = v->x*cos(r) - v->y*sin(r);
  double rot_y = v->x*sin(r) + v->y*cos(r);

  v->x = rot_x ; v->y = rot_y;
}

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
  if (display_scale <= 0)
    display_scale= 1;

  display_width  = (BUFFER_WIDTH  * display_scale);
  display_height = (BUFFER_HEIGHT * display_scale);

  al_set_new_window_title("Cross Astronoughts War");
  display = al_create_display(display_width, display_height);
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
  al_draw_scaled_bitmap(buffer, 0, 0, BUFFER_WIDTH, BUFFER_HEIGHT, 0, 0,
                        display_width,display_height, 0);

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

void main_menu_init()
{
  josh = al_load_bitmap("josh.png");
  must_init(josh, "josh");
}

void game_init()
{
  ship_init();
  bullets_init();
  charge_init();
  asteroids_init();
  nac_boards_init();
}

void ship_init()
{
  ships[0].x = 2*BORDER_PADDING ; ships[0].y = BORDER_LENGTH/2 ; ships[0].r = 0;
  ships[0].dx = 0 ; ships[0].dy = 0;
  ships[0].thrust = 0.1;
  ships[0].rot_speed = 0.1;
  ships[0].fire_delay = 0.0;
  ships[0].lockout_time = 0.0;
  ships[0].colour = P1_COLOUR;
  ships[0].id = 1;

  ships[1].x = BORDER_LENGTH ; ships[1].y = BORDER_LENGTH/2 ; ships[1].r = ALLEGRO_PI;
  ships[1].dx = 0 ; ships[1].dy = 0;
  ships[1].thrust = 0.1;
  ships[1].rot_speed = 0.1;
  ships[1].fire_delay = 0.0;
  ships[1].lockout_time = 0.0;
  ships[1].colour = P2_COLOUR;
  ships[1].id = 2;

  ship_v[0] = (ALLEGRO_VERTEX) { .x =  8, .y =  0, .z = 0, .color = DEBUG_COLLIDER_COLOUR };
  ship_v[1] = (ALLEGRO_VERTEX) { .x = -6, .y = -4, .z = 0, .color = DEBUG_COLLIDER_COLOUR };
  ship_v[2] = (ALLEGRO_VERTEX) { .x = -8, .y =  0, .z = 0, .color = DEBUG_COLLIDER_COLOUR };
  ship_v[3] = (ALLEGRO_VERTEX) { .x = -6, .y =  4, .z = 0, .color = DEBUG_COLLIDER_COLOUR };

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

void charge_init()
{
  charge.x = rand_int(BORDER_PADDING, BORDER_PADDING + BORDER_LENGTH);
  charge.y = rand_int(BORDER_PADDING, BORDER_PADDING + BORDER_LENGTH);
  charge.dx = 0.0;
  charge.dy = 0.0;
  charge.radius = 2.5;
  charge.state = 0;
  charge.timer = 0.0;
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

    double x_weighting = 0;
    double y_weighting = 0;

    // Creating the asteroid's unique vertex array
    for (int j = 0; j < ASTEROID_VERTICES_COUNT; j++)
    {
      double start_x = 0.0 ; double start_y = rand_double(3.0, 12.0);

      asteroids[i].template_v[j] = 
        (ALLEGRO_VERTEX) { .x = start_x, .y = start_y, .z = 0, .color = ERROR_COLOUR };

      rotate2D(&asteroids[i].template_v[j], (j * (ALLEGRO_PI/3)));

      x_weighting += asteroids[i].template_v[j].x;
      y_weighting += asteroids[i].template_v[j].y;

      double final_x = asteroids[i].template_v[j].x + asteroids[i].x;
      double final_y = asteroids[i].template_v[j].y + asteroids[i].y;

      asteroids[i].transformed_v[j] = 
        (ALLEGRO_VERTEX) { .x = final_x, .y = final_y, .z = 0, .color = ASTEROID_COLOUR };
    }

    for (int j = 0; j < ASTEROID_VERTICES_COUNT; j++)
    {
      asteroids[i].template_v[j].x -= x_weighting / ASTEROID_VERTICES_COUNT;
      asteroids[i].template_v[j].y -= y_weighting / ASTEROID_VERTICES_COUNT;
    }
  }
}

void nac_boards_init()
{
  u_nac_board.x_0 = 160;
  u_nac_board.y_0 = 160;
  u_nac_board.length = 480;
  u_nac_board.winner = 0;
  u_nac_board.padding = 20;

  active_grid = NULL;

  for (int i = 0; i < 3; i++)
  {
    for (int j = 0; j < 3; j++)
    {
      nac_boards[i][j].length = 120;
      nac_boards[i][j].winner = 0;
      nac_boards[i][j].x_0 = u_nac_board.x_0 + (nac_boards[i][j].length + 2*u_nac_board.padding) * i;
      nac_boards[i][j].y_0 = u_nac_board.y_0 + (nac_boards[i][j].length + 2*u_nac_board.padding) * j;
      nac_boards[i][j].padding = 20;
      for (int k = 0; k < 3; k++)
      {
        for (int l = 0; l < 3; l++)
        {
          nac_boards[i][j].cells[k][l].state = 0;
          nac_boards[i][j].cells[k][l].length = nac_boards[i][j].length / 3;

          nac_boards[i][j].cells[k][l].x_0 = nac_boards[i][j].x_0 +
                                             nac_boards[i][j].padding + 
                                             nac_boards[i][j].cells[k][l].length 
                                             * k;

          nac_boards[i][j].cells[k][l].y_0 = nac_boards[i][j].y_0 + 
                                             nac_boards[i][j].padding + 
                                             nac_boards[i][j].cells[k][l].length 
                                             * l;
        }
      }
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

void charge_set(SHIP* ship)
{
  charge.x = ship->x;
  charge.y = ship->y;

  charge.state = 0;
  charge.last_touch = ship->id;

  charge.timer = 0.0;

  charge.dx = ship->dx + 5.0*cos(ship->r);
  charge.dy = ship->dy + 5.0*sin(ship->r);
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

bool charge_collision(double x, double y)
{
  if (charge.state == 0 && charge.timer > 3.0)
  {
    if(circular_collision(charge.x, charge.y, x, y)) //Within boundaries
    {
      return true;
    }
  }

  return false;
}

bool asteroid_collision(double x, double y)
{
  for (int i = 0; i < ASTEROIDS_COUNT; i++)
  {
    if (circular_collision(asteroids[i].x, asteroids[i].y, x, y)
        && DEBUG_NO_ASTEROID_COLLISION)
      return true;
  }

  return false;
}


bool within_nac_board(double x, double y, int mark)
{
    for (int i = 2; i >= 0; i--)
    {
        for (int j = 2; j >= 0; j--)
        {
            if (x > nac_boards[i][j].x_0 &&
                x < nac_boards[i][j].x_0 + nac_boards[i][j].length + u_nac_board.padding*2 &&
                y > nac_boards[i][j].y_0 &&
                y < nac_boards[i][j].y_0 + nac_boards[i][j].length + u_nac_board.padding*2 &&
                nac_boards[i][j].winner == 0 &&
                (&nac_boards[i][j] == active_grid || active_grid == NULL))
            {
              return within_cell(x, y, i, j, mark);
            }
        }
    }
    return false;
}

bool within_cell(double x, double y, int i, int j, int mark)
{
    for (int k = 2; k >= 0; k--)
    {
        for (int l = 2; l >= 0; l--)
        {
            if (x > nac_boards[i][j].cells[k][l].x_0 &&
                x < nac_boards[i][j].cells[k][l].x_0 + nac_boards[i][j].cells[k][l].length &&
                y > nac_boards[i][j].cells[k][l].y_0 &&
                y < nac_boards[i][j].cells[k][l].y_0 + nac_boards[i][j].cells[k][l].length)
            {
                if (nac_boards[i][j].cells[k][l].state == 0)
                {
                    // Set cell's mark to be charge's last owner
                    nac_boards[i][j].cells[k][l].state = mark;

                    nac_boards[i][j].marks++;

                    // Set new active grid
                    if (nac_boards[k][l].marks >= 9)
                    {
                      active_grid = NULL;
                    }
                    else
                    {
                      active_grid = &nac_boards[k][l];
                    }

                    check_nac_board(i, j, mark);
                    return true;
                }
            }
        }
    }
    return false;
}

void check_nac_board(int i, int j, int mark)
{
  for (int l = 0; l < 3; l++)
  {
    int win = 0;
    for (int k = 0; k < 3; k++)
    {
      if(nac_boards[i][j].cells[k][l].state == mark)
      {
        win++;
      }
    }
    if (win == 3)
    {
      nac_boards[i][j].winner = mark;
      return;
    }
  }

  for (int l = 0; l < 3; l++)
  {
    int win = 0;
    for (int k = 0; k < 3; k++)
    {
      if(nac_boards[i][j].cells[k][l].state == mark)
        win++;
    }
    if (win == 3)
    {
      nac_boards[i][j].winner = mark;
      return;
    }
  }
  if (nac_boards[i][j].cells[0][0].state == mark &&
  nac_boards[i][j].cells[1][1].state == mark &&
  nac_boards[i][j].cells[2][2].state == mark)
  {
    nac_boards[i][j].winner = mark;
    return;
  }
  if (nac_boards[i][j].cells[2][0].state == mark &&
  nac_boards[i][j].cells[1][1].state == mark &&
  nac_boards[i][j].cells[0][2].state == mark)
  {
    nac_boards[i][j].winner = mark;
    return;
  }
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


void main_menu_update()
{
  if (key[ALLEGRO_KEY_ENTER])
  {
    current_screen = GAME;
  }
}


void game_update()
{
  input_update();
  ship_update(&ships[0]);
  ship_update(&ships[1]);
  bullets_update();
  charge_update();
  asteroids_update(); 
}


void input_update()
{
  if (ships[0].lockout_time <= 0.0)
  {
    if (key[ALLEGRO_KEY_A])
      ships[0].r -= ships[0].rot_speed;
    if (key[ALLEGRO_KEY_D])
      ships[0].r += ships[0].rot_speed;

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

    if (key[ALLEGRO_KEY_Z])
    {
      if (charge.state == 1)
        charge_set(&ships[0]);
    }
  }

  if (ships[1].lockout_time <= 0.0)
  {
    if (key[ALLEGRO_KEY_LEFT])
      ships[1].r -= ships[1].rot_speed;
    if (key[ALLEGRO_KEY_RIGHT])
      ships[1].r += ships[1].rot_speed;

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

    if (key[ALLEGRO_KEY_M])
    {
      if (charge.state == 2)
        charge_set(&ships[1]);
    }
  }
}

void ship_update(SHIP* ship)
{
  // If ship's controls are locked out, reduce timer
  if (ship->lockout_time > 0.0)
  {
    ship->lockout_time -= 0.1;

    ship->r += ship->dr;

    // If the ship has just been unlocked, do these things
    if (ship->lockout_time <= 0.0)
      ship->dr = 0.0;
  }
  else
  {
    // Check if ship collided
    if (bullet_collision(ship->x, ship->y) ||
        asteroid_collision(ship->x, ship->y))
    {
      ship->lockout_time = 9.0;

      ship->dr = rand_double(-0.3, 0.3);

      if (charge.state == ship->id)
      {
        charge.x = ship->x;
        charge.y = ship->y; 

        charge.state = 0;
        charge.timer = 0.0;
      }
    }

    // Check if ship has picked up the charge
    if (charge_collision(ship->x, ship->y))
    {
      charge.state = ship->id;
    }
  }

  ship->x += ship->dx ; ship->y += ship->dy;

  // Wrapping polar cords
  if (ship->r < 0)
    ship->r = 2*ALLEGRO_PI - ship->r;
  else if(ship->r > 2*ALLEGRO_PI)
    ship->r = 0 + (ship->r - 2*ALLEGRO_PI);

  // Horizontal screen wrap
  if (ship->x <= BORDER_PADDING)
    ship->x =  BORDER_LENGTH + BORDER_PADDING - 1;
  if (ship->x >= BORDER_LENGTH + BORDER_PADDING)
    ship->x =  BORDER_PADDING + 1;

  // Vertical screen wrap
  if (ship->y <= BORDER_PADDING)
    ship->y =  BORDER_LENGTH + BORDER_PADDING - 1;
  if (ship->y >= BORDER_LENGTH + BORDER_PADDING)
    ship->y =  BORDER_PADDING + 1;

  ship->fire_delay -= 0.1;

  // Move ship's transform
  for (int i = 0; i < ARRAY_SIZE(ship_v); i++)
  {
    ship->transformed_v[i].x = ship_v[i].x;
    ship->transformed_v[i].y = ship_v[i].y;

    rotate2D(&ship->transformed_v[i], ship->r);

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
      bullets[i].transformed_v[j].x = bullet_v[j].x;
      bullets[i].transformed_v[j].y = bullet_v[j].y;

      rotate2D(&bullets[i].transformed_v[j], bullets[i].r);

      bullets[i].transformed_v[j].x += bullets[i].x ; bullets[i].transformed_v[j].y += bullets[i].y;
    }
  }
}

void charge_update()
{
  charge.timer += 0.1;
  charge.x += charge.dx;
  charge.y += charge.dy;

  if (charge.dx < 0.0)
    charge.dx += 0.05;
  else
    charge.dx -= 0.05;

  if (charge.dy < 0.0)
    charge.dy += 0.05;
  else
    charge.dy -= 0.05;

  if (charge.dx <= 0.05 && charge.dx >= -0.05)
    charge.dx = 0;

  if (charge.dy <= 0.05 && charge.dy >= -0.05)
    charge.dy = 0;

  if (charge.dx == 0 && charge.dy == 0 && charge.last_touch != 0)
  {
    if(within_nac_board(charge.x, charge.y, charge.last_touch))
    {
        charge_init();
    }
    charge.last_touch = 0;
  }

  if (charge.x <= BORDER_PADDING)
    charge.x =  BORDER_LENGTH + BORDER_PADDING - 1;
  if (charge.x >= BORDER_LENGTH + BORDER_PADDING)
    charge.x =  BORDER_PADDING + 1;

  if (charge.y <= BORDER_PADDING)
    charge.y =  BORDER_LENGTH + BORDER_PADDING - 1;
  if (charge.y >= BORDER_LENGTH + BORDER_PADDING)
    charge.y =  BORDER_PADDING + 1;
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
      asteroids[i].transformed_v[j].x = asteroids[i].template_v[j].x;
      asteroids[i].transformed_v[j].y = asteroids[i].template_v[j].y;

      rotate2D(&asteroids[i].transformed_v[j], asteroids[i].r);

      asteroids[i].transformed_v[j].x += asteroids[i].x ; asteroids[i].transformed_v[j].y += asteroids[i].y;
    }
  }
}

bool game_end_update()
{
  return false;
}


void main_menu_draw()
{
  al_draw_bitmap(josh, 0, 0, 0);
}


void game_draw()
{
  nac_boards_draw();
  nac_boards_mark();

  ship_draw();
  bullets_draw();
  asteroids_draw();
  charge_draw();

  border_draw();
}


void nac_boards_draw()
{
  // Draw u_nac_board
  al_draw_line(u_nac_board.x_0 + u_nac_board.length/3,
      u_nac_board.y_0,
      u_nac_board.x_0 + u_nac_board.length/3,
      u_nac_board.y_0 + u_nac_board.length,
      U_NAC_BOARD_COLOUR, 1);

  al_draw_line(u_nac_board.x_0 + 2*u_nac_board.length/3, 
      u_nac_board.y_0,
      u_nac_board.x_0 + 2*u_nac_board.length/3,
      u_nac_board.y_0 + u_nac_board.length,
      U_NAC_BOARD_COLOUR, 1);

  al_draw_line(u_nac_board.x_0,
      u_nac_board.y_0 + u_nac_board.length/3,
      u_nac_board.x_0 + u_nac_board.length, 
      u_nac_board.y_0 + u_nac_board.length/3,
      U_NAC_BOARD_COLOUR, 1);

  al_draw_line(u_nac_board.x_0,
      u_nac_board.y_0 + 2*u_nac_board.length/3,
      u_nac_board.x_0 + u_nac_board.length,
      u_nac_board.y_0 + 2*u_nac_board.length/3,
      U_NAC_BOARD_COLOUR, 1);

  // Draw nac_boards
  for (int i = 0; i < 3; i++)
  {
    for (int j = 0; j < 3; j++)
    {
      al_draw_line(nac_boards[i][j].x_0 + u_nac_board.padding + nac_boards[i][j].length/3,
          nac_boards[i][j].y_0 + u_nac_board.padding,
          nac_boards[i][j].x_0 + u_nac_board.padding + nac_boards[i][j].length/3,
          nac_boards[i][j].y_0 + u_nac_board.padding + nac_boards[i][j].length,
          NAC_BOARD_COLOUR, 1);

      al_draw_line(nac_boards[i][j].x_0 + u_nac_board.padding + 2*nac_boards[i][j].length/3,
          nac_boards[i][j].y_0 + u_nac_board.padding,
          nac_boards[i][j].x_0 + u_nac_board.padding + 2*nac_boards[i][j].length/3,
          nac_boards[i][j].y_0 + u_nac_board.padding + nac_boards[i][j].length,
          NAC_BOARD_COLOUR, 1);

      al_draw_line(nac_boards[i][j].x_0 + u_nac_board.padding,
          nac_boards[i][j].y_0 + u_nac_board.padding + nac_boards[i][j].length/3,
          nac_boards[i][j].x_0 + u_nac_board.padding + nac_boards[i][j].length,
          nac_boards[i][j].y_0 + u_nac_board.padding + nac_boards[i][j].length/3,
          NAC_BOARD_COLOUR, 1);

      al_draw_line(nac_boards[i][j].x_0 + u_nac_board.padding,
          nac_boards[i][j].y_0 + u_nac_board.padding + 2*nac_boards[i][j].length/3,
          nac_boards[i][j].x_0 + u_nac_board.padding + nac_boards[i][j].length,
          nac_boards[i][j].y_0 + u_nac_board.padding + 2*nac_boards[i][j].length/3,
          NAC_BOARD_COLOUR, 1);
    }
  }

  // Draw square around active grid
  if (active_grid == NULL)
  {
    // Draw square around whole board
    double x_0 = u_nac_board.x_0 - u_nac_board.padding/2;
    double y_0 = u_nac_board.y_0 - u_nac_board.padding/2;
    double x_1 = x_0 + u_nac_board.length + u_nac_board.padding;
    double y_1 = y_0 + u_nac_board.length + u_nac_board.padding;

    al_draw_rectangle(x_0, y_0, x_1, y_1, al_map_rgb_f(0, 1, 0), 1);
  }
  else
  {
    // Draw square around selected nac grid
    double x_0 = active_grid->x_0 + u_nac_board.padding/2; 
    double y_0 = active_grid->y_0 + u_nac_board.padding/2;
    double x_1 = x_0 + active_grid->length + u_nac_board.padding;
    double y_1 = y_0 + active_grid->length + u_nac_board.padding;

    al_draw_rectangle(x_0, y_0, x_1, y_1, al_map_rgb_f(0, 1, 0), 1);
  }
}

void nac_boards_mark()
{
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if (nac_boards[i][j].winner == 1)
            {
              big_o_draw(nac_boards[i][j].x_0, nac_boards[i][j].y_0);
            }
            else if (nac_boards[i][j].winner == 2)
            {
              big_x_draw(nac_boards[i][j].x_0, nac_boards[i][j].y_0);
            }
            for (int k = 0; k < 3; k++)
            {
                for (int l = 0; l < 3; l++)
                {
                    if (nac_boards[i][j].cells[k][l].state == 1 && nac_boards[i][j].winner == 0)
                    {
                        o_draw(nac_boards[i][j].cells[k][l].x_0, nac_boards[i][j].cells[k][l].y_0);
                    }
                    else if (nac_boards[i][j].cells[k][l].state == 2 && nac_boards[i][j].winner == 0)
                    {
                        x_draw(nac_boards[i][j].cells[k][l].x_0, nac_boards[i][j].cells[k][l].y_0);
                    }
                }
            }
        }
    }
}

void x_draw(double x_0, double y_0)
{
  x_0 += 10;
  y_0 += 10;
  double x_1 = x_0 + 20;
  double y_1 = y_0 + 20;
  al_draw_line(x_0, y_0, x_1, y_1, al_map_rgb_f(1, 0, 0), 1);

  x_0 += 20;
  x_1 -= 20;
  al_draw_line(x_0, y_0, x_1, y_1, al_map_rgb_f(1, 0, 0), 1);
}

void big_x_draw(double x_0, double y_0)
{
  x_0 += 10 * 3;
  y_0 += 10 * 3;
  double x_1 = x_0 + 20 * 5;
  double y_1 = y_0 + 20 * 5;
  al_draw_line(x_0, y_0, x_1, y_1, al_map_rgb_f(1, 0, 0), 2.5);

  x_0 += 20 * 5;
  x_1 -= 20 * 5;
  al_draw_line(x_0, y_0, x_1, y_1, al_map_rgb_f(1, 0, 0), 2.5);
}

void o_draw(double x_0, double y_0)
{
  x_0 += 10;
  y_0 += 10;
  double r = 10;
  x_0 += r;
  y_0 += r;
  al_draw_circle(x_0, y_0, r, al_map_rgb_f(0, 0, 1), 1);
}

void big_o_draw(double x_0, double y_0)
{
  x_0 += 10 * 3;
  y_0 += 10 * 3;
  double r = 10 * 5;
  x_0 += r;
  y_0 += r;
  al_draw_circle(x_0, y_0, r, al_map_rgb_f(0, 0, 1), 2.5);
}


void ship_draw()
{
  al_draw_prim(ships[0].transformed_v, NULL, NULL, 0, ARRAY_SIZE(ship_v), ALLEGRO_PRIM_LINE_LOOP);

  al_draw_prim(ships[1].transformed_v, NULL, NULL, 0, ARRAY_SIZE(ship_v), ALLEGRO_PRIM_LINE_LOOP);
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

void charge_draw()
{
  if (charge.state == 0)
  {
    al_draw_filled_circle(charge.x, charge.y, charge.radius, al_map_rgb_f(1, 0, 1));
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
  double x_0 = BORDER_PADDING                 , y_0 = BORDER_PADDING;
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

  flag_reader(argc, argv);

  display_init();
  audio_init();

  al_register_event_source(queue, al_get_keyboard_event_source());
  al_register_event_source(queue, al_get_display_event_source(display));
  al_register_event_source(queue, al_get_timer_event_source(timer));

  srand(time(NULL));

  main_menu_init();
  game_init(); 

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
        switch(current_screen)
        {
          case MAIN_MENU:
            main_menu_update();
            break;

          case GAME:
            game_update();
            break;
        }

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

      switch(current_screen)
      {
        case MAIN_MENU:
          main_menu_draw();
          break;

        case GAME:
          game_draw();
          break;
      }

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

  al_destroy_timer(timer);
  al_destroy_event_queue(queue);

  return 0;
}
