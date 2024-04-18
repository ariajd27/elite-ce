#include "ship.h"
#include "ship_data.h"
#include <graphx.h>
#include "xorgfx.h"
#include "linear.h"
#include "variables.h"
#include "intmath.h"
#include "generation.h"
#include "flight.h"

#include <debug.h>

struct Ship ships[MAX_SHIPS];

unsigned char numShips = 0;

struct Ship* NewShip(unsigned char shipType, struct vector_t position, struct intmatrix_t orientation)
{
	ships[numShips].shipType = shipType;
	ships[numShips].position.x = position.x;
	ships[numShips].position.y = position.y;
	ships[numShips].position.z = position.z;
	for (unsigned char i = 0; i < 9; i++) ships[numShips].orientation.a[i] = orientation.a[i];

	ships[numShips].isHostile = false;
	ships[numShips].aggro = 64;
	// ships[numShips].target not initialized

	ships[numShips].isExploding = 0;
	ships[numShips].toExplode = 0;
	ships[numShips].laserFiring = 0;

	ships[numShips].visibility = bp_header_vectors[shipType][BP_GENERAL][BP_VISDIST_IND];
	ships[numShips].numVertices = bp_header_vectors[shipType][BP_GENERAL][BP_VERT_L_IND] / BP_L_VERT;
	ships[numShips].numEdges = bp_header_vectors[shipType][BP_GENERAL][BP_EDGE_Q_IND];
	ships[numShips].numFaces = bp_header_vectors[shipType][BP_GENERAL][BP_FACE_L_IND] / BP_L_FACE;

	ships[numShips].explosionSize = EXPLOSION_START_SIZE;
	ships[numShips].explosionCount = bp_header_vectors[shipType][BP_GENERAL][BP_EXPLCT_IND];
	// ships[numShips].explosionRand isn't initialized until the explosion begins

	ships[numShips].speed = 0;
	ships[numShips].acceleration = 0;
	ships[numShips].pitch = 0;
	ships[numShips].roll = 0;

	ships[numShips].energy = bp_header_vectors[shipType][BP_GENERAL][BP_MAX_ENERGY];

	numShips++;
	return &ships[numShips - 1];
}

// this is usually unecessary, as wiping all ships can be easily done simply by
// setting numShips = 0 so the other ones aren't processed and are overwritten
void RemoveShip(unsigned char shipIndex)
{
	for (unsigned char i = shipIndex; i < numShips; i++)
	{
		ships[i] = ships[i + 1];

		// flag missiles to not have a target if their target is destroyed,
		// otherwise simply keep them on track for their targets
		if (ships[i].target != shipIndex) ships[i].target -= 1;
		else ships[i].target = MAX_SHIPS;
	}
	numShips--;
}

struct int_point_t ProjPoint(struct vector_t toProject) 
{
	struct int_point_t output;

