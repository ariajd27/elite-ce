#include "ship.h"
#include "ship_data.h"

#include <graphx.h>
#include "xorgfx.h"
#include "linear.h"

#include "variables.h"

#include <debug.h>

struct Ship ships[MAX_SHIPS];
unsigned char numShips = 0;

struct Ship* NewShip(unsigned char shipType, struct vector_t position, struct matrix_t orientation)
{
	dbg_printf("creating ship type %d at index %d and position (%d, %d, %d)\n", shipType, numShips, position.x, position.y, position.z);

	ships[numShips].shipType = shipType;
	ships[numShips].position.x = position.x;
	ships[numShips].position.y = position.y;
	ships[numShips].position.z = position.z;
	for (unsigned char i = 0; i < 9; i++) ships[numShips].orientation.a[i] = orientation.a[i];

	ships[numShips].aggro = 255;

	ships[numShips].isExploding = 0;
	ships[numShips].toExplode = 0;
	ships[numShips].laserFiring = 0;

	ships[numShips].visibility = bp_header_vectors[shipType][BP_GENERAL][BP_VISDIST_IND];
	ships[numShips].numVertices = bp_header_vectors[shipType][BP_GENERAL][BP_VERT_L_IND] / BP_L_VERT;
	ships[numShips].numEdges = bp_header_vectors[shipType][BP_GENERAL][BP_EDGE_Q_IND];
	ships[numShips].numFaces = bp_header_vectors[shipType][BP_GENERAL][BP_FACE_L_IND] / BP_L_FACE;

	ships[numShips].explosionSize = 18;
	ships[numShips].explosionCount = bp_header_vectors[shipType][BP_GENERAL][BP_EXPLCT_IND];
	// ships[numShips].explosionRand not initialized

	ships[numShips].speed = 0;
	ships[numShips].acceleration = 0;
	ships[numShips].pitch = 0;
	ships[numShips].roll = 0;

	numShips++;
	return &ships[numShips - 1];
}

void RemoveShip(unsigned char shipIndex)
{
	for (unsigned char i = shipIndex; i < numShips; i++) ships[i] = ships[i + 1];
	numShips--;
}

struct int_point_t ProjPoint(struct vector_t toProject) 
{
	struct int_point_t output;

	if (toProject.x < 32768 && toProject.x > -32768 && toProject.y < 32768 && toProject.y > -32768)
	{
		output.x = (signed int)VIEW_HCENTER + (toProject.x * 256) / toProject.z;
		output.y = (signed int)VIEW_VCENTER + (toProject.y * 256) / toProject.z;
	}
	else
	{
		output.x = (signed int)VIEW_HCENTER + toProject.x / (toProject.z / 256);
		output.y = (signed int)VIEW_VCENTER + toProject.y / (toProject.z / 256);
	}

	return output;
}

void ShipAsPoint(unsigned char shipIndex)
{
	struct int_point_t screenPoint = ProjPoint(ships[shipIndex].position);

	screenPoint.x -= (signed int)SHPPT_WIDTH / 2;
	screenPoint.y -= (signed int)SHPPT_HEIGHT / 2;

	dbg_printf("drawing ship as point at (%d, %d)... ", screenPoint.x, screenPoint.y);

	xor_FillRectangle(screenPoint.x, screenPoint.y, SHPPT_WIDTH, SHPPT_HEIGHT);
}

