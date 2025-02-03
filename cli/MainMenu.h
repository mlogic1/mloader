#pragma once

#include "Menu.h"

namespace mloader
{
	class AppContext;
}

class MainMenu : public Menu
{
	public:
		MainMenu(AppContext* appContext);

		ExecuteActionResult ExecuteAction(uint32_t actionIndex) override;

		const StringList& GetOptions() override;

	private:
		StringList m_options;
		AppContext* m_appContext;
};