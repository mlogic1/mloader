#include "SplashWindow.h"
#include "MainWindow.h"
#include <mloader/AppContext.h>
#include <iostream>
#include <memory>
#include <thread>
#include <future>

std::unique_ptr<SplashWindow> splashWindow = nullptr;
std::unique_ptr<MainWindow> mainWindow = nullptr;
AppContext* mloaderContext = nullptr;

// Callback function to be called when mloader reports a new status
gboolean OnMLoaderAppContextCreateStatusCallbackMainThread(gpointer data) {
	const char* status = (const char*)data;
	splashWindow->UpdateStatusLabelText(status);
	
	return false;	// Return FALSE to remove this function from the idle list
}

// Callback function to be called when mloader completes initialization
gboolean OnMLoaderAppContextCreateStatusCompletedMainThread(gpointer data) {
	AppContext* context = (AppContext*)data;
	if (context == NULL)
	{
		// TODO: show a message why
		// gtk_main_quit();
	}
	else
	{
		splashWindow.reset();
		splashWindow = nullptr;
		mloaderContext = context;
		mainWindow = std::make_unique<MainWindow>(mloaderContext);
	}
	
	return false;	// Return FALSE to remove this function from the idle list
}

// Careful! 
// MloaderAppContextCreateStatusCallback and MloaderAppContextCreateCompletedCallback
// are callbacks being called from a background thread. 

void MloaderAppContextCreateStatusCallback(const char* status)
{
	g_idle_add(OnMLoaderAppContextCreateStatusCallbackMainThread, (gpointer)status);
}

void MloaderAppContextCreateCompletedCallback(AppContext* context)
{
	g_idle_add(OnMLoaderAppContextCreateStatusCompletedMainThread, (gpointer)context);
}

int main(int argc, char* argv[])
{
	gtk_init(&argc, &argv);
	splashWindow = std::make_unique<SplashWindow>();

	CreateLoaderContextAsync(MloaderAppContextCreateCompletedCallback, MloaderAppContextCreateStatusCallback);

	gtk_main();

	splashWindow.reset();
	mainWindow.reset();
	DestroyLoaderContext(mloaderContext);
	return 0;
}