void ShipAsWireframe(unsigned char shipIndex)
{
	dbg_printf("drawing ship as wireframe...\n");

	unsigned char distance = ships[shipIndex].position.z >> 8;
	if (distance > 31) distance = 31;

	struct matrix_t transformation = transpose(ships[shipIndex].orientation);

	// backface culling
	unsigned int faceVisible;
	if(ships[shipIndex].isExploding == 1) faceVisible = 0xffffff;
	else
	{
		faceVisible = 0x008000; // only bit 15 is true
		for (unsigned char i = 0; i < ships[shipIndex].numFaces; i++)
		{
			unsigned char* faceData = bp_header_vectors[ships[shipIndex].shipType][BP_FACES] + i * BP_L_FACE;
			unsigned char faceVisibility = faceData[0] & 0b00011111;

			if (distance > faceVisibility) faceVisible |= 1 << i;
			else
			{
				signed int const dataX = (signed int)faceData[1];
				signed int const dataY = (signed int)faceData[2];
				signed int const dataZ = (signed int)faceData[3];

				struct vector_t const faceNormal = {
					(faceData[0] & 0b10000000) != 0 ? -1 * dataX : dataX,
					(faceData[0] & 0b01000000) != 0 ? -1 * dataY : dataY,
					(faceData[0] & 0b00100000) != 0 ? dataZ : -1 * dataZ,
				};

				struct vector_t lineOfSight = 
					add(vMul(ships[shipIndex].orientation, ships[shipIndex].position), faceNormal);

				if (dot(lineOfSight, faceNormal) < 0) faceVisible |= 1 << i;
				else faceVisible &= ~(1 << i);
			}
		}
	}

	// render polygons
	struct {
		bool calculated;
		struct int_point_t screenPosition;
	} vertices[28];

	for (unsigned char i = 0; i < ships[shipIndex].numVertices; i++) vertices[i].calculated = false;

	for (unsigned char i = 0; i < ships[shipIndex].numEdges; i++)
	{
		unsigned char* edgeData = bp_header_vectors[ships[shipIndex].shipType][BP_EDGES] + i * BP_L_EDGE;

		if (distance > edgeData[0]) continue;

		bool const face1Invisible = (faceVisible & (1 << ((edgeData[1] & 0xf0) >> 4))) == 0;
		bool const face2Invisible = (faceVisible & (1 << (edgeData[1] & 0x0f))) == 0;
		if (face1Invisible && face2Invisible) continue;

		// only project vertices if not already done, otherwise reuse the earlier math
		for (unsigned char j = 2; j <= 3; j++) 
		{
			if (vertices[edgeData[j] >> 2].calculated) continue;
			
			unsigned char* vertexData = bp_header_vectors[ships[shipIndex].shipType][BP_VERTICES] 
				+ (edgeData[j] >> 2) * BP_L_VERT;

			struct vector_t vertexVector = {
				(vertexData[3] & 0b10000000) != 0 ? -1 * vertexData[0] : vertexData[0],
				(vertexData[3] & 0b01000000) != 0 ? -1 * vertexData[1] : vertexData[1],
				(vertexData[3] & 0b00100000) != 0 ? vertexData[2] : -1 * vertexData[2]
			};

			vertexVector = add(vMul(transformation, vertexVector), ships[shipIndex].position);

			vertices[edgeData[j] >> 2].screenPosition = ProjPoint(vertexVector);
			vertices[edgeData[j] >> 2].calculated = true;
		}

		// clipping is built into this (hopefully!)
		xor_Line(vertices[edgeData[2] >> 2].screenPosition.x, vertices[edgeData[2] >> 2].screenPosition.y,
				vertices[edgeData[3] >> 2].screenPosition.x, vertices[edgeData[3] >> 2].screenPosition.y);
	}
}

void ShipAsPlanet(unsigned char shipIndex)
{
	ShipAsPoint(shipIndex);

	dbg_printf("drawing as planet...\n");
}

void DrawShip(unsigned char shipIndex)
{
	dbg_printf("drawing ship at index %d...\n", shipIndex);
	dbg_printf("position: (%d, %d, %d)\n", 
			ships[shipIndex].position.x, ships[shipIndex].position.y, ships[shipIndex].position.z);

	if (ships[shipIndex].shipType > BP_ESCAPEPOD)
	{
		ShipAsPlanet(shipIndex);
		return;
	}

	if (ships[shipIndex].toExplode)
	{
		dbg_printf("beginning explosion for ship\n");

		ships[shipIndex].laserFiring = false;
		ships[shipIndex].toExplode = false;
		ships[shipIndex].isExploding = true;
		ships[shipIndex].acceleration = 0;
		ships[shipIndex].pitch = 0;

		for (unsigned char i = 0; i < 4; i++) ships[shipIndex].explosionRand[i] = rand();
	}

	// why have a bunch of "||" when we can just do this?
	if (ships[shipIndex].position.z <= 0) return;
	if ((unsigned int)ships[shipIndex].position.z >= 0xc00000) return;
	if (ships[shipIndex].position.x >= ships[shipIndex].position.z) return;
	if (ships[shipIndex].position.y >= ships[shipIndex].position.z) return;

	dbg_printf("ship is within screen area\n");

	if (ships[shipIndex].position.z / 512 > ships[shipIndex].visibility) ShipAsPoint(shipIndex);
	else ShipAsWireframe(shipIndex);
}

void DoAI(unsigned char shipIndex)
{
	if (ships[shipIndex].shipType == BP_CORIOLIS)
	{
		ships[shipIndex].roll = 127;
		return;
	}

	// target vector goes away from player by default
	struct vector_t goVector = ships[shipIndex].position;
	signed int goAlign = dot(goVector, getRow(ships[shipIndex].orientation, 2));

	// if not too close and feeling aggressive, set target towards player instead
	if (((ships[shipIndex].position.x | ships[shipIndex].position.y) & 0xfe00) == 0)
	{
		unsigned char const a = rand() % 128;
		if (a < ships[shipIndex].aggro)
		{
			goVector = mul(goVector, -1);
			goAlign *= -1;
		}
	}

	// pitch is simple
	ships[shipIndex].pitch = -3 * dot(goVector, getRow(ships[shipIndex].orientation, 1));
	dbg_printf("pitch set: %d\n", ships[shipIndex].pitch);

	// roll is not... skip roll processing if already in a roll
	if ((ships[shipIndex].roll < 0 ? -1 * ships[shipIndex].roll : ships[shipIndex].roll) < 16)
	{
		// roll clockwise if dot product sign is the same as pitch counter sign, else counter-clockwise
		if ((dot(goVector, getRow(ships[shipIndex].orientation, 0)) < 0) == (ships[shipIndex].pitch < 0))
		{
			ships[shipIndex].roll = 5;
		}
		else
		{
			ships[shipIndex].roll = -5;
		}
	}

	if (goAlign >= 22 * 256) // speed up if charging towards target
	{
		ships[shipIndex].acceleration = 3;
	}

	// don't slow down if we still need to do lots of turning
	else if (goAlign < 18 * 256 && goAlign > -18 * 256) ships[shipIndex].acceleration = 0;

	// if no return yet, slow down, missiles faster than other things
	else if (ships[shipIndex].shipType == BP_MISSILE) ships[shipIndex].acceleration = -2;
	else ships[shipIndex].acceleration = -1;
}

