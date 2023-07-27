#include "pch.h"
#include "DownloadManager.h"
#include <shellapi.h>

bool DownloadManager::initDownload(DFile* currFile){
	if(!currFile) return false;
	std::ofstream file=std::ofstream((this->path+currFile->title),std::ios::binary);
	cpr::Session session=cpr::Session();
	std::cout << "Downloading: " << currFile->location << std::endl;
	std::cout << "Location: " << this->path+currFile->title << std::endl;
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
	if(res.status_code==200) currFile->isDownloaded=true;
	currFile->isDownloading=false;
    std::cout << "Downloaded: " << currFile->title << " : " << (int)res.downloaded_bytes/1024 << "kb" << std::endl;
    return true;

}

inline float fround(float var)
{
    float value = (int)(var * 100 + .5);
    return (float)value / 100;
}

bool DownloadManager::DComponent(DFile* file){
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
	const int original=textSize.y-10;
	while(textSize.x >threshold){
		textSize.y+=original;
		textSize.x-=threshold;
	}

	const ImVec2 size(window->Size.x-15,textSize.y+40.0f);
	ImVec2 pos=window->DC.CursorPos;
    const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
    ImGui::ItemSize(bb,0);
    if (!ImGui::ItemAdd(bb, id)) return false;

    bool isHovered=false;
    bool isHeld=false;
    bool isClicked=ImGui::ButtonBehavior(bb, id, &isHovered,&isHeld,ImGuiButtonFlags_AllowOverlap | ImGuiButtonFlags_PressedOnDoubleClick);
	window->DrawList->AddRectFilled(pos, bb.Max,ImGui::GetColorU32(isHovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg),2);
	if(file->isDownloaded && isClicked){
		std::cout << "Opening.. " << std::endl;
		ShellExecute(0, 0, (path+file->title).c_str(), 0, 0 , SW_SHOW );
	}
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
		ImGui::GetColorU32(ImGuiCol_ButtonHovered),
		2);

	static char speed[16];
	sprintf(speed,"%.2f MiB/s",file->speed);
	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[2]);
	ImGui::RenderText(ImVec2(pos.x+5,pos.y+textSize.y+15),speed);
	static char progress[16];
	sprintf(progress, "%.2f/%.2f MiB",file->downloaded/1048576.0f,file->size/1048576.0f);
	ImGui::RenderText(ImVec2(window->Size.x-150,pos.y+textSize.y+15),progress);
	ImGui::PopFont();	

	static std::string x=(file->title+"bid");
	const ImGuiID idx = window->GetID(x.c_str());

	ImVec2 bPos{window->Size.x-40,pos.y+((size.y-20)*0.5f)};
	ImRect buttonSize{bPos,{bPos.x+20,bPos.y+20}};
    const ImVec2 label_size = ImGui::CalcTextSize(file->isDownloading ? ICON_FA_XMARK : ICON_FA_TRASH, NULL, true);

    bool isButtonHovered=false;
    bool isButtonHeld=false;
    bool isButtonClicked=ImGui::ButtonBehavior(buttonSize, idx, &isButtonHovered,&isButtonHeld,ImGuiButtonFlags_PressedOnClick);

    const ImU32 col = ImGui::GetColorU32((isButtonHeld && isButtonHovered) ? ImGuiCol_ButtonActive : isButtonHovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
    ImGui::RenderNavHighlight(buttonSize, idx);
    ImGui::RenderFrame(buttonSize.Min, buttonSize.Max, col, true, style.FrameRounding);
    ImGui::RenderTextClipped(ImVec2{buttonSize.Min.x+style.FramePadding.x,buttonSize.Min.y+style.FramePadding.y}, ImVec2{buttonSize.Max.x-style.FramePadding.x,buttonSize.Max.y-style.FramePadding.y},file->isDownloading ? ICON_FA_XMARK : ICON_FA_TRASH, NULL, &label_size, style.ButtonTextAlign, &buttonSize);
    return isButtonClicked;
}


void DownloadManager::addDownload(File* file){
	DFile* d_file=new DFile();
	d_file->title=file->title;
	d_file->location=file->location;
	d_file->isDownloading=true;

	downloads.push_back(d_file);
	futures.push_back(std::async(std::launch::async,&DownloadManager::initDownload,this,d_file));
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
    		(*it)->isDownloaded=false;
    		std::cout << "Downloads: " << downloads.size() << " Futures: " << futures.size() << std::endl;
    		std::cout << "Deleting:" << (*it)->title << std::endl;;
    		std::string title=(*it)->title;
    		delete *it;
    		it=this->downloads.erase(it);
    		itf=this->futures.erase(itf);
    		std::cout << "Downloads: " << downloads.size() << " Futures: " << futures.size() << std::endl;
    		std::cout << "Delete Path: " << (this->path+title) << std::endl;
    		if(std::filesystem::exists((this->path+title))) std::filesystem::remove((this->path+title));
    	}else{
    		it++;
    		itf++;
    	}

    }
    ImGui::End();
}