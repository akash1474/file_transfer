#include "cpr/cpr.h"
#include "DownloadManager.h"
#include "string"
#include "stack"
#include <future>
#include "ini.h"

class Browser{
	std::string basePath{0};
	DownloadManager m_DownloadManager;
	std::vector<File> files;

	//Settings
	bool showDownloads=false;
	bool showSearchBar=false;
	bool showChangeUrl=false;
    bool themeSelected[3] = {0, 0, 0};
    int usrTheme=0;
    bool showFps=false;
    bool vSyncEnabled=true;
    bool settingsUpdate=false;
    std::string downloadsLocation{0};
    bool defaultDownloadLocation=false;
    std::vector<std::string> IPs;
    mINI::INIFile* file{0};
    mINI::INIStructure ini;


	float width{0};
	float height{0};
	std::stack<std::string> stk;
	std::future<bool> fetchFuture;
	std::vector<std::string> paths;
    char query[100];

	bool fetchURLContent(std::string url);
	void keyBindings(int& selected,int max);
	void globalKeyBindings();
	void showChangeUrlPopUp();
	void renderMenuBar();
public:
	Browser(const char* url,int w,int h);

	void render();
	void renderSearch();
	void saveSettings();

	~Browser(){
		basePath.clear();
		saveSettings();
	}

};