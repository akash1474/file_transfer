#include "FontAwesome6.h"
#include "GLFW/glfw3.h"
#include "images.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "pch.h"
#include "Browser.h"
#include <algorithm>
#include <iterator>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vcruntime.h>
#include <vcruntime_string.h>


Browser::Browser(){
    std::string userFolder(getenv("USERPROFILE"));
    std::replace(userFolder.begin(), userFolder.end(), '\\', '/');
    std::string savePath=userFolder+"/file_transer";
    if(!std::filesystem::exists(savePath)) std::filesystem::create_directory(savePath);
    savePath+="/ft.ini";
    file=new mINI::INIFile(savePath);
    file->read(ini);
    if(!std::filesystem::exists(savePath)){
        //Initilze
        std::cout << "Initializing Settings" << std::endl;
        ini["settings"]["fps"]="0";
        ini["settings"]["vsync"]="1";
        ini["settings"]["dloc"]=(userFolder+"/Downloads/File Transfer");
        this->downloadsLocation=userFolder+"/Downloads/File Transfer";
        ini["settings"]["default"]="0";
        ini["settings"]["theme"]="0";
        themeSelected[0]=1;
        file->write(ini,true);
    }else{
        //Load
        std::cout << "Loading Settings" << std::endl;
        if(!ini.has("settings")) ini["settings"];
        this->showFps= ini["settings"].has("fps") ? stoi(ini["settings"]["fps"]): false;
        this->vSyncEnabled= ini["settings"].has("vsync") ? stoi(ini["settings"]["vsync"]): true;
        if(!vSyncEnabled) glfwSwapInterval(0);
        this->downloadsLocation= ini["settings"].has("dloc") ? ini["settings"]["dloc"]: (userFolder+"/Downloads/File Transfer");
        if(!std::filesystem::exists(downloadsLocation)) downloadsLocation=userFolder+"/Downloads/File Transfer";
        this->defaultDownloadLocation= ini["settings"].has("default") ? stoi(ini["settings"]["default"]): false;
        usrTheme= ini["settings"].has("theme") ? stoi(ini["settings"]["theme"]): 0;
        if( 0 > usrTheme || usrTheme > 2) usrTheme=0;
        this->themeSelected[usrTheme]=1;
        
        switch(usrTheme){
        case 0:
            ImGui::StyleColorsDark();
            break;
        case 1:
            ImGui::StyleColorsLight();
            break;
        case 2:
            ImGui::StyleColorsClassic();
            break;
        }
        if(ini.has("ips")){
            for(auto const& it:ini["ips"]){
                this->IPs.push_back(it.second);
                std::cout << it.second << std::endl;
            }
        }
        saveSettings();
    }
}

void processVector(std::string inputStr, std::vector<std::string>& strVec) {
    if(strVec.empty()){
        strVec.push_back(inputStr);
        return;
    }
    auto it = std::find(strVec.begin(), strVec.end(), inputStr);
    if (it != strVec.end()) {
        std::rotate(strVec.begin(), it, it + 1);
    } else {
        strVec.insert(strVec.begin(), inputStr);
        if (strVec.size() > 3) strVec.pop_back();
    }
}

std::string modifyUrl(const std::string& inputUrl) {
    std::regex urlRegex(R"((\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}):(\d+)/?)");
    std::smatch match;
    if (std::regex_search(inputUrl, match, urlRegex)) {
        if (match.size() == 3) {
            std::string ip = match[1].str();
            std::string port = match[2].str();
            return "http://" + ip + ":" + port + "/";
        }
    }
    return "";
}

bool Browser::initBrowser(std::string url){
    std::string temp=basePath;
    basePath=modifyUrl(url);
    if(fetchURLContent(url)){
        while(!stk.empty()) stk.pop();
        paths.clear();
        stk.push(basePath);
        paths.push_back("Root");
        processVector(basePath, IPs);
        char ip[4]="";
        for(int i=0;i<(int)IPs.size();i++){
            sprintf(ip, "ip%d",i);
            ini["ips"][ip]=IPs[i];
        }
        file->write(ini,true);
        return true;
    }
    basePath=temp;
    return false;
}


