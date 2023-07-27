#include "cpr/cpr.h"
#include "structs.h"
#include "vector"
#include "string"
#include "future"

class DownloadManager{
public:
	std::string path{0};
	std::vector<DFile*> downloads;
	std::vector<std::future<bool>> futures;
	DownloadManager(){}
	bool initDownload(DFile* file=nullptr);
	void addDownload(File* file=nullptr);
	bool DownloadComponent(DFile* file=nullptr);
	bool DComponent(DFile* file);
	void render();
};