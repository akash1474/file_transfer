#include <lunasvg.h>
#include <filesystem>
#include <stdlib.h>
#include <gl/gl.h>

#define GL_CLAMP_TO_EDGE 0x812F
#define GL_CLAMP 0x2900
#define GL_CLAMP_TO_BORDER 0x812D

class SVG{
public:
	lunasvg::Bitmap img_data;
	unsigned int texture{0};
	SVG(){}
	SVG(std::string file_path,int width=0,int height=0){load_from_file(file_path,width,height);}
    SVG(const char* buff,int width=0,int height=0){load_from_buffer(buff,width,height);}
	void load_from_file(std::string file_path,int width=0,int height=0){
		if(!std::filesystem::exists(file_path)){
			std::cout << "INVALID SVG PATH: " << file_path << std::endl;
			exit(1);
		}
        std::unique_ptr<lunasvg::Document> doc=lunasvg::Document::loadFromFile(file_path);
        if(!width || !height){
        	width=doc->width();
        	height=doc->height();
        }
        img_data=doc->renderToBitmap(width,height);
        if(!img_data.valid()){
        }
        img_data.convertToRGBA();
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img_data.data());
        glBindTexture(GL_TEXTURE_2D, 0);

	}

    void load_from_buffer(const char* data,int width=0,int height=0){
        std::unique_ptr<lunasvg::Document> doc=lunasvg::Document::loadFromData(data);
        img_data=doc->renderToBitmap(width,height);
        if(!img_data.valid()){
        }
        img_data.convertToRGBA();
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img_data.data());
        glBindTexture(GL_TEXTURE_2D, 0);
    }
};