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
      using device_index = std::vector<VkPhysicalDevice>::size_type;
      for (device_index i = 0; i < physical_devices.size(); i++) {
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

  uint32_t queue_family_property_count;
  std::vector<VkQueueFamilyProperties> queue_family_properties;
  vkGetPhysicalDeviceQueueFamilyProperties(physical_devices[0], 
					   &queue_family_property_count, 
					   nullptr);
  std::cout << "Found " << queue_family_property_count << " queue " 
	    << (queue_family_property_count == 1 ? "family " : "families ") 
	    << "for this physical device." << std::endl;
  queue_family_properties.resize(queue_family_property_count);
  vkGetPhysicalDeviceQueueFamilyProperties(physical_devices[0],
					   &queue_family_property_count,
					   queue_family_properties.data());

  using queue_fam_idx = std::vector<VkQueueFamilyProperties>::size_type;
  std::cout << "Family\tQueues" << std::endl;
  for (queue_fam_idx i = 0; i < queue_family_properties.size(); i++)
    std::cout << i << "\t" << queue_family_properties[i].queueCount << std::endl;
  
  uint32_t queue_family_idx = 0;
  uint32_t queue_family_queue_count
    = queue_family_properties[queue_family_idx].queueCount;
  
  VkPhysicalDeviceFeatures supported_features;
  vkGetPhysicalDeviceFeatures(physical_devices[0], &supported_features);

  VkDeviceQueueCreateInfo device_queue_create_info = {};
  device_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  device_queue_create_info.pNext = nullptr;
  device_queue_create_info.flags = 0;
  device_queue_create_info.queueFamilyIndex = queue_family_idx;
  device_queue_create_info.queueCount = queue_family_queue_count;
  device_queue_create_info.pQueuePriorities = nullptr;
  VkDeviceQueueCreateInfo device_queue_create_infos[1] = 
    {device_queue_create_info};

  VkDeviceCreateInfo device_create_info = {};
  device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  device_create_info.pNext = nullptr;
  device_create_info.flags = 0;
  device_create_info.queueCreateInfoCount = 1;
  device_create_info.pQueueCreateInfos = device_queue_create_infos;
  device_create_info.enabledLayerCount = 0;
  device_create_info.ppEnabledLayerNames = nullptr;
  device_create_info.enabledExtensionCount = 0;
  device_create_info.ppEnabledExtensionNames = nullptr;
  device_create_info.pEnabledFeatures = &supported_features;

  VkDevice device;
  std::cout << "Creating device..." << std::endl;
  res = vkCreateDevice(physical_devices[0],
   		       &device_create_info,
   		       nullptr,
   		       &device);

  if (res == VK_SUCCESS)
    std::cout << "Device created successfully!" << std::endl;
  else
    std::cout << "Failed to create device..." << std::endl;

  std::cout << "Destroying device..." << std::endl;
  vkDestroyDevice(device, nullptr);

  std::cout << "Destroying instance..." << std::endl;
  vkDestroyInstance(inst, nullptr);
  
  return 0;
}
