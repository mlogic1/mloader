#include "Menu.h"

Menu::~Menu()
{
	
}

ExecuteActionResult Menu::ExecuteCancelAction()
{
	return ExecuteActionResult { .Result = ActionResult::Success };
}