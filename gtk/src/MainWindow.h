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

#include <mloader/App.h>
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
		gboolean OnFilterFunction(GtkTreeModel *model, GtkTreeIter *iter);
		void OnAdbDeviceListChanged();
		void OnAdbDeviceSelectionChanged();
		void OnAppSelectionChanged();
		void OnDownloadButtonClicked();
		void OnInstallButtonClicked();
		void OnAppStatusChanged(App* app);
		void OnMenuBarAboutButtonClicked();

	private:
		void InitializeLayout();
		void SetupCallbackEvents();
		void SetupMenuBarEvents();
		void RefreshAppList();
		void RefreshDeviceList();
		void RefreshInstallDownloadButtons();
		void RefreshDetailsPane();

		void ClearPixBuffer();

	private:
		AppContext* m_appContext;
		App** m_appList = nullptr;
		int m_numApps = 0;
		App* m_selectedApp = nullptr;
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
		GtkTreeView* m_mainAppTree;
		GtkButton* m_downloadBtn;
		GtkButton* m_installBtn;
		GtkImage* m_imageThumbPreview;
		GdkPixbuf* m_imageThumbBuffer = nullptr;
		GtkImage* m_imageNotePlaceholder;
		GdkPixbuf* m_imageNotePlaceholderBuffer = nullptr;
		GdkPixbuf* m_imageThumbPlaceholderBuffer = nullptr;
		GtkLabel* m_appNoteLabel;

		static constexpr const char* LAYOUT_RESOURCE = "/mlres/layouts/layout_main.glade";
};
