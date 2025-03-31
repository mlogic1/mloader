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

#include "model/DeviceDetails.h"
#include <mloader/VrpApp.h>
#include <mloader/AdbDevice.h>
#include <gtk/gtk.h>
#include <string>
#include <vector>

struct AppContext;

class MainWindow
{
	public:
		MainWindow(AppContext* appContext);
		~MainWindow();

		// Events
		void OnAppFilterChanged();
		void OnViewTypeChanged();
		void ResizeGridView(GtkAllocation* allocation);
		gboolean OnFilterFunction(GtkTreeModel *model, GtkTreeIter *iter);
		void OnAdbDeviceListChanged();
		void OnAdbDeviceSelectionChanged();
		void OnAppSelectionChanged();
		void OnDownloadButtonClicked();
		void OnInstallButtonClicked();
		void OnAppStatusChanged(VrpApp* app);
		void OnMenuBarClearDownloadsClicked();

	private:
		void InitializeLayout();
		void SetupCallbackEvents();
		void SetupMenuBarEvents();
		void RefreshAppList();
		void RefreshDeviceList();
		void RefreshInstallDownloadButtons();
		void RefreshAppDetailsPane();
		void RefreshDeviceDetailsPane();

		void ClearPixBuffer();

	private:
		AppContext* m_appContext;
		VrpApp** m_appList = nullptr;
		int m_numApps = 0;
		VrpApp* m_selectedApp = nullptr;
		AdbDevice** m_adbDeviceList = nullptr;
		AdbDevice* m_selectedAdbDevice = nullptr;
		int m_numAdbDevices = 0;

		GtkBuilder* m_builder;
		GtkWidget* m_window;
		GtkListStore* m_mainAppTreeListStore;
		GtkListStore* m_mainDeviceListStore;
		GtkComboBox* m_mainDeviceListComboBox;
		GtkEntry* m_mainDeviceListComboBoxEntry;
		GtkEntry* m_entryFilter;
		GtkTreeModelFilter* m_appTreeModelFilter;
		GtkComboBox* m_mainViewTypeCombo;
		GtkTreeView* m_mainAppTree;
		GtkScrolledWindow* m_mainAppTreeScrolledWindow;
		GtkScrolledWindow* m_mainAppTreeScrolledWindowGrid;
		GtkFlowBox* m_mainAppGrid;
		GtkWidget* m_gridViewBoxParent;
		GtkStack* m_mainContentStack;
		GtkButton* m_downloadBtn;
		GtkButton* m_installBtn;
		GtkImage* m_imageThumbPreview;
		GdkPixbuf* m_imageThumbBuffer = nullptr;
		GtkImage* m_imageNotePlaceholder;
		GdkPixbuf* m_imageNotePlaceholderBuffer = nullptr;
		GdkPixbuf* m_imageThumbPlaceholderBuffer = nullptr;
		GtkLabel* m_appNoteLabel;

		// Details pane
		std::vector<DeviceDetails> m_detailsObjects =
		{
			{	"ro.product.manufacturer",	"device_details_label_body_manufacturer",	nullptr	},
			{	"ro.product.model",			"device_details_label_body_model", 			nullptr	},
			{	"ro.serialno",				"device_details_label_body_serialno",		nullptr	},
			{	"ro.build.version.release",	"device_details_label_body_androidver",		nullptr	},
			{	"ro.build.version.sdk",		"device_details_label_body_sdkver",			nullptr	},
			{	"ro.build.branch",			"device_details_label_body_branchver",		nullptr	}
		};

		static constexpr const char* LAYOUT_RESOURCE = "/mlres/layouts/layout_main.glade";
		static constexpr const char* GRIDVIEW_ELEMENT_RESOURCE = "/mlres/layouts/grid_view_element.ui";
};