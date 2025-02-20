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
