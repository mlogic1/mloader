#include <mloader/AppContext.h>
#include <termios.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <string>

#ifdef _WIN32
	#define OS_CLEAR "cls"
#else
	#define OS_CLEAR "clear"
#endif

#define KEY_Q 'q'
#define KEY_ARROW_UP 65
#define KEY_ARROW_DOWN 66
#define KEY_RETURN 10
#define MLOADER_VERSION 0.1

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


static std::vector<std::string> menuoptions = {
	"thing 1",
	"thing 2",
	"thing 3",
	"Quit"
};

static int32_t selectedIndex = 0;

void ClearScreen()
{
	system(OS_CLEAR);
}

void PrintMenu()
{
	for(int i = 0; i < menuoptions.size(); ++i)
	{
		if (i == selectedIndex)
		{
			std::cout << "> ";
		}

		std::cout << menuoptions.at(i) << std::endl;
	}
}

int main(int argc, char* argv[])
{
	set_terminal_mode();
	std::cout << "mloader" << std::endl;
	
	mloader::AppContext* context = mloader::CreateLoaderContext();

	ClearScreen();
	PrintMenu();
	
	int c;
	while(c != 'q')
	{
		c = getchar();

		// std::cout << (int)c << std::endl;
		
		if (c == 27)
		{
			// escape sequences
			c = getchar();
			c = getchar();

			// arrow keys
			if (c == KEY_ARROW_UP)
			{
				--selectedIndex;
				if (selectedIndex < 0)
				{
					selectedIndex = 0;
				}
			}
			if (c == KEY_ARROW_DOWN)
			{
				++selectedIndex;
				if (selectedIndex > menuoptions.size() - 1)
				{
					selectedIndex = menuoptions.size() - 1;
				}
			}
		}
		else if (c == KEY_RETURN)
		{
			// execute action
			mloader::RefreshMetadata(context);
		}
		else
		{
			// std::cout << c << std::endl;
			continue;
		}
		
		ClearScreen();
		PrintMenu();
	}

	reset_terminal_mode();

	mloader::DestroyLoaderContext(context);
	return 0;
}
