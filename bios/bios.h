#pragma once

#pragma region INCLUDES
//
#include "helpers.h"
//
#pragma endregion INCLUDES

class bios_t
{
public:

	unsigned char biosImage[0x4000];

	bool biosFound;
	bool unMapBios;
	int32_t expectedBiosSize;

public:

	bios_t() : biosFound(false), unMapBios(true), biosImage{0}
	{
		;
	}
};