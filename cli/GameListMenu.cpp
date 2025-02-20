#include "GameListMenu.h"
#include <mloader/AppContext.h>
#include <map>

GameListMenu::GameListMenu(AppContext* appContext)
	: m_appContext(appContext)
{

}

GameListMenu::~GameListMenu()
{
	if (m_appList != nullptr)
	{
		// FreeAppList(m_appList);
	}
}

ExecuteActionResult GameListMenu::ExecuteAction(uint32_t actionIndex)
{
	// static std::map<decltype(actionIndex), std::func

	// Exit
	if (actionIndex == m_options.size() - 1)
	{
		return ExecuteActionResult
		{ 
			.Result = ActionResult::ChangeMenu,
			.Parameter = "MainMenu"
		};
	}
	else
	{
		// Download game (this should be try catched)
		DownloadApp(m_appContext, m_optionsIndexToApp.at(actionIndex));
	}

	return ExecuteActionResult{ .Result = ActionResult::Success };
}

ExecuteActionResult GameListMenu::ExecuteCancelAction()
{
	return ExecuteActionResult
	{ 
		.Result = ActionResult::ChangeMenu,
		.Parameter = "MainMenu"
	};
}

const StringList& GameListMenu::GetOptions()
{
	m_options.clear();
	m_optionsIndexToApp.clear();

	if (m_appList == nullptr)
	{
		m_appList = GetAppList(m_appContext, &m_numApps);
	}

	for(int i = 0; i < m_numApps; ++i)
	{
		m_options.emplace_back(m_appList[i]->GameName);
		m_optionsIndexToApp.emplace((uint32_t)i, m_appList[i]);
	}

	m_options.emplace_back("Return to main menu");
	return m_options;
}
