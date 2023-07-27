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
	bool showConnectionError=false;
	bool showHomePage=true;
	bool showDownloads=false;
	bool showSearchBar=false;
	bool showHelp=false;
	bool showChangeUrl=false;
    bool themeSelected[3] = {0, 0, 0};
    int usrTheme=0;
    bool showFps=false;
    bool vSyncEnabled=true;
    bool settingsUpdate=false;
    std::string downloadsLocation{0};
    bool defaultDownloadLocation=true;
    std::vector<std::string> IPs;
    mINI::INIFile* file{0};
    mINI::INIStructure ini;


	float width{400};
	float height{600};
	std::string usrRootDir{0};
	std::stack<std::string> stk;
	std::future<bool> fetchFuture;
	std::vector<std::string> paths;
    char query[100]="";

	bool fetchURLContent(std::string url);
	void keyBindings(int& selected,int max);
	void globalKeyBindings();
	void showChangeUrlPopUp();
	void renderMenuBar();
	bool initBrowser(std::string url);
	void renderHomePage();

public:
	Browser();
	bool shouldCloseWindow=false;
	void render();
	void renderSearch();
	void saveSettings();
};