void RotateShip(signed int* x, signed int* y, signed char amount)
{
	signed int oldX = *x;

	// 1 - 1/512 is cosine of some tiny angle
	// of which 1 / 16 is the sine
	if (amount > 0)
	{
		*x = *x * (1 - 1 / 512) + *y / 16;
		*y = *y * (1 - 1 / 512) - oldX / 16;
	}
	else
	{
		*x = *x * (1 - 1 / 512) - *y / 16;
		*y = *y * (1 - 1 / 512) + oldX / 16;
	}
}

void MoveShip(unsigned char shipIndex)
{
	// move ship along nose vector
	struct vector_t const noseVector = getRow(ships[shipIndex].orientation, 2);
	ships[shipIndex].position = add(ships[shipIndex].position, 
			sDiv(mul(noseVector, ships[shipIndex].speed), 64));

	// apply acceleration, clamping between 0 and max speed for ship type
	if (ships[shipIndex].acceleration > 0 || ships[shipIndex].speed >= -1 * ships[shipIndex].acceleration)
	{
		ships[shipIndex].speed += ships[shipIndex].acceleration;

		unsigned char const maxSpeed = bp_header_vectors[ships[shipIndex].shipType][BP_GENERAL][BP_MAX_SPEED];
		if (ships[shipIndex].speed > maxSpeed) ships[shipIndex].speed = maxSpeed;
	}

	ships[shipIndex].acceleration = 0;

	// apply roll
	if (ships[shipIndex].roll != 0)
	{
		if (ships[shipIndex].roll > 0 && ships[shipIndex].roll < 127) ships[shipIndex].roll--;
		else if (ships[shipIndex].roll < 0 && ships[shipIndex].roll > -128) ships[shipIndex].roll++;

		RotateShip(&ships[shipIndex].orientation.a[3], 
				&ships[shipIndex].orientation.a[0], 
				ships[shipIndex].roll);
		RotateShip(&ships[shipIndex].orientation.a[4], 
				&ships[shipIndex].orientation.a[1], 
				ships[shipIndex].roll);
		RotateShip(&ships[shipIndex].orientation.a[5], 
				&ships[shipIndex].orientation.a[2], 
				ships[shipIndex].roll);
	}

	// apply pitch
	if (ships[shipIndex].pitch != 0)
	{
		if (ships[shipIndex].pitch > 0 && ships[shipIndex].pitch < 127) ships[shipIndex].pitch--;
		else if (ships[shipIndex].pitch < 0 && ships[shipIndex].pitch > -128) ships[shipIndex].pitch++;

		RotateShip(&ships[shipIndex].orientation.a[3], 
				&ships[shipIndex].orientation.a[6], 
				ships[shipIndex].pitch);
		RotateShip(&ships[shipIndex].orientation.a[4], 
				&ships[shipIndex].orientation.a[7], 
				ships[shipIndex].pitch);
		RotateShip(&ships[shipIndex].orientation.a[5], 
				&ships[shipIndex].orientation.a[8], 
				ships[shipIndex].pitch);
	}
}

void FlipAxes(enum viewDirMode_t viewDirMode)
{
	if (viewDirMode == FRONT) return;

	for (unsigned char i = 0; i < numShips; i++)
	{
		if (viewDirMode == REAR)
		{
			ships[i].position.x *= -1;
			ships[i].position.z *= -1;
			for (unsigned char j = 0; j < 9; j++)
			{
				if (j % 3 == 1) continue;
				ships[i].orientation.a[j] *= -1;
			}
		}
		else
		{
			signed char const zMul = viewDirMode == LEFT ? -1 : 1;
			signed int oldX = ships[i].position.x;
			ships[i].position.x = -1 * zMul * ships[i].position.z;
			ships[i].position.z = zMul * oldX;
			for (unsigned char j = 0; j < 9; j += 3)
			{
				oldX = ships[i].orientation.a[j];
				ships[i].orientation.a[j] = -1 * zMul * ships[i].orientation.a[j + 2];
				ships[i].orientation.a[j + 2] = zMul * oldX;
			}
		}
	}
}

void RestoreAxes(enum viewDirMode_t viewDirMode)
{
	switch (viewDirMode)
	{
		case FRONT:
			return;
		case REAR:
			FlipAxes(REAR);
			break;
		case LEFT:
			FlipAxes(RIGHT);
			break;
		case RIGHT:
			FlipAxes(LEFT);
	}
}
