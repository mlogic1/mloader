#ifndef APP_CONTEXT_H
#define APP_CONTEXT_H

#include "App.h"

typedef struct AppContext AppContext;

AppContext* CreateLoaderContext(const char* customCacheDir = "", const char* customDownloadDir = "");
void DestroyLoaderContext(AppContext* handle);


void RefreshMetadata(AppContext* context);
void GetAppList(AppContext* context, App** app, int* num);
void FreeAppList(App* app);
bool DownloadApp(AppContext* context, App* app);


#endif //  APP_CONTEXT_H