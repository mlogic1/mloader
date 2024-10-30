#pragma once
#include <string>

namespace autolog
{
	struct Vehicle
	{
		uint64_t ID;
		std::string Make;
		std::string Model;
		int Year;
		std::string Plate;
	};
}