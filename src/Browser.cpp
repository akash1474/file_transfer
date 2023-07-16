#include "pch.h"
#include "FontAwesome6.h"
#include "imgui.h"
#include "Browser.h"
#include <sstream>

bool Browser::fetchURLContent(std::string url){
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
        if(text==".."){
            file.isFolder=true;
            file.title=text;
            this->files.push_back(file);
            it=match.suffix().first;
            continue;
        }

        if(text[text.length()-1]=='/'){
            file.isFolder=true;
            file.title=text;
            file.location=(this->basePath+href);
            // std::cout << "Folder: - ";
        }else{
            file.title=text;
            file.location=(this->basePath+href);
        }
        // std::cout << "File: " <<  file.title << std::endl;
        // std::cout << "Location:" << file.location << std::endl;
        this->files.push_back(file);
        it = match.suffix().first;
    }
    std::stable_partition(files.begin(), files.end(), [](const File& file) {
        return file.isFolder;
    });
    return true;
}




void Browser::render(){
    ImGui::SetNextWindowPos({0,0});
    ImGui::SetNextWindowSize({(float)this->width,40.0f});
    ImGui::Begin("##Header",0,ImGuiWindowFlags_NoNav|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize);
    ImGui::SetCursorPos({15,5});
    int i=0;
    if(paths.size()>3) i=paths.size()-3;
    std::ostringstream path_str;
    while(i!=paths.size()){
        path_str << paths[i] << " " << ICON_FA_ANGLE_RIGHT << " ";
        i++;
    }
    ImGui::TextWrapped("%s", path_str.str().c_str());
    ImGui::SetCursorPos({ImGui::GetWindowWidth()-45,8.0f});
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
    // if(ImGui::IconButton(showDownloads ? ICON_FA_FOLDER_TREE : ICON_FA_DOWNLOAD, ImColor(ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered]),ImColor(ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]))) this->showDownloads=!showDownloads;
    if(ImGui::Button(showDownloads ? ICON_FA_FOLDER_TREE : ICON_FA_DOWNLOAD,ImVec2(30,0)))this->showDownloads=!showDownloads;
    ImGui::PopFont();
    ImGui::End();

    ImGui::SetNextWindowPos({0,40});
    ImGui::SetNextWindowSize({(float)this->width,(float)this->height-40});
    if(this->showDownloads){
        this->m_DownloadManager.render();
        return;
    }
    // ImGui::GetStyle().Colors[ImGuiCol_Button]
    ImGui::Begin("##Browser",0,ImGuiWindowFlags_NoNav|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize);
    if(files.empty()){
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[3]);
        static ImVec2 iconSize=ImGui::CalcTextSize(ICON_FA_ROTATE_RIGHT);
        ImVec2 window=ImGui::GetWindowSize();
        ImVec2 position((window.x-iconSize.x)*0.5f,(window.y-iconSize.y)*0.5f);
        ImGui::SetCursorPos(position);
        if(ImGui::IconButton(ICON_FA_ROTATE_RIGHT, ImColor(25,155,255), ImColor(36,103,255))){
            if(!fetchURLContent(stk.top())){
                ImGui::OpenPopup("Connection Error");
            }
        }
        ImGui::PopFont();
        static bool openPopup=true;
        if (ImGui::BeginPopupModal("Connection Error", &openPopup, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Unable to connect the device");
            ImGui::Separator();
            ImGui::SetCursorPos(ImVec2{(ImGui::GetWindowWidth()-50)*0.5f,ImGui::GetCursorPosY()});
            if (ImGui::Button("OK", ImVec2(50, 0))) {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
        ImGui::End();
        return;
    }
    int count=0;
    static int selected=-1;
    std::stringstream oss;
    ImGui::ShowDemoWindow();
    for(const auto& file:this->files){
        if(file.isFolder) oss << " " <<  ICON_FA_FOLDER << "  ";
        else oss << " " << ICON_FA_FILE << "  ";
        
        oss << file.title;
        if(file.isFolder) ImGui::PushStyleColor(ImGuiCol_Text,ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]);
        if (ImGui::Selectable(oss.str().c_str(), selected == count)) selected = count;
        if(file.isFolder) ImGui::PopStyleColor();
        if (((ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) || ImGui::IsKeyPressed(ImGuiKey_Enter) ||
            ImGui::IsKeyPressed(ImGuiKey_O))) {
            if (selected == -1) selected = 0;
            if(this->files[selected].title==".."){
                stk.pop();
                paths.erase(paths.end()-1);
                this->fetchURLContent(stk.top());
                continue;
            }
            if (this->files.at(selected).isFolder) {
                File* currFolder=&this->files[selected];
                std::cout << "PATH: " << currFolder->location << std::endl;
                stk.push(currFolder->location);
                paths.push_back(currFolder->title.substr(0,currFolder->title.size()-1));
                this->fetchURLContent(currFolder->location);
                selected = 0;
            }else{
                File* currFile=&this->files[selected];
                if(currFile->isDownloading || std::filesystem::exists(currFile->title)){
                    std::cout << "Exists" << std::endl;
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
    static bool openPopup=true;
    if (ImGui::BeginPopupModal("Download Exists", &openPopup, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("File already exists!");
        ImGui::Separator();
        ImGui::Text("Do you want to re-download file?");
        ImVec2 spacingWidth{(ImGui::GetWindowWidth()-140.0f)*0.5f,10.f};
        ImGui::InvisibleButton("##spacing",spacingWidth);
        ImGui::SameLine();
        if (ImGui::Button("Yes", ImVec2(50, 0))) {
            m_DownloadManager.addDownload(&this->files[selected]);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine(spacingWidth.x+80.f);
        if (ImGui::Button("No", ImVec2(50, 0))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    ImGui::End();
}
