#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <math.h>

#include "vec_help.h"

#define SWIDTH 69
#define SHEIGHT 20
#define BUF_SIZE (SWIDTH + 1) * SHEIGHT + 1

#define MAX_STEPS 200
#define MIN_DIST 0.00001

char symbols[] = "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,\"^`'. ";
const float symb_size = sizeof(symbols) - 2;

typedef struct
{
	vec3 origin;
	vec3 direction;

	float sum_dist;
	int steps;
} ray;

typedef struct
{
	vec3 pos;
	vec3 direction;
	vec2 fov;
} camera;

typedef struct
{
	ivec2 pos; // in pixels
	char *obuf;
	camera *cam;
} shade_args;

void draw(char *buf)
{
	printf("\033c");
	printf("%s", buf);
}

float de(vec3 *pos)
{
	vec3 tmp = {.x = fmod(pos->x, 1.0f) - 0.5f, .y = fmod(pos->y, 1.0f) - 0.5f, .z = pos->z};
	return length3(&tmp) - 0.3; // sphere DE
}

void march_ray(ray *ray)
{
	ray->sum_dist = 0.0f;
	for (; ray->steps < MAX_STEPS; ray->steps++)
	{
		vec3 new_pos;
		vmult3v(&ray->direction, ray->sum_dist, &new_pos);
		nvadd3(&new_pos, &ray->origin);
		float d = de(&new_pos);

		if (d < MIN_DIST)
			break;

		ray->sum_dist += d;
	}
}

void *pix_shader(void *a)
{
	shade_args *args = (shade_args *)a;

	// float res = (args->pos.x + args->pos.y * SWIDTH) % (int)symb_size / symb_size;
	ray ray;

	vec2 norm_pos = {.x = args->pos.x / SWIDTH, .y = args->pos.y / SHEIGHT};

	march_ray(&ray);

	float res = ray.steps / (float)MAX_STEPS;
	int my_pix_id = args->pos.x + args->pos.y * (SWIDTH + 1);
	args->obuf[my_pix_id] = symbols[(int)(res * symb_size)];

	return NULL;
}

pthread_t tid_arr[SHEIGHT * SWIDTH];
shade_args shader_args[SHEIGHT * SWIDTH];
void render(char *buf, camera *cam)
{
	for (int y = 0; y < SHEIGHT; y++)
	{
		for (int x = 0; x < SWIDTH; x++)
		{
			shade_args *cur_arg = &shader_args[x + y * SWIDTH];

			cur_arg->pos.x = x;
			cur_arg->pos.y = y;
			cur_arg->obuf = buf;
			cur_arg->cam = cam;

			pthread_create(&tid_arr[x + y * SWIDTH], NULL, pix_shader, cur_arg);
		}
	}

	for (int i = 0; i < SWIDTH * SHEIGHT; i++) // Wait unitl all pixel get calculated
		pthread_join(tid_arr[i], NULL);
}