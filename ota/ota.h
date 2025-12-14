#pragma once

#include <helpers.h>

extern FLAG isOtaPossible;

class ota_t
{
public:
	ota_t();
	~ota_t();
	bool checkForUpdates(boost::property_tree::ptree& pt);
	bool upgrade(boost::property_tree::ptree& pt);
};