#include "MainMenu.h"
#include <mloader/AppContext.h>
#include <map>
#include <functional>

MainMenu::MainMenu(AppContext* appContext)
	: m_appContext(appContext)
{
	m_options = { 			
		"View and download apps and games",
		"View and install downloaded games",
		"Manage devices",
		"Refresh metadata",
		"Quit"
	};
}

ExecuteActionResult MainMenu::ExecuteAction(uint32_t actionIndex)
{
	// static std::map<decltype(actionIndex), std::func

	// Exit
	if (actionIndex == m_options.size() - 1)
	{
		return ExecuteActionResult{ .Result = ActionResult::ExitApplication };
	}

	if (actionIndex == 0)
	{
		return ExecuteActionResult
		{ 
			.Result = ActionResult::ChangeMenu,
			.Parameter = "GameListMenu"
		};
	}

	if (actionIndex == 3)
	{
		RefreshMetadata(m_appContext);
	}

	return ExecuteActionResult{ .Result = ActionResult::Success };
}

const StringList& MainMenu::GetOptions()
{
	return m_options;
}