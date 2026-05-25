#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#define WIDTH 66
#define HEIGHT 22

// Systems
int GAMEOVER = 0;
int WIN = 0;
int APPLE_EXIST = 0;
char buffer[WIDTH * HEIGHT];

// Characters
char SPACE = '.';
char WALLS = '#';
char HEAD = 'S';
char BODY = 'O';
char APPLE = '@';

// Snake variables
int pos[WIDTH * HEIGHT][2];
int body_length = 1;
// 0 -> right
// 1 -> left
// 2 -> up
// 3 -> down
int direction = 0;

// Core functons
void draw_chr(int x, int y, char chr) { buffer[x + y * WIDTH] = chr; }
char get_chr(int x, int y) { return buffer[x + y * WIDTH]; }

void Check() {
  APPLE_EXIST = 0;
  WIN = 1;
  for (int i=0; i<WIDTH*HEIGHT; i++) {
    if (buffer[i] == APPLE) APPLE_EXIST = 1;
    if (buffer[i] == SPACE) WIN = 0;
  }
}

void draw_walls() {
  for (int i = 0; i < WIDTH; i++) {
    draw_chr(i, 0, '#');
    draw_chr(i, HEIGHT - 1, '#');
  }
  for (int i = 0; i < HEIGHT; i++) {
    draw_chr(0,i,'#');
    draw_chr(1, i, '#');
    draw_chr(WIDTH - 1, i, '#');
  }
}

void process_snake() {
  draw_chr(pos[body_length - 1][0], pos[body_length - 1][1], SPACE);
  for (int i = body_length - 1; i > 0; i--) {
    pos[i][0] = pos[i - 1][0];
    pos[i][1] = pos[i - 1][1];
  }
  switch (direction) {
  case 0:
    pos[0][0]++;
    break;
  case 1:
    pos[0][0]--;
    break;
  case 2:
    pos[0][1]--;
    break;
  case 3:
    pos[0][1]++;
    break;
  default:
    return;
  }

  char next_block = get_chr(pos[0][0], pos[0][1]);
  if ((next_block == WALLS) || (next_block == BODY)) {
    GAMEOVER = 1;
    return;
  } else if (next_block == APPLE) {
    body_length++;
    APPLE_EXIST = 0;
  }

  for (int i = 0; i < body_length; i++) {
    draw_chr(pos[i][0], pos[i][1], i == 0 ? HEAD : BODY);
  }
}

void change_direction_by_input(int key) {
  if (direction != 1 && key == 'd') {
    // Can be changed to 0
    direction = 0;
  } else if (direction != 0 && key == 'a') {
    // Can be changed to 1
    direction = 1;
  } else if (direction != 3 && key == 'w') {
    // Can be changed to 2
    direction = 2;
  } else if (direction != 2 && key == 's') {
    // Can be changed to 3
    direction = 3;
  }
}

void place_apple() {
  int x = rand() % WIDTH;
  int y = rand() % HEIGHT;
  int block = get_chr(x, y);

  if (block == SPACE) {
    draw_chr(x, y, APPLE);
    APPLE_EXIST = 1;
  } else if (WIN == 0) {
    place_apple();
  }
}

struct termios orig_termios;

void reset_terminal_mode() { tcsetattr(0, TCSANOW, &orig_termios); }

void set_conio_terminal_mode() {
  struct termios new_termios;

  /* take two copies - one for now, one for later */
  tcgetattr(0, &orig_termios);
  memcpy(&new_termios, &orig_termios, sizeof(new_termios));

  /* register cleanup handler, and set the new terminal mode */
  atexit(reset_terminal_mode);
  cfmakeraw(&new_termios);
  tcsetattr(0, TCSANOW, &new_termios);
}

int kbhit() {
  struct timeval tv = {0L, 0L};
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(0, &fds);
  return select(1, &fds, NULL, NULL, &tv) > 0;
}

int getch() {
  int r;
  unsigned char c;
  if ((r = read(0, &c, sizeof(c))) < 0) {
    return r;
  } else {
    return c;
  }
}

void message(char *msg, size_t msg_length) {
  int x_center = WIDTH / 2 - (msg_length / 2);
  int y_center = HEIGHT / 2;

  for (int i = 0; i < msg_length + 2; i++) {
    draw_chr(x_center + i - 1, y_center - 1, '-');
    draw_chr(x_center + i - 1, y_center + 1, '-');
  }
  draw_chr(x_center - 1, y_center, '|');
  draw_chr(x_center + msg_length, y_center, '|');
  for (int i = 0; i < msg_length; i++) {
    draw_chr(x_center + i, y_center, msg[i]);
  }
}

// Main function
int main() {
  srand(time(NULL));

  printf("\033[2J\033[?25l");
  memset(buffer, '.', WIDTH * HEIGHT);

  set_conio_terminal_mode();

  draw_walls();
  pos[0][0] = WIDTH / 4;
  pos[0][1] = HEIGHT / 2;

  // for (int i = 0; i < WIDTH * HEIGHT; i++) {
  //   place_apple();
  // }

  while (1) {
    if (kbhit()) {
      int key = getch();
      change_direction_by_input(key);
      if (key == 27)
        break;
    }

    process_snake();

    Check();

    if (GAMEOVER == 1) {
      message("GAME OVER", 9);
    }

    if (APPLE_EXIST == 0) {
      if (WIN == 0) {
        place_apple();
      } else {
        message("YOU WIN", 7);
        GAMEOVER = 1;
      }
    }

    printf("\033[H");
    for (int k = 1; k < WIDTH * HEIGHT; k++) {
      if (k % WIDTH == 0) {
        printf("\r\n");
      } else
        printf("%c", buffer[k]);
    }
    fflush(stdout);

    printf("\r\nSCORE: %d", body_length - 1);
    fflush(stdout);

    if (GAMEOVER == 1)
      break;
    usleep(1000 * 100);
  }

  return 0;
}
