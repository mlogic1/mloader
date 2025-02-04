#pragma once

#include <gtk/gtk.h>
#include <string>

class SplashWindow
{
	public:
		SplashWindow();
		~SplashWindow();

		void UpdateStatusLabelText(const char* text);

	private:
		void InitializeLayout();

	private:
		GtkBuilder* m_builder;
		GtkWidget* m_window;
		GtkWidget* m_labelStatus;

		static constexpr const char* LAYOUT_FILE = "data/layouts/layout_splash.glade";
};
