typedef struct
{
	float x;
	float y;
} vec2;

typedef struct
{
	float x;
	float y;
	float z;
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

void vadd3(vec3 *v1, vec3 *v2, vec3 *vo)
{
	vo->x = v1->x + v2->x;
	vo->y = v1->y + v2->y;
	vo->z = v1->z + v2->z;
}

void vadd2(vec2 *v1, vec2 *v2, vec2 *vo)
{
	vo->x = v1->x + v2->x;
	vo->y = v1->y + v2->y;
}

void vmult3v(vec3 *v1, float val, vec3 *vo)
{
	vo->x = v1->x * val;
	vo->y = v1->y * val;
	vo->z = v1->z * val;
}

void vmult2v(vec2 *v1, float val, vec2 *vo)
{
	vo->x = v1->x * val;
	vo->y = v1->y * val;
}

void vmult3(vec3 *v1, vec3 *v2, vec3 *vo)
{
	vo->x = v1->x * v2->x;
	vo->y = v1->y * v2->y;
	vo->z = v1->z * v2->z;
}

void vmult2(vec2 *v1, vec2 *v2, vec2 *vo)
{
	vo->x = v1->x * v2->x;
	vo->y = v1->y * v2->y;
}

void nvadd3(vec3 *v1, vec3 *v2)
{
	v1->x += v2->x;
	v1->y += v2->y;
	v1->z += v2->z;
}

void nvadd2(vec2 *v1, vec2 *v2)
{
	v1->x += v2->x;
	v1->y += v2->y;
}

void nvmult3v(vec3 *v1, float val)
{
	v1->x *= val;
	v1->y *= val;
	v1->z *= val;
}

void nvmult2v(vec2 *v1, float val)
{
	v1->x *= val;
	v1->y *= val;
}

void nvmult3(vec3 *v1, vec3 *v2)
{
	v1->x *= v2->x;
	v1->y *= v2->y;
	v1->z *= v2->z;
}

void nvmult2(vec2 *v1, vec2 *v2)
{
	v1->x *= v2->x;
	v1->y *= v2->y;
}

void vdiv3v(vec3 *v1, float val, vec3 *vo)
{
	vo->x = v1->x / val;
	vo->y = v1->y / val;
	vo->z = v1->z / val;
}

void vdiv2v(vec2 *v1, float val, vec2 *vo)
{
	vo->x = v1->x / val;
	vo->y = v1->y / val;
}

void vdiv3(vec3 *v1, vec3 *v2, vec3 *vo)
{
	vo->x = v1->x / v2->x;
	vo->y = v1->y / v2->y;
	vo->z = v1->z / v2->z;
}

void vdiv2(vec2 *v1, vec2 *v2, vec2 *vo)
{
	vo->x = v1->x / v2->x;
	vo->y = v1->y / v2->y;
}

void nvdiv3v(vec3 *v1, float val)
{
	v1->x /= val;
	v1->y /= val;
	v1->z /= val;
}

void nvdiv2v(vec2 *v1, float val)
{
	v1->x /= val;
	v1->y /= val;
}

void nvdiv3(vec3 *v1, vec3 *v2)
{
	v1->x /= v2->x;
	v1->y /= v2->y;
	v1->z /= v2->z;
}

void nvdiv2(vec2 *v1, vec2 *v2)
{
	v1->x /= v2->x;
	v1->y /= v2->y;
}

//////////////////

float length3(vec3 *vec)
{
	return vec->x * vec->x + vec->y * vec->y + vec->z * vec->z;
}

float length2(vec2 *vec)
{
	return vec->x * vec->x + vec->y * vec->y;
}