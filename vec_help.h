

#ifndef VECH_PROT
#define VECH_PROT

typedef struct
{
	double x;
	double y;
} vec2;

typedef struct
{
	double x;
	double y;
	double z;
} vec3;

typedef struct
{
	int x;
	int y;
} ivec2;

typedef struct
{
	int x;
	int y;
	int z;
} ivec3;

typedef struct
{
	vec3 pos;
	double r;
} sphere;

#define SET_VEC3_FORWARD(vec) \
	vec.x = 1.0;              \
	vec.y = 0.0;              \
	vec.z = 0.0

#define VEC3_ZERO                    \
	{                                \
		.x = 0.0, .y = 0.0, .z = 0.0 \
	}

#define VEC3_FORWARD                 \
	{                                \
		.x = 1.0, .y = 0.0, .z = 0.0 \
	}

#define VEC3 \
	(xx, yy, zz) { .x = xx, .y = yy, .z = zz }

#define COPY_VEC3(vec)                     \
	{                                      \
		.x = vec.x, .y = vec.y, .z = vec.z \
	}

#define COPY_VEC2(vec)         \
	{                          \
		.x = vec.x, .y = vec.y \
	}

#define SET_VEC3(vec, xx, yy, zz) \
	vec.x = xx;                   \
	vec.y = yy;                   \
	vec.z = zz

#define SET_VEC2(vec, xx, yy) \
	vec.x = xx;               \
	vec.y = yy

#define PI 3.141592653589793238
#define D2RAD 0.017453292519943295

#endif