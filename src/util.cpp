#include "util.hpp"

#include <iostream>

bool supports_mem_reqs(unsigned int memory_type_idx,
		       const std::vector<VkMemoryRequirements>& mem_reqs) {
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
	       VkDeviceSize size) {
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
