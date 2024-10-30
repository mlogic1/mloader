#ifndef APP_CONTEXT_H
#define APP_CONTEXT_H

namespace mloader
{
	typedef struct AppContext AppContext;

	AppContext* CreateLoaderContext(const char* customCacheDir = "", const char* customDownloadDir = "");
	void DestroyLoaderContext(AppContext* handle);


	void RefreshMetadata(AppContext* context);
}


#endif //  APP_CONTEXT_H