void Browser::saveSettings(){
    std::cout << "Saving Settings" << std::endl;
    ini["settings"]["fps"]=this->showFps ? "1" : "0";
    ini["settings"]["vsync"]=this->vSyncEnabled ? "1":"0";
    ini["settings"]["dloc"]=this->downloadsLocation;
    ini["settings"]["default"]=this->defaultDownloadLocation ? "1": "0";
    ini["settings"]["theme"]=std::to_string(usrTheme);
    file->write(ini,true);
    this->settingsUpdate=false;
}



bool Browser::fetchURLContent(std::string url)
{
    std::cout << "Fetching: " << url << std::endl;
    cpr::Response response = cpr::Get(cpr::Url(url));

    if (response.status_code != 200) {
        std::cerr << "Error: Failed to fetch URL. Status code: " << response.status_code << std::endl;
        std::cerr << "Error: " << url << std::endl;
        return false;
    }
    std::regex anchorRegex("<a\\s+href=\"([^\"]+)\">([^<]+)</a>");
    std::smatch match;

    this->files.clear();
    auto it = response.text.cbegin();
    while (std::regex_search(it, response.text.cend(), match, anchorRegex)) {
        std::string href = match[1].str();
        std::string text = match[2].str();
        File file;
        if (text == "..") {
            file.isFolder = true;
            file.title = text;
            this->files.push_back(file);
            it = match.suffix().first;
            continue;
        }

        if (text[text.length() - 1] == '/') {
            file.isFolder = true;
            file.title = text;
            file.location = (this->basePath + href);
        } else {
            file.title = text;
            file.location = (this->basePath + href);
        }
        this->files.push_back(file);
        it = match.suffix().first;
    }
    std::stable_partition(files.begin(), files.end(), [](const File& file) { return file.isFolder; });
    return true;
}


void Browser::renderMenuBar()
{
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Menu")) {
            if(ImGui::MenuItem("Change IP/URL",0,false,!showHomePage)) showChangeUrl=true;
            if(ImGui::MenuItem("Find File",0,false,!showHomePage)) showSearchBar=true;
            if (ImGui::BeginMenu("Theme")) {
                if (ImGui::MenuItem("Dark",0, themeSelected[0])){
                    memset(themeSelected,0,sizeof(themeSelected));
                    themeSelected[0]=true;
                    usrTheme=0;
                    ImGui::StyleColorsDark();
                    this->settingsUpdate=true;
                }
                if (ImGui::MenuItem("Light",0, themeSelected[1])){
                    memset(themeSelected,0,sizeof(themeSelected));
                    usrTheme=1;
                    themeSelected[1]=true;
                    this->settingsUpdate=true;
                    ImGui::StyleColorsLight();
                }
                if (ImGui::MenuItem("Classic",0, themeSelected[2])){
                    memset(themeSelected,0,sizeof(themeSelected));
                    this->settingsUpdate=true;
                    themeSelected[2]=true;
                    usrTheme=2;
                    ImGui::StyleColorsClassic();
                }
                ImGui::EndMenu();
            }
            ImGui::MenuItem("Check Update");
            ImGui::MenuItem("Exit");
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Options")) {
            if(ImGui::MenuItem("Show FPS",0,showFps)){
                showFps=!showFps;
                this->settingsUpdate=true;
            }
            if(ImGui::MenuItem("Vsync",0,vSyncEnabled)){
                vSyncEnabled=!vSyncEnabled;
                this->settingsUpdate=true;
                vSyncEnabled ? glfwSwapInterval(1) :  glfwSwapInterval(0);
            }
            if(ImGui::BeginMenu("Downloads")){
                if(ImGui::MenuItem("Current Folder",0,!defaultDownloadLocation)){
                    defaultDownloadLocation=false;
                    settingsUpdate=true;
                }
                if(ImGui::MenuItem("Downloads",0,defaultDownloadLocation)){
                    defaultDownloadLocation=true;
                    settingsUpdate=true;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help",true)) {
            ImGui::Spacing();
            ImGui::Text("©Akash Pandit. All Rights Reserved");
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Text("Version: 0.1.2");
            ImGui::Text("Contact: panditakash38@gmail.com");
            ImGui::Separator();
            ImGui::MenuItem("Help","F1");
            ImGui::EndMenu();
        }
        if(showFps){
            static char fps[16];
            sprintf_s(fps,"%.2f FPS",ImGui::GetIO().Framerate);
            static float width=(ImGui::GetContentRegionAvail().x-ImGui::CalcTextSize(fps).x)-10;
            ImGui::Dummy({width,10});
            ImGui::Text("%s", fps);
        }
        ImGui::EndMenuBar();
    }
}

