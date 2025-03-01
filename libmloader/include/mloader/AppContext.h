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
bool MLoaderInstallApp(AppContext* context, App* app, AdbDevice* device);
void MLoaderDeleteApp(AppContext* context, App* app);
AdbDevice** GetDeviceList(AppContext* context, int* num);
char* MLoaderGetDeviceProperty(AppContext* context, AdbDevice* device, const char* propertyName);
void MLoaderSetSelectedAdbDevice(AppContext* context, AdbDevice* device);
void SetADBDeviceListChangedCallback(AppContext* context, ADBDeviceListChangedCallback callback, void* userData = NULL);
void ClearADBDeviceListChangedCallback(AppContext* context);
void SetAppStatusChangedCallback(AppContext* context, AppStatusChangedCallback callback, void* userData = NULL);
void ClearAppStatusChangedCallback(AppContext* context);

char* GetAppThumbImage(AppContext* context, App* app);

char* MLoaderGetLibraryVersion();

#endif //  APP_CONTEXT_H
