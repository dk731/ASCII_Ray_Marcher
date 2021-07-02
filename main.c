#include "helper.h"

char screen_buf[BUF_SIZE];

float hor_fow = 45.0f;

int main()
{
  memset(screen_buf, 0, BUF_SIZE);
  for (int y = 0; y < SHEIGHT; y++)
    screen_buf[SWIDTH + y * (SWIDTH + 1)] = '\n';

  camera main_camera = {.direction = {.x = 0.0f, .y = 0.0f, .z = 0.0f},
                        .fov = {.x = hor_fow, .y = SHEIGHT / SWIDTH * hor_fow},
                        .pos = {.x = 0.0f, .y = 0.0f, .z = 0.0f}};

  while (1)
  {
    render(screen_buf, &main_camera);
    draw(screen_buf);
  }

  getchar();
  // std::cout << "\033[H" << symb_size;
}