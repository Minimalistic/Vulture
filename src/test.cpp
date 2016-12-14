#include <iostream>
#include <vector>
#include <mutex>
#include <cstring>

#include <vulkan/vulkan.h>

#include "allocator.hpp"
#include "util.hpp"

#define APP_SHORT_NAME     "VultureTest"
#define ENGINE_SHORT_NAME  "Vulture"

#define SHOW_INSTANCE_LAYERS            false
#define SHOW_PHYSICAL_DEVICE_LAYERS     false

#define SHOW_INSTANCE_EXTENSIONS        false
#define SHOW_PHYSICAL_DEVICE_EXTENSIONS false

#define ENABLE_STANDARD_VALIDATION      false

#define CUSTOM_ALLOCATOR                false

#define RESOURCE_BUFFER                 0
#define RESOURCE_IMAGE                  1

#define BUFFER_COUNT                    13
#define IMAGE_COUNT                     21
#define COMMAND_BUFFER_COUNT            5

#define MAX_QUEUES                      64

#define READ_OFFSET                     0
#define READ_LENGTH                     64

#define HOST_COHERENT(X) ((X & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) != 0)
#define HOST_VISIBLE(X)  ((X & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0)

std::mutex device_mutex;
std::mutex instance_mutex;
std::vector<std::mutex> buffer_mutex(BUFFER_COUNT);
std::vector<std::mutex> buffer_view_mutex(BUFFER_COUNT);
std::vector<std::mutex> image_mutex(IMAGE_COUNT);
std::vector<std::mutex> image_view_mutex(IMAGE_COUNT);
std::mutex memory_mutex[2];
std::mutex command_pool_mutex;
std::vector<std::mutex> command_buffer_mutex(COMMAND_BUFFER_COUNT);
std::vector<std::mutex> queue_mutex(MAX_QUEUES);

allocator my_alloc = {};

uint32_t phys_device_idx = 0;
uint32_t queue_family_idx = 0;
uint32_t queue_family_queue_count;
uint32_t mem_types[2] = {UINT32_MAX, UINT32_MAX};

VkResult res;
VkInstance inst;
VkAllocationCallbacks alloc_callbacks = my_alloc;
std::vector<VkPhysicalDevice> physical_devices;
VkPhysicalDeviceMemoryProperties physical_device_mem_props;
std::vector<VkQueueFamilyProperties> queue_family_properties;
VkDevice device;
std::vector<VkBuffer> buffers;
std::vector<VkImage> images;
std::vector<VkSubresourceLayout> subresource_layouts;
std::vector<VkMemoryRequirements> buf_mem_requirements;
std::vector<VkMemoryRequirements> img_mem_requirements;
VkDeviceSize mem_size[2];
VkDeviceMemory memory[2];
std::vector<VkBufferView> buffer_views;
std::vector<VkImageView> image_views;
std::vector<VkQueue> queues;

void create_instance()
{
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

  std::cout << "Creating " << ENGINE_SHORT_NAME 
	    << " application instance (" APP_SHORT_NAME << ")..."
	    << std::endl;
  res = vkCreateInstance(&inst_info,
			 CUSTOM_ALLOCATOR ? &alloc_callbacks : nullptr,
			 &inst);
  if (res == VK_SUCCESS)
    std::cout << "Instance created successfully!" << std::endl;
  else
    std::cout << "Instance creation failed..." << std::endl;
}

void enumerate_physical_devices()
{
  uint32_t physical_device_count;
  
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
	std::cout << "Physical Device " << i << ": " 
		  << device_props.deviceName << std::endl;
      }
    } else
      std::cout << "Failed to load physical devices..." << std::endl;
  } else
    std::cout << "Could not get number of physical devices..." << std::endl;
}

void get_physical_device_memory_properties()
{
  std::cout << "Getting memory properties for physical device "
	    << phys_device_idx << "..." << std::endl;

  vkGetPhysicalDeviceMemoryProperties(physical_devices[phys_device_idx],
				      &physical_device_mem_props);
  std::printf("%-10s%-10s%-20s%-20s%-20s\n",
	      "Type", "Heap", "Size", "Host_Coherent", "Host_Visible");
  for (uint32_t i = 0; i < physical_device_mem_props.memoryTypeCount; i++) {
    VkMemoryType& memType = physical_device_mem_props.memoryTypes[i];
    VkMemoryHeap& memHeap =
      physical_device_mem_props.memoryHeaps[memType.heapIndex];
    std::printf("%-10i%-10i%-20lu%-20s%-20s\n",
		i,
		memType.heapIndex,
		(unsigned long) memHeap.size,
		HOST_COHERENT(memType.propertyFlags) ? "Y" : "N",
		HOST_VISIBLE(memType.propertyFlags) ? "Y" : "N");
  }
}

