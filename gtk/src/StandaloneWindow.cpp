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

#include "StandaloneWindow.h"
#include "AboutWindow.h"
#include "GtkGeneric.h"
#include <functional>
#include <map>
#include <mloader/SAContext.h>

static void gquit(GtkWidget* widget, gpointer data)
{
	gtk_main_quit();
}

// The following static functions serve as a binding between GTK C api and this cpp class

static void devicelist_device_changes(GtkWidget* widget, gpointer data)
{
	StandaloneWindow* mainWindow = static_cast<StandaloneWindow*>(data);
	mainWindow->OnAdbDeviceSelectionChanged();
}

static gboolean device_list_changed_callback_event(gpointer data)
{
	StandaloneWindow* mainWindow = static_cast<StandaloneWindow*>(data);
	mainWindow->OnAdbDeviceListChanged();
	return false;
}

StandaloneWindow::StandaloneWindow(SAContext* saContext)
	: 	m_saContext(saContext),
		m_adbDeviceList(nullptr),
		m_selectedAdbDevice(nullptr)
{
	InitializeLayout();
}

StandaloneWindow::~StandaloneWindow()
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
}

static void on_files_drop(GtkWidget *widget, GdkDragContext *context, gint x, gint y, GtkSelectionData *data, guint info, guint time, gpointer user_data)
{
	StandaloneWindow* window = static_cast<StandaloneWindow*>(user_data);
	std::vector<fs::path> files;
	gchar **uris = gtk_selection_data_get_uris(data);
	if (uris != NULL) {
		for (int i = 0; uris[i] != NULL; i++) {
			gchar *filename = g_filename_from_uri(uris[i], NULL, NULL);
			if (filename != NULL) {
				files.push_back(filename);
				g_free(filename);
			}
			g_free(uris[i]);
		}
		g_free(uris);
	}
	gtk_drag_finish(context, TRUE, FALSE, time);
	window->OnDragAndDropFinish(files);
}

void StandaloneWindow::InitializeLayout()
{
	m_builder						= gtk_builder_new_from_resource(LAYOUT_RESOURCE);
	m_window 						= GTK_WIDGET(gtk_builder_get_object(m_builder, "standalone_window"));
	m_mainDeviceListStore			= GTK_LIST_STORE(gtk_builder_get_object(m_builder, "liststoreDeviceList"));
	m_mainDeviceListComboBox		= GTK_COMBO_BOX(gtk_builder_get_object(m_builder, "main_combo_device_list"));
	m_mainDeviceListComboBoxEntry 	= GTK_ENTRY(gtk_builder_get_object(m_builder, "main_combo_device_list_entry"));
	m_obbPackageEntry				= GTK_ENTRY(gtk_builder_get_object(m_builder, "box_controls_obb_package"));
	GtkWidget* dropZone				= GTK_WIDGET(gtk_builder_get_object(m_builder, "image_drop_zone"));

	gtk_drag_dest_unset(GTK_WIDGET(m_obbPackageEntry));
	gtk_drag_dest_unset(GTK_WIDGET(m_mainDeviceListComboBoxEntry));
	gtk_drag_dest_set(dropZone, GTK_DEST_DEFAULT_ALL, NULL, 0, GDK_ACTION_COPY);
	gtk_drag_dest_add_uri_targets(dropZone);

	g_signal_connect(m_window, "destroy", G_CALLBACK(gquit), this);
	g_signal_connect(m_mainDeviceListComboBox, "changed", G_CALLBACK(devicelist_device_changes), this);
	g_signal_connect(dropZone, "drag-data-received", G_CALLBACK(on_files_drop), this);

	SetupMenuBarEvents();

	gtk_builder_connect_signals(m_builder, NULL);

	gtk_widget_show_all(m_window);
	gtk_window_present(GTK_WINDOW (m_window));

	SetupCallbackEvents();
	RefreshDeviceList();
}

void StandaloneWindow::SetupCallbackEvents()
{
	// Important: ADB device changes can be called from a background thread. It's important to use g_idle_add to do any UI updates from the main thread
	MLoaderSASetADBDeviceListChangedCallback(m_saContext, [](SAContext* context, void* usrData)
	{
		g_idle_add(device_list_changed_callback_event, usrData);
	}, this);
}

void StandaloneWindow::SetupMenuBarEvents()
{
	GtkMenuItem* menuItem;

	menuItem = GTK_MENU_ITEM(gtk_builder_get_object(m_builder, "menubar_btn_about"));
	auto aboutButtonCallback = +[](GtkWidget* widget, gpointer data)
	{
		GtkWindow* window = static_cast<GtkWindow*>(data);
		AboutWindow aboutWindow(window);
	};
	g_signal_connect(menuItem, "activate", G_CALLBACK(aboutButtonCallback), m_window);

	menuItem = GTK_MENU_ITEM(gtk_builder_get_object(m_builder, "menubar_btn_quit"));
	auto quitButtonCallback = +[](GtkWidget* widget, gpointer data)
	{
		GtkWidget* mainWindow = static_cast<GtkWidget*>(data);
		gtk_widget_destroy(mainWindow);
	};
	g_signal_connect(menuItem, "activate", G_CALLBACK(quitButtonCallback), m_window);
}

void StandaloneWindow::RefreshDeviceList()
{
	m_selectedAdbDevice = nullptr;
	gtk_list_store_clear(m_mainDeviceListStore);

	m_adbDeviceList = MLoaderSAGetDeviceList(m_saContext, &m_numAdbDevices);

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
				{ AdbDeviceStatus::NoPermissions,	"No Permissions"	},
				{ AdbDeviceStatus::Offline,			"Offline" 			},
				{ AdbDeviceStatus::UnAuthorized,	"Unauthorized"		},
				{ AdbDeviceStatus::Unknown,			"Unknown"			}
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

void StandaloneWindow::OnAdbDeviceListChanged()
{
	gtk_combo_box_set_active(m_mainDeviceListComboBox, -1);
	RefreshDeviceList();
}

void StandaloneWindow::OnAdbDeviceSelectionChanged()
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
}

void StandaloneWindow::OnDragAndDropFinish(const std::vector<fs::path>& files)
{
	if (m_selectedAdbDevice == nullptr)
	{
		ShowGenericMessageDialog(GTK_WINDOW(m_window), "Please select a device to which the packages will be installed.");
	}

	for (const fs::path& file : files)
	{
		if(file.extension() == ".apk")
		{
			if (!MLoaderSAInstallFile(m_saContext, file.c_str(), m_selectedAdbDevice, ""))
			{
				ShowGenericMessageDialog(GTK_WINDOW(m_window), MLoaderSAGetErrorMessage());
			}
		}
		else if (file.extension() == ".obb")
		{
			std::string obbPackage{gtk_entry_get_text(m_obbPackageEntry)};
			if (obbPackage.empty())
			{
				std::string errMsg = "OBB Package (directory name) must be specified. It's the directory under /sdcard/Android/obb/ where your .obb file is going to be placed.";
				ShowGenericMessageDialog(GTK_WINDOW(m_window), errMsg);				
				return;
			}
			if (!MLoaderSAInstallFile(m_saContext, file.c_str(), m_selectedAdbDevice, obbPackage.c_str()))
			{
				ShowGenericMessageDialog(GTK_WINDOW(m_window), MLoaderSAGetErrorMessage());
			}
		}
		else
		{
			std::string errMsg = "Unknown file extension for file " + file.string();
			ShowGenericMessageDialog(GTK_WINDOW(m_window), errMsg);
		}
	}
}