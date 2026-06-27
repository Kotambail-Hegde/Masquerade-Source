#pragma once

#include <helpers.h>

extern FLAG isOtaPossible;

class ota_t
{
public:
	ota_t();
	~ota_t();
	bool checkForUpdates(MasqConfig_t& pt);
	bool upgrade(MasqConfig_t& pt);
};