void get_queue_family_properties()
{
  uint32_t queue_family_property_count;
  vkGetPhysicalDeviceQueueFamilyProperties(physical_devices[phys_device_idx], 
					   &queue_family_property_count, 
					   nullptr);
  std::cout << "Found " << queue_family_property_count << " queue " 
	    << (queue_family_property_count == 1 ? "family " : "families ") 
	    << "for this physical device." << std::endl;
  queue_family_properties.resize(queue_family_property_count);
  vkGetPhysicalDeviceQueueFamilyProperties(physical_devices[phys_device_idx],
					   &queue_family_property_count,
					   queue_family_properties.data());

  using queue_fam_idx = std::vector<VkQueueFamilyProperties>::size_type;
  std::cout << "Family\tQueues" << std::endl;
  for (queue_fam_idx i = 0; i < queue_family_properties.size(); i++)
    std::cout << i << "\t" << queue_family_properties[i].queueCount 
	      << std::endl;
}

void create_device()
{
  VkPhysicalDeviceFeatures supported_features;
  vkGetPhysicalDeviceFeatures(physical_devices[phys_device_idx], &supported_features);

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

  std::cout << "Creating device..." << std::endl;
  res = vkCreateDevice(physical_devices[phys_device_idx],
   		       &device_create_info,
   		       CUSTOM_ALLOCATOR ? &alloc_callbacks : nullptr,
   		       &device);

  if (res == VK_SUCCESS)
    std::cout << "Device created successfully!" << std::endl;
  else
    std::cout << "Failed to create device..." << std::endl;
}

void create_buffers()
{
  std::vector<VkBufferCreateInfo> buf_create_infos;
  buf_create_infos.resize(BUFFER_COUNT);
  for (unsigned int i = 0; i != BUFFER_COUNT; i++) {
    buf_create_infos[i].sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buf_create_infos[i].pNext = nullptr;
    buf_create_infos[i].flags = 0;
    buf_create_infos[i].size = 1024*1024;
    buf_create_infos[i].usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT
      | VK_BUFFER_USAGE_TRANSFER_DST_BIT
      | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
    buf_create_infos[i].sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buf_create_infos[i].queueFamilyIndexCount = 0;
    buf_create_infos[i].pQueueFamilyIndices = nullptr;
  }
  
  buffers.resize(BUFFER_COUNT);
  std::cout << "Creating buffers (" << BUFFER_COUNT << ")..." << std::endl;
  for (int i = 0; i != BUFFER_COUNT; i++) {
    res = vkCreateBuffer(device,
			 &buf_create_infos[i],
			 CUSTOM_ALLOCATOR ? &alloc_callbacks : nullptr,
			 &buffers[i]);
    if (res == VK_SUCCESS)
      std::cout << "Buffer " << i << " created successfully!" << std::endl;
    else
      std::cout << "Failed to create buffer " << i << "..." << std::endl;
  }
}