void showConnectionErrorUI(){
    bool openPopup = true;
    if (ImGui::BeginPopupModal("Connection Error", &openPopup, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Unable to connect the device");
        ImGui::Separator();
        ImGui::SetCursorPos(ImVec2{(ImGui::GetWindowWidth() - 50) * 0.5f, ImGui::GetCursorPosY()});
        if (ImGui::Button("OK", ImVec2(50, 0))) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
}

void Browser::showChangeUrlPopUp(){
    bool showPopUp = true;
    if (ImGui::BeginPopupModal("Change URL", &showPopUp, ImGuiWindowFlags_AlwaysAutoResize)) {
        static bool isSuccess=true;
        this->showChangeUrl=false;
        ImGui::Text("Enter the new IP or select from below");
        ImGuiIO& io=ImGui::GetIO();
        static char ip[64]="";
        if(ImGui::InputText("##ip_input",ip,IM_ARRAYSIZE(ip),ImGuiInputTextFlags_EnterReturnsTrue) || ImGui::Button("Open",ImVec2{60,0})){
            if(initBrowser(std::string(ip))){
                memset(ip,0,IM_ARRAYSIZE(ip));
                isSuccess=true;
                ImGui::CloseCurrentPopup();
            }else{
                isSuccess=false;
            }
        }
        if(!IPs.empty()){
            ImGui::Text("Recent URL/IP");
            for(const auto& ip:IPs){
                if(ImGui::Button(ip.c_str(),ImVec2{250,0})){
                    if(initBrowser(ip)){
                        ImGui::CloseCurrentPopup();                        
                    }else{
                        isSuccess=false;
                    }
                }
            }
        }

        if(!isSuccess){
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(255, 119, 0, 255),"Invalid URL/IP");
        }
        ImGui::EndPopup();
    }

}

void Browser::globalKeyBindings(){
    if(ImGui::IsMouseClicked(ImGuiMouseButton_Right)){
        this->showDownloads=!this->showDownloads;
    }
    if(ImGui::IsKeyPressed(ImGuiKey_Escape)){
        this->showDownloads=false;
        this->showSearchBar=false;
    }
    if(ImGui::IsKeyPressed(ImGuiKey_Slash)){
        this->showSearchBar=true;
    }
    if(ImGui::IsKeyPressed(ImGuiKey_F2)){
        this->showChangeUrl=true;
    }
    if(ImGui::IsKeyPressed(ImGuiKey_F5)){
        if(!fetchURLContent(stk.top())){
            this->showConnectionError=true; 
        }
    }
}


void Browser::keyBindings(int& selected,int max){
    if(ImGui::IsKeyDown(ImGuiKey_ModCtrl) && ImGui::IsKeyPressed(ImGuiKey_C)){
        clipboardxx::clipboard clipboard;
        clipboard << this->files[selected].location;
    }

    if(selected==-1) selected=0;
    if(ImGui::IsKeyPressed(ImGuiKey_DownArrow)){
        selected++;
        if(selected > 20) ImGui::SetScrollY(selected*20);
        if(selected==max){
            selected=0;
            ImGui::SetScrollY(0.0f);
        }
    }
    if(ImGui::IsKeyPressed(ImGuiKey_UpArrow)){
        selected--;
        if(selected >= 0) ImGui::SetScrollY(selected*15);
        if(selected==-1){
            selected=max-1;
            ImGui::SetScrollY(ImGui::GetScrollMaxY());
        }
    }
}


char* toLower(char* s)
{
    for (char* p = s; *p; p++) *p = tolower(*p);
    return s;
}

std::vector<File> searchFile(std::string sEl,std::vector<File>& files)
{
    static const size_t npos = -1;
    int idx=0;
    std::vector<File> foundFiles;
    for (const auto el : files) {
        std::string title=el.title;
        if (!el.isFolder && std::string(toLower((char*)title.c_str())).find(toLower((char*)sEl.c_str())) != npos) {
            foundFiles.push_back(el);
        }
        idx++;
    }
    return foundFiles;
}

void Browser::renderSearch(){
    static bool isFocused=false;
    ImGui::SetNextWindowPos({0, this->height-30.0f});
    ImGui::SetNextWindowSize({(float)this->width,30.0f});
    ImGui::Begin("##SearchBar", 0,
                 ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoScrollbar| ImGuiWindowFlags_NoScrollWithMouse);

    ImGui::Text(ICON_FA_MAGNIFYING_GLASS); ImGui::SameLine();
    if(!isFocused && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0)){
        ImGui::SetKeyboardFocusHere(0);
        isFocused=true;
    }
    ImGui::InputText("##search_text", query, IM_ARRAYSIZE(query));
    ImGui::SameLine();
    if(ImGui::Button("Close",ImVec2{ImGui::GetContentRegionAvail().x,0})){
        showSearchBar=false;
        isFocused=false;
        memset(query, 0, IM_ARRAYSIZE(query));
    }
    ImGui::End();
}


