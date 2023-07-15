
#include <stdint.h>
#include "string"
#include "chrono"
struct File{
	bool isFolder{0};
	std::string location{0};
	std::string title{0};
	uint32_t size{0};
	bool isDownloading{0};
	float speed{0.0f};
	float downloaded{0};
	float progress{0.0f};
	bool getSpeed=false;
	float dps=0.0;
	std::chrono::time_point<std::chrono::high_resolution_clock> start,end;
	std::chrono::duration<float> duration;
};