	if (intabs(toProject.x) <= 0x7fff && intabs(toProject.y) <= 0x7fff)
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

// this is used by both the wireframe and explosion drawing algorithms
struct int_point_t ProjVertex(const unsigned char shipIndex,
							  const unsigned char vertexIndex,
							  const struct intmatrix_t transformation)
{
	const unsigned char* vertexData = 
		bp_header_vectors[ships[shipIndex].shipType][BP_VERTICES] + vertexIndex * BP_L_VERT;

	const struct vector_t vertexVector = {
		(vertexData[3] & 0b10000000) != 0 ? -1 * vertexData[0] : vertexData[0],
		(vertexData[3] & 0b01000000) != 0 ? -1 * vertexData[1] : vertexData[1],
		(vertexData[3] & 0b00100000) != 0 ? -1 * vertexData[2] : vertexData[2]
	};

	return ProjPoint(add(vMul(transformation, vertexVector), ships[shipIndex].position));
}

void ShipAsExplosion(unsigned char shipIndex)
{
	// the cloud expands each time it's drawn
	ships[shipIndex].explosionSize += EXPLOSION_GROW_RATE;

	// if we've overflowed, the explosion is over, so get rid of the ship
	if (ships[shipIndex].explosionSize < EXPLOSION_START_SIZE)
	{
		RemoveShip(shipIndex);
		return;
	}

	// save the main program's seed -- we reseed it here
	const unsigned int savedSeed = rand() % 0xffffff;

	// this is needed for vector projections
	const struct intmatrix_t transformation = transpose(ships[shipIndex].orientation);

	// make the cloud larger on-screen if actually bigger, but smaller if more distant
	const unsigned char drawnSize = 
		EXPLOSION_SCALE * ships[shipIndex].explosionSize / ships[shipIndex].position.z;

	// how many particles are we drawing per vertex?
	unsigned char particleCt = ships[shipIndex].explosionSize;
	if (particleCt >= 128) particleCt ^= 0xff; // flip all bits, so we count up to 128 then back down
	particleCt /= ships[shipIndex].explosionCount;
	const unsigned char remCt = (ships[shipIndex].explosionSize ^ 0xff) % ships[shipIndex].explosionCount;
	if (particleCt < 1) particleCt = 1;

	// iterate through all vertices flagged for explosion particle drawing
	for (unsigned char i = 0; i < ships[shipIndex].explosionCount; i++)
	{
		// get the random seed *for this vertex*
		srand(ships[shipIndex].explosionRand ^ i);

		// where is this vertex on the screen?
		const struct int_point_t vertexPos = ProjVertex(shipIndex, i, transformation);

		// iterate through all the particles to draw for this vertex
		for (unsigned char j = i < remCt ? 0 : 1; j <= particleCt; j++)
		{
			const signed int x = vertexPos.x + (signed int)(rand() % 256 - 128) * drawnSize / 256;
			const signed int y = vertexPos.y + (signed int)(rand() % 256 - 128) * drawnSize / 256;

			xor_FillRectangle(x, y, EXPLOSION_PARTICLE_SIZE, EXPLOSION_PARTICLE_SIZE);
		}
	}

	srand(savedSeed);
}

void ShipAsPoint(unsigned char shipIndex)
{
	struct int_point_t screenPoint = ProjPoint(ships[shipIndex].position);

	screenPoint.x -= (signed int)SHPPT_WIDTH / 2;
	screenPoint.y -= (signed int)SHPPT_HEIGHT / 2;

	xor_FillRectangle(screenPoint.x, screenPoint.y, SHPPT_WIDTH, SHPPT_HEIGHT);
}

void ShipAsWireframe(unsigned char shipIndex)
{
	unsigned char distance = ships[shipIndex].position.z >> 8;
	if (distance > 31) distance = 31;

	const struct intmatrix_t transformation = transpose(ships[shipIndex].orientation);

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
					(faceData[0] & 0b00100000) != 0 ? -1 * dataZ : dataZ,
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
	} vertices[28] = {
		{ 
			false, 
			(struct int_point_t){ 0, 0 } 
		}
	};

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

			vertices[edgeData[j] >> 2].screenPosition = 
				ProjVertex(shipIndex, edgeData[j] >> 2, transformation);
			vertices[edgeData[j] >> 2].calculated = true;
		}

		// clipping is built into this (hopefully!)
		xor_Line(vertices[edgeData[2] >> 2].screenPosition.x, vertices[edgeData[2] >> 2].screenPosition.y,
				vertices[edgeData[3] >> 2].screenPosition.x, vertices[edgeData[3] >> 2].screenPosition.y);
	}
}

