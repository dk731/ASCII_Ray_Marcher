#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <cblas.h>
#include <ncurses.h>
#include <inttypes.h>
#include <time.h>
#include <math.h>

#include "vec_help.h"
#include "de_list.h"

#define SWIDTH 90
#define SHEIGHT 30
#define RSWIDTH (SWIDTH + 2)
#define BUF_SIZE RSWIDTH *SHEIGHT + 1

#define MAX_STEPS 200
#define MAX_DIST 1000000.0
#define MIN_DIST 0.0001
#define MAX_INP_LEN 1024

#define DEBUG_PRINT

typedef struct
{
	char inp_chars[MAX_INP_LEN];
	int top_ind;
} inp_str;

typedef struct
{
	vec3 origin;
	vec3 direction;

	double sum_dist;
	int steps;
} ray;

typedef struct
{
	vec3 pos;
	vec3 direction_ang;
	vec3 direction_vec;
	vec2 fov;

	double *trans_mat; // 3x3 matrix
} camera;

typedef struct
{
	ivec2 pos; // in pixels
	char *obuf;
	camera *cam;
} shade_args;

char symbols[] = "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,\"^`'. ";
const double symb_size = sizeof(symbols) - 2;

pthread_mutex_t stdout_lock;
pthread_mutex_t inp_lock;
inp_str inp;
int64_t last_draw_time;
int64_t time_delta;

int64_t millis()
{
	struct timespec now;
	timespec_get(&now, TIME_UTC);
	return ((int64_t)now.tv_sec) * 1000 + ((int64_t)now.tv_nsec) / 1000000;
}

void draw(char *buf)
{
	pthread_mutex_lock(&stdout_lock);
	printf("\033c");
	printf("%s", buf);
#ifdef DEBUG_PRINT
	int64_t cur_time = millis();
	time_delta = cur_time - last_draw_time;
	last_draw_time = millis();
	printf("\r\nFPS: %.2f    | input buffer len: %d\r\n", 1000.0f / (float)time_delta, inp.top_ind + 1);
#endif
	pthread_mutex_unlock(&stdout_lock);
}

void march_ray(ray *pray)
{
	pray->sum_dist = 0.0f;
	pray->steps = 0;
	for (; pray->steps < MAX_STEPS; pray->steps++)
	{
		vec3 new_pos;
		memcpy(&new_pos, &pray->direction, sizeof(double) * 3);
		cblas_dscal(3, pray->sum_dist, (double *)&new_pos, 1);
		cblas_daxpy(3, 1.0, (double *)&pray->origin, 1, (double *)&new_pos, 1);

		double d = de(&new_pos);

		if (d < MIN_DIST)
			return;
		else if (pray->sum_dist > MAX_DIST)
		{
			pray->steps = MAX_STEPS;
			return;
		}

		pray->sum_dist += d;
	}
}

void *pix_shader(void *a)
{
	shade_args *args = (shade_args *)a;

	// double res = (args->pos.x + args->pos.y * SWIDTH) % (int)symb_size / symb_size;
	ray pray;
	pray.origin = args->cam->pos;

	vec2 my_ang = {.x = (args->pos.x / (double)SWIDTH - 0.5) * args->cam->fov.x, .y = (args->pos.y / (double)SHEIGHT - 0.5) * args->cam->fov.y};

	double cam_z = args->cam->direction_ang.z;
	double *my_rot = (double[]){
		cos(my_ang.x) * cos(my_ang.y),
		cos(my_ang.x) * sin(my_ang.y) * sin(cam_z) - sin(my_ang.x) * cos(cam_z),
		cos(my_ang.x) * sin(my_ang.y) * cos(cam_z) + sin(my_ang.x) * sin(cam_z),
		sin(my_ang.x) * cos(my_ang.y),
		sin(my_ang.x) * sin(my_ang.y) * sin(cam_z) + cos(my_ang.x) * cos(cam_z),
		sin(my_ang.x) * sin(my_ang.y) * cos(cam_z) - cos(my_ang.x) * sin(cam_z),
		-sin(my_ang.y),
		cos(my_ang.y) * sin(cam_z),
		cos(my_ang.y) * cos(cam_z)};

	cblas_dgemv(CblasRowMajor, CblasNoTrans, 3, 3, 1.0, my_rot, 3, &(args->cam->direction_vec.x), 1, 0.0, &(pray.direction.x), 1);

	march_ray(&pray);

	// double res = fabs(norm_pos.x + norm_pos.y - 1.0);
	double res = (double)pray.steps / (double)MAX_STEPS;
	int my_pix_id = args->pos.x + args->pos.y * RSWIDTH;
	args->obuf[my_pix_id] = symbols[(int)(res * symb_size)];

	return NULL;
}

pthread_t tid_arr[SHEIGHT * SWIDTH];
shade_args shader_args[SHEIGHT * SWIDTH];
void render(char *buf, camera *cam) // before calling render, camera should contain updated direction vector
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

void *input_listener(void *args)
{
	void (*callback)(char) = args;
	char c;
	char dc;
	while (1)
	{
		c = getc(stdin);

		pthread_mutex_lock(&stdout_lock);
		if (c == 27 && getc(stdin) == 91)
		{
			switch (getc(stdin))
			{
			case 65: // up
				dc = 1;
				break;
			case 66: // down
				dc = 2;
				break;
			case 67: // right
				dc = 3;
				break;
			case 68: // left
				dc = 4;
				break;
			default:
				dc = 0;
			}
		}
		else
			dc = c;
		printf("%c[2K\r", 27);
		pthread_mutex_unlock(&stdout_lock);

		if (dc && callback != NULL)
			callback(dc);

		pthread_mutex_lock(&inp_lock);	// add key press to stack
		if (inp.top_ind >= MAX_INP_LEN) // roll top id
			inp.top_ind = 0;
		inp.inp_chars[++inp.top_ind] = dc;
		pthread_mutex_unlock(&inp_lock);
	}
}

void init_lib(char *screen_buf)
{
	pthread_mutex_init(&stdout_lock, NULL);
	pthread_mutex_init(&inp_lock, NULL);

	memset(screen_buf, 0, BUF_SIZE);
	for (int y = 0; y < SHEIGHT; y++)
	{
		int ind = SWIDTH + y * RSWIDTH;
		screen_buf[ind++] = '\r';
		screen_buf[ind] = '\n';
	}
	last_draw_time = millis();
}

void init_input(void (*callback)(char))
{
	initscr();
	inp.top_ind = 0;
	memset(inp.inp_chars, 0, MAX_INP_LEN);
	pthread_t inpt_id;
	pthread_create(&inpt_id, NULL, input_listener, callback);
}

void clear_lib()
{
	endwin();
}