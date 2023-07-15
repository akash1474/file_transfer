#include "cpr/cpr.h"
#include "DownloadManager.h"
#include "string"
#include "stack"

class Browser{
	std::string basePath{0};
	DownloadManager m_DownloadManager;
	std::vector<File> files;
	bool showDownloads=false;
	int width{0};
	int height{0};
	std::stack<std::string> stk;


	void fetchURLContent(std::string url);

public:
	Browser(const char* url,int w,int h):basePath(url),width(w),height(h){
		stk.push(basePath);
		fetchURLContent(basePath);
		// File file;
		// file.downloaded=459;
		// file.size=1000;
		// file.title="[TIF]_S03_E03_Jack_Ryan_720p_Eng.mkv";
		// file.speed=2.5;
		// file.progress=0.459;
		// m_DownloadManager.downloads.push_back(file);
		// file.title="Guardians Of The Galaxy Volume III";
		// m_DownloadManager.downloads.push_back(file);
	}

	void render();

	~Browser(){
		basePath.clear();
	}

};