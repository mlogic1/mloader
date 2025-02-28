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

#include "AboutWindow.h"
#include <mloader/SAContext.h>

AboutWindow::AboutWindow(GtkWindow* parent)	:
	m_parent(parent)
{
	InitializeLayout();
}

AboutWindow::~AboutWindow()
{
	if (m_window)
	{
		gtk_widget_destroy(m_window);
		m_window = nullptr;
	}

	if (m_builder)
	{
		g_object_unref(G_OBJECT(m_builder));
		m_builder = nullptr;
	}
}

void AboutWindow::InitializeLayout()
{
	m_builder = gtk_builder_new_from_resource(LAYOUT_FILE);
	m_window = GTK_WIDGET(gtk_builder_get_object(m_builder, "dialog_about"));

	gtk_window_set_transient_for(GTK_WINDOW(m_window), m_parent);
	gtk_window_set_keep_above(GTK_WINDOW(m_window), true);
	GtkLabel* versionLabel = GTK_LABEL(gtk_builder_get_object(m_builder, "about_label_app_version"));
	GtkButton* buttonClose = GTK_BUTTON(gtk_builder_get_object(m_builder, "about_button_close"));

	g_signal_connect(buttonClose, "clicked", G_CALLBACK(+[](GtkWidget* widget, gpointer data)
	{
		GtkWidget* dialogWindow = static_cast<GtkWidget*>(data);
		gtk_widget_destroy(dialogWindow);
	}), m_window);

	const char* gtkTimestamp = GTK_BUILD_TIMESTAMP;
	char stringBuff[1024];
	char* libVersion = MLoaderGetLibraryVersion();
	snprintf(stringBuff, sizeof(stringBuff), "MLoader GTK(%s)\nMLoader Library v.%s", gtkTimestamp, libVersion);

	free(libVersion);
	gtk_label_set_text(versionLabel, stringBuff);
	gtk_builder_connect_signals(m_builder, NULL);
	gtk_dialog_run(GTK_DIALOG(m_window));
}
