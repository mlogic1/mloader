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

#include <mloader/AdbDevice.h>
#include <gtk/gtk.h>
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

struct SAContext;

class StandaloneWindow
{
	public:
		StandaloneWindow(SAContext* saContext);
		~StandaloneWindow();

		// Events
		void OnAdbDeviceListChanged();
		void OnAdbDeviceSelectionChanged();
		void OnDragAndDropFinish(const std::vector<fs::path>& files);

	private:
		void InitializeLayout();
		void RefreshDeviceList();
		void SetupCallbackEvents();
		void SetupMenuBarEvents();
		/*void RefreshAppList();

		void RefreshInstallDownloadButtons();
		void RefreshDetailsPane();*/

	private:
		SAContext* m_saContext;
		AdbDevice** m_adbDeviceList = nullptr;
		AdbDevice* m_selectedAdbDevice = nullptr;
		int m_numAdbDevices = 0;

		GtkListStore* m_mainDeviceListStore;
		GtkComboBox* m_mainDeviceListComboBox;
		GtkEntry* m_mainDeviceListComboBoxEntry;
		GtkEntry* m_obbPackageEntry;

		GtkBuilder* m_builder;
		GtkWidget* m_window;

		static constexpr const char* LAYOUT_RESOURCE = "/mlres/layouts/layout_standalone.glade";
};
