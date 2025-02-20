#ifndef GTK_GENERIC_H
#define GTK_GENERIC_H

#include <gtk/gtk.h>
#include <string>

void ShowGenericMessageDialog(GtkWindow* parent, const std::string& message);

gint ShowGenericConfirmationDialog(GtkWindow* parent, const std::string& message);

#endif // GTK_GENERIC_H