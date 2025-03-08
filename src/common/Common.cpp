#include "Common.h"

#include <stacktrace>
#include "VKSC_Utils.h"

namespace Vu {

    void VkCheck(VkResult res) {
        if (res != VK_SUCCESS) {
            auto st = std::stacktrace::current();
            if (st.empty()) {
                std::cout << "Vk error: Source unknown (Stack Trace cannot be captured)" << std::endl;
            }
            auto msg = std::format("[ERROR] VkResult is {0} at {1} line {2}",
                                   VkResultToString(res),
                                   st[1].source_file(), st[1].source_line());

            std::cerr << msg << std::endl;
            throw std::runtime_error(msg.c_str());
        }
    }
}