void Browser::renderHomePage(){
    static bool isFocused=false;
    static bool isClicked=true;
    static SVG logo_img;
    if(!isFocused){
        logo_img.load_from_buffer((const char*)logo,70,70);
    }

    ImGui::SetNextWindowSize({width,height});
    ImGui::Begin("##HomePage", 0, ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_MenuBar |ImGuiWindowFlags_NoScrollbar);
    renderMenuBar();
    ImVec2 size=ImGui::GetWindowSize();
    ImGui::SetCursorPos({(size.x-70.0f)*0.5f,190.0f});
    ImGui::Image((void*)(intptr_t)logo_img.texture,{70,70});

    ImGui::SetCursorPos({(size.x-ImGui::CalcTextSize("File Transfer").x)*0.5f,260.0f});
    ImGui::Text("File Transfer");

    ImVec2 pos{(size.x-300.0f)*0.5f,(size.y-30)*0.5f};
    ImGui::PushItemWidth(300.0f);
    static char buff[32]="";
    ImGui::SetCursorPos(pos);
    isClicked=ImGui::InputText("##url",buff, IM_ARRAYSIZE(buff),ImGuiInputTextFlags_EnterReturnsTrue);
    ImGui::SetCursorPos({(size.x-70.0f)*0.5f,pos.y+30});
    isClicked=(isClicked || ImGui::Button("Proceed",ImVec2{70,0}));


    if(isClicked){
        if(initBrowser(std::string(buff))){
            this->showHomePage=false;
            isClicked=false;
        }
        isClicked=false;
    }

    if(!IPs.empty()){
        int y=100;
        const char* txt="Recent URL/IP";
        ImGui::SetCursorPos({(size.x-ImGui::CalcTextSize(txt).x)*0.5f,pos.y+y});
        ImGui::Text("%s", txt);
        y+=30;
        for(const auto& ip:IPs){
            ImGui::SetCursorPos({(size.x-250.0f)*0.5f,pos.y+y});
            if(ImGui::Button(ip.c_str(),ImVec2{250,0})){
                if(initBrowser(ip)){
                    this->showHomePage=false;
                    std::cout << basePath << std::endl;
                }
            }
            y+=30;
        }
    }


    ImGui::End();
}


