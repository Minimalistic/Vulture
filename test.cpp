#include <iostream>
#include <vector>

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
  else {
    std::cout << "Instance creation failed..." << std::endl;
    exit(-1);
  }
  
  uint32_t physical_device_count;
  std::vector<VkPhysicalDevice> physical_devices;
  
  res = vkEnumeratePhysicalDevices(inst, &physical_device_count, nullptr);
  if (res == VK_SUCCESS) {
    std::cout << "Found " << physical_device_count << " physical device" << (physical_device_count == 1 ? "." : "s.") << std::endl;
    physical_devices.resize(physical_device_count);
    res = vkEnumeratePhysicalDevices(inst, &physical_device_count, physical_devices.data());
    if (res == VK_SUCCESS) {
      std::cout << "Loaded " << physical_devices.size() << " physical device" << (physical_devices.size() == 1 ? "." : "s.") << std::endl;
      VkPhysicalDeviceProperties device_props;
      for (int i = 0; i < physical_devices.size(); i++) {
	vkGetPhysicalDeviceProperties(physical_devices[i], &device_props);
	std::cout << "Device " << i << ": " << device_props.deviceName << std::endl;
      }
    } else {
      std::cout << "Failed to load physical devices..." << std::endl;
      exit(-1);
    }
  } else {
    std::cout << "Could not get number of physical devices..." << std::endl;
    exit(-1);
  }
  
  std::cout << "Destroying instance..." << std::endl;
  vkDestroyInstance(inst, nullptr);
  
  return 0;
}
