#include "imgui.h"
#include "pch.h"
#include "Browser.h"
#include <sstream>

void Browser::fetchURLContent(std::string url){
    std::cout << "Fetching: " << url << std::endl;
	cpr::Response response = cpr::Get(cpr::Url(url));

    if (response.status_code != 200) {
        std::cerr << "Error: Failed to fetch URL. Status code: " << response.status_code << std::endl;
        std::cerr << "Error: " << url << std::endl;
        return;
    }

    // std::cout << response.text << std::endl;

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
}



void Browser::render(){
    ImGui::SetNextWindowPos({0,0});
    ImGui::SetNextWindowSize({(float)this->width,40.0f});
    ImGui::Begin("##Header",0,ImGuiWindowFlags_NoNav|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize);
    ImGui::SetCursorPos({15,5});
    ImGui::Text("Home > Download > Telegram");
    ImGui::SetCursorPos({ImGui::GetWindowWidth()-40,10});
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
    if(ImGui::IconButton(ICON_FA_ARROW_DOWN_LONG, ImColor(7,167,118), ImColor(0,218,155))) this->showDownloads=!showDownloads;
    ImGui::PopFont();
    ImGui::End();

    ImGui::SetNextWindowPos({0,40});
    ImGui::SetNextWindowSize({(float)this->width,(float)this->height});
    if(this->showDownloads){
        this->m_DownloadManager.render();
        return;
    }
    ImGui::Begin("##Browser",0,ImGuiWindowFlags_NoNav|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize);
    int count=0;
    static int selected=-1;
    std::stringstream oss;
    for(const auto& file:this->files){
        if(file.isFolder) oss << " " <<  ICON_FA_FOLDER << "  ";
        else oss << " " << ICON_FA_FILE << "  ";
        
        oss << file.title;
        if(file.isFolder) ImGui::PushStyleColor(ImGuiCol_Text,ImColor(230,138,8).Value);
        if (ImGui::Selectable(oss.str().c_str(), selected == count)) selected = count;
        if(file.isFolder) ImGui::PopStyleColor();
        if (((ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) || ImGui::IsKeyPressed(ImGuiKey_Enter) ||
            ImGui::IsKeyPressed(ImGuiKey_O))) {
            if (selected == -1) selected = 0;
            if(this->files[selected].title==".."){
                stk.pop();
                this->fetchURLContent(stk.top());
                continue;
            }
            if (this->files.at(selected).isFolder) {
                std::cout << "PATH: " << this->files[selected].location << std::endl;
                stk.push(this->files[selected].location);
                this->fetchURLContent(this->files[selected].location);
                selected = 0;
            }else{
                m_DownloadManager.addDownload(&this->files[selected]);
            }
        }
        oss.clear();
        oss.str("");
        count++;
    }
    ImGui::End();
}
