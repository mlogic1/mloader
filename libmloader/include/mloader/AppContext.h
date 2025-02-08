#ifndef APP_CONTEXT_H
#define APP_CONTEXT_H

#include "App.h"
#include <stddef.h>

typedef struct AppContext AppContext;

typedef void (* CreateLoaderContextStatusCallback)(const char*);
typedef void (* CreateLoaderContextAsyncCompletedCallback)(AppContext*);
typedef void (* RefreshMetadataAsyncCompletedCallback)(AppContext*);

AppContext* CreateLoaderContext(CreateLoaderContextStatusCallback callback = 0, const char* customCacheDir = "", const char* customDownloadDir = "");
void CreateLoaderContextAsync(CreateLoaderContextAsyncCompletedCallback completedCallback, CreateLoaderContextStatusCallback callback = NULL, const char* customCacheDir = "", const char* customDownloadDir = "");
void DestroyLoaderContext(AppContext* handle);

void RefreshMetadata(AppContext* context);
void RefreshMetadataAsync(RefreshMetadataAsyncCompletedCallback completedCallback, AppContext* context);
void GetAppList(AppContext* context, App** app, int* num);
void FreeAppList(App* app);
bool DownloadApp(AppContext* context, App* app);


#endif //  APP_CONTEXT_H