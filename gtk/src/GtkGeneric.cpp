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

#include "GtkGeneric.h"

void ShowGenericMessageDialog(GtkWindow* parent, const std::string& message)
{
	char* msgBuf;
	msgBuf = strdup(message.c_str());
	GtkWidget* dialog = gtk_message_dialog_new( parent, 
												GTK_DIALOG_DESTROY_WITH_PARENT,
												GTK_MESSAGE_INFO,
												GTK_BUTTONS_OK,
												"%s",
												msgBuf);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
	free(msgBuf);
}

gint ShowGenericConfirmationDialog(GtkWindow* parent, const std::string& message)
{
	return 1337;
}
