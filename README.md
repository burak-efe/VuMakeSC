# VuMakeSC
VuMakeSC: very basic VulkanSC renderer

This is a port of my regular Vulkan renderer VuMake to VulkanSC, so some parts won't make sense for a SC environment, like the dynamic resource system.
This project is just a practice and it's not following any coding standards like MISRA, do not use it on your airplane 🤓

Used libs: <br>
- VKSC-Headers <br>
- VKSC-Emulation <br>
- VKSC-Loader <br>

Also: <br>
- fastgltf : for gltf loading <br>
- glm : for math <br>
- stb_image : for texture load <br>
- slang : shader language and as spirv emitter  <br>
Also I let loader to get its VKSC-ValidationLayers from regular NVIDIA driver, instead of compiling myself. <br>

Used Vulkan Features: <br>
- buffer device address <br>
- scalar block layout <br>
- descriptor indexing <br>
- khr display (via emulation) <br>
- vertex pulling <br>
- sync 2 <br>


![VuMakeSC_gif](https://github.com/user-attachments/assets/1740189a-0406-493e-82c7-184c77309719)
