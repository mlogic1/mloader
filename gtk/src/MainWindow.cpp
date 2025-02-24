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

#include "MainWindow.h"
#include "GtkGeneric.h"
#include <functional>
#include <map>
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

static void button_install_clicked(GtkWidget* widget, gpointer data)
{
	MainWindow* mainWindow = static_cast<MainWindow*>(data);
	mainWindow->OnInstallButtonClicked();
}

static gboolean app_changed_callback_event(gpointer data)
{
	std::pair<MainWindow*, App*>* params = static_cast<std::pair<MainWindow*, App*>*>(data);
	params->first->OnAppStatusChanged(params->second);
	delete params;
	return false;
}

static gboolean device_list_changed_callback_event(gpointer data)
{
	MainWindow* mainWindow = static_cast<MainWindow*>(data);
	mainWindow->OnAdbDeviceListChanged();
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
	m_adbDeviceList = nullptr;

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
	free(m_imageNotePlaceholderBuffer);
	m_imageNotePlaceholderBuffer = nullptr;

	free(m_imageThumbPlaceholderBuffer);
	m_imageThumbPlaceholderBuffer = nullptr;
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
	m_builder 						= gtk_builder_new_from_file(LAYOUT_FILE);
	m_window 						= GTK_WIDGET(gtk_builder_get_object(m_builder, "main_window"));
	m_mainAppTree 					= GTK_TREE_VIEW(gtk_builder_get_object(m_builder, "main_app_tree"));
	m_mainAppTreeListStore 			= GTK_LIST_STORE(gtk_builder_get_object(m_builder, "liststoreAppList"));
	m_mainDeviceListStore			= GTK_LIST_STORE(gtk_builder_get_object(m_builder, "liststoreDeviceList"));
	m_entryFilter					= GTK_ENTRY(gtk_builder_get_object(m_builder, "main_entry_search_filter"));
	m_appTreeModelFilter			= GTK_TREE_MODEL_FILTER(gtk_builder_get_object(m_builder, "listStoreAppListModelFilter"));
	m_mainDeviceListComboBox		= GTK_COMBO_BOX(gtk_builder_get_object(m_builder, "main_combo_device_list"));
	m_mainDeviceListComboBoxEntry 	= GTK_ENTRY(gtk_builder_get_object(m_builder, "main_combo_device_list_entry"));
	m_downloadBtn					= GTK_BUTTON(gtk_builder_get_object(m_builder, "main_button_download"));
	m_installBtn					= GTK_BUTTON(gtk_builder_get_object(m_builder, "main_button_install"));
	m_imageThumbPreview				= GTK_IMAGE(gtk_builder_get_object(m_builder, "main_bottom_image_app_thumb"));
	m_imageNotePlaceholder			= GTK_IMAGE(gtk_builder_get_object(m_builder, "main_bottom_image_note_placeholder"));
	m_appNoteLabel					= GTK_LABEL(gtk_builder_get_object(m_builder, "main_bottom_label_note"));
	GError* err;
	m_imageNotePlaceholderBuffer	= gdk_pixbuf_new_from_file_at_scale("data/ui/note.png", 50, 75, true, &err);
	m_imageThumbPlaceholderBuffer	= gdk_pixbuf_new_from_file_at_scale("data/ui/thumb.png", 75, 75, true, &err);
	gtk_image_set_from_pixbuf(m_imageNotePlaceholder, m_imageNotePlaceholderBuffer);
	gtk_image_set_from_pixbuf(m_imageThumbPreview, m_imageThumbPlaceholderBuffer);

	gtk_tree_model_filter_set_visible_func(m_appTreeModelFilter, applist_visibility_func, this, NULL);

	g_signal_connect(m_window, "destroy", G_CALLBACK(gquit), this);
	g_signal_connect(m_entryFilter, "changed", G_CALLBACK(applist_filter_changed), this);
	g_signal_connect(m_mainDeviceListComboBox, "changed", G_CALLBACK(devicelist_device_changes), this);
	g_signal_connect(m_mainAppTree, "cursor-changed", G_CALLBACK(applist_row_selected), this);
	g_signal_connect(m_downloadBtn, "clicked", G_CALLBACK(button_download_clicked), this);
	g_signal_connect(m_installBtn, "clicked", G_CALLBACK(button_install_clicked), this);

	SetupMenuBarEvents();

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

	// Important: ADB device changes can be called from a background thread. It's important to use g_idle_add to do any UI updates from the main thread
	SetADBDeviceListChangedCallback(m_appContext, [](AppContext* context, void* usrData)
	{
		g_idle_add(device_list_changed_callback_event, usrData);
	}, this);
}

