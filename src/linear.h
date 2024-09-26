#ifndef LINEAR_H
#define LINEAR_H

#include <float.h>

typedef struct {
	float x;
	float y;
	float z;
} vector_t;

typedef struct {
	vector_t vx;
	vector_t vy;
	vector_t vz;
} matrix_t;

float v_norm(const vector_t* vec);
void v_normalize(vector_t* vec);

vector_t sv_mul(const float s, const vector_t* vec);

vector_t vv_sum(const vector_t* vec1, const vector_t* vec2);
float vv_dot(const vector_t* vec1, const vector_t* vec2);
vector_t vv_cross(const vector_t* vec1, const vector_t* vec2);
vector_t vv_proj(const vector_t* vec1, const vector_t* vec2);

void sm_mul(const float s, matrix_t* mx);
vector_t vm_mul(const vector_t* vec, const matrix_t* mx);

void m_transpose(matrix_t* mx);
void m_orthonormalize(matrix_t* mx);

#endif
