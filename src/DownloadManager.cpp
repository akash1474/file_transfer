#include "imgui.h"
#include "pch.h"
#include "DownloadManager.h"

bool DownloadManager::initDownload(File* currFile){
	if(!currFile) return false;
	std::ofstream file=std::ofstream(currFile->title,std::ios::binary);
	cpr::Session session=cpr::Session();
	session.SetUrl(cpr::Url{currFile->location});
	session.SetLowSpeed(cpr::LowSpeed(1000,1000));
	currFile->start=std::chrono::high_resolution_clock::now();
	session.SetProgressCallback(cpr::ProgressCallback([&currFile](cpr::cpr_off_t downloadTotal, cpr::cpr_off_t downloadNow, cpr::cpr_off_t uploadTotal, cpr::cpr_off_t uploadNow, intptr_t userdata)->bool{
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
	std::cout << res.header["content-type"] << std::endl;
	std::cout << res.header["content-disposition"] << std::endl;
    std::cout << "-- " << currFile->title << " -- " << (int)res.downloaded_bytes/1024 << "kb" << std::endl;
    return true;

}

bool DownloadManager::DownloadComponent(File* file){
	file->end=std::chrono::high_resolution_clock::now();
	file->duration=file->end - file->start;
	if(file->duration.count()*1000.0f>=200.0f && file->isDownloading){
		file->getSpeed=true;
		file->start=std::chrono::high_resolution_clock::now();
	}
	ImVec2 pos=ImGui::GetCursorPos();
	ImGui::TextWrapped(file->title.c_str());
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,2.0f);
	ImGui::PushStyleColor(ImGuiCol_PlotHistogram,ImColor(0,173,181).Value);
	ImGui::ProgressBar(file->progress,ImVec2(ImGui::GetContentRegionAvail().x-60,10));
	ImGui::PopStyleColor();
	ImGui::PopStyleVar();
	ImGui::PushStyleColor(ImGuiCol_Text,ImColor(160,217,149).Value);
	ImGui::Text(ICON_FA_ARROW_DOWN_LONG);ImGui::SameLine();
	ImGui::PopStyleColor();
	ImGui::Text("%.2f MiB/s",file->speed);
	ImGui::SameLine(ImGui::GetContentRegionAvail().x-120);
	ImGui::Text("%.2f/%.2f MB",file->downloaded/1048576,file->size/1048576.0f);
	ImGui::SetCursorPos({ImGui::GetWindowWidth()-30,pos.y+15});
	if(ImGui::IconButton(ICON_FA_X, ImColor(230,0,100),ImColor(30,30,30))) return true;
	ImGui::InvisibleButton("##Inv",ImVec2{1.0f,40.0f});
	return false;
}

void DownloadManager::addDownload(File* file){
	file->isDownloading=true;
	downloads.push_back(file);
	futures.push_back(std::async(std::launch::async,&DownloadManager::initDownload,this,file));
}

void DownloadManager::render(){
    ImGui::Begin("##Downloads",0,ImGuiWindowFlags_NoNav|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoResize);
    int i=0;
   	for(auto& file:downloads){
   		if(DownloadComponent(file)){ //Delete
   			auto it=this->downloads.begin()+i;
   			file->isDownloading=false;
   			auto itf=this->futures.begin()+i;
   			this->downloads.erase(it,++it);
   			itf->_Abandon();
   			this->futures.erase(itf,itf+1);
   		}
   		i++;
   	}
    ImGui::End();
}