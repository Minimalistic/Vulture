#ifndef UTIL_HPP_
#define UTIL_HPP_

#include <mutex>
#include <vector>

#include <vulkan/vulkan.h>

void print_mem(VkDevice device, VkDeviceMemory memory,
	       std::mutex& mutex, VkDeviceSize offset,
	       VkDeviceSize size);

bool supports_mem_reqs(unsigned int memory_type_idx,
		       const std::vector<VkMemoryRequirements>& mem_reqs);

#endif
