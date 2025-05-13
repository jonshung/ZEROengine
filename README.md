<!-- PROJECT SHIELDS -->
[![GPLv3 License][license-shield]][license-url]

<!-- PROJECT LOGO -->
<br />

# ZEROEngine: 3D modular and multi-platform rendering engine
ZEROEngine is a general, multi-platform and multi-API rendering engine that supports for modular expansion and compilation of graphical application. With minimal overhead and intuitive design philosophy as the guiding principles, ZEROEngine aims to provide a solution for quick development, deployment on multiple platform.

<!-- GETTING STARTED -->
## Prerequisites

- C++17 or later compiler
- CMake version 3.25+
- Vulkan SDK 1.3+
- glm
- XCB

## Building

> It is recommended to use CMake for compilation.
</br>

An example of how to build and link a simple application using CMake is presented in the `tests` folder.

Currently, support for installation has not yet been implemented. Manual installation and linking is needed

```Shell
$ mkdir build && cd build
$ cmake ..
$ make
```

The built library file will be popluated with the built `Core` unit at `build/ZEROEngineCore/libZEROEngineCore.dll/a` and `Graphical` unit at `build/ZEROEngineGraphical/ZEROEngineVulkan/libZEROEngineVulkan.dll/a` since only Vulkan is supported at the latest.
</br>

Copy the libraries file to an installation location. Then, copy the include directories in `ZEROEngineCore/include/zeroegine_core` and `ZEROEngineGraphical/ZEROEngineVulkan/include/zeroengine_vulkan` to an `include` directory.

## Using the platform

### Example usage
Include the `Core` and `Graphical` units:
```cpp
...
#include "zeroengine_core/ZEROengine.hpp"
#include "zeroengine_vulkan/VulkanGraphicalModule.hpp"
...
```

Instantiate the logical process and graphical module:
```cpp
...
ZEROengine::ZEROengine app;
VulkanGraphicalModule *vk_graphical_module = new VulkanGraphicalModule;
app.bindGraphicalModule(vk_graphical_module);
app.init();
...
```

Get the `GraphicsPipelineBuffer`, template forward pass pipeline and request pipeline creation:
```cpp
...
VulkanGraphicsPipelineBuffer *pipeline_buffer = vk_graphical_module->getGraphicsPipelineBuffer();
VulkanGraphicsPipelineTemplate *pipeline_template = vk_graphical_module->getForwardPassPipelineTemplate();

...
std::vector<ShaderData> shader_data = { {vert_data, frag_data} };
...

pipeline_buffer->requestGraphicsPipelines(
    vk_graphical_module->getVulkanContext()->getDevice(), 
    *pipeline_template,
    vk_graphical_module->getRenderWindow()->getRenderPass(),
    shader_data
);
...
```

And proceed to the main loop:
```cpp
try {
    app.run();
} catch(const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
}
```

### Building the application

First, add an imported library of the compiled `Core` and `Graphical` unit:
```cmake
...
add_library(ZEROengine::ZEROengineCore SHARED IMPORTED) # Create targets for the libraries
add_library(ZEROengine::ZEROengineVulkan SHARED IMPORTED)

set_target_properties(ZEROengine::ZEROengineCore PROPERTIES IMPORTED_LOCATION <path to libZEROEngineCore.dll/a>)
set_target_properties(ZEROengine::ZEROengineVulkan PROPERTIES IMPORTED_LOCATION <path to libZEROEngineVulkan.dll/a>)
...
```

And link the libraries:
```cmake
target_link_libraries(TARGET
PRIVATE
    ZEROengine::ZEROengineCore
    ZEROengine::ZEROengineVulkan
)
```

Then, include the header files directory:
```cmake
...
target_include_directories(TARGET PUBLIC <path to the include directories>)
...
```

Finally, build the application:
```Shell
$ mkdir build && cd build
$ cmake ..
$ make
```

<!-- CONTRIBUTING -->
## Contributing

Since this is a personal resource, I do not anticipate additional contribution to the repo. But if you have a suggestion that would make this better, please fork the repo and create a pull request. I'll try to allocate time to review and push the changes as soon as possible.

<!-- LICENSE -->
## License

Distributed under the GPLv3 License. See `LICENSE` for more information.

<!-- CONTACT -->
## Contact

[![LinkedIn][linkedin-shield]][linkedin-url]
<br/>
Hung, Ngu Kiet (jonshung) - jonsphilogy@gmail.com

<!-- MARKDOWN LINKS & IMAGES -->
<!-- https://www.markdownguide.org/basic-syntax/#reference-style-links -->
[license-shield]: https://www.gnu.org/graphics/gplv3-127x51.png
[license-url]: https://www.gnu.org/licenses/gpl-3.0.html
[linkedin-shield]: https://img.shields.io/badge/-LinkedIn-black.svg?style=for-the-badge&logo=linkedin&colorB=555
[linkedin-url]: https://www.linkedin.com/in/jonshung/
