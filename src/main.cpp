#include "FontAwesome6.h"
#include "GLFW/glfw3.h"
#include "imgui.h"
#include "pch.h"
#include "Browser.h"

#define WIDTH 400
#define HEIGHT 600


void window_close_callback(GLFWwindow* window)
{
    glfwSetWindowShouldClose(window, GLFW_TRUE);
    glfwDestroyWindow(window);
}

int main(void){
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
    ImGui_ImplGlfw_InitForOpenGL(window, true);

    if (!ImGui_ImplOpenGL2_Init()) std::cout << "Failed to initit OpenGL 2" << std::endl;

    io.Fonts->Clear();
    ImFontConfig icon_config;
    icon_config.MergeMode = true;
    icon_config.PixelSnapH = true;
    static const ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_FA};
    size_t font_data_size = sizeof(data_font);
    size_t icon_data_size = sizeof(data_icon);

    io.Fonts->AddFontFromMemoryTTF(data_font, (int)font_data_size,16);
    io.Fonts->AddFontFromMemoryTTF(data_icon, (int)icon_data_size,20*2.0f/3.0f,&icon_config,icons_ranges);

    io.Fonts->AddFontFromMemoryTTF(data_font, (int)font_data_size,18);
    io.Fonts->AddFontFromMemoryTTF(data_icon, (int)icon_data_size,18*2.0f/3.0f,&icon_config,icons_ranges);

    io.Fonts->AddFontFromMemoryTTF(data_font, (int)font_data_size,14);
    io.Fonts->AddFontFromMemoryTTF(data_icon, (int)icon_data_size,14*2.0f/3.0f,&icon_config,icons_ranges);

    io.Fonts->AddFontFromMemoryTTF(data_font, (int)font_data_size,26);
    io.Fonts->AddFontFromMemoryTTF(data_icon, (int)icon_data_size,26*2.0f/3.0f,&icon_config,icons_ranges);

    glfwSwapInterval(1);
    Browser browser;
    ImGuiStyle& style = ImGui::GetStyle();
    style.FrameRounding = 2.0f;
    style.ItemSpacing.y=6.0f;
    style.ScrollbarRounding=2.0f;
    glfwSetWindowCloseCallback(window, window_close_callback);

    while (!glfwWindowShouldClose(window)) {
        if(browser.shouldCloseWindow){
            glfwDestroyWindow(window);
            break;
        }
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