void MainWindow::SetupMenuBarEvents()
{
	GtkMenuItem* menuItem;

	menuItem = GTK_MENU_ITEM(gtk_builder_get_object(m_builder, "menubar_btn_preferences"));
	// continue implementation of menu bar callbacks;
}

void MainWindow::RefreshAppList()
{
	gtk_entry_set_text(m_entryFilter, "");
	gtk_list_store_clear(m_mainAppTreeListStore);

	m_selectedApp = nullptr;

	m_appList = GetAppList(m_appContext, &m_numApps);
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
	m_selectedAdbDevice = nullptr;
	gtk_list_store_clear(m_mainDeviceListStore);

	m_adbDeviceList = GetDeviceList(m_appContext, &m_numAdbDevices);

	GtkTreeIter iter;
	for(int i = 0; i < m_numAdbDevices; ++i)
	{
		gtk_list_store_append(m_mainDeviceListStore, &iter);

		std::string deviceString = m_adbDeviceList[i]->DeviceId;
		if (strlen(m_adbDeviceList[i]->Model) > 0)
		{
			deviceString = m_adbDeviceList[i]->Model + std::string(" - ") + deviceString;
		}

		if (m_adbDeviceList[i]->DeviceStatus != AdbDeviceStatus::OK)
		{
			static const std::map<AdbDeviceStatus, std::string> DEVICE_STATUS_MAP =
			{
				{ AdbDeviceStatus::NoPermissions, "No Permissions" 	},
				{ AdbDeviceStatus::Offline, "Offline" 				},
				{ AdbDeviceStatus::UnAuthorized, "Unauthorized"		},
				{ AdbDeviceStatus::Unknown, "Unknown" 				}
			};

			deviceString += "\t" + DEVICE_STATUS_MAP.at(m_adbDeviceList[i]->DeviceStatus);
		}

		gtk_list_store_set(m_mainDeviceListStore, &iter, 0, deviceString.c_str(), -1);
	}

	// Select a device by default
	if (m_numAdbDevices > 0)
	{
		gtk_combo_box_set_active(m_mainDeviceListComboBox, 0);
	}
}

void MainWindow::OnAdbDeviceListChanged()
{
	gtk_combo_box_set_active(m_mainDeviceListComboBox, -1);
	RefreshDeviceList();
}

void MainWindow::OnAdbDeviceSelectionChanged()
{
	gint index = gtk_combo_box_get_active(m_mainDeviceListComboBox);
	if (index == -1)
	{
		m_selectedAdbDevice = nullptr;
		gtk_entry_set_text(m_mainDeviceListComboBoxEntry, "");
	}
	else
	{
		m_selectedAdbDevice = m_adbDeviceList[index];
	}
	MLoaderSetSelectedAdbDevice(m_appContext, m_selectedAdbDevice);
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
		gtk_label_set_text(m_appNoteLabel, "");
		gtk_widget_set_visible(GTK_WIDGET(m_imageNotePlaceholder), true);
		return;
	}

	gtk_label_set_text(m_appNoteLabel, m_selectedApp->Note);
	gtk_widget_set_visible(GTK_WIDGET(m_imageNotePlaceholder), strlen(m_selectedApp->Note) == 0);

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
	gtk_widget_set_sensitive(GTK_WIDGET(m_installBtn), m_selectedApp->Status == AppStatus::Downloaded && m_selectedAdbDevice != nullptr && m_selectedAdbDevice->DeviceStatus == AdbDeviceStatus::OK);
}

void MainWindow::OnDownloadButtonClicked()
{
	if (!m_selectedApp) { return; } // This shouldn't occur, but just in case

	gtk_widget_set_sensitive(GTK_WIDGET(m_downloadBtn), false);
	DownloadAppAsync(m_appContext, m_selectedApp);
}

void MainWindow::OnInstallButtonClicked()
{
	if (!m_selectedApp) { return; } // This shouldn't occur, but just in case

	ShowGenericMessageDialog(GTK_WINDOW(m_window), "Install something something");

	// TODO: Begin installation
	MLoaderInstallAppAsync(m_appContext, m_selectedApp, m_selectedAdbDevice);
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

	RefreshInstallDownloadButtons();
}

void MainWindow::ClearPixBuffer()
{
	if (!m_imageThumbBuffer)
	{
		free(m_imageThumbBuffer);
		m_imageThumbBuffer = nullptr;
	}
}