#include "cpr/cpr.h"
#include "structs.h"
#include "vector"
#include "string"
#include "future"

class DownloadManager{
	std::string path{0};
public:
	std::vector<File*> downloads;
	std::vector<std::future<bool>> futures;
	DownloadManager(){}
	bool initDownload(File* file=nullptr);
	void addDownload(File* file=nullptr);
	bool DownloadComponent(File* file=nullptr);
	void render();
	~DownloadManager(){
		path.clear();
		downloads.clear();
	}
};