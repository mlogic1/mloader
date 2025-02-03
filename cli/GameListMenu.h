#pragma once

#include "Menu.h"
#include "unordered_map"
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

		App* m_appList = nullptr;
		std::unordered_map<uint32_t, App*> m_optionsIndexToApp;
};