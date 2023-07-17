#include "pch.h"
#include "FontAwesome6.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "DownloadManager.h"
#include <stdio.h>

bool DownloadManager::initDownload(File* currFile){
	if(!currFile) return false;
	std::ofstream file=std::ofstream(currFile->title,std::ios::binary);
	cpr::Session session=cpr::Session();
	session.SetUrl(cpr::Url{currFile->location});
	session.SetLowSpeed(cpr::LowSpeed(1000,1000));
	currFile->start=std::chrono::high_resolution_clock::now();
	session.SetProgressCallback(cpr::ProgressCallback([&currFile](cpr::cpr_off_t downloadTotal, cpr::cpr_off_t downloadNow, cpr::cpr_off_t uploadTotal, cpr::cpr_off_t uploadNow, intptr_t userdata)->bool{
		if(!currFile->isDownloading) return false;
		currFile->dps+=(float)(downloadNow-currFile->downloaded);
		currFile->size=downloadTotal;
		currFile->progress=(float)downloadNow/downloadTotal;
		if(currFile->getSpeed){
			currFile->speed=(currFile->dps/1048576)*5; // Converting B -> MiB * 5 to get MiB/s 
			currFile->dps=0.0;
			currFile->getSpeed=false;
		}
		currFile->downloaded=(float)downloadNow;
		return true;
	}));
	cpr::Response res=session.Download(file);
    std::cout << "Downloaded: " << currFile->title << " : " << (int)res.downloaded_bytes/1024 << "kb" << std::endl;
    return true;

}

inline float fround(float var)
{
    float value = (int)(var * 100 + .5);
    return (float)value / 100;
}

bool DComponent(File* file){
	//Speed Calculation
	file->end=std::chrono::high_resolution_clock::now();
	file->duration=file->end - file->start;
	if(file->duration.count()*1000.0f>=200.0f && file->isDownloading){
		file->getSpeed=true;
		file->start=std::chrono::high_resolution_clock::now();
	}


	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems) return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(file->title.c_str());
	const int threshold=window->Size.x-80;


	ImVec2 textSize=ImGui::CalcTextSize(file->title.c_str());
	textSize.y+=10;
	if(textSize.x > threshold){
		textSize.y=(textSize.y*2)-5;
	}
	const ImVec2 size(window->Size.x-15,textSize.y+40);

	ImVec2 pos=window->DC.CursorPos;
    const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
    ImGui::ItemSize(bb,0);
    if (!ImGui::ItemAdd(bb, id)) return false;

    // bool isHovered=false;
    // ImGui::ButtonBehavior(bb, id, &isHovered,0);

	static ImColor bgColor=ImColor(30,30,30).Value;    
	// static ImColor hColor=ImColor(50,50,50).Value;    
	// if(isHovered) window->DrawList->AddRectFilled(pos, bb.Max,hColor,4);
	window->DrawList->AddRectFilled(pos, bb.Max,bgColor,4);


	ImGui::RenderTextWrapped({pos.x+5,pos.y+5}, file->title.c_str(),0,threshold);
	window->DrawList->AddRectFilled(
		{pos.x+5,pos.y+textSize.y},
		ImVec2(pos.x+window->Size.x-60,pos.y+textSize.y+10),
		ImColor(10,10,10),
		2);
	float width=file->progress*abs(window->Size.x-60-5.0);
	window->DrawList->AddRectFilled(
		{pos.x+5,pos.y+textSize.y},
		ImVec2(pos.x+5+width,pos.y+textSize.y+10),
		ImColor(style.Colors[ImGuiCol_ButtonHovered]),
		2);

	static char speed[16];
	sprintf(speed,"%.2f MiB/s",file->speed);
	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[2]);
	ImGui::RenderText(ImVec2(pos.x+5,pos.y+textSize.y+15),speed);
	static char progress[16];
	sprintf(progress, "%.2f/%.2f MiB",file->downloaded/1048576.0f,file->size/1048576.0f);
	ImGui::RenderText(ImVec2(window->Size.x-150,pos.y+textSize.y+15),progress);
	ImGui::PopFont();


	// const ImGuiID bid = window->GetID(ICON_FA_XMARK);
	ImVec2 bPos{window->Size.x-40,pos.y+((size.y-20)*0.5f)};
	ImRect buttonSize{bPos,{bPos.x+20,bPos.y+20}};
    const ImVec2 label_size = ImGui::CalcTextSize(ICON_FA_XMARK, NULL, true);

    bool isButtonHovered=false;
    bool isButtonHeld=false;
    bool isButtonClicked=ImGui::ButtonBehavior(buttonSize, id, &isButtonHovered,&isButtonHeld);
    if(isButtonClicked){
    	std::cout << "Clicked" << std::endl;
    }
    const ImU32 col = ImGui::GetColorU32((isButtonHeld && isButtonHovered) ? ImGuiCol_ButtonActive : isButtonHovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
    ImGui::RenderNavHighlight(buttonSize, id);
    ImGui::RenderFrame(buttonSize.Min, buttonSize.Max, col, true, style.FrameRounding);
    ImGui::RenderTextClipped(buttonSize.Min + style.FramePadding, ImVec2{buttonSize.Max.x-style.FramePadding.x,buttonSize.Max.y-style.FramePadding.y}, ICON_FA_XMARK, NULL, &label_size, style.ButtonTextAlign, &buttonSize);

	return isButtonClicked;
}


void DownloadManager::addDownload(File* file){
	file->isDownloading=true;
	downloads.push_back(file);
	futures.push_back(std::async(std::launch::async,&DownloadManager::initDownload,this,file));
}

void DownloadManager::render(){
    ImGui::Begin("##Downloads",0,ImGuiWindowFlags_NoNav|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize);
    if(downloads.empty()){
    	const char* msg="No Downloads Available!";
    	ImVec2 textSize=ImGui::CalcTextSize(msg);
    	ImVec2 window=ImGui::GetWindowSize();
    	ImGui::SetCursorPos({(window.x-textSize.x)*0.5f,15+(window.y-textSize.y)*0.5f});
    	ImGui::Text("%s", msg);
    	ImGui::End();
    	return;
    }
    auto itf=futures.begin();
    for(auto it=downloads.begin();it!=downloads.end();){
    	if(DComponent(*it)){
    		(*it)->isDownloading=false;
    		std::cout << "Downloads: " << downloads.size() << " Futures: " << futures.size() << std::endl;
    		std::cout << "Deleting:" << (*it)->title;
    		it=this->downloads.erase(it);
    		itf=this->futures.erase(itf);
    		std::cout << "Downloads: " << downloads.size() << " Futures: " << futures.size() << std::endl;
    	}else{
    		it++;
    		itf++;
    	}

    }
    ImGui::End();
}