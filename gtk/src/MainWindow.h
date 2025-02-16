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
		void OnAppSelectionChanged();
		void OnDownloadButtonClicked();
		void OnAppStatusChanged(App* app);

	private:
		void InitializeLayout();
		void SetupCallbackEvents();
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
		AdbDevice* m_adbDeviceList;
		AdbDevice* m_selectedAdbDevice;
		int m_numAdbDevices = 0;

		GtkBuilder* m_builder;
		GtkWidget* m_window;
		GtkListStore* m_mainAppTreeListStore;
		GtkListStore* m_mainDeviceListStore;
		GtkComboBox* m_mainDeviceListComboBox;
		GtkEntry* m_entryFilter;
		GtkTreeModelFilter* m_appTreeModelFilter;
		GtkTreeView* m_mainAppTree;
		GtkButton* m_downloadBtn;
		GtkButton* m_installBtn;
		GtkImage* m_imageThumbPreview;
		GdkPixbuf* m_imageThumbBuffer = nullptr;

		static constexpr const char* LAYOUT_FILE = "data/layouts/layout_main.glade";
};
