#pragma once

#include <variant>
#include <string>

class Menu;

enum class ActionResult
{
	Success,
	Fail,
	ChangeMenu,
	ExitApplication
};

struct ExecuteActionResult
{
	ActionResult Result;
	std::variant<std::string> Parameter;
};