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
	bool showSearchBar=false;
	bool showChangeUrl=false;
	float width{0};
	float height{0};
	std::stack<std::string> stk;
	std::future<bool> fetchFuture;
	std::vector<std::string> paths;
    char query[100];

	bool fetchURLContent(std::string url);
	void keyBindings(int& selected,int max);
	void globalKeyBindings();
public:
	Browser(const char* url,int w,int h):basePath(url),width(w),height(h){
		stk.push(basePath);
		paths.push_back("Root");
		fetchFuture=std::async(std::launch::async,&Browser::fetchURLContent,this,basePath);
	}

	void render();
	void renderSearch();

	~Browser(){
		basePath.clear();
	}

};