void Browser::render()
{
    if(showHomePage){
        renderHomePage();
        return;
    }
    this->globalKeyBindings();
    if(this->settingsUpdate) saveSettings();
    ImGui::ShowDemoWindow();
    const float headerHeight = 60.0f;
    ImGui::SetNextWindowPos({0, 0});
    ImGui::SetNextWindowSize({(float)this->width, headerHeight});
    ImGui::Begin("##Header", 0, ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_MenuBar |ImGuiWindowFlags_NoScrollbar);
    renderMenuBar();

    ImGui::SetCursorPos({5, 28.0f});
    if (!showDownloads) {
        int i = 0;
        if (paths.size() > 3) i = paths.size() - 3;
        std::ostringstream path_str;
        while (i != paths.size()) {
            path_str << paths[i] << " " << ((i && i==paths.size()-1) ? "" : ICON_FA_ANGLE_RIGHT ) << " ";
            i++;
        }
        ImGui::PushTextWrapPos(ImGui::GetWindowWidth()-55.0f);
        ImGui::Text("%s", path_str.str().c_str());
        ImGui::PopTextWrapPos();
    } else {
        ImGui::Text("Downloads");
        ImGui::SetCursorPos({ImGui::GetWindowWidth() - 170, 28.0f});
        if(ImGui::Button("Open Downloads",ImVec2{0,24})) system("explorer .");
    }


    ImGui::SetCursorPos({ImGui::GetWindowWidth() - 45, 28.0f});
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
    if (ImGui::Button(showDownloads ? ICON_FA_FOLDER_TREE : ICON_FA_DOWNLOAD, ImVec2(30, 0))) this->showDownloads = !showDownloads;
    ImGui::PopFont();
    ImGui::End();

    ImGui::SetNextWindowPos({0, headerHeight});
    float windowHeight=(float)this->height - headerHeight;
    if(!this->showDownloads && this->showSearchBar) windowHeight-=30.0f;
    ImGui::SetNextWindowSize({(float)this->width, windowHeight});
    if (this->showDownloads) {
        this->m_DownloadManager.render();
        return;
    }


    if(showSearchBar) renderSearch();


    ImGui::Begin("##Browser", 0, ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

    if(this->showChangeUrl) ImGui::OpenPopup("Change URL");
    if(this->showConnectionError){
        ImGui::OpenPopup("Connection Error");
        this->showConnectionError=false;
    }

    //No Files Present -- Connection Error
    if (files.empty()) {
        ImVec2 window = ImGui::GetWindowSize();
        ImVec2 position((window.x - 120.0f) * 0.5f, (window.y - 25) * 0.5f);
        ImGui::SetCursorPos(position);
        if (ImGui::Button(ICON_FA_ROTATE_RIGHT" Refresh",ImVec2{120.0f,25})) {
            if (!fetchURLContent(stk.top())) {
                ImGui::OpenPopup("Connection Error");
            }
        }
        position.y+=35.0f;
        ImGui::SetCursorPos(position);
        if (ImGui::Button(ICON_FA_LINK" Change URL",ImVec2{120.0f,25})){
            ImGui::OpenPopup("Change URL");
        }
        // ImGui::PopFont();
        showConnectionErrorUI();
        showChangeUrlPopUp();
        ImGui::End();
        return;
    }


    int count = 0;
    static int selected = -1;
    keyBindings(selected, files.size());
    std::stringstream oss;
    std::vector<File> searchedFiles;
    if(showSearchBar) searchedFiles=searchFile(this->query, files);
    for (const auto& file : showSearchBar ? searchedFiles : files ) {
        oss << " " << (file.isFolder ? ICON_FA_FOLDER : ICON_FA_FILE) << "  ";

        oss << file.title;
        if (file.isFolder) ImGui::PushStyleColor(ImGuiCol_Text,ImGui::GetColorU32(ImGuiCol_ButtonHovered));
        if (ImGui::Selectable(oss.str().c_str(), selected == count)) selected = count;
        if(!file.isFolder && ImGui::IsItemHovered()){
            if(ImGui::CalcTextSize(file.title.c_str()).x > (this->width-30)){
                ImGui::BeginTooltip();
                ImGui::PushTextWrapPos(300);
                ImGui::Text(file.title.c_str());
                ImGui::PopTextWrapPos();
                ImGui::EndTooltip();
            }
        }
        if (file.isFolder) ImGui::PopStyleColor();
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
            if (selected == -1) selected = 0;
            if (!showSearchBar && this->files[selected].title == "..") {
                std::string last=stk.top();
                stk.pop();
                if(!this->fetchURLContent(stk.top())){
                    ImGui::OpenPopup("Connection Error");
                    stk.push(last);
                    continue;
                }
                paths.pop_back();
                continue;
            }
            if (!showSearchBar && this->files.at(selected).isFolder) {
                File* currFolder = &this->files[selected];
                std::cout << "PATH: " << currFolder->location << std::endl;
                stk.push(currFolder->location);
                paths.push_back(currFolder->title.substr(0, currFolder->title.size() - 1));
                if(!this->fetchURLContent(currFolder->location)){
                    ImGui::OpenPopup("Connection Error");
                    stk.pop();
                    paths.pop_back();
                    continue;
                }
                selected = 0;
            } else {
                File* currFile = (File*)&file;
                auto it=std::find_if(m_DownloadManager.downloads.begin(),m_DownloadManager.downloads.end(),[&currFile](const DFile* d_file){
                    return d_file->title==currFile->title;
                });
                if (it!=m_DownloadManager.downloads.end() && ((*it)->isDownloading || std::filesystem::exists((*it)->title))) {
                    oss.clear();
                    oss.str("");
                    count++;
                    ImGui::OpenPopup("Download Exists");
                    continue;
                }
                m_DownloadManager.addDownload(currFile);
            }
        }
        oss.clear();
        oss.str("");
        count++;
    }

    bool openPopup = true;
    if (ImGui::BeginPopupModal("Download Exists", &openPopup, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("File already exists!");
        ImGui::Separator();
        ImGui::Text("Do you want to re-download file?");
        ImVec2 spacingWidth{(ImGui::GetWindowWidth() - 140.0f) * 0.5f, 10.f};
        ImGui::InvisibleButton("##spacing", spacingWidth);
        ImGui::SameLine();
        if (ImGui::Button("Yes", ImVec2(50, 0))) {
            std::string title=files[selected].title;
            auto it=std::find_if(m_DownloadManager.downloads.begin(),m_DownloadManager.downloads.end(),
                [&title](const DFile* file){
                return  file->title==title;
            });
            if(it!=m_DownloadManager.downloads.end()){
                int idx=std::distance(m_DownloadManager.downloads.begin(),it);
                (*it)->isDownloading=false;
                delete *it;
                m_DownloadManager.downloads.erase(it);
                m_DownloadManager.futures.erase(m_DownloadManager.futures.begin()+idx);
            }

            m_DownloadManager.addDownload(&this->files[selected]);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine(spacingWidth.x + 80.f);
        if (ImGui::Button("No", ImVec2(50, 0))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
    showConnectionErrorUI();
    showChangeUrlPopUp();
    ImGui::End();
}
