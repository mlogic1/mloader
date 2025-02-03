#pragma once

#include "ActionResult.h"
#include <stdint.h>
#include <string>
#include <vector>

using StringList = std::vector<std::string>;
struct AppContext;

class Menu
{
	public:
		virtual ~Menu() = 0;

		virtual ExecuteActionResult ExecuteAction(uint32_t actionIndex) = 0;
		virtual ExecuteActionResult ExecuteCancelAction();
		virtual const StringList& GetOptions() = 0;
};