void create_images()
{
  std::vector<VkImageCreateInfo> img_create_infos;
  img_create_infos.resize(IMAGE_COUNT);
  for (unsigned int i = 0; i != IMAGE_COUNT; i++) {
    img_create_infos[i].sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    img_create_infos[i].pNext = nullptr;
    img_create_infos[i].flags = 0;
    img_create_infos[i].imageType = VK_IMAGE_TYPE_2D;
    img_create_infos[i].format = VK_FORMAT_R8G8B8A8_UNORM;
    VkExtent3D dimensions = {};
    dimensions.width = 1024;
    dimensions.height = 1024;
    dimensions.depth = 1;
    img_create_infos[i].extent = dimensions;
    img_create_infos[i].mipLevels = 1;
    img_create_infos[i].arrayLayers = 1;
    img_create_infos[i].samples = VK_SAMPLE_COUNT_1_BIT;
    img_create_infos[i].tiling = VK_IMAGE_TILING_LINEAR;  
    img_create_infos[i].usage = VK_IMAGE_USAGE_SAMPLED_BIT |
      VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
      VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    img_create_infos[i].sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    img_create_infos[i].queueFamilyIndexCount = 0;
    img_create_infos[i].pQueueFamilyIndices = nullptr;
    img_create_infos[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  }

  images.resize(IMAGE_COUNT);
  std::cout << "Creating images (" << IMAGE_COUNT << ")..." << std::endl;
  for (unsigned int i = 0; i != IMAGE_COUNT; i++) {
    res = vkCreateImage(device,
			&img_create_infos[i],
			CUSTOM_ALLOCATOR ? &alloc_callbacks : nullptr,
			&images[i]);
    if (res == VK_SUCCESS)
      std::cout << "Image " << i << " created successfully!" << std::endl;
    else if (res == VK_ERROR_OUT_OF_HOST_MEMORY)
      std::cout << "Failed to create image " << i
		<< ": out of host memory" << std::endl;
    else if (res == VK_ERROR_OUT_OF_DEVICE_MEMORY)
      std::cout << "Failed to create image " << i
		<< ": out of device memory" << std::endl;
    else if (res == VK_ERROR_VALIDATION_FAILED_EXT)
      std::cout << "Failed to create image " << i << ": validation failed"
		<< std::endl;
    else
      std::cout << "Failed to create image " << i << ": unknown error"
		<< std::endl;
  }
}

void get_subresource_layouts()
{
  subresource_layouts.resize(IMAGE_COUNT);
  
  VkImageSubresource img_subresource = {};
  img_subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  img_subresource.mipLevel = 0;
  img_subresource.arrayLayer = 0;

  for (unsigned int i = 0; i != IMAGE_COUNT; i++) {
    std::cout << "Getting subresource layout for image " << i << "..."
	      << std::endl;
    vkGetImageSubresourceLayout(device,
				images[i],
				&img_subresource,
				&subresource_layouts[i]);
  }
}

void get_buffer_memory_requirements()
{
  buf_mem_requirements.resize(BUFFER_COUNT);
  mem_size[RESOURCE_BUFFER] = 0;
  for (unsigned int i = 0; i != BUFFER_COUNT; i++) {
    std::cout << "Fetching memory requirements for buffer "
	      << i << "..." << std::endl;
    vkGetBufferMemoryRequirements(device,
				  buffers[i],
				  &buf_mem_requirements[i]);
    mem_size[RESOURCE_BUFFER] += buf_mem_requirements[i].size;
  }
}

void get_image_memory_requirements()
{
  img_mem_requirements.resize(IMAGE_COUNT);
  mem_size[RESOURCE_IMAGE] = 0;
  for (unsigned int i = 0; i != IMAGE_COUNT; i++) {
    std::cout << "Fetching memory requirements for image "
	      << i << "..." << std::endl;
    vkGetImageMemoryRequirements(device,
				 images[i],
				 &img_mem_requirements[i]);
    mem_size[RESOURCE_IMAGE] += img_mem_requirements[i].size;
  }
}

void find_memory_types()
{  
  for (uint32_t cur = 0;
       cur < physical_device_mem_props.memoryTypeCount;
       cur++) {
    VkMemoryType& mem_type = physical_device_mem_props.memoryTypes[cur];
    VkMemoryHeap& mem_heap =
      physical_device_mem_props.memoryHeaps[mem_type.heapIndex];
    
    if (mem_heap.size <= 0 ||
	!HOST_COHERENT(mem_type.propertyFlags) ||
	!HOST_VISIBLE(mem_type.propertyFlags))
      continue;
    
    if (mem_types[RESOURCE_BUFFER] == UINT32_MAX &&
	mem_size[RESOURCE_BUFFER] <= mem_heap.size &&
	supports_mem_reqs(cur, buf_mem_requirements))
      mem_types[RESOURCE_BUFFER] = cur;
    
    if (mem_types[RESOURCE_IMAGE] == UINT32_MAX &&
	mem_size[RESOURCE_IMAGE] <= mem_heap.size &&
	supports_mem_reqs(cur, img_mem_requirements))
      if (cur == mem_types[RESOURCE_BUFFER] &&
	  mem_size[RESOURCE_BUFFER]+mem_size[RESOURCE_IMAGE] > mem_heap.size)
	continue;
      else
	mem_types[RESOURCE_IMAGE] = cur;

    if (mem_types[RESOURCE_BUFFER] != UINT32_MAX
	&& mem_types[RESOURCE_IMAGE] != UINT32_MAX)
      break;
  }
}

void allocate_buffer_memory()
{
  VkMemoryAllocateInfo buf_mem_allocate_info = {};
  buf_mem_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  buf_mem_allocate_info.pNext = nullptr;
  buf_mem_allocate_info.allocationSize = mem_size[RESOURCE_BUFFER];
  buf_mem_allocate_info.memoryTypeIndex = mem_types[RESOURCE_BUFFER];
  
  std::cout << "Allocating buffer memory..." << std::endl;
  res = vkAllocateMemory(device,
			 &buf_mem_allocate_info,
			 CUSTOM_ALLOCATOR ? &alloc_callbacks : nullptr,
			 &memory[RESOURCE_BUFFER]);
  if (res == VK_SUCCESS)
    std::cout << "Buffer memory allocated successfully!" << std::endl;
  else
    std::cout << "Failed to allocate buffer memory..." << std::endl;
}

void allocate_image_memory()
{
  VkMemoryAllocateInfo img_mem_allocate_info = {};
  img_mem_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  img_mem_allocate_info.pNext = nullptr;
  img_mem_allocate_info.allocationSize = mem_size[RESOURCE_IMAGE];
  img_mem_allocate_info.memoryTypeIndex = mem_types[RESOURCE_IMAGE];
  
  std::cout << "Allocating image memory..." << std::endl;
  res = vkAllocateMemory(device,
			 &img_mem_allocate_info,
			 CUSTOM_ALLOCATOR ? &alloc_callbacks : nullptr,
			 &memory[RESOURCE_IMAGE]);
  if (res == VK_SUCCESS)
    std::cout << "Image memory allocated successfully!" << std::endl;
  else
    std::cout << "Failed to allocate image memory..." << std::endl;
}

void write_buffer_memory()
{
  void* buf_data;
  std::cout << "Mapping buffer memory..." << std::endl;
  std::lock_guard<std::mutex> lock(memory_mutex[RESOURCE_BUFFER]);
  res = vkMapMemory(device,
		    memory[RESOURCE_BUFFER],
		    0,
		    VK_WHOLE_SIZE,
		    0,
		    &buf_data);
  if (res == VK_SUCCESS)
    std::cout << "Buffer memory mapped successfully!" << std::endl;
  else
    std::cout << "Failed to map buffer memory..." << std::endl;
 
  char* str = new char[mem_size[RESOURCE_BUFFER]];
  unsigned int k = 0;
  for (unsigned int i = 0; i != BUFFER_COUNT; i++) {
    for (unsigned int j = 0; j != buf_mem_requirements[i].size; j++) {
      int off = rand() % 26;
      unsigned char c = 'A' + off;
      str[k++] = c;
    }
  }
  str[mem_size[RESOURCE_BUFFER]-1] = '\0';
  memcpy(buf_data, str, mem_size[RESOURCE_BUFFER]);
  delete[](str);

  std::cout << "Unmapping buffer memory..." << std::endl;
  vkUnmapMemory(device,
		memory[RESOURCE_BUFFER]);
}

void write_image_memory()
{
  void* img_data = nullptr;
  std::cout << "Mapping image memory..." << std::endl;
  std::lock_guard<std::mutex> lock(memory_mutex[RESOURCE_IMAGE]);
  res = vkMapMemory(device,
		    memory[RESOURCE_IMAGE],
		    0,
		    VK_WHOLE_SIZE,
		    0,
		    &img_data);
  if (res == VK_SUCCESS)
    std::cout << "Image memory mapped successfully!" << std::endl;
  else
    std::cout << "Failed to map image memory..." << std::endl;

  // TODO: Write data to image memory

  std::cout << "Unmapping image memory..." << std::endl;
  vkUnmapMemory(device,
		memory[RESOURCE_IMAGE]);
}

void bind_buffer_memory()
{
  std::vector<std::unique_lock<std::mutex>> locks;
  for (auto& mut : buffer_mutex)
    locks.emplace_back(mut, std::defer_lock);
  VkDeviceSize offset = 0;
  for (unsigned int i = 0; i != BUFFER_COUNT; i++) {
    locks[i].lock();
    std::cout << "Binding buffer memory to buffer " << i
	      << "..." << std::endl;
    res = vkBindBufferMemory(device,
			     buffers[i],
			     memory[RESOURCE_BUFFER],
			     offset);
    offset += buf_mem_requirements[i].size;
    if (res == VK_SUCCESS)
      std::cout << "Buffer memory bound to buffer " << i
		<< " successfully!" << std::endl;
    else
      std::cout << "Failed to bind buffer memory to buffer " << i
		<< "..." << std::endl;
    locks[i].unlock();
  }
}

void bind_image_memory()
{
  std::vector<std::unique_lock<std::mutex>> locks;
  for (auto& mut : image_mutex)
    locks.emplace_back(mut, std::defer_lock);
  VkDeviceSize offset = 0;
  for (unsigned int i = 0; i != IMAGE_COUNT; i++) {
    std::cout << "Binding image memory to image "
	      << i << "..." << std::endl;
    locks[i].lock();
    res = vkBindImageMemory(device,
			    images[i],
			    memory[RESOURCE_IMAGE],
			    offset);
    offset += img_mem_requirements[i].size;
    if (res == VK_SUCCESS)
      std::cout << "Image memory bound for image " << i
		<< " successfully!" << std::endl;
    else
      std::cout << "Failed to bind image memory for image "
		<< i << "..." << std::endl;
    locks[i].unlock();
  }
}

void create_buffer_views()
{
  buffer_views.resize(BUFFER_COUNT);
  for (unsigned int i = 0; i != BUFFER_COUNT; i++) {
    VkBufferViewCreateInfo buf_view_create_info = {};
    buf_view_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
    buf_view_create_info.pNext = nullptr;
    buf_view_create_info.flags = 0;
    buf_view_create_info.buffer = buffers[i];
    buf_view_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    buf_view_create_info.offset = 0;
    buf_view_create_info.range = VK_WHOLE_SIZE;

    std::cout << "Creating buffer view " << i << "..." << std::endl;
    res = vkCreateBufferView(device,
			     &buf_view_create_info,
			     CUSTOM_ALLOCATOR ? &alloc_callbacks : nullptr,
			     &buffer_views[i]);
    if (res == VK_SUCCESS)
      std::cout << "Buffer view " << i << " created successfully!"
		<< std::endl;
    else
      std::cout << "Failed to create buffer view " << i
		<< "..." << std::endl;
  }
}

void create_image_views()
{
  image_views.resize(IMAGE_COUNT);
  for (unsigned int i = 0; i != IMAGE_COUNT; i++) {
    VkImageViewCreateInfo img_view_create_info = {};
    img_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    img_view_create_info.pNext = nullptr;
    img_view_create_info.flags = 0;
    img_view_create_info.image = images[i];
    img_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    img_view_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    VkComponentMapping component_mapping = {};
    component_mapping.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    component_mapping.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    component_mapping.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    component_mapping.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    img_view_create_info.components = component_mapping;
    VkImageSubresourceRange subresource_range = {};
    subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresource_range.baseMipLevel = 0;
    subresource_range.levelCount = 1;
    subresource_range.baseArrayLayer = 0;
    subresource_range.layerCount = 1;
    img_view_create_info.subresourceRange = subresource_range;
    std::cout << "Creating image view " << i << "..." << std::endl;
    res = vkCreateImageView(device,
			    &img_view_create_info,
			    CUSTOM_ALLOCATOR ? &alloc_callbacks : nullptr,
			    &image_views[i]);
    if (res == VK_SUCCESS)
      std::cout << "Image view " << i << " created successfully!"
		<< std::endl;
    else if (res == VK_ERROR_OUT_OF_HOST_MEMORY)
      std::cout << "Failed to create image view " << i
		<< ": out of host memory" << std::endl;
    else if (res == VK_ERROR_OUT_OF_DEVICE_MEMORY)
      std::cout << "Failed to create image view " << i
		<< ": out of device memory" << std::endl;
    else if (res == VK_ERROR_VALIDATION_FAILED_EXT)
      std::cout << "Failed to create image view " << i
		<< ": validation failed" << std::endl;
    else
      std::cout << "Failed to create image view " << i
		<< ": unknown error" << std::endl;
  }
}

void get_queues()
{
  queues.resize(queue_family_queue_count);
  for (unsigned int queue_idx = 0;
       queue_idx != queue_family_queue_count;
       queue_idx++) {
    std::cout << "Obtaining device queue " << (queue_idx+1)
	      << "/" << queue_family_queue_count << "..." << std::endl;
    vkGetDeviceQueue(device,
		     queue_family_idx,
		     queue_idx,
		     &queues[queue_idx]);
  }
}

int main(int argc, const char* argv[])
{
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
  
  if (CUSTOM_ALLOCATOR)
    std::cout << "Using custom allocator..." << std::endl;

  create_instance();
  
  enumerate_physical_devices();

  if (SHOW_PHYSICAL_DEVICE_LAYERS) {
    uint32_t device_layer_count;
    std::vector<VkLayerProperties> device_layer_props;
    vkEnumerateDeviceLayerProperties(physical_devices[phys_device_idx],
				     &device_layer_count, 
				     nullptr);
    device_layer_props.resize(device_layer_count);
    vkEnumerateDeviceLayerProperties(physical_devices[phys_device_idx],
				     &device_layer_count, 
				     device_layer_props.data());
    if (device_layer_count != 0) {
      std::cout << "\nPhysical Device " << phys_device_idx
		<< " Layers:" << std::endl;
      for (auto& layer : device_layer_props)
	std::cout << layer.layerName << std::endl;
      std::cout << std::endl;
    } else
      std::cout << "No device layers available for physical device "
		<< phys_device_idx << "..." << std::endl;
  }

  if (SHOW_PHYSICAL_DEVICE_EXTENSIONS) {
    uint32_t device_ext_count;
    std::vector<VkExtensionProperties> device_ext_props;
    vkEnumerateDeviceExtensionProperties(physical_devices[phys_device_idx],
					 nullptr,
					 &device_ext_count,
					 nullptr);
    device_ext_props.resize(device_ext_count);
    vkEnumerateDeviceExtensionProperties(physical_devices[phys_device_idx],
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

  get_physical_device_memory_properties();

  get_queue_family_properties();
  
  queue_family_queue_count =
    queue_family_properties[queue_family_idx].queueCount;

  create_device();

  create_buffers();

  create_images();

  get_subresource_layouts();

  for (unsigned int i = 0; i != IMAGE_COUNT; i++)
    std::cout << "Subresource " << i << ": off="
	      << subresource_layouts[i].offset << ", size="
	      << subresource_layouts[i].size << ", rowpitch="
	      << subresource_layouts[i].rowPitch << ", arraypitch="
	      << subresource_layouts[i].arrayPitch << ", depthpitch="
	      << subresource_layouts[i].depthPitch << std::endl;

  get_buffer_memory_requirements();
  
  get_image_memory_requirements();

  std::cout << "Buffer memory size: "
	    << mem_size[RESOURCE_BUFFER] << std::endl;
  std::cout << "Image memory size: "
	    << mem_size[RESOURCE_IMAGE] << std::endl;
  
  find_memory_types();

  if (mem_types[RESOURCE_BUFFER] != UINT32_MAX)
    std::cout << "Found suitable memory type for buffers: "
	      << mem_types[RESOURCE_BUFFER] << std::endl;
  else
    std::cout << "Could not find a suitable memory type for buffers..."
	      << std::endl;

  if (mem_types[RESOURCE_IMAGE] != UINT32_MAX)
    std::cout << "Found suitable memory type for images: "
	      << mem_types[RESOURCE_IMAGE] << std::endl;
  else
    std::cout << "Could not find a suitable memory type for images..."
	      << std::endl;

  allocate_buffer_memory();
  allocate_image_memory();

  write_buffer_memory();
  write_image_memory();

  bind_buffer_memory();
  bind_image_memory();

  create_buffer_views();
  create_image_views();

  get_queues();
  
  VkCommandPool command_pool;
  std::cout << "Creating command pool..." << std::endl;
  VkCommandPoolCreateInfo cmd_pool_create_info = {};
  cmd_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  cmd_pool_create_info.pNext = nullptr;
  cmd_pool_create_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT |
    VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  cmd_pool_create_info.queueFamilyIndex = queue_family_idx;
  res = vkCreateCommandPool(device,
			    &cmd_pool_create_info,
			    CUSTOM_ALLOCATOR ? &alloc_callbacks : nullptr,
			    &command_pool);
  if (res == VK_SUCCESS)
    std::cout << "Command pool created successfully!" << std::endl;
  else
    std::cout << "Failed to create command pool..." << std::endl;
  
  // Allocate command buffers  
  std::vector<VkCommandBuffer> command_buffers;
  {
    std::lock_guard<std::mutex> lock(command_pool_mutex);
    VkCommandBufferAllocateInfo cmd_buf_alloc_info = {};
    cmd_buf_alloc_info.sType =
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmd_buf_alloc_info.pNext = nullptr;
    cmd_buf_alloc_info.commandPool = command_pool;
    cmd_buf_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cmd_buf_alloc_info.commandBufferCount = COMMAND_BUFFER_COUNT;
    command_buffers.resize(COMMAND_BUFFER_COUNT);
    std::cout << "Allocating command buffers ("
	      << COMMAND_BUFFER_COUNT << ")..."
	      << std::endl;
    res = vkAllocateCommandBuffers(device,
				   &cmd_buf_alloc_info,
				   command_buffers.data());
    if (res == VK_SUCCESS)
      std::cout << "Command buffers allocated successfully!" << std::endl;
    else
      std::cout << "Failed to allocate command buffers..." << std::endl;
  }

  VkCommandBufferBeginInfo cmd_buf_begin_info = {};
  cmd_buf_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  cmd_buf_begin_info.pNext = nullptr;
  cmd_buf_begin_info.flags = 0;
  cmd_buf_begin_info.pInheritanceInfo = nullptr;
  // Begin recording
  {
    std::cout << "Beginning command buffers (" << COMMAND_BUFFER_COUNT
	      << ")..." << std::endl;
    std::lock_guard<std::mutex> lock(command_pool_mutex);
    std::vector<std::unique_lock<std::mutex>> locks;
    for (auto& mut : command_buffer_mutex)
      locks.emplace_back(mut, std::defer_lock);
    for (unsigned int i = 0; i != COMMAND_BUFFER_COUNT; i++) {
      locks[i].lock();
      res = vkBeginCommandBuffer(command_buffers[i],
				 &cmd_buf_begin_info);
      if (res == VK_SUCCESS)
	std::cout << "Command buffer " << i << " is now recording."
		  << std::endl;
      else
	std::cout << "Failed to begin command buffer " << i << "..."
		  << std::endl;      
      locks[i].unlock();
    }
  }

  // Add commands
  {
    std::lock_guard<std::mutex> lock(command_pool_mutex);
    std::vector<std::unique_lock<std::mutex>> locks;
    for (auto& mut : command_buffer_mutex)
      locks.emplace_back(mut, std::defer_lock);
    for (unsigned int i = 0; i != COMMAND_BUFFER_COUNT; i++) {
      std::cout << "Adding vkCmdCopyBuffer from buffer " << 2*i
		<< " to buffer " << 2*i+1 << " to command buffer " << i
		<< "..." << std::endl;
      locks[i].lock();
      VkBuffer src_buf = buffers[2*i];
      VkBuffer dst_buf = buffers[2*i+1];
      std::vector<VkBufferCopy> copies(1);
      copies[0].srcOffset = 0;
      copies[0].dstOffset = 0;
      copies[0].size = 128;
      vkCmdCopyBuffer(command_buffers[i],
		      src_buf,
		      dst_buf,
		      static_cast<uint32_t>(copies.size()),
		      copies.data());
      locks[i].unlock();
    }
  }

  // End recording
  {
    std::cout << "Ending command buffers (" << COMMAND_BUFFER_COUNT
	      << ")..." << std::endl;
    std::lock_guard<std::mutex> lock(command_pool_mutex);
    std::vector<std::unique_lock<std::mutex>> locks;
    for (auto& mut : command_buffer_mutex)
      locks.emplace_back(mut, std::defer_lock);
    for (unsigned int i = 0; i != COMMAND_BUFFER_COUNT; i++) {
      locks[i].lock();
      res = vkEndCommandBuffer(command_buffers[i]);
      if (res == VK_SUCCESS)
	std::cout << "Command buffer " << i << " is no longer recording."
		  << std::endl;
      else
	std::cout << "Failed to end command buffer " << i << "..."
		  << std::endl;      
      locks[i].unlock();
    }
  }

  std::cout << "Before submit:" << std::endl;
  std::cout << "Buffer 0 (offset=" << READ_OFFSET << ", len="
	    << READ_LENGTH << "): ";
  print_mem(device,
	    memory[RESOURCE_BUFFER],
	    memory_mutex[RESOURCE_BUFFER],
	    READ_OFFSET,
	    READ_LENGTH);
  std::cout << "Buffer 1 (offset=" << READ_OFFSET << ", len="
	    << READ_LENGTH << "): ";
  print_mem(device,
	    memory[RESOURCE_BUFFER],
	    memory_mutex[RESOURCE_BUFFER],
	    READ_OFFSET + buf_mem_requirements[0].size,
	    READ_LENGTH);

  std::vector<VkSubmitInfo> submit_infos(1);
  submit_infos[0].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_infos[0].pNext = nullptr;
  submit_infos[0].waitSemaphoreCount = 0;
  submit_infos[0].pWaitSemaphores = nullptr;
  submit_infos[0].pWaitDstStageMask = nullptr;
  submit_infos[0].commandBufferCount =
    static_cast<uint32_t>(command_buffers.size());
  submit_infos[0].pCommandBuffers = command_buffers.data();
  submit_infos[0].signalSemaphoreCount = 0;
  submit_infos[0].pSignalSemaphores = nullptr;
  uint32_t submit_queue_idx = 0;
  // Submit all command buffers to queue
  {
    std::cout << "Submitting command buffers to queue "
	      << submit_queue_idx << "..." << std::endl;
    std::lock_guard<std::mutex> lock(queue_mutex[submit_queue_idx]);
    res = vkQueueSubmit(queues[submit_queue_idx],
			static_cast<uint32_t>(submit_infos.size()),
			submit_infos.data(),
			VK_NULL_HANDLE);
    if (res == VK_SUCCESS)
      std::cout << "Command buffers submitted to queue " << submit_queue_idx
		<< " successfully!" << std::endl;
    else
      std::cout << "Failed to submit command buffers to queue "
		<< submit_queue_idx << "..." << std::endl;
  }

  // Wait on queue to finish executing command buffers. Not recommended:
  // use fence instead.
  {
    std::cout << "Waiting for queue " << submit_queue_idx
	      << " to idle..." << std::endl;
    res = vkQueueWaitIdle(queues[submit_queue_idx]);
    if (res == VK_SUCCESS)
      std::cout << "Queue " << submit_queue_idx << " idled successfully!"
		<< std::endl;
    else
      std::cout << "Failed to wait for queue " << submit_queue_idx
		<< "...";
  }

  std::cout << "After submit:" << std::endl;
  std::cout << "Buffer 0 (offset=" << READ_OFFSET << ", len="
	    << READ_LENGTH << "): ";
  print_mem(device,
	    memory[RESOURCE_BUFFER],
	    memory_mutex[RESOURCE_BUFFER],
	    READ_OFFSET,
	    READ_LENGTH);
  std::cout << "Buffer 1 (offset=" << READ_OFFSET << ", len="
	    << READ_LENGTH << "): ";
  print_mem(device,
	    memory[RESOURCE_BUFFER],
	    memory_mutex[RESOURCE_BUFFER],
	    READ_OFFSET + buf_mem_requirements[0].size,
	    READ_LENGTH);

  // Reset command buffers
  {
    std::vector<std::unique_lock<std::mutex>> locks;
    for (auto& mut : command_buffer_mutex)
      locks.emplace_back(mut, std::defer_lock);
    for (unsigned int i = 0; i != COMMAND_BUFFER_COUNT; i++) {
      std::cout << "Resetting command buffer " << i << "..." << std::endl;
      locks[i].lock();
      res = vkResetCommandBuffer(command_buffers[i], 0);
      if (res == VK_SUCCESS)
	std::cout << "Command buffer " << i << " reset successfully!"
		  << std::endl;
      else
	std::cout << "Failed to reset command buffer " << i << "..."
		  << std::endl;
      locks[i].unlock();
    }
  }

  // Free command buffers
  {
    std::lock_guard<std::mutex> lock(command_pool_mutex);
    std::vector<std::unique_lock<std::mutex>> locks;
    for (auto& mut : command_buffer_mutex)
      locks.emplace_back(mut, std::defer_lock);
    for (auto& lck : locks)
      lck.lock();
    std::cout << "Freeing command buffers..." << std::endl;
    vkFreeCommandBuffers(device,
			 command_pool,
			 COMMAND_BUFFER_COUNT,
			 command_buffers.data());
    for (auto& lck : locks)
      lck.unlock();
  }
  
  // Destroy command pool
  {
    std::lock_guard<std::mutex> lock(command_pool_mutex);
    std::cout << "Destroying command pool..." << std::endl;
    vkDestroyCommandPool(device,
			 command_pool,
			 CUSTOM_ALLOCATOR ? &alloc_callbacks : nullptr);
  }

  // Destroy buffer views
  {
    std::vector<std::unique_lock<std::mutex>> locks;
    for (auto& mut : buffer_view_mutex)
      locks.emplace_back(mut, std::defer_lock);
    for (unsigned int i = 0; i != BUFFER_COUNT; i++) {
      std::cout << "Destroying buffer view " << i << "..." << std::endl;
      locks[i].lock();
      vkDestroyBufferView(device,
			  buffer_views[i],
			  CUSTOM_ALLOCATOR ? &alloc_callbacks : nullptr);
      locks[i].unlock();
    }
  }

  // Destroy image views
  {
    std::vector<std::unique_lock<std::mutex>> locks;
    for (auto& mut : image_view_mutex)
      locks.emplace_back(mut, std::defer_lock);
    for (unsigned i = 0; i != IMAGE_COUNT; i++) {
      locks[i].lock();
      std::cout << "Destroying image view " << i << "..." << std::endl;
      vkDestroyImageView(device,
			 image_views[i],
			 CUSTOM_ALLOCATOR ? &alloc_callbacks : nullptr);
      locks[i].unlock();
    }
  }

  // Free buffer memory
  {
    std::lock_guard<std::mutex> lock(memory_mutex[RESOURCE_BUFFER]);
    std::cout << "Freeing buffer memory..." << std::endl;
    vkFreeMemory(device,
		 memory[RESOURCE_BUFFER],
		 CUSTOM_ALLOCATOR ? &alloc_callbacks : nullptr);
  }

  // Free image memory
  {
    std::lock_guard<std::mutex> lock(memory_mutex[RESOURCE_IMAGE]);
    std::cout << "Freeing image memory..." << std::endl;
    vkFreeMemory(device,
		 memory[RESOURCE_IMAGE],
		 CUSTOM_ALLOCATOR ? &alloc_callbacks : nullptr);
  }
  
  // Destroy buffers
  {
    std::vector<std::unique_lock<std::mutex>> locks;
    for (auto& mut : buffer_mutex)
      locks.emplace_back(mut, std::defer_lock);
    for (int i = 0; i != BUFFER_COUNT; i++) {
      locks[i].lock();
      std::cout << "Destroying buffer " << i << "..." << std::endl;
      vkDestroyBuffer(device,
		      buffers[i],
		      CUSTOM_ALLOCATOR ? &alloc_callbacks : nullptr);
      locks[i].unlock();
    }
  }

  // Destroy images
  {
    std::vector<std::unique_lock<std::mutex>> locks;
    for (auto& mut : image_mutex)
      locks.emplace_back(mut, std::defer_lock);
    for (unsigned int i = 0; i != IMAGE_COUNT; i++) {
      locks[i].lock();
      std::cout << "Destroying image " << i << "..." << std::endl;
      vkDestroyImage(device,
		     images[i],
		     CUSTOM_ALLOCATOR ? &alloc_callbacks : nullptr);
      locks[i].unlock();
    }
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
