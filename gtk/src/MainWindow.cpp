#include "MainWindow.h"
#include <mloader/AppContext.h>
#include <mloader/App.h>
#include <iostream>

static void gquit(GtkWidget* widget, gpointer data)
{
	gtk_main_quit();
}

// The following static functions serve as a binding between GTK C api and this cpp class
static void applist_filter_changed(GtkWidget* widget, gpointer data)
{
	MainWindow* mainWindow = static_cast<MainWindow*>(data);
	mainWindow->OnAppFilterChanged();
}

gboolean test_filter_func(GtkTreeModel *model, GtkTreeIter *iter,	gpointer      data)
{
	MainWindow* mainWindow = static_cast<MainWindow*>(data);
	return mainWindow->OnFilterFunction(model, iter);
}

MainWindow::MainWindow(AppContext* appContext)
	: m_appContext(appContext)
{
	InitializeLayout();
}

MainWindow::~MainWindow()
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

	if (m_appList != nullptr)
	{
		FreeAppList(m_appList);
		m_appList = nullptr;
	}
}

void MainWindow::OnAppFilterChanged()
{
	gtk_tree_model_filter_refilter(m_appTreeModelFilter);	
}

gboolean MainWindow::OnFilterFunction(GtkTreeModel *model, GtkTreeIter *iter)
{
	const gchar *searchText = gtk_entry_get_text(GTK_ENTRY(m_entryFilter));
	const size_t searchLen = strlen(searchText);
	if (searchLen == 0)
	{
		return true;
	}

	gchar* value;	// value is application name
	gtk_tree_model_get(model, iter, 0, &value, -1);

	if (value == NULL)
	{
		return true;
	}

	// First try doing a simple search, if that doesn't work try strcasestr
	if (strncasecmp(searchText, value, searchLen) == 0)
	{
		g_free(value);
		return true;
	}

	if (strcasestr(value, searchText) != NULL)
	{
		g_free(value);
		return true;
	}

	g_free(value);
	return false;
}

void MainWindow::InitializeLayout()
{
	m_builder 				= gtk_builder_new_from_file(LAYOUT_FILE);
	m_window 				= GTK_WIDGET(gtk_builder_get_object(m_builder, "main_window"));
	m_mainAppTree 			= GTK_TREE_VIEW(gtk_builder_get_object(m_builder, "main_app_tree"));
	m_mainAppTreeListStore 	= GTK_LIST_STORE(gtk_builder_get_object(m_builder, "liststoreAppList"));
	m_entryFilter			= GTK_ENTRY(gtk_builder_get_object(m_builder, "main_entry_search_filter"));
	m_appTreeModelFilter	= GTK_TREE_MODEL_FILTER(gtk_builder_get_object(m_builder, "listStoreAppListModelFilter"));

	gtk_tree_model_filter_set_visible_func(m_appTreeModelFilter, test_filter_func, this, NULL);

	g_signal_connect(m_window, "destroy", G_CALLBACK(gquit), this);
	g_signal_connect(m_entryFilter, "changed", G_CALLBACK(applist_filter_changed), this);

	gtk_builder_connect_signals(m_builder, NULL);

	gtk_widget_show_all(m_window);
	gtk_window_present(GTK_WINDOW (m_window));

	RefreshList();
}

void MainWindow::RefreshList()
{
	gtk_entry_set_text(m_entryFilter, "");
	gtk_list_store_clear(m_mainAppTreeListStore);

	int numApps;
	if (m_appList != nullptr)
	{
		FreeAppList(m_appList);
		m_appList = nullptr;
	}

	GetAppList(m_appContext, &m_appList, &numApps);

	GtkTreeIter iter;

	for(int i = 0; i < numApps; ++i)
	{
		gtk_list_store_append(m_mainAppTreeListStore, &iter);

		gtk_list_store_set(m_mainAppTreeListStore, &iter, 
			0, m_appList[i].GameName,
			1, m_appList[i].ReleaseName,
			2, m_appList[i].PackageName,
			3, m_appList[i].VersionCode,
			4, m_appList[i].LastUpdated,
			5, m_appList[i].SizeMB,
			6, m_appList[i].Downloads, 
		-1);
	}
}