void ShipAsBody(unsigned char shipIndex)
{
	const struct int_point_t center = ProjPoint(ships[shipIndex].position);
	const unsigned int radius = 24576 / (ships[shipIndex].position.z >> 8);

	if (ships[shipIndex].shipType == SUN)
	{
		xor_FillCircle(center.x, center.y, radius);
		return;
	}

	// if we get here, this is a planet...
	xor_Circle(center.x, center.y, radius);

	// i removed the equator/meridian planets because they look like goddamn beach balls
	if (!gen_PlanetHasCrater()) return;

	// only the draw the crater if it's facing the camera
	if (ships[shipIndex].orientation.a[5] < 0) return;
	
	// draw the crater!!!!!
	xor_Ellipse(center.x + 83 * ships[shipIndex].orientation.a[3] / (ships[shipIndex].position.z >> 8),
				center.y + 83 * ships[shipIndex].orientation.a[4] / (ships[shipIndex].position.z >> 8),
				96 * ships[shipIndex].orientation.a[6] / (ships[shipIndex].position.z / 128),
				96 * ships[shipIndex].orientation.a[7] / (ships[shipIndex].position.z / 128),
				96 * ships[shipIndex].orientation.a[0] / (ships[shipIndex].position.z / 128),
				96 * ships[shipIndex].orientation.a[1] / (ships[shipIndex].position.z / 128),
				64);
}

void DrawShip(unsigned char shipIndex)
{
	if (ships[shipIndex].toExplode)
	{
		// if we flagged starting an explosion, we need to set up
		// that explosion before doing anything else

		ships[shipIndex].laserFiring = false;
		ships[shipIndex].toExplode = false;
		ships[shipIndex].isExploding = true;

		ships[shipIndex].speed = 0;
		ships[shipIndex].acceleration = 0;
		ships[shipIndex].pitch = 0;
		ships[shipIndex].roll = 0;

		// let's get us a seed!
		ships[shipIndex].explosionRand = rand() % 256;
	}

	if (ships[shipIndex].position.z <= 0) return;

	// suns and planets jump off the bus earlier, because since they can be so large,
	// they need to be drawn even if they're way off screen bc they might spill over into
	// the screen anyway
	if (ships[shipIndex].shipType > BP_ESCAPEPOD)
	{
		ShipAsBody(shipIndex);
		return;
	}

	if ((unsigned int)ships[shipIndex].position.z >= 0xc00000) return;
	if (ships[shipIndex].position.x >= ships[shipIndex].position.z) return;
	if (ships[shipIndex].position.x < -1 * ships[shipIndex].position.z) return;
	if (ships[shipIndex].position.y >= ships[shipIndex].position.z) return;
	if (ships[shipIndex].position.y < -1 * ships[shipIndex].position.z) return;
	
	// finally, the three main ways to draw a ship:
	// if it's exploding, as a cloud of dust
	if (ships[shipIndex].isExploding) ShipAsExplosion(shipIndex);
	// if it's far away, as a dot
	else if ((ships[shipIndex].position.z >> 9) > ships[shipIndex].visibility) ShipAsPoint(shipIndex);
	// otherwise, as a beautiful 3d wireframe
	else ShipAsWireframe(shipIndex);
}

