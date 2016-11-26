#include <iostream>
#include <vector>
#include <mutex>

#include <vulkan/vulkan.h>

#include "allocator.hpp"

#define APP_SHORT_NAME     "VulkanTest"
#define ENGINE_SHORT_NAME  "DummyEngine"

#define SHOW_INSTANCE_LAYERS            false
#define SHOW_PHYSICAL_DEVICE_LAYERS     false

#define SHOW_INSTANCE_EXTENSIONS        false
#define SHOW_PHYSICAL_DEVICE_EXTENSIONS false

#define ENABLE_STANDARD_VALIDATION      false

#define CUSTOM_ALLOCATOR                false

std::mutex device_mutex;
std::mutex instance_mutex;
std::mutex buffer_mutex;
std::mutex buffer_view_mutex;
std::mutex image_mutex;
std::mutex image_view_mutex;

int main(int argc, const char* argv[]) {
  VkApplicationInfo app_info = {};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pNext = nullptr;
  app_info.pApplicationName = APP_SHORT_NAME;
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName = ENGINE_SHORT_NAME;
  app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.apiVersion = VK_API_VERSION_1_0;

  if (SHOW_INSTANCE_LAYERS) {
    uint32_t inst_layer_count;
    std::vector<VkLayerProperties> inst_layer_props;
    vkEnumerateInstanceLayerProperties(&inst_layer_count, nullptr);
    inst_layer_props.resize(inst_layer_count);
    vkEnumerateInstanceLayerProperties(&inst_layer_count, 
     				       inst_layer_props.data());
    if (inst_layer_count != 0) {
      std::cout << "Instance Layers:" << std::endl;
      for (auto& layer : inst_layer_props)
    	std::cout << layer.layerName << std::endl;
      std::cout << std::endl;
    } else
      std::cout << "No instance layers available." << std::endl;
  }

  if (SHOW_INSTANCE_EXTENSIONS) {
    uint32_t inst_ext_count;
    std::vector<VkExtensionProperties> inst_ext_props;
    vkEnumerateInstanceExtensionProperties(nullptr,
					   &inst_ext_count,
					   nullptr);
    inst_ext_props.resize(inst_ext_count);
    vkEnumerateInstanceExtensionProperties(nullptr,
					   &inst_ext_count,
					   inst_ext_props.data());
    if (inst_ext_count != 0) {
      std::cout << "Instance Extensions:" << std::endl;
      for (auto& extension : inst_ext_props)
	std::cout << extension.extensionName << std::endl;
      std::cout << std::endl;
    } else
      std::cout << "No instance extensions available..." << std::endl;
  }

  VkInstanceCreateInfo inst_info = {};
  inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  inst_info.pNext = nullptr;
  inst_info.flags = 0;
  inst_info.pApplicationInfo = &app_info;
  inst_info.enabledExtensionCount = 0;
  inst_info.ppEnabledExtensionNames = nullptr;
  if (ENABLE_STANDARD_VALIDATION) {
    std::cout << "Enabling LunarG standard validation instance layer..." 
	      << std::endl;
    const char* enabled_layer_names[] = {
      "VK_LAYER_LUNARG_standard_validation"
    };
    inst_info.enabledLayerCount = 1;
    inst_info.ppEnabledLayerNames = enabled_layer_names;
  } else {
    inst_info.enabledLayerCount = 0;
    inst_info.ppEnabledLayerNames = nullptr;
  }

  VkInstance inst;
  VkResult res;

  allocator my_alloc;
  VkAllocationCallbacks alloc_callbacks = my_alloc;
  
  if (CUSTOM_ALLOCATOR)
    std::cout << "Using custom allocator..." << std::endl;
  std::cout << "Creating " << APP_SHORT_NAME 
	    << " application instance..." << std::endl;
  res = vkCreateInstance(&inst_info,
			 CUSTOM_ALLOCATOR ? &alloc_callbacks : nullptr,
			 &inst);
  if (res == VK_SUCCESS)
    std::cout << "Instance created successfully!" << std::endl;
  else
    std::cout << "Instance creation failed..." << std::endl;
  
  uint32_t physical_device_count;
  std::vector<VkPhysicalDevice> physical_devices;
  
  res = vkEnumeratePhysicalDevices(inst, &physical_device_count, nullptr);
  if (res == VK_SUCCESS) {
    std::cout << "Found " << physical_device_count << " physical device" 
	      << (physical_device_count == 1 ? "." : "s.") << std::endl;
    physical_devices.resize(physical_device_count);
    res = vkEnumeratePhysicalDevices(inst,
				     &physical_device_count, 
				     physical_devices.data());
    if (res == VK_SUCCESS) {
      std::cout << "Loaded " << physical_devices.size()
		<< " physical device"
		<< (physical_devices.size() == 1 ? "." : "s.") << std::endl;
      VkPhysicalDeviceProperties device_props;
      using device_index = std::vector<VkPhysicalDevice>::size_type;
      for (device_index i = 0; i < physical_devices.size(); i++) {
	vkGetPhysicalDeviceProperties(physical_devices[i], &device_props);
	std::cout << "Device " << i << ": " 
		  << device_props.deviceName << std::endl;
      }
    } else
      std::cout << "Failed to load physical devices..." << std::endl;
  } else
    std::cout << "Could not get number of physical devices..." << std::endl;

  if (SHOW_PHYSICAL_DEVICE_LAYERS) {
    uint32_t device_layer_count;
    std::vector<VkLayerProperties> device_layer_props;
    vkEnumerateDeviceLayerProperties(physical_devices[0], 
				     &device_layer_count, 
				     nullptr);
    device_layer_props.resize(device_layer_count);
    vkEnumerateDeviceLayerProperties(physical_devices[0],
				     &device_layer_count, 
				     device_layer_props.data());
    if (device_layer_count != 0) {
      std::cout << "\nDevice Layers:" << std::endl;
      for (auto& layer : device_layer_props)
	std::cout << layer.layerName << std::endl;
      std::cout << std::endl;
    } else
      std::cout << "No device layers available..." << std::endl;
  }

  if (SHOW_PHYSICAL_DEVICE_EXTENSIONS) {
    uint32_t device_ext_count;
    std::vector<VkExtensionProperties> device_ext_props;
    vkEnumerateDeviceExtensionProperties(physical_devices[0],
					 nullptr,
					 &device_ext_count,
					 nullptr);
    device_ext_props.resize(device_ext_count);
    vkEnumerateDeviceExtensionProperties(physical_devices[0],
					 nullptr,
					 &device_ext_count,
					 device_ext_props.data());
    if (device_ext_count != 0) {
      std::cout << "\nDevice Extensions:" << std::endl;
      for (auto& extension : device_ext_props)
	std::cout << extension.extensionName << std::endl;
      std::cout << std::endl;
    } else
      std::cout << "No device extensions available..." << std::endl;
  }

  std::cout << "Getting physical device memory properties..." << std::endl;
  VkPhysicalDeviceMemoryProperties physical_device_mem_props;
  vkGetPhysicalDeviceMemoryProperties(physical_devices[0],
				      &physical_device_mem_props);
  std::cout << "Type\tHeap\tSize" << std::endl;
  for (uint32_t i = 0; i < physical_device_mem_props.memoryTypeCount; i++) {
    VkMemoryType& memType = physical_device_mem_props.memoryTypes[i];
    VkMemoryHeap& memHeap = physical_device_mem_props.memoryHeaps[memType.heapIndex];
    std::cout << i << "\t" << memType.heapIndex << "\t" << memHeap.size << std::endl;
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
    std::cout << i << "\t" << queue_family_properties[i].queueCount 
	      << std::endl;
  
  uint32_t queue_family_idx = 0;
  uint32_t queue_family_queue_count
    = queue_family_properties[queue_family_idx].queueCount;
  
  VkPhysicalDeviceFeatures supported_features;
  vkGetPhysicalDeviceFeatures(physical_devices[0], &supported_features);

  std::vector<float> queue_priorities(queue_family_queue_count, 0.0);
  VkDeviceQueueCreateInfo device_queue_create_info = {};
  device_queue_create_info.sType = 
    VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  device_queue_create_info.pNext = nullptr;
  device_queue_create_info.flags = 0;
  device_queue_create_info.queueFamilyIndex = queue_family_idx;
  device_queue_create_info.queueCount = queue_family_queue_count;
  device_queue_create_info.pQueuePriorities = queue_priorities.data();
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
   		       CUSTOM_ALLOCATOR ? &alloc_callbacks : nullptr,
   		       &device);

  if (res == VK_SUCCESS)
    std::cout << "Device created successfully!" << std::endl;
  else
    std::cout << "Failed to create device..." << std::endl;

  VkBufferCreateInfo buf_create_info = {};
  buf_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buf_create_info.pNext = nullptr;
  buf_create_info.flags = 0;
  buf_create_info.size = 1024*1024;
  buf_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT
    | VK_BUFFER_USAGE_TRANSFER_DST_BIT
    | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
  buf_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  buf_create_info.queueFamilyIndexCount = 0;
  buf_create_info.pQueueFamilyIndices = nullptr;
  
  VkBuffer buffer;
  std::cout << "Creating buffer..." << std::endl;
  res = vkCreateBuffer(device,
		       &buf_create_info,
		       CUSTOM_ALLOCATOR ? &alloc_callbacks : nullptr,
		       &buffer);
  if (res == VK_SUCCESS)
    std::cout << "Buffer created successfully!" << std::endl;
  else
    std::cout << "Failed to create buffer..." << std::endl;

  // TODO: Bind memory to buffer, uncomment below

  // VkBufferViewCreateInfo buf_view_create_info = {};
  // buf_view_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
  // buf_view_create_info.pNext = nullptr;
  // buf_view_create_info.flags = 0;
  // buf_view_create_info.buffer = buffer;
  // buf_view_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
  // buf_view_create_info.offset = 0;
  // buf_view_create_info.range = 1024;

  // VkBufferView buffer_view;
  // std::cout << "Creating buffer view..." << std::endl;
  // res = vkCreateBufferView(device,
  // 			   &buf_view_create_info,
  // 			   CUSTOM_ALLOCATOR ? &alloc_callbacks : nullptr,
  // 			   &buffer_view);
  // if (res == VK_SUCCESS)
  //   std::cout << "Buffer view created successfully!" << std::endl;
  // else
  //   std::cout << "Failed to create buffer view..." << std::endl;

  // Destroy buffer view
  // {
  //   std::lock_guard<std::mutex> lock(buffer_view_mutex);
  //   std::cout << "Destroying buffer view..." << std::endl;
  //   vkDestroyBufferView(device,
  // 			buffer_view,
  // 			CUSTOM_ALLOCATOR ? &alloc_callbacks : nullptr);
  // }

  // Destroy buffer
  {
    std::lock_guard<std::mutex> lock(buffer_mutex);
    std::cout << "Destroying buffer..." << std::endl;
    vkDestroyBuffer(device,
		    buffer,
		    CUSTOM_ALLOCATOR ? &alloc_callbacks : nullptr);
  }

  VkImageCreateInfo img_create_info = {};
  img_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  img_create_info.pNext = nullptr;
  img_create_info.flags = 0;
  img_create_info.imageType = VK_IMAGE_TYPE_2D;
  img_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
  VkExtent3D dimensions = {};
  dimensions.width = 1024;
  dimensions.height = 1024;
  dimensions.depth = 1;
  img_create_info.extent = dimensions;
  img_create_info.mipLevels = 1;
  img_create_info.arrayLayers = 1;
  img_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
  img_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;  
  img_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
  img_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  img_create_info.queueFamilyIndexCount = 0;
  img_create_info.pQueueFamilyIndices = nullptr;
  img_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

  VkImage image;
  std::cout << "Creating image..." << std::endl;
  res = vkCreateImage(device,
		      &img_create_info,
		      CUSTOM_ALLOCATOR ? &alloc_callbacks : nullptr,
		      &image);
  if (res == VK_SUCCESS)
    std::cout << "Image created successfully!" << std::endl;
  else if (res == VK_ERROR_OUT_OF_HOST_MEMORY)
    std::cout << "Failed to create image: out of host memory" << std::endl;
  else if (res == VK_ERROR_OUT_OF_DEVICE_MEMORY)
    std::cout << "Failed to create image: out of device memory" << std::endl;
  else if (res == VK_ERROR_VALIDATION_FAILED_EXT)
    std::cout << "Failed to create image: validation failed" << std::endl;
  else
    std::cout << "Failed to create image: unknown error" << std::endl;

  VkImageSubresource img_subresource = {};
  img_subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  img_subresource.mipLevel = 0;
  img_subresource.arrayLayer = 0;

  VkSubresourceLayout subresource_layout;
  std::cout << "Getting subresource layout for image..."
	    << std::endl;
  vkGetImageSubresourceLayout(device,
			      image,
			      &img_subresource,
			      &subresource_layout);
  std::cout << "Subresource Offset: " << subresource_layout.offset
	    << std::endl;
  std::cout << "Subresource Size: " << subresource_layout.size
	    << std::endl;

  // TODO: Bind memory to image, uncomment below

  // VkImageViewCreateInfo img_view_create_info = {};
  // img_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  // img_view_create_info.pNext = nullptr;
  // img_view_create_info.flags = 0;
  // img_view_create_info.image = image;
  // img_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
  // img_view_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
  // VkComponentMapping component_mapping = {};
  // component_mapping.r = VK_COMPONENT_SWIZZLE_IDENTITY;
  // component_mapping.g = VK_COMPONENT_SWIZZLE_IDENTITY;
  // component_mapping.b = VK_COMPONENT_SWIZZLE_IDENTITY;
  // component_mapping.a = VK_COMPONENT_SWIZZLE_IDENTITY;
  // img_view_create_info.components = component_mapping;
  // VkImageSubresourceRange subresource_range = {};
  // subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  // subresource_range.baseMipLevel = 0;
  // subresource_range.levelCount = 1;
  // subresource_range.baseArrayLayer = 0;
  // subresource_range.layerCount = 1;
  // img_view_create_info.subresourceRange = subresource_range;
  // VkImageView image_view;
  // std::cout << "Creating image view..." << std::endl;
  // res = vkCreateImageView(device,
  // 			  &img_view_create_info,
  // 			  nullptr,
  // 			  &image_view);
  // if (res == VK_SUCCESS)
  //   std::cout << "Image view created successfully!" << std::endl;
  // else if (res == VK_ERROR_OUT_OF_HOST_MEMORY)
  //   std::cout << "Failed to create image view: out of host memory" << std::endl;
  // else if (res == VK_ERROR_OUT_OF_DEVICE_MEMORY)
  //   std::cout << "Failed to create image view: out of device memory" << std::endl;
  // else if (res == VK_ERROR_VALIDATION_FAILED_EXT)
  //   std::cout << "Failed to create image view: validation failed" << std::endl;
  // else
  //   std::cout << "Failed to create image view: unknown error" << std::endl;

  // Destroy image view
  // {
  //   std::lock_guard<std::mutex> lock(image_view_mutex);
  //   std::cout << "Destroying image view..." << std::endl;
  //   vkDestroyImageView(device,
  // 		       image_view,
  // 		       CUSTOM_ALLOCATOR ? &alloc_callbacks : nullptr);
  // }

  // Destroy image
  {
    std::lock_guard<std::mutex> lock(image_mutex);
    std::cout << "Destroying image..." << std::endl;
    vkDestroyImage(device,
		   image,
		   CUSTOM_ALLOCATOR ? &alloc_callbacks : nullptr);
  }

  std::cout << "Waiting for device to idle..." << std::endl;
  res = vkDeviceWaitIdle(device);
  if (res == VK_SUCCESS)
    std::cout << "Device is idle..." << std::endl;
  else if (res == VK_ERROR_OUT_OF_HOST_MEMORY)
    std::cout << "Wait failed: no available host memory..." << std::endl;
  else if (res == VK_ERROR_OUT_OF_DEVICE_MEMORY)
    std::cout << "Wait failed: no available device memory..." << std::endl;
  else if (res == VK_ERROR_DEVICE_LOST)
    std::cout << "Wait failed: device was lost..." << std::endl;

  // Synchronize access to device while destroying it
  {
    std::lock_guard<std::mutex> lock(device_mutex);
    std::cout << "Destroying device..." << std::endl;
    vkDestroyDevice(device,
		    CUSTOM_ALLOCATOR ? &alloc_callbacks : nullptr);
  }

  // Synchronize instance while destroying it
  {
    std::lock_guard<std::mutex> lock(instance_mutex);
    std::cout << "Destroying instance..." << std::endl;
    vkDestroyInstance(inst,
		      CUSTOM_ALLOCATOR ? &alloc_callbacks : nullptr);
  }

  return 0;
}
