#include <iostream>

#include <vulkan/vulkan.h>

#define APP_SHORT_NAME     "VulkanTest"
#define ENGINE_SHORT_NAME  "DummyEngine"

int main(int argc, const char* argv[]) {
  VkApplicationInfo app_info = {};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pNext = nullptr;
  app_info.pApplicationName = APP_SHORT_NAME;
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName = ENGINE_SHORT_NAME;
  app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo inst_info = {};
  inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  inst_info.pNext = nullptr;
  inst_info.flags = 0;
  inst_info.pApplicationInfo = &app_info;
  inst_info.enabledExtensionCount = 0;
  inst_info.ppEnabledExtensionNames = nullptr;
  inst_info.enabledLayerCount = 0;
  inst_info.ppEnabledLayerNames = nullptr;

  VkInstance inst;
  VkResult res;

  std::cout << "Creating " << APP_SHORT_NAME << " application instance..." << std::endl;
  res = vkCreateInstance(&inst_info, nullptr, &inst);
  if (res == VK_SUCCESS)
    std::cout << "Instance created successfully!" << std::endl;
  else
    std::cout << "Instance creation failed..." << std::endl;
  std::cout << "Destroying instance..." << std::endl;
  vkDestroyInstance(inst, nullptr);

  return 0;
}
