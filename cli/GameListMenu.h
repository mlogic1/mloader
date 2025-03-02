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

#pragma once

#include "Menu.h"
#include <unordered_map>
#include <mloader/App.h>

class GameListMenu : public Menu
{
	public:
		GameListMenu(AppContext* appContext);
		~GameListMenu();

		ExecuteActionResult ExecuteAction(uint32_t actionIndex) override;
		ExecuteActionResult ExecuteCancelAction() override;

		const StringList& GetOptions() override;

	private:
		StringList m_options;
		AppContext* m_appContext;

		App** m_appList = nullptr;
		int m_numApps = 0;
		std::unordered_map<uint32_t, App*> m_optionsIndexToApp;
	
		static constexpr const int COL_NAME_WIDTH = 52;
		static constexpr const int COL_SIZE_WIDTH = 10;
};
