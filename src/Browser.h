#include "cpr/cpr.h"
#include "DownloadManager.h"
#include "string"
#include "stack"
#include <future>

class Browser{
	std::string basePath{0};
	DownloadManager m_DownloadManager;
	std::vector<File> files;
	bool showDownloads=false;
	int width{0};
	int height{0};
	std::stack<std::string> stk;
	std::future<bool> fetchFuture;
	std::vector<std::string> paths;

	bool fetchURLContent(std::string url);

public:
	Browser(const char* url,int w,int h):basePath(url),width(w),height(h){
		stk.push(basePath);
		paths.push_back("Root");
		fetchFuture=std::async(std::launch::async,&Browser::fetchURLContent,this,basePath);
	}

	void render();

	~Browser(){
		basePath.clear();
	}

};