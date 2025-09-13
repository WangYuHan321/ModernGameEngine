#include "Vulkan.h"

int main()
{
    RHIVulkan rhiVulkan;
    rhiVulkan.InitWindow();
    rhiVulkan.InitVulkan();
    rhiVulkan.MainLoop();
    
    return 0;
}






