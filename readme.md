# File Transer
## Setup
### CPRLIB
Build as static library using cmake with release config
`cmake --build build --config Release`
`cmake --install . --config Release`
copy the lib and include directory from the install location into `packages/cpr`


### GLFW
Build as static library using cmake with release config and copy include and lib dir to packages/glfw

### ImGui LunaSVG spdlog
Just clone the repository and the respective folder and that's it.
Note:
	Place all the required imgui files from the repository along with the one used for interfacing with glfw backend and that's all. All the files should be located in the `packages/imgui` 