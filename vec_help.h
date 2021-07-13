

#ifndef VECH_PROT
#define VECH_PROT

#define VEC3_SIZE 24
#define VEC2_SIZE 16

#define PI 3.141592653589793238
#define D2RAD 0.017453292519943295

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

#define VEC3(v1, v2, v3)          \
	{                             \
		.x = v1, .y = v2, .z = v3 \
	}

#define VEC2(xx, yy)     \
	{                    \
		.x = xx, .y = yy \
	}

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

#define GET_VEC(vec) &(vec.x)

#define VEC3_DLEN(v1, v2, resv, tmp)                       \
	MCOPY_VEC3(tmp, v1);                                   \
	cblas_daxpy(3, -1.0, GET_VEC(v2), 1, GET_VEC(tmp), 1); \
	resv = cblas_dnrm2(3, GET_VEC(tmp), 1)

#define MCOPY_VEC3(v1, v2) memcpy(GET_VEC(v1), GET_VEC(v2), VEC3_SIZE)
#define MCOPY_VEC2(v1, v2) memcpy(GET_VEC(v1), GET_VEC(v2), VEC2_SIZE)

#endif