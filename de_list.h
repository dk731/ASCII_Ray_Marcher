#include "vec_help.h"
#include <cblas.h>
#include <math.h>

// #define SERP_TRIANGLE
#define MOD_SPHERES
// #define MAND_BULB

#ifdef MOD_SPHERES
sphere main_sphere = {.pos = {.x = 0.5, .y = 0.5, .z = 0.5}, 0.01};
double de(vec3 *pos)
{
	pos->x = fabs(fmod(pos->x, 1.0));
	pos->y = fabs(fmod(pos->y, 1.0));
	pos->z = fabs(fmod(pos->z, 1.0));
	cblas_daxpy(3, -1.0, &main_sphere.pos.x, 1, &pos->x, 1);
	return cblas_dnrm2(3, &pos->x, 1) - main_sphere.r;
}
#elif defined(SERP_TRIANGLE)
int itterations = 1;
#define SCALE 2.0
double de(vec3 *zz)
{
	vec3 a1 = VEC3(1.0, 1.0, 1.0);
	vec3 a2 = VEC3(-1.0, -1.0, 1.0);
	vec3 a3 = VEC3(1.0, -1.0, -1.0);
	vec3 a4 = VEC3(-1.0, 1.0, -1.0);
	vec3 z = COPY_VEC3((*zz));
	int n = 0;
	float dist, d;
	while (n < itterations)
	{
		vec3 c = COPY_VEC3(a1);
		vec3 tmp_res;

		VEC3_DLEN(z, a1, dist, tmp_res); // dist = length(z - a1)

		VEC3_DLEN(z, a2, d, tmp_res);
		if (d < dist)
		{
			MCOPY_VEC3(c, a2);
			dist = d;
		}

		VEC3_DLEN(z, a3, d, tmp_res);
		if (d < dist)
		{
			MCOPY_VEC3(c, a3);
			dist = d;
		}

		VEC3_DLEN(z, a4, d, tmp_res);
		if (d < dist)
		{
			MCOPY_VEC3(c, a4);
			dist = d;
		}

		cblas_dscal(3, (SCALE - 1.0), GET_VEC(c), 1);
		cblas_daxpy(3, -1.0, GET_VEC(c), 1, GET_VEC(z), 1);
		// z = SCALE * z - c * (SCALE - 1.0);
		n++;
	}

	return cblas_dnrm2(3, GET_VEC(z), 1) * pow(SCALE, (double)-n);
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