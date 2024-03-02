#ifndef linear_include_file
#define linear_include_file

struct point_t
{
	unsigned int x;
	unsigned char y;
};

struct vector_t
{
	signed int x; // vector components should be Q15.8 magnitude
	signed int y;
	signed int z; 
};

struct vector_t add(struct vector_t a, struct vector_t b);
struct vector_t sub(struct vector_t a, struct vector_t b);
signed int dot(struct vector_t a, struct vector_t b);
struct vector_t cross(struct vector_t a, struct vector_t b);
struct vector_t mul(struct vector_t a, signed int b);
struct vector_t sDiv(struct vector_t a, signed int b);
struct vector_t proj(struct vector_t a, struct vector_t b);
unsigned int magnitude(struct vector_t a);
struct vector_t normalize(struct vector_t a);

struct matrix_t
{
	signed int a[9];
};

struct matrix_t Matrix(signed int a0, signed int a1, signed int a2, signed int a3, 
		signed int a4, signed int a5, signed int a6, signed int a7, signed int a8);
struct vector_t getCol(struct matrix_t a, unsigned char b);
struct vector_t getRow(struct matrix_t a, unsigned char b);

struct matrix_t sMul(struct matrix_t a, signed int b);
struct vector_t vMul(struct matrix_t a, struct vector_t b);
struct matrix_t transpose(struct matrix_t a);
struct matrix_t orthonormalize(struct matrix_t a);

#endif
