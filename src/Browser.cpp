#include "imgui.h"
#include "pch.h"
#include "Browser.h"
#include <iterator>

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


void renderMenuBar()
{
    if (ImGui::BeginMenuBar()) {
        static bool isselected = false;
        static bool themeSelected[3] = {1, 0, 0};
        // memset(themeSelected,0,sizeof(themeSelected));
        if (ImGui::BeginMenu("Menu")) {
            ImGui::MenuItem("Change URL");
            if (ImGui::BeginMenu("Theme")) {
                if (ImGui::MenuItem("Dark", 0, themeSelected[0])){
                    memset(themeSelected,0,sizeof(themeSelected));
                    themeSelected[0]=true;
                    ImGui::StyleColorsDark();
                }
                if (ImGui::MenuItem("Light", 0, themeSelected[1])){
                    memset(themeSelected,0,sizeof(themeSelected));
                    themeSelected[1]=true;
                    ImGui::StyleColorsLight();
                }
                if (ImGui::MenuItem("Classic", 0, themeSelected[2])){
                    memset(themeSelected,0,sizeof(themeSelected));
                    themeSelected[2]=true;
                    ImGui::StyleColorsClassic();
                }
                ImGui::EndMenu();
            }
            ImGui::MenuItem("Check Update");
            ImGui::MenuItem("About");
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help")) {
            ImGui::MenuItem("Key Bindings");
            ImGui::MenuItem("Usage");
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
}

void showConnectionErrorUI(){
    static bool openPopup = true;
    if (ImGui::BeginPopupModal("Connection Error", &openPopup, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Unable to connect the device");
        ImGui::Separator();
        ImGui::SetCursorPos(ImVec2{(ImGui::GetWindowWidth() - 50) * 0.5f, ImGui::GetCursorPosY()});
        if (ImGui::Button("OK", ImVec2(50, 0))) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
}


void keyBindings(int& selected,int max){
    if(selected==-1) selected=0;
    if(ImGui::IsKeyPressed(ImGuiKey_J)){
        selected++;
        if(selected > 20) ImGui::SetScrollY(selected*20);
        if(selected==max){
            selected=0;
            ImGui::SetScrollY(0.0f);
        }
    }
    if(ImGui::IsKeyPressed(ImGuiKey_K)){
        selected--;
        if(selected >= 0) ImGui::SetScrollY(selected*15);
        if(selected==-1){
            selected=max-1;
            ImGui::SetScrollY(ImGui::GetScrollMaxY());
        }
    }
}


void Browser::render()
{
    ImGui::ShowDemoWindow();
    const float headerHeight = 60.0f;
    ImGui::SetNextWindowPos({0, 0});
    ImGui::SetNextWindowSize({(float)this->width, headerHeight});
    ImGui::Begin("##Header", 0,
                 ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_MenuBar |
                     ImGuiWindowFlags_NoScrollbar);
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
    }


    ImGui::SetCursorPos({ImGui::GetWindowWidth() - 45, 28.0f});
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
    if (ImGui::Button(showDownloads ? ICON_FA_FOLDER_TREE : ICON_FA_DOWNLOAD, ImVec2(30, 0))) this->showDownloads = !showDownloads;
    ImGui::PopFont();
    showConnectionErrorUI();
    ImGui::End();

    ImGui::SetNextWindowPos({0, headerHeight});
    ImGui::SetNextWindowSize({(float)this->width, (float)this->height - headerHeight});
    if (this->showDownloads) {
        this->m_DownloadManager.render();
        return;
    }
    // ImGui::GetStyle().Colors[ImGuiCol_Button]
    ImGui::Begin("##Browser", 0, ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
    if (files.empty()) {
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[3]);
        static ImVec2 iconSize = ImGui::CalcTextSize(ICON_FA_ROTATE_RIGHT);
        ImVec2 window = ImGui::GetWindowSize();
        ImVec2 position((window.x - iconSize.x) * 0.5f, (window.y - iconSize.y) * 0.5f);
        ImGui::SetCursorPos(position);
        if (ImGui::IconButton(ICON_FA_ROTATE_RIGHT, ImColor(25, 155, 255), ImColor(36, 103, 255))) {
            if (!fetchURLContent(stk.top())) {
                ImGui::OpenPopup("Connection Error");
            }
        }
        ImGui::PopFont();
        ImGui::End();
        return;
    }
    int count = 0;
    static int selected = -1;
    keyBindings(selected, files.size());
    std::stringstream oss;
    for (const auto& file : this->files) {
        oss << " " << (file.isFolder ? ICON_FA_FOLDER : ICON_FA_FILE) << "  ";

        oss << file.title;
        if (file.isFolder) ImGui::PushStyleColor(ImGuiCol_Text,ImGui::GetColorU32(ImGuiCol_ButtonHovered));
        // if(file.isDownloading || file.isDownloaded) ImGui::PushStyleColor(ImGuiCol_Text,ImGui::GetColorU32(ImGuiCol_ScrollbarGrabActive));
        if (ImGui::Selectable(oss.str().c_str(), selected == count)) selected = count;
        // if(file.isDownloading || file.isDownloaded) ImGui::PopStyleColor();
        if (file.isFolder) ImGui::PopStyleColor();
        if (((ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) || ImGui::IsKeyPressed(ImGuiKey_Enter) ||
             ImGui::IsKeyPressed(ImGuiKey_O))) {
            if (selected == -1) selected = 0;
            if (this->files[selected].title == "..") {
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
            if (this->files.at(selected).isFolder) {
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
                File* currFile = &this->files[selected];
                if (currFile->isDownloading || std::filesystem::exists(currFile->title)) {
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
    static bool openPopup = true;
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
                [&title](const File* file){
                return  file->title==title;
            });
            if(it!=m_DownloadManager.downloads.end()){
                int idx=std::distance(m_DownloadManager.downloads.begin(),it);
                (*it)->isDownloading=false;
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
    ImGui::End();
}
