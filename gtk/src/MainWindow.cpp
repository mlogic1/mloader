#include "MainWindow.h"
#include <functional>
#include <mloader/AppContext.h>
#include <mloader/App.h>

static void gquit(GtkWidget* widget, gpointer data)
{
	gtk_main_quit();
}

// TODO: investigate the option of using this:
// (sigc::mem_fun(*this, 

// The following static functions serve as a binding between GTK C api and this cpp class
static void applist_filter_changed(GtkWidget* widget, gpointer data)
{
	MainWindow* mainWindow = static_cast<MainWindow*>(data);
	mainWindow->OnAppFilterChanged();
}

static gboolean applist_visibility_func(GtkTreeModel *model, GtkTreeIter *iter,	gpointer      data)
{
	MainWindow* mainWindow = static_cast<MainWindow*>(data);
	return mainWindow->OnFilterFunction(model, iter);
}

static void devicelist_device_changes(GtkWidget* widget, gpointer data)
{
	MainWindow* mainWindow = static_cast<MainWindow*>(data);
	mainWindow->OnAdbDeviceSelectionChanged();
}

static void applist_row_selected(GtkWidget* widget, gpointer data)
{
	MainWindow* mainWindow = static_cast<MainWindow*>(data);
	mainWindow->OnAppSelectionChanged();
}

static void button_download_clicked(GtkWidget* widget, gpointer data)
{
	MainWindow* mainWindow = static_cast<MainWindow*>(data);
	mainWindow->OnDownloadButtonClicked();
}

static gboolean app_changed_callback_event(gpointer data)
{
	std::pair<MainWindow*, App*>* params = static_cast<std::pair<MainWindow*, App*>*>(data);
	params->first->OnAppStatusChanged(params->second);
	delete params;
	return false;
}

MainWindow::MainWindow(AppContext* appContext)
	: 	m_appContext(appContext),
		m_adbDeviceList(nullptr),
		m_selectedAdbDevice(nullptr)
{
	InitializeLayout();
}

MainWindow::~MainWindow()
{
	if (m_adbDeviceList != nullptr)
	{
		FreeDeviceList(m_adbDeviceList, &m_numAdbDevices);
		m_adbDeviceList = nullptr;
	}

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
		m_selectedApp = nullptr;
		m_appList = nullptr;
	}

	ClearPixBuffer();
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
	m_builder 					= gtk_builder_new_from_file(LAYOUT_FILE);
	m_window 					= GTK_WIDGET(gtk_builder_get_object(m_builder, "main_window"));
	m_mainAppTree 				= GTK_TREE_VIEW(gtk_builder_get_object(m_builder, "main_app_tree"));
	m_mainAppTreeListStore 		= GTK_LIST_STORE(gtk_builder_get_object(m_builder, "liststoreAppList"));
	m_mainDeviceListStore		= GTK_LIST_STORE(gtk_builder_get_object(m_builder, "liststoreDeviceList"));
	m_entryFilter				= GTK_ENTRY(gtk_builder_get_object(m_builder, "main_entry_search_filter"));
	m_appTreeModelFilter		= GTK_TREE_MODEL_FILTER(gtk_builder_get_object(m_builder, "listStoreAppListModelFilter"));
	m_mainDeviceListComboBox	= GTK_COMBO_BOX(gtk_builder_get_object(m_builder, "main_combo_device_list"));
	m_downloadBtn				= GTK_BUTTON(gtk_builder_get_object(m_builder, "main_button_download"));
	m_installBtn				= GTK_BUTTON(gtk_builder_get_object(m_builder, "main_button_install"));
	m_imageThumbPreview			= GTK_IMAGE(gtk_builder_get_object(m_builder, "main_bottom_image_app_thumb"));
	
	gtk_tree_model_filter_set_visible_func(m_appTreeModelFilter, applist_visibility_func, this, NULL);

	g_signal_connect(m_window, "destroy", G_CALLBACK(gquit), this);
	g_signal_connect(m_entryFilter, "changed", G_CALLBACK(applist_filter_changed), this);
	g_signal_connect(m_mainDeviceListComboBox, "changed", G_CALLBACK(devicelist_device_changes), this);
	g_signal_connect(m_mainAppTree, "cursor-changed", G_CALLBACK(applist_row_selected), this);
	g_signal_connect(m_downloadBtn, "clicked", G_CALLBACK(button_download_clicked), this);


	gtk_builder_connect_signals(m_builder, NULL);

	gtk_widget_show_all(m_window);
	gtk_window_present(GTK_WINDOW (m_window));

	SetupCallbackEvents();
	RefreshAppList();
	RefreshDeviceList();
}

void MainWindow::SetupCallbackEvents()
{
	// Important: App status changed can be called from a background thread. It's important to use g_idle_add to do any UI updates from the main thread
	SetAppStatusChangedCallback(m_appContext, [](AppContext* context, App* app, void* usrdata){
		std::pair<MainWindow*, App*>* params = new std::pair<MainWindow*, App*>(static_cast<MainWindow*>(usrdata), app);
		g_idle_add(app_changed_callback_event, params);
	}, this);
}

