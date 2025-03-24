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

#include "SplashWindow.h"
#include "MainWindow.h"
#include "StandaloneWindow.h"
#include <mloader/AppContext.h>
#include <mloader/SAContext.h>
#include <iostream>
#include <memory>
#include <thread>
#include <future>

std::unique_ptr<SplashWindow> splashWindow = nullptr;
std::unique_ptr<MainWindow> mainWindow = nullptr;
std::unique_ptr<StandaloneWindow> standaloneWindow = nullptr;
AppContext* mloaderContext = nullptr;
SAContext* standaloneContext = nullptr;

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

gboolean OnMLoaderSAContextCreateStatusCompletedMainThread(gpointer data) {
	SAContext* context = (SAContext*)data;
	if (context == NULL)
	{
		// TODO: show a message why
		// gtk_main_quit();
	}
	else
	{
		splashWindow.reset();
		splashWindow = nullptr;
		standaloneContext = context;
		standaloneWindow = std::make_unique<StandaloneWindow>(standaloneContext);
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

void MloaderSAContextCreateCompletedCallback(SAContext* context)
{
	g_idle_add(OnMLoaderSAContextCreateStatusCompletedMainThread, (gpointer)context);
}

int main(int argc, char* argv[])
{
	gtk_init(&argc, &argv);

	bool standaloneLoader = false;

	if (argc > 1 && strcmp(argv[1], "--no-vrp") == 0)
	{
		standaloneLoader = true;
	}

	splashWindow = std::make_unique<SplashWindow>();
	if (standaloneLoader)
	{
		MLoaderSACreateLoaderContextAsync(MloaderSAContextCreateCompletedCallback, MloaderAppContextCreateStatusCallback);
	}
	else
	{
		CreateLoaderContextAsync(MloaderAppContextCreateCompletedCallback, MloaderAppContextCreateStatusCallback, "", "");
	}

	gtk_main();

	if (standaloneLoader)
	{
		standaloneWindow.reset();
		MLoaderSADestroyLoaderContext(standaloneContext);
	}
	else
	{
		splashWindow.reset();
		mainWindow.reset();
		DestroyLoaderContext(mloaderContext);
	}

	return 0;
}