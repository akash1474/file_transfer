#include "pch.h"
#include "Browser.h"

#define WIDTH 400
#define HEIGHT 600

namespace ImGui{
    bool IconButton(const char* icon, const ImU32& hoverColor, const ImU32& heldColor)
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return false;
        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(icon);

        const ImVec2 size(g.FontSize, g.FontSize);
        const ImVec2 pos = window->DC.CursorPos;
        const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
        ImGui::ItemSize(bb, style.FramePadding.y);
        if (!ImGui::ItemAdd(bb, id)) return false;


        bool isHovered, held;
        bool clicked = ImGui::ButtonBehavior(bb, id, &isHovered, &held);

        // window->DrawList->AddRectFilled(pos, bb.Max, ImColor(255,255,255,100));
        if (held) {
            ImGui::PushStyleColor(ImGuiCol_Text, heldColor);
            ImGui::RenderText(ImVec2(pos.x, pos.y), icon);
            ImGui::PopStyleColor();
            return clicked;
        }


        if (isHovered) ImGui::PushStyleColor(ImGuiCol_Text, hoverColor);
        ImGui::RenderText(ImVec2(pos.x, pos.y), icon);
        if (isHovered) ImGui::PopStyleColor();

        // if(clicked) return true;
        return clicked;
    }

    int distance(const ImVec2& d1,const ImVec2& d2){
        return std::sqrt(std::pow(d1.x-d2.x,2)+std::pow(d1.y-d2.y,2));
    }

    bool IconButtonRounded(const char* icon,int radius,const ImColor& iconColor,const ImColor& bgColor, const ImColor& hoverColor, const ImColor& heldColor)
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems) return false;
        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(icon);

        const ImVec2 size(radius*2,radius*2);
        const ImVec2 pos = window->DC.CursorPos;
        const ImVec2 mousePos=ImGui::GetMousePos();
        const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));

        //Checking Events
        const bool isHovered=(distance(pos,mousePos) <= radius);
        const bool isClicked=(isHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left));

        ImGui::ItemSize(bb, style.FramePadding.y);

        //Drawing Circle
        if(!isHovered && !isClicked) window->DrawList->AddCircle(pos,radius, bgColor);
        if(!isClicked && isHovered){
            window->DrawList->AddCircleFilled(pos, radius, hoverColor);
        }else if(isClicked){
            window->DrawList->AddCircleFilled(pos, radius, heldColor);
        }

        if (!ImGui::ItemAdd(bb, id)) return false;

        //Rendering Text
        ImVec2 fontSize=ImGui::CalcTextSize(icon);
        const ImVec2 textPosition(pos.x-(fontSize.x+2*style.FramePadding.x)/4,pos.y-(fontSize.y+2*style.FramePadding.y)/4);

        ImGui::PushStyleColor(ImGuiCol_Text, iconColor.Value);
        ImGui::RenderText(textPosition, icon);
        ImGui::PopStyleColor();


        // if (isHovered) ImGui::PushStyleColor(ImGuiCol_Text, hoverColor);
        // ImGui::RenderText(ImVec2(pos.x, pos.y), icon);
        // if (isHovered) ImGui::PopStyleColor();

        return isClicked;
    }
};


int main(){
    GLFWwindow* window;

    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "File Transfer", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Initialize ImGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);

    if (!ImGui_ImplOpenGL2_Init()) std::cout << "Failed to initit OpenGL 2" << std::endl;

    io.Fonts->Clear();
    ImFontConfig icon_config;
    icon_config.MergeMode = true;
    icon_config.PixelSnapH = true;
    static const ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_FA};

    io.Fonts->AddFontFromFileTTF("./assets/fonts/recursive_linear_medium.ttf", 16);
    io.Fonts->AddFontFromFileTTF(FONT_ICON_FILE_NAME_FAS, 16 * 2.0f / 3.0f, &icon_config, icons_ranges);

    io.Fonts->AddFontFromFileTTF("./assets/fonts/recursive_linear_medium.ttf", 18);
    io.Fonts->AddFontFromFileTTF(FONT_ICON_FILE_NAME_FAS, 18 * 2.0f / 3.0f, &icon_config, icons_ranges);

    Browser browser("http://192.168.43.1:12345/",WIDTH,HEIGHT);

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        // Render Other Stuff


        // Render Imgui Stuff
        browser.render();


        // End of render
        ImGui::Render();
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