void DoAI(unsigned char shipIndex)
{
	// is this even a real thing?
	if (shipIndex >= numShips) return;

	// some things don't have AI at all
	if (ships[shipIndex].shipType >= BP_CORIOLIS
			&& ships[shipIndex].shipType != BP_ESCAPEPOD
			&& ships[shipIndex].shipType != BP_MISSILE) return;

	// remove ships that fly super far away
	if (ships[shipIndex].position.x > 224 * 256 
			|| ships[shipIndex].position.y > 224 * 256 
			|| ships[shipIndex].position.z > 224 * 256
			|| ships[shipIndex].position.x < -224 * 256
			|| ships[shipIndex].position.y < -224 * 256
			|| ships[shipIndex].position.z < -224 * 256)
	{
		if (ships[shipIndex].shipType == BP_CORIOLIS) stationSoi = false;
		RemoveShip(shipIndex);
		return;
	}

	dbg_printf("doing AI for ship %u, shipType %u...\n", shipIndex, ships[shipIndex].shipType);

	// recharge energy
	if (ships[shipIndex].energy < bp_header_vectors[ships[shipIndex].shipType][BP_GENERAL][BP_MAX_ENERGY])
	{
		ships[shipIndex].energy++;
	}

	// which way are we trying to go?
	struct vector_t goVector;
	if (ships[shipIndex].shipType == BP_MISSILE)
	{
		// check if the missile has hit its target
		const unsigned int range = ships[shipIndex].isHostile ? magnitude(ships[shipIndex].position) 
			: magnitude(sub(ships[ships[shipIndex].target].position, ships[shipIndex].position));
		if (range < MISSILE_HIT_RANGE)
		{
			// did we hit the player?
			if (ships[shipIndex].isHostile)
			{
				flt_DamagePlayer(MISSILE_DIRECT_DAMAGE, ships[shipIndex].position.z < 0);
			}
			else
			{
				// just kill the non-player that got hit, unless it's a station
				if (ships[ships[shipIndex].target].shipType != BP_CORIOLIS)
				{
					ships[ships[shipIndex].target].toExplode = true;
				}

				// is the player nearby to the blast to get splash damage?
				if (magnitude(ships[shipIndex].position) < MISSILE_SPLASH_RANGE)
				{
					flt_DamagePlayer(MISSILE_SPLASH_DAMAGE, ships[shipIndex].position.z < 0);
				}
			}

			// the missile blows up and is gone
			ships[shipIndex].toExplode = true;
			return;
		}

		// destroy missiles facing ECM or whose targets are dead
		if (ecmTimer > 0 || ships[shipIndex].target == MAX_SHIPS)
		{
			ships[shipIndex].toExplode = true;
			return;
		}

		// missiles head towards their target
		if (ships[shipIndex].isHostile) goVector = mul(ships[shipIndex].position, -1);
		else goVector = sub(ships[ships[shipIndex].target].position, ships[shipIndex].position);
	}
	else if (ships[shipIndex].isHostile)
	{
		// most things head away from the player (but can switch to towards)
		goVector = ships[shipIndex].position;

		// if we are not a cop, we shouldn't attack in the safe zone
		if (stationSoi || ships[shipIndex].shipType == BP_VIPER)
		{
			// this is the easiest way to pacify a ship, but it might need changed later
			ships[shipIndex].isHostile = false;
		}
		else
		{
			// if not too close and feeling aggressive, we go towards the player. otherwise away.
			const unsigned int distance = intabs(ships[shipIndex].position.x)
							   			| intabs(ships[shipIndex].position.y)
							   			| intabs(ships[shipIndex].position.z);
			if ((distance >> 9) != 0 && rand() % 64 < ships[shipIndex].aggro)
			{
				dbg_printf("aggressive mode!\n");
				goVector = mul(goVector, -1);
			}
		}
	}
	else
	{
		// escape pods flee towards the planet
		unsigned char planetIndex;
		for (planetIndex = 0; ships[planetIndex].shipType != PLANET; planetIndex++);
		goVector = sub(ships[planetIndex].position, ships[shipIndex].position);
	}

	// normalizing means that goAlign will have a consistent meaning
	goVector = normalize(goVector);
	const signed int goAlign = dot(goVector, getRow(ships[shipIndex].orientation, 2));

	// TODO consider firing weapons
	
	// consider using ECM
	if (ships[shipIndex].hasEcm)
	{
		// is there a missile chasing us?
		for (unsigned char missileIndex = 0; missileIndex < numShips; missileIndex++)
		{
			if (ships[missileIndex].shipType == BP_MISSILE && ships[missileIndex].target == shipIndex)
			{
				// if so, turn on ECM
				ecmTimer = ECM_DURATION;
				break;
			}
		}
	}

	// steering and thrust!
	dbg_printf("current position: (%d, %d, %d)\n", ships[shipIndex].position.x, ships[shipIndex].position.y, ships[shipIndex].position.z);
	dbg_printf("target vector: (%d, %d, %d)\n", goVector.x, goVector.y, goVector.z);
	dbg_printf("current orientation: (%d,%d,%d, %d,%d,%d, %d,%d,%d)\n", ships[shipIndex].orientation.a[0], 
ships[shipIndex].orientation.a[1], ships[shipIndex].orientation.a[2], ships[shipIndex].orientation.a[3], 
ships[shipIndex].orientation.a[4], ships[shipIndex].orientation.a[5], ships[shipIndex].orientation.a[6], 
ships[shipIndex].orientation.a[7], ships[shipIndex].orientation.a[8]);
	dbg_printf("current speed: %u\n", ships[shipIndex].speed);
	dbg_printf("current nose alignment: %d\n", goAlign);

	// pitch is simple
	const signed int roofAlign = dot(goVector, getRow(ships[shipIndex].orientation, 1));
	dbg_printf("current roof alignment: %d\n", roofAlign);
	ships[shipIndex].pitch = roofAlign > 0 ? -3 : 3;
	dbg_printf("pitch set: %d\n", ships[shipIndex].pitch);

	// roll is not...
	// first, non-missiles might make wacky swerves for variety
	if (ships[shipIndex].shipType != BP_MISSILE && rand() % 128 < 8)
	{
		ships[shipIndex].roll = rand() % 2 ? 20 : -20;
	}

	// then skip normal roll processing if already in a roll
	if (intabs(ships[shipIndex].roll) < 16)
	{
		// here is normal roll processing
		const signed int sideAlign = dot(goVector, getRow(ships[shipIndex].orientation, 0)); 
		dbg_printf("current side alignment: %d\n", sideAlign);

		// roll so that pitch will be towards the target
		ships[shipIndex].roll = (sideAlign > 0) == (ships[shipIndex].pitch > 0) ? 5 : -5;

		dbg_printf("roll set: %d\n", ships[shipIndex].roll);
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

	dbg_printf("acceleration set: %d\n", ships[shipIndex].acceleration);
}

void RotateShip(signed int* x, signed int* y, signed char amount)
{
	signed int oldX = *x;

	// 1 - 1/512 is the cosine of 3.6 degrees, and 1 / 16 is the sine
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
		RotateShip(&ships[shipIndex].orientation.a[3], 
				&ships[shipIndex].orientation.a[0], 
				ships[shipIndex].roll);
		RotateShip(&ships[shipIndex].orientation.a[4], 
				&ships[shipIndex].orientation.a[1], 
				ships[shipIndex].roll);
		RotateShip(&ships[shipIndex].orientation.a[5], 
				&ships[shipIndex].orientation.a[2], 
				ships[shipIndex].roll);
		
		if (ships[shipIndex].roll > 0 && ships[shipIndex].roll < 127) ships[shipIndex].roll--;
		else if (ships[shipIndex].roll < 0 && ships[shipIndex].roll > -128) ships[shipIndex].roll++;

	}

	// apply pitch
	if (ships[shipIndex].pitch != 0)
	{
		RotateShip(&ships[shipIndex].orientation.a[3], 
				&ships[shipIndex].orientation.a[6], 
				ships[shipIndex].pitch);
		RotateShip(&ships[shipIndex].orientation.a[4], 
				&ships[shipIndex].orientation.a[7], 
				ships[shipIndex].pitch);
		RotateShip(&ships[shipIndex].orientation.a[5], 
				&ships[shipIndex].orientation.a[8], 
				ships[shipIndex].pitch);

		if (ships[shipIndex].pitch > 0 && ships[shipIndex].pitch < 127) ships[shipIndex].pitch--;
		else if (ships[shipIndex].pitch < 0 && ships[shipIndex].pitch > -128) ships[shipIndex].pitch++;
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

void DamageShip(unsigned char shipIndex, unsigned char damage)
{
	if (ships[shipIndex].energy <= damage)
	{
		ships[shipIndex].toExplode = true;
		ships[shipIndex].energy = 0;
	}
	else
	{
		ships[shipIndex].energy -= damage;
		ships[shipIndex].isHostile = true; // don't shoot the station, or else!
	}
}
