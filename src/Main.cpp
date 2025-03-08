// #define VK_NO_PROTOTYPES
// #define VOLK_IMPLEMENTATION
// #include "volk.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include "Common.h"
#include "Scene0.h"

#include "VKSC_Utils.h"


int main(int argc, char* argv[]) {

    PrintAvailableInstanceExtensions();
    Vu::Scene0 scen{};

    try {
        scen.Run();
    } catch (const std::exception& e) {
        std::puts(e.what());
        system("pause");
    }
    return EXIT_SUCCESS;
}
