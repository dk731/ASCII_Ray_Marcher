#include "vec_help.h"
#include <cblas.h>
#include <math.h>

#define SERP_TRIANGLE
// #define MOD_SPHERES
// #define MAND_BULB

#ifdef MOD_SPHERES
sphere main_sphere = {.pos = {.x = 0.5, .y = 0.5, .z = 0.5}, 0.35};
double de(vec3 *pos)
{
	pos->x = fabs(fmod(pos->x, 1.0));
	// pos->y = fabs(fmod(pos->y, 1.0));
	pos->z = fabs(fmod(pos->z, 1.0));
	cblas_daxpy(3, -1.0, &main_sphere.pos.x, 1, &pos->x, 1);
	return cblas_dnrm2(3, &pos->x, 1) - main_sphere.r;
}
#elif defined(SERP_TRIANGLE)
int iterations = 10;
double scale = 20.0;
vec3 offset = VEC3_ZERO;
double de(vec3 *z)
{
	int n = 0;
	double tmp;
	double *zp = GET_PVEC(z);

	while (n < iterations)
	{
		if (z->x + z->y < 0.0)
		{
			tmp = -z->x;
			z->x = -z->y;
			z->y = tmp;
		}
		if (z->x + z->z < 0.0)
		{
			tmp = -z->x;
			z->x = -z->z;
			z->z = tmp;
		}
		if (z->y + z->z < 0.0)
		{
			tmp = -z->z;
			z->z = -z->y;
			z->y = tmp;
		}
		cblas_dscal(3, scale, zp, 1);
		cblas_daxpy(3, -(scale - 1.0), GET_VEC(offset), 1, zp, 1);
		n++;
	}
	return cblas_dnrm2(3, zp, 1) * pow(scale, (double)(-n));
}
#elif defined(MAND_BULB)
float DE(vec3 pos)
{
	vec3 z = pos;
	float dr = 1.0;
	float r = 0.0;
	for (int i = 0; i < Iterations; i++)
	{
		r = length(z);
		if (r > Bailout)
			break;

		// convert to polar coordinates
		float theta = acos(z.z / r);
		float phi = atan(z.y, z.x);
		dr = pow(r, Power - 1.0) * Power * dr + 1.0;

		// scale and rotate the point
		float zr = pow(r, Power);
		theta = theta * Power;
		phi = phi * Power;

		// convert back to cartesian coordinates
		z = zr * vec3(sin(theta) * cos(phi), sin(phi) * sin(theta), cos(theta));
		z += pos;
	}
	return 0.5 * log(r) * r / dr;
}
#else
const sphere main_sphere = {.pos = {.x = 0.0, .y = 0.0, .z = 0.0}, .r = 3.0};
double de(vec3 *pos)
{
	cblas_daxpy(3, -1.0, &(main_sphere.pos.x), 1, &(pos->x), 1);
	return fabs(cblas_dnrm2(3, &(pos->x), 1) - main_sphere.r);
}
#endif