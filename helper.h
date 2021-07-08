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
#include <errno.h>

#include "vec_help.h"
#include "de_list.h"

#define ADD_BUF_SIZE 2

#define MAX_STEPS 200
#define MAX_DIST 1000000.0
#define MIN_DIST 0.0001
#define MAX_INP_LEN 1024
#define MAX_CALLBACK_LEN 100

// #define MAX_THREADS_COUNT 16384
#define MAX_THREADS_COUNT 1024

#define DEBUG_PRINT

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

struct buffer;

typedef struct
{
	ivec2 pos; // in pixels
	struct buffer *obuf;
	camera *cam;
} shade_args;

typedef struct buffer
{
	char *data;
	ivec2 size;

	int rw;
	long last_draw;
	float dtime;

	pthread_t *tid_arr; // alloc only once at buffer initialization
	shade_args *shader_args;
} buffer;

typedef struct
{
	char *pattern;
	void (*func)(char);
} callback;

typedef struct
{
	callback *callback_list[MAX_CALLBACK_LEN];
	int top;
} inp_callbacks;

typedef struct
{
	char inp_chars[MAX_INP_LEN];
	int top;
} inp_str;

char symbols[] = "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,\"^`'. ";
const double symb_size = sizeof(symbols) - 2;

pthread_mutex_t stdout_lock;
pthread_mutex_t inp_lock;
pthread_mutex_t waiting_char;
pthread_mutex_t pause_draw;
inp_str inp;
inp_callbacks inp_calls;
int active_threads_amount = 0;

int64_t millis()
{
	struct timespec now;
	timespec_get(&now, TIME_UTC);
	return ((int64_t)now.tv_sec) * 1000 + ((int64_t)now.tv_nsec) / 1000000;
}

#ifndef DEBUG_PRINT
void draw(buffer *buf)
#else
void draw(buffer *buf, camera *cam)
#endif
{
	pthread_mutex_lock(&pause_draw);
	pthread_mutex_lock(&stdout_lock);
	printf("\033c");
	printf("%s", buf->data);
#ifdef DEBUG_PRINT
	int64_t cur_time = millis();
	buf->dtime = (cur_time - buf->last_draw) / 1000.0f;
	buf->last_draw = millis();
	printf("\r\nFPS: %.2f | Pos: %.2f, %.2f, %.2f | Rot: %.2f, %.2f, %.2f | Dir: %.2f, %.2f, %2.f | Inp Buf: %d\r\n",
		   1.0f / buf->dtime,
		   cam->pos.x,
		   cam->pos.y,
		   cam->pos.z,
		   cam->direction_ang.x * 57.2958,
		   cam->direction_ang.y * 57.2958,
		   cam->direction_ang.z * 57.2958,
		   cam->direction_vec.x,
		   cam->direction_vec.y,
		   cam->direction_vec.z,
		   inp.top + 1);
#endif
	pthread_mutex_unlock(&stdout_lock);
	pthread_mutex_unlock(&pause_draw);
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
	active_threads_amount++;
	shade_args *args = (shade_args *)a;

	// double res = (args-16384pos.x + args->pos.y * SWIDTH) % (int)symb_size / symb_size;
	ray pray;
	pray.origin = args->cam->pos;

	vec2 my_ang = {.x = (args->pos.x / (double)args->obuf->size.x - 0.5) * args->cam->fov.x, .y = (args->pos.y / (double)args->obuf->size.y - 0.5) * args->cam->fov.y};

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
	int my_pix_id = args->pos.x + args->pos.y * args->obuf->rw;
	args->obuf->data[my_pix_id] = symbols[(int)(res * symb_size)];

	active_threads_amount--;
	return NULL;
}

void render(buffer *buf, camera *cam) // before calling render, camera should contain updated direction vector
{
	int y, x;
	for (y = 0; y < buf->size.y; y++)
	{
		for (x = 0; x < buf->size.x; x++)
		{
			int cur_id = x + y * buf->size.x;
			shade_args *cur_arg = &buf->shader_args[cur_id];

			cur_arg->pos.x = x;
			cur_arg->pos.y = y;
			cur_arg->obuf = buf;
			cur_arg->cam = cam;

			while (active_threads_amount > MAX_THREADS_COUNT)
				usleep(100);

			pthread_create(&buf->tid_arr[cur_id], NULL, pix_shader, cur_arg);
			pthread_detach(buf->tid_arr[cur_id]);
		}
	}

	while (!active_threads_amount)
		usleep(100);

	// for (int i = 0; i < buf->size.x * buf->size.y; i++) // Wait unitl all pixel get calculated
	// 	pthread_join(buf->tid_arr[i], NULL);
}

void *input_listener()
{
	char c;
	char dc;
	while (1)
	{
		pthread_mutex_lock(&waiting_char);
		c = getc(stdin);
		pthread_mutex_unlock(&waiting_char);

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

		for (int i = 0; i <= inp_calls.top; i++)
			for (size_t j = 0; j < strlen(inp_calls.callback_list[i]->pattern); j++)
				if (dc == inp_calls.callback_list[i]->pattern[j])
					inp_calls.callback_list[i]->func(dc);

		pthread_mutex_lock(&inp_lock); // add key press to stack
		if (inp.top >= MAX_INP_LEN)	   // roll top id
			inp.top = 0;
		inp.inp_chars[++inp.top] = dc;
		pthread_mutex_unlock(&inp_lock);
	}
}

void init_lib()
{
	pthread_mutex_init(&stdout_lock, NULL);
	pthread_mutex_init(&inp_lock, NULL);
	pthread_mutex_init(&waiting_char, NULL);
}

buffer *init_draw_buf(int width, int height)
{
	buffer *obuf = malloc(sizeof(buffer));
	obuf->size.x = width;
	obuf->size.y = height;
	obuf->rw = width + ADD_BUF_SIZE;
	int buf_size = obuf->rw * height + 1;
	obuf->data = malloc(buf_size);
	memset(obuf->data, 0, buf_size);
	for (int y = 0; y < height; y++)
	{
		int ind = obuf->size.x + y * obuf->rw;
		obuf->data[ind++] = '\r';
		obuf->data[ind] = '\n';
	}
	obuf->last_draw = millis();

	obuf->tid_arr = malloc(sizeof(pthread_t) * width * height);
	obuf->shader_args = malloc(sizeof(shade_args) * width * height);

	return obuf;
}
void clear_buf(buffer *buf)
{
	free(buf->data);
	free(buf->tid_arr);
	free(buf->shader_args);

	free(buf);
}

void high_res_screenshot(buffer *buf, camera *cam) // creates one image from current cammera view in higher resolution
{
	pthread_mutex_lock(&pause_draw);
	printf("\r\nPlease Enter desired image with and height: ");
	int w, h;

	scanf("%d %d", &w, &h);

	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	char file_name[200];

	sprintf(file_name, "./screenshots/%02d-%02d_%02d:%02d:%02d_%dx%d.txt", tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, w, h);

	buffer *tmp_buf = init_draw_buf(w, h);
	camera tmp_cam = {.direction_vec = COPY_VEC3(cam->direction_vec),
					  .direction_ang = COPY_VEC3(cam->direction_ang),
					  .pos = COPY_VEC3(cam->pos),
					  .fov = VEC2(cam->fov.x, (cam->fov.x * buf->size.y) / buf->size.x * 2.0)};

	render(tmp_buf, &tmp_cam);

	FILE *ofile = fopen(file_name, "w");
	fputs(tmp_buf->data, ofile);

	fclose(ofile);

	clear_buf(tmp_buf);

	pthread_mutex_unlock(&pause_draw);
}

void init_input()
{
	initscr();
	memset(inp_calls.callback_list, 0, sizeof(callback) * MAX_CALLBACK_LEN);
	memset(inp.inp_chars, 0, MAX_INP_LEN);

	inp_calls.top = -1;
	inp.top = -1;

	pthread_t inpt_id;
	pthread_create(&inpt_id, NULL, input_listener, NULL);
}

void add_callback(callback *callb)
{
	inp_calls.callback_list[++inp_calls.top] = callb;
}

void clear_lib()
{
	endwin();
}