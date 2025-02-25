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

		static constexpr const char* LAYOUT_FILE = "/mlres/layouts/layout_splash.glade";
};