void MainWindow::RefreshAppList()
{
	gtk_entry_set_text(m_entryFilter, "");
	gtk_list_store_clear(m_mainAppTreeListStore);

	m_selectedApp = nullptr;

	m_appList = GetAppList(m_appContext, m_appList, &m_numApps);
	GtkTreeIter iter;

	for(int i = 0; i < m_numApps; ++i)
	{
		gtk_list_store_append(m_mainAppTreeListStore, &iter);

		gtk_list_store_set(m_mainAppTreeListStore, &iter, 
			0, m_appList[i]->GameName,
			1, m_appList[i]->StatusCStr,
			2, m_appList[i]->ReleaseName,
			3, m_appList[i]->PackageName,
			4, m_appList[i]->VersionCode,
			5, m_appList[i]->LastUpdated,
			6, m_appList[i]->SizeMB,
			7, m_appList[i]->Downloads,
		-1);
	}
}

void MainWindow::RefreshDeviceList()
{
	gtk_list_store_clear(m_mainDeviceListStore);
	if (m_adbDeviceList != nullptr)
	{
		FreeDeviceList(m_adbDeviceList, &m_numAdbDevices);
		m_adbDeviceList = nullptr;
	}

	GetDeviceList(m_appContext, m_adbDeviceList, &m_numAdbDevices);

	GtkTreeIter iter;
	for(int i = 0; i < m_numAdbDevices; ++i)
	{
		gtk_list_store_append(m_mainDeviceListStore, &iter);
		gtk_list_store_set(m_mainDeviceListStore, &iter, 0, m_adbDeviceList[i].DeviceId, -1);
	}
}

void MainWindow::OnAdbDeviceSelectionChanged()
{
	// m_mainDeviceListStore
	gint index = gtk_combo_box_get_active(m_mainDeviceListComboBox);
	if (index == -1)
	{
		m_selectedAdbDevice = nullptr;
	}
	else
	{
		m_selectedAdbDevice = &m_adbDeviceList[index];
	}
	RefreshInstallDownloadButtons();
}

void MainWindow::OnAppSelectionChanged()
{
	GtkTreeSelection* selection = gtk_tree_view_get_selection(m_mainAppTree);

	if (gtk_tree_selection_count_selected_rows(selection) < 1)	// in case the tree is empty, do not search
	{
		m_selectedApp = nullptr;
	}
	else
	{
		GtkTreeModel* model = GTK_TREE_MODEL(m_mainAppTreeListStore);
		GtkTreeIter iter;
		gtk_tree_selection_get_selected(selection, &model, &iter);
	
		gchar* releaseName;
		gtk_tree_model_get(model, &iter, 2, &releaseName, -1);
	
		for (int i = 0; i < m_numApps; ++i)
		{
			if (strcmp(releaseName, m_appList[i]->ReleaseName) == 0)
			{
				m_selectedApp = m_appList[i];
				break;
			}
		}
	}

	RefreshInstallDownloadButtons();
	RefreshDetailsPane();
}

void MainWindow::RefreshDetailsPane()
{
	if (m_selectedApp == nullptr)
	{
		// clear everything

		return;
	}

	char* imagePath = GetAppThumbImage(m_appContext, m_selectedApp);

	if (imagePath != NULL)
	{
		ClearPixBuffer();
		GError* err = NULL;
		m_imageThumbBuffer = gdk_pixbuf_new_from_file_at_scale(imagePath, 262, 150, true, &err);
		if (m_imageThumbBuffer == NULL)
		{
			// Log this
			free(err);
		}
		else
		{
			gtk_image_set_from_pixbuf(m_imageThumbPreview, m_imageThumbBuffer);
		}
		
		free(imagePath);
	}
}

void MainWindow::RefreshInstallDownloadButtons()
{
	// this function enables/disables download and install buttons depending on conditions
	if (!m_selectedApp)
	{
		gtk_widget_set_sensitive(GTK_WIDGET(m_downloadBtn), false);
		gtk_widget_set_sensitive(GTK_WIDGET(m_installBtn), false);
		return;
	}

	gtk_widget_set_sensitive(GTK_WIDGET(m_downloadBtn), m_selectedApp->Status == AppStatus::NoInfo);
}

void MainWindow::OnDownloadButtonClicked()
{
	if (!m_selectedApp) { return; } // This shouldn't occur, but just in case

	DownloadAppAsync(m_appContext, m_selectedApp);
}

void MainWindow::OnAppStatusChanged(App* app)
{
	GtkTreeIter iter;
	gboolean iterValid;
	gboolean found = false;
	iterValid = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(m_mainAppTreeListStore), &iter);

	while(iterValid)
	{
		gchar* valueReleaseName;

		gtk_tree_model_get(GTK_TREE_MODEL(m_mainAppTreeListStore), &iter, 2, &valueReleaseName, -1);

		if (g_strcmp0(valueReleaseName, app->ReleaseName) == 0)
		{
			free(valueReleaseName);
			found = true;
			break;
		}
		free(valueReleaseName);
		iterValid = gtk_tree_model_iter_next(GTK_TREE_MODEL(m_mainAppTreeListStore), &iter);
	}

	if (found)
	{
		gtk_list_store_set(m_mainAppTreeListStore, &iter, 1, app->StatusCStr, -1);
		GtkTreePath *path = gtk_tree_model_get_path(GTK_TREE_MODEL(m_mainAppTreeListStore), &iter);
		gtk_tree_model_row_changed(GTK_TREE_MODEL(m_mainAppTreeListStore), path, &iter);
		gtk_tree_path_free(path);
	}
}

void MainWindow::ClearPixBuffer()
{
	if (!m_imageThumbBuffer)
	{
		free(m_imageThumbBuffer);
		m_imageThumbBuffer = nullptr;
	}	
}