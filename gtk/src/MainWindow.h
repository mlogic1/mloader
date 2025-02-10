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
		void OnAdbDeviceSelectionChanged();

	private:
		void InitializeLayout();
		void SetupCallbackEvents();
		void RefreshAppList();
		void RefreshDeviceList();
		void RefreshInstallDownloadButtons();

	private:
		AppContext* m_appContext;
		App* m_appList = nullptr;
		AdbDevice* m_adbDeviceList;
		AdbDevice* m_selectedAdbDevice;
		int m_numAdbDevices;

		GtkBuilder* m_builder;
		GtkWidget* m_window;
		GtkListStore* m_mainAppTreeListStore;
		GtkListStore* m_mainDeviceListStore;
		GtkComboBox* m_mainDeviceListComboBox;
		GtkEntry* m_entryFilter;
		GtkTreeModelFilter* m_appTreeModelFilter;
		GtkTreeView* m_mainAppTree;

		static constexpr const char* LAYOUT_FILE = "data/layouts/layout_main.glade";
};
