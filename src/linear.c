#include <stdlib.h>
#include "linear.h"
#include "intmath.h"

struct vector_t add(struct vector_t a, struct vector_t b)
{
	struct vector_t newVector;
	newVector.x = a.x + b.x;
	newVector.y = a.y + b.y;
	newVector.z = a.z + b.z;
	return newVector;
}

struct vector_t sub(struct vector_t a, struct vector_t b)
{
	struct vector_t newVector;
	newVector.x = a.x - b.x;
	newVector.y = a.y - b.y;
	newVector.z = a.z - b.z;
	return newVector;
}

signed int dot(struct vector_t a, struct vector_t b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

struct vector_t cross(struct vector_t a, struct vector_t b)
{
	struct vector_t newVector;
	newVector.x = (a.y * b.z - a.z * b.y) / 256;
	newVector.y = (a.z * b.x - a.x * b.z) / 256;
	newVector.z = (a.x * b.y - a.y * b.x) / 256;
	return newVector;
}

struct vector_t mul(struct vector_t a, signed int b)
{
	struct vector_t newVector;
	newVector.x = a.x * b;
	newVector.y = a.y * b;
	newVector.z = a.z * b;
	return newVector;
}

struct vector_t sDiv(struct vector_t a, signed int b)
{
	struct vector_t newVector;
	newVector.x = a.x / b;
	newVector.y = a.y / b;
	newVector.z = a.z / b;
	return newVector;
}

struct vector_t proj(struct vector_t a, struct vector_t b)
{
	return mul(b, dot(a, b) / dot (b, b));
}

#include <debug.h>

unsigned int magnitude(struct vector_t a)
{
	dbg_printf("magnitude of (%d, %d, %d): ", a.x, a.y, a.z);

	// prepare everything to be modified
	unsigned int x = a.x < 0 ? -1 * a.x : a.x;
	unsigned int y = a.y < 0 ? -1 * a.y : a.y;
	unsigned int z = a.z < 0 ? -1 * a.z : a.z;
	unsigned char scale = 0;

	// scale down to avoid overflows
	while (x > 0x03ff || y > 0x03ff || z > 0x03ff)
	{
		x >>= 1;
		y >>= 1;
		z >>= 1;
		scale++;
	}

	unsigned int mag = intsqrt(x * x + y * y + z * z);

	// scale back up to compensate
	while (scale > 0)
	{
		mag <<= 1;
		scale--;
	}

	dbg_printf("%u\n", mag);

	return mag;
}

struct vector_t normalize(struct vector_t a)
{
	dbg_printf("normalizing (%d, %d, %d)...\n", a.x, a.y, a.z);

	struct vector_t newVector;
	signed int const mag = magnitude(a) & 0x7fffff;

	unsigned char scale1 = 8;
	unsigned char scale2 = 0;
	unsigned int scalingTrialInt = intabs(a.x) | intabs(a.y) | intabs(a.z);
	while (scalingTrialInt > 0x7fff)
	{
		scalingTrialInt >>= 1;
		scale1 -= 1;
		scale2 += 1;
	}

	newVector.x = (a.x << scale1) / mag << scale2;
	newVector.y = (a.y << scale1) / mag << scale2;
	newVector.z = (a.z << scale1) / mag << scale2;

	dbg_printf("normalized: (%d, %d, %d)\n", newVector.x, newVector.y, newVector.z);

	return newVector;
}

struct matrix_t Matrix(signed int a0, signed int a1, signed int a2, signed int a3, 
		signed int a4, signed int a5, signed int a6, signed int a7, signed int a8)
{
	struct matrix_t newMatrix;
	newMatrix.a[0] = a0;
	newMatrix.a[1] = a1;
	newMatrix.a[2] = a2;
	newMatrix.a[3] = a3;
	newMatrix.a[4] = a4;
	newMatrix.a[5] = a5;
	newMatrix.a[6] = a6;
	newMatrix.a[7] = a7;
	newMatrix.a[8] = a8;
	return newMatrix;
}

struct vector_t getCol(struct matrix_t a, unsigned char b)
{
	struct vector_t newVector;
	newVector.x = a.a[0 + b];
	newVector.y = a.a[3 + b];
	newVector.z = a.a[6 + b];
	return newVector;
}

struct vector_t getRow(struct matrix_t a, unsigned char b)
{
	struct vector_t newVector;
	newVector.x = a.a[3 * b + 0];
	newVector.y = a.a[3 * b + 1];
	newVector.z = a.a[3 * b + 2];
	return newVector;
}

struct matrix_t sMul(struct matrix_t a, signed int b)
{
	struct matrix_t newMatrix;
	for (unsigned char i = 0; i < 9; i++) newMatrix.a[i] = a.a[i] * b;
	return newMatrix;
}

struct vector_t vMul(struct matrix_t a, struct vector_t b)
{
	struct vector_t newVector;
	newVector.x = (a.a[0] * b.x + a.a[1] * b.y + a.a[2] * b.z) / 256;
	newVector.y = (a.a[3] * b.x + a.a[4] * b.y + a.a[5] * b.z) / 256;
	newVector.z = (a.a[6] * b.x + a.a[7] * b.y + a.a[8] * b.z) / 256;
	return newVector;
}

struct matrix_t transpose(struct matrix_t a)
{
	struct matrix_t newMatrix;
	newMatrix.a[0] = a.a[0];
	newMatrix.a[1] = a.a[3];
	newMatrix.a[2] = a.a[6];
	newMatrix.a[3] = a.a[1];
	newMatrix.a[4] = a.a[4];
	newMatrix.a[5] = a.a[7];
	newMatrix.a[6] = a.a[2];
	newMatrix.a[7] = a.a[5];
	newMatrix.a[8] = a.a[8];
	return newMatrix;
}

struct matrix_t orthonormalize(struct matrix_t a)
{
	// modified from original algorithm
	struct vector_t u1 = getRow(a, 2);
	u1 = normalize(u1); // done with u1!

	struct vector_t u2 = getRow(a, 1);
	u2 = sub(u2, proj(u1, u2)); // ig this just isn't very efficient on the 6502...
	u2 = normalize(u2);         // whatever. i'm doing it here, bc i can

	// now to save a bunch of time
	struct vector_t u3 = cross(u1, u2); // magnitude already equals 1 x 1 = 1... that was easy

	struct matrix_t output = Matrix(u3.x, u3.y, u3.z, u2.x, u2.y, u2.z, u1.x, u1.y, u1.z);

	return output;
}
