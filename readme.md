
# File Transer

<p align="center">
  <img width="256" height="256" src="https://raw.githubusercontent.com/akash1474/file_transfer/abb7d72fe8b68efcd46e5af9df474f1af73720e3/assets/iot.svg"> 
</p>
A Simple Software Application for transferring files from phone to pc. This is done by starting a server on your android device using the application called SimpleHttpServer. The IP/URL displayed after starting the server is required to be entered to be connected. Once connected android device files and folder can be explored and downloaded.

The application allows multiple files located in different folder to be downloaded simultaneously. I also provides a minimal and interactive user interface.


> **Warning**
> The application is still in development stage some unwanted bugs might occur. Feel free to raise issues on this github page.


## Setup
Requirements:
* Premake5 - Configuration Generator
* Visual Studio 2022 - Compiling 
* SimpleHttpServer

### CPRLIB
Build as static library using cmake with release config
`cmake --build build --config Release`
`cmake --install . --config Release`
copy the lib and include directory from the install location into `packages/cpr`


### GLFW
Build as static library using cmake with release config and copy include and lib dir to packages/glfw

### ImGui LunaSVG spdlog
Just clone the repository and the respective folder and that's it.
> **Note**
> Place all the required imgui files from the repository along with the one used for interfacing with glfw backend and that's all. All the files should be located in the `packages/imgui` 