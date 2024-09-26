#include "linear.h"

float v_norm(const vector_t* vec)
{
	return vec->x * vec->x + vec->y * vec->y + vec->z * vec->z;
}

void v_normalize(vector_t* vec)
{
	float norm = v_norm(vec);

	vec->x /= norm;
	vec->y /= norm;
	vec->z /= norm;
}

vector_t sv_mul(const float s, const vector_t* vec)
{
	return (vector_t){
		s * vec->x,
		s * vec->y,
		s * vec->z
	};
}

vector_t vv_add(const vector_t* vec1, const vector_t* vec2)
{
	return (vector_t){
		vec1->x + vec2->x,
		vec1->y + vec2->y,
		vec1->z + vec2->z
	};
}

vector_t vv_sub(const vector_t* vec1, const vector_t* vec2)
{
	return (vector_t){
		vec1->x - vec2->x,
		vec1->y - vec2->y,
		vec1->z - vec2->z
	};
}

float vv_dot(const vector_t* vec1, const vector_t* vec2)
{
	return vec1->x * vec2->x + vec1->y * vec2->y + vec1->z * vec2->z;
}

vector_t vv_cross(const vector_t* vec1, const vector_t* vec2)
{
	return (vector_t){
		vec1->y * vec2->z - vec1->z * vec2->y,
		vec1->z * vec2->x - vec1->x * vec2->z,
		vec1->x * vec2->y - vec1->y * vec2->x
	};
}

vector_t vv_proj(const vector_t* vec1, const vector_t* vec2)
{
	return sv_mul(vv_dot(vec1, vec2) / (v_norm(vec2) * v_norm(vec2)), vec2);
}

void sm_mul(const float s, matrix_t* mx)
{
	mx->vx = sv_mul(s, &mx->vx);
	mx->vy = sv_mul(s, &mx->vy);
	mx->vz = sv_mul(s, &mx->vz);
}

vector_t vm_mul(const vector_t* vec, const matrix_t* mx)
{
	return (vector_t){
		vec->x * mx->vx.x + vec->x * mx->vx.y + vec->x * mx->vx.z,
		vec->y * mx->vy.x + vec->y * mx->vy.y + vec->y * mx->vy.z,
		vec->z * mx->vz.x + vec->z * mx->vz.y + vec->z * mx->vz.z
	};
}

void m_transpose(matrix_t* mx)
{
	float temp = mx->vx.y;
	mx->vx.y = mx->vy.x;
	mx->vy.x = temp;

	temp = mx->vx.z;
	mx->vx.z = mx->vz.x;
	mx->vz.x = temp;

	temp = mx->vy.z;
	mx->vy.z = mx->vz.y;
	mx->vz.y = temp;
}

void m_orthonormalize(matrix_t* mx)
{
	v_normalize(&mx->vx);

	vector_t proj = vv_proj(&mx->vy, &mx->vx);
	mx->vy = vv_sub(&mx->vy, &proj);
	v_normalize(&mx->vy);

	mx->vz = vv_cross(&mx->vx, &mx->vy);
}
