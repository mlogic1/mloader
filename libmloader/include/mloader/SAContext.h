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

#ifndef STANDALONE_CONTEXT_H
#define STANDALONE_CONTEXT_H

#include "AdbDevice.h"
#include <stddef.h>

typedef struct SAContext SAContext;

typedef void (* CreateLoaderContextStatusCallback)(const char*);
typedef void (* CreateSALoaderContextAsyncCompletedCallback)(SAContext*);
typedef void (* SAADBDeviceListChangedCallback)(SAContext*, void*);

const char* MLoaderSAGetErrorMessage();
SAContext* MLoaderSACreateLoaderContext(CreateLoaderContextStatusCallback callback = 0, const char* customCacheDir = "");
void MLoaderSACreateLoaderContextAsync(CreateSALoaderContextAsyncCompletedCallback completedCallback, CreateLoaderContextStatusCallback callback = NULL, const char* customCacheDir = "");
void MLoaderSADestroyLoaderContext(SAContext* context);
bool MLoaderSAInstallFile(SAContext* context, const char* apkFile, AdbDevice* device, const char* obbPackageName);
AdbDevice** MLoaderSAGetDeviceList(SAContext* context, int* num);
void MLoaderSASetSelectedAdbDevice(SAContext* context, AdbDevice* device);
void MLoaderSASetADBDeviceListChangedCallback(SAContext* context, SAADBDeviceListChangedCallback callback, void* userData = NULL);
void MLoaderSAClearADBDeviceListChangedCallback(SAContext* context);

extern char* MLoaderGetLibraryVersion();

#endif //  STANDALONE_CONTEXT_H
