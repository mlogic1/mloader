// Copyright (c) 2025 mlogic1
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

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