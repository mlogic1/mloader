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
#include "GameListMenu.h"
#include <mloader/AppContext.h>
#include <termios.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>

#ifdef _WIN32
	#define OS_CLEAR "cls"
#else
	#define OS_CLEAR "clear"
#endif

#define KEY_Q 'q'
#define KEY_ARROW_UP 65
#define KEY_ARROW_DOWN 66
#define KEY_PGUP 53
#define KEY_PGDN 54
#define KEY_BKSP 127
#define KEY_RETURN 10
#define MLOADER_VERSION 0.1

void InitializationCallbackMessage(const char* message)
{
	std::cout << message << std::endl;
}

void set_terminal_mode() {
	struct termios newt;
	tcgetattr(STDIN_FILENO, &newt);
	newt.c_lflag &= ~(ICANON | ECHO); // Disable canonical mode and echo
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	std::cout << "\033[?25l" << std::endl;
}

void reset_terminal_mode() {
	struct termios newt;
	tcgetattr(STDIN_FILENO, &newt);
	newt.c_lflag |= (ICANON | ECHO); // Enable canonical mode and echo
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	std::cout << "\033[?25h" << std::endl;
}

static int32_t selectedIndex = 0;
std::unordered_map<std::string, Menu*> allMenus;

void ClearScreen()
{
	system(OS_CLEAR);
}

void PrintPage(const StringList& options, int page, int pageSize)
{
	int start = page * pageSize;
	int end = std::min(start + pageSize, static_cast<int>(options.size()));

	for (int i = start; i < end; ++i) {
		if (i == selectedIndex)
		{
			std::cout << "> ";
		}
		std::cout << options[i] << std::endl;
	}
}

// used for game list menu
void PagedPrintMenu(Menu* menu)
{
	const auto& options = menu->GetOptions();
	constexpr int pageSize = 20;
	const int numPages = (options.size() + pageSize - 1) / pageSize;

	int currentPage = selectedIndex / pageSize;
	PrintPage(options, currentPage, pageSize);
	std::cout << std::endl << std::endl << std::endl;
	std::cout << "Page " << currentPage + 1 << " of " << numPages << std::endl << std::endl;
	std::cout << "Press the arrow keys to navigate the list." << std::endl;
	std::cout << "Press PGDN or PGUP to navigate to next or previous page." << std::endl;
	std::cout << "Press Backspace to return to previous menu." << std::endl;
}

void PrintMenu(Menu* menu)
{
	const auto& options = menu->GetOptions();
	
	for (int i = 0; i < options.size(); ++i)
	{
		if (i == selectedIndex)
		{
			std::cout << "> ";
		}

		std::cout << options[i] << std::endl;
	}
}

static Menu* currentMenu = nullptr;

void ChangeMenu(const std::string& menuName)
{
	currentMenu = allMenus.at(menuName);
	selectedIndex = 0;
}

std::unordered_map<std::string, Menu*> SetupMenus(AppContext* context)
{
	std::unordered_map<std::string, Menu*> result =
	{
		{ "MainMenu", new MainMenu(context) },
		{ "GameListMenu", new GameListMenu(context) }
	};
	
	return result;
}

int main(int argc, char* argv[])
{
	set_terminal_mode();
	
	AppContext* context = CreateLoaderContext(InitializationCallbackMessage);
	allMenus = SetupMenus(context);
	ChangeMenu("MainMenu");

	ClearScreen();
	PrintMenu(currentMenu);
	
	int c;
	while(c != 'q')
	{
		c = getchar();
		const size_t numOptions = currentMenu->GetOptions().size();
		
		if (c == 27)
		{
			// escape sequences
			c = getchar();
			c = getchar();

			// arrow keys
			if (c == KEY_ARROW_UP)
			{
				--selectedIndex;
			}
			if (c == KEY_ARROW_DOWN)
			{
				++selectedIndex;
			}
			if (c == KEY_PGUP)
			{
				selectedIndex -=20;
			}
			if (c == KEY_PGDN)
			{
				selectedIndex +=20;
			}

			if (selectedIndex < 0)
			{
				selectedIndex = 0;
			}

			if (selectedIndex > numOptions - 1)
			{
				selectedIndex = numOptions - 1;
			}
		}
		else if (c == KEY_RETURN)
		{
			// execute action
			ExecuteActionResult executeActionResult = currentMenu->ExecuteAction((uint32_t)selectedIndex);

			if (executeActionResult.Result == ActionResult::ExitApplication)
			{
				break;
			}

			if (executeActionResult.Result == ActionResult::Fail)
			{
				std::cout << std::get<std::string>(executeActionResult.Parameter) << std::endl;
			}
			
			if (executeActionResult.Result == ActionResult::ChangeMenu)
			{
				ChangeMenu(std::get<std::string>(executeActionResult.Parameter));
			}
		}
		else if (c == KEY_BKSP)
		{
			ExecuteActionResult executeActionResult = currentMenu->ExecuteCancelAction();

			if (executeActionResult.Result == ActionResult::ChangeMenu)
			{
				ChangeMenu(std::get<std::string>(executeActionResult.Parameter));
			}
		}
		else
		{
		}
		
		ClearScreen();

		if (dynamic_cast<GameListMenu*>(currentMenu))
		{
			PagedPrintMenu(currentMenu);
		}
		else
		{
			PrintMenu(currentMenu);
		}
	}

	// Cleanup
	reset_terminal_mode();

	for (auto menu : allMenus)
	{
		delete menu.second;
		menu.second = nullptr;
	}

	DestroyLoaderContext(context);
	return 0;
}
