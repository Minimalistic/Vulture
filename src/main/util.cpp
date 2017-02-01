#include "util.hpp"

#include <iostream>
#include <cstring>

bool supported_surface_present_mode(VkPresentModeKHR mode,
				    const std::vector<VkPresentModeKHR>& modes)
{
  for (auto& md : modes)
    if (md == mode)
      return true;

  return false;
}

bool supported_surface_format(VkFormat format,
			      const std::vector<VkSurfaceFormatKHR>& surf_fmts)
{
  for (auto& fmt : surf_fmts)
    if (fmt.format == format)
      return true;
  
  return false;
}

bool supports_mem_reqs(unsigned int memory_type_idx,
		       const std::vector<VkMemoryRequirements>& mem_reqs)
{
  unsigned long mem_type_bit = 1 << memory_type_idx;
  for (auto& mem_requirement : mem_reqs)
    if ((mem_requirement.memoryTypeBits & mem_type_bit) == 0)
      return false;
  return true;
}

void print_mem(VkDevice device,
	       VkDeviceMemory memory,
	       std::mutex& mem_mutex,
	       VkDeviceSize offset,
	       VkDeviceSize size)
{
  std::lock_guard<std::mutex> lock(mem_mutex);
  void* data;
  VkResult res = vkMapMemory(device,
			     memory,
			     offset,
			     size+1,
			     0,
			     &data);
  if (res != VK_SUCCESS)
    std::cout << "Failed to print memory (Offset=" << offset
	      << ",Size=" << size << ")..." << std::endl;
  char* str = static_cast<char*>(data);
  str[size] = '\0';
  std::cout << str << std::endl;
  vkUnmapMemory(device, memory);
}

void print_all_buffers(VkDevice device,
		       VkDeviceMemory memory,
		       std::mutex& mem_mutex,
		       VkDeviceSize offset,
		       VkDeviceSize length,
		       const std::vector<VkMemoryRequirements>& buf_mem_reqs)
{
  int cur_offset = offset;
  for (unsigned int i = 0; i != buf_mem_reqs.size(); i++) {
    std::cout << "Buffer " << i << " (offset=" << offset << ", len="
	      << length << "): ";
    print_mem(device,
	      memory,
	      mem_mutex,
	      cur_offset,
	      length);
    cur_offset += buf_mem_reqs[i].size;
  }
}

uint32_t make_data(const char* str)
{
  if (strlen(str) != 4) {
    std::cout << "Failed make_data(const char*): string must be size 4!" << std::endl;
    return VT_BAD_DATA;
  }
  return (str[3] << 3*BYTE_SIZE)
    + (str[2] << 2*BYTE_SIZE)
    + (str[1] << BYTE_SIZE)
    + str[0];
}
