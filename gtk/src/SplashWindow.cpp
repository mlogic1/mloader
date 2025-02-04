#include "SplashWindow.h"
#include <mloader/AppContext.h>

static void gquit(GtkWidget* widget, gpointer data)
{
	// gtk_main_quit();
}

SplashWindow::SplashWindow()
{
	InitializeLayout();
}

SplashWindow::~SplashWindow()
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

void SplashWindow::UpdateStatusLabelText(const char* text)
{
	// m_labelText = text;
	gtk_label_set_text(GTK_LABEL(m_labelStatus), text);
}

void SplashWindow::InitializeLayout()
{
	m_builder = gtk_builder_new_from_file(LAYOUT_FILE);
	m_window = GTK_WIDGET(gtk_builder_get_object(m_builder, "splash_window"));
	m_labelStatus = GTK_WIDGET(gtk_builder_get_object(m_builder, "splash_label_status"));

	gtk_label_set_text(GTK_LABEL(m_labelStatus), "-");

	// g_signal_connect(m_window, "destroy", G_CALLBACK(gquit), this);

	gtk_builder_connect_signals(m_builder, NULL);

	gtk_widget_show_all(m_window);
	// gtk_window_present(GTK_WINDOW (m_window));
}
