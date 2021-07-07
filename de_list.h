#include "vec_help.h"
#include <cblas.h>
#include <math.h>

// #define SERP_TRIANGLE
// #define MOD_SPHERES

#ifdef MOD_SPHERES
const sphere main_sphere = {.pos = {.x = 0.0, .y = 0.0, .z = 0.0}, 0.05};
double de(vec3 *pos)
{
	pos->x = fmod(pos->x, 1.0);
	pos->y = fmod(pos->y, 1.0);
	pos->z = fmod(pos->z, 1.0);
	cblas_daxpy(3, -1.0, &main_sphere.pos.x, 1, &pos->x, 1);
	return cblas_dnrm2(3, &pos->x, 1) - main_sphere.r;
}
#elif defined(SERP_TRIANGLE)
#define ITERATIONS 2
#define SCALE 2.0
float de(vec3 *pos)
{
	float r;
	int n = 0;
	while (n < ITERATIONS)
	{
		if (pos->x + pos->y < 0)
		{
			pos->x = -pos->x;
			pos->y = -pos->y;
		}
		if (pos->x + pos->z < 0)
		{
			pos->x = -pos->x;
			pos->z = -pos->z;
		}
		if (pos->y + pos->y < 0)
		{
			pos->y = -pos->y;
			pos->y = -pos->y;
		}
		cblas_dscal(3, SCALE, &(pos->x), 1);
		n++;
	}
	return cblas_dnrm2(3, &(pos->x), 1) * pow(SCALE, -(double)n);
}
#else
const sphere main_sphere = {.pos = {.x = 20.0, .y = 0.0, .z = 0.0}, .r = 1.0};
double de(vec3 *pos)
{
	cblas_daxpy(3, -1.0, &main_sphere.pos.x, 1, &(pos->x), 1);
	return cblas_dnrm2(3, &(pos->x), 1) - main_sphere.r;
}
#endif