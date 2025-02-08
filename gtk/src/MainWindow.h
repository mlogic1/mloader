#pragma once

#include <mloader/App.h>
#include <gtk/gtk.h>
#include <string>

struct AppContext;

class MainWindow
{
	public:
		MainWindow(AppContext* appContext);
		~MainWindow();

		// Events
		void OnAppFilterChanged();
		gboolean OnFilterFunction(GtkTreeModel *model, GtkTreeIter *iter);

	private:
		void InitializeLayout();
		void RefreshList();

	private:
		AppContext* m_appContext;
		App* m_appList = nullptr;

		GtkBuilder* m_builder;
		GtkWidget* m_window;
		GtkListStore* m_mainAppTreeListStore;
		GtkEntry* m_entryFilter;
		GtkTreeModelFilter* m_appTreeModelFilter;
		GtkTreeView* m_mainAppTree;

		static constexpr const char* LAYOUT_FILE = "data/layouts/layout_main.glade";
};
