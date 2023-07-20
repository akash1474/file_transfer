#include "FontAwesome6.h"
#include "GLFW/glfw3.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "pch.h"
#include "Browser.h"
#include <iterator>
#include <stdio.h>
#include <stdlib.h>

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
            // std::cout << "Folder: - ";
        } else {
            file.title = text;
            file.location = (this->basePath + href);
        }
        // std::cout << "File: " <<  file.title << std::endl;
        // std::cout << "Location:" << file.location << std::endl;
        this->files.push_back(file);
        it = match.suffix().first;
    }
    std::stable_partition(files.begin(), files.end(), [](const File& file) { return file.isFolder; });
    return true;
}


void renderMenuBar(bool& showChangeURL,bool& showSearchBar)
{
    if (ImGui::BeginMenuBar()) {
        static bool isselected = false;
        static bool themeSelected[3] = {1, 0, 0};
        bool static showFps=false;
        bool static vSyncEnabled=true;
        // memset(themeSelected,0,sizeof(themeSelected));
        if (ImGui::BeginMenu("Menu")) {
            if(ImGui::MenuItem("Change IP/URL")) showChangeURL=true;
            if(ImGui::MenuItem("Find File")) showSearchBar=true;
            if (ImGui::BeginMenu("Theme")) {
                if (ImGui::MenuItem("Dark",0, themeSelected[0])){
                    memset(themeSelected,0,sizeof(themeSelected));
                    themeSelected[0]=true;
                    ImGui::StyleColorsDark();
                }
                if (ImGui::MenuItem("Light",0, themeSelected[1])){
                    memset(themeSelected,0,sizeof(themeSelected));
                    themeSelected[1]=true;
                    ImGui::StyleColorsLight();
                }
                if (ImGui::MenuItem("Classic",0, themeSelected[2])){
                    memset(themeSelected,0,sizeof(themeSelected));
                    themeSelected[2]=true;
                    ImGui::StyleColorsClassic();
                }
                ImGui::EndMenu();
            }
            ImGui::MenuItem("Check Update");
            ImGui::MenuItem("Exit");
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Options")) {
            if(ImGui::MenuItem("Show FPS",0,showFps)) showFps=!showFps;
            if(ImGui::MenuItem("Vsync",0,vSyncEnabled)){
                vSyncEnabled=!vSyncEnabled;
                vSyncEnabled ? glfwSwapInterval(1) :  glfwSwapInterval(0);
            }
            if(ImGui::BeginMenu("Downloads")){
                if(ImGui::BeginMenu("Location")){
                    ImGui::MenuItem("Current Folder");
                    ImGui::MenuItem("Downloads");
                    ImGui::EndMenu();
                }
                if(ImGui::MenuItem("Change Location")){}
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
        static bool logged=false;
        this->showChangeUrl=false;
        ImGui::Text("Enter the new IP or select from below");
        ImGuiIO& io=ImGui::GetIO();
        static char ip[64];
        if(!logged){
            std::cout << io.IniFilename << std::endl;
            std::cout << getenv("USERPROFILE") << std::endl;
            std::cout << std::filesystem::current_path().generic_string() << std::endl;
            // ImGuiTable table;
            // ImGuiSettingsHandler ini_handler;
            // ini_handler.TypeName = "Window";
            // ini_handler.TypeHash = ImHashStr("Window");
            // ini_handler.ClearAllFn = ImWindowSettingsHandler_ClearAll;
            // ini_handler.ReadOpenFn = WindowSettingsHandler_ReadOpen;
            // ini_handler.ReadLineFn = WindowSettingsHandler_ReadLine;
            // ini_handler.ApplyAllFn = WindowSettingsHandler_ApplyAll;
            // ini_handler.WriteAllFn = WindowSettingsHandler_WriteAll;
            // AddSettingsHandler(&ini_handler);
            // table.
            // ImGui::TableSaveSettings(ImGuiTable *table)
            // ImGui::SaveIniSettingsToDisk("usr_settings.ini");
            // ImGui::LoadIniSettingsFromDisk("usr_settings.init");
            logged=true;
        }
        if(ImGui::InputText("##ip_input",ip,IM_ARRAYSIZE(ip))){
            std::cout << ip << std::endl;
        }
        ImGui::Button("Open",ImVec2{60,0});
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
}


void Browser::keyBindings(int& selected,int max){
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
    ImGui::SetNextWindowPos({0, this->height-30.0f});
    ImGui::SetNextWindowSize({(float)this->width,30.0f});
    ImGui::Begin("##SearchBar", 0,
                 ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoScrollbar| ImGuiWindowFlags_NoScrollWithMouse);

    ImGui::Text(ICON_FA_MAGNIFYING_GLASS); ImGui::SameLine();
    if(!ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0)){
        ImGui::SetKeyboardFocusHere(0);
    }
    ImGui::InputText("##search_text", query, IM_ARRAYSIZE(query));
    ImGui::SameLine();
    if(ImGui::Button("Close",ImVec2{ImGui::GetContentRegionAvail().x,0})) showSearchBar=false;
    ImGui::End();
}



void Browser::render()
{
    this->globalKeyBindings();
    ImGui::ShowDemoWindow();
    const float headerHeight = 60.0f;
    ImGui::SetNextWindowPos({0, 0});
    ImGui::SetNextWindowSize({(float)this->width, headerHeight});
    ImGui::Begin("##Header", 0, ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_MenuBar |ImGuiWindowFlags_NoScrollbar);
    renderMenuBar(this->showChangeUrl,this->showSearchBar);

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

    //No Files Present -- Connection Error
    if (files.empty()) {
        // ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[3]);
        // static ImVec2 iconSize = ImGui::CalcTextSize(ICON_FA_ROTATE_RIGHT);
        // ImVec2 window = ImGui::GetWindowSize();
        // ImVec2 position((window.x - iconSize.x) * 0.5f, (window.y - iconSize.y) * 0.5f);
        // ImGui::SetCursorPos(position);
        // if (ImGui::IconButton(ICON_FA_ROTATE_RIGHT, ImColor(25, 155, 255), ImColor(36, 103, 255))) {
        //     if (!fetchURLContent(stk.top())) {
        //         ImGui::OpenPopup("Connection Error");
        //     }
        // }
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
