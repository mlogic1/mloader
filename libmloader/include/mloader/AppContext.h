#ifndef APP_CONTEXT_H
#define APP_CONTEXT_H

#include "App.h"
#include "AdbDevice.h"
#include <stddef.h>

typedef struct AppContext AppContext;

typedef void (* CreateLoaderContextStatusCallback)(const char*);
typedef void (* CreateLoaderContextAsyncCompletedCallback)(AppContext*);
typedef void (* RefreshMetadataAsyncCompletedCallback)(AppContext*);
typedef void (* ADBDeviceListChangedCallback)(AppContext*, void*);
typedef void (* AppStatusChangedCallback)(AppContext*, App*, void*);

AppContext* CreateLoaderContext(CreateLoaderContextStatusCallback callback = 0, const char* customCacheDir = "", const char* customDownloadDir = "");
void CreateLoaderContextAsync(CreateLoaderContextAsyncCompletedCallback completedCallback, CreateLoaderContextStatusCallback callback = NULL, const char* customCacheDir = "", const char* customDownloadDir = "");
void DestroyLoaderContext(AppContext* context);

void RefreshMetadata(AppContext* context);
void RefreshMetadataAsync(RefreshMetadataAsyncCompletedCallback completedCallback, AppContext* context);
App** GetAppList(AppContext* context, int* num);
bool DownloadApp(AppContext* context, App* app);
void DownloadAppAsync(AppContext* context, App* app);
bool MLoaderInstallApp(AppContext* context, App* app, AdbDevice* device);
void MLoaderInstallAppAsync(AppContext* context, App* app, AdbDevice* device);
AdbDevice** GetDeviceList(AppContext* context, int* num);
void SetADBDeviceListChangedCallback(AppContext* context, ADBDeviceListChangedCallback callback, void* userData = NULL);
void ClearADBDeviceListChangedCallback(AppContext* context);
void SetAppStatusChangedCallback(AppContext* context, AppStatusChangedCallback callback, void* userData = NULL);
void ClearAppStatusChangedCallback(AppContext* context);

char* GetAppThumbImage(AppContext* context, App* app);

#endif //  APP_CONTEXT_H
