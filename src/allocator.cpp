#include "allocator.hpp"

#include <cstdlib>
#include <iostream>

void* VKAPI_CALL allocator::allocateMem(void*  pUserData,
					size_t size,
					size_t alignment,
					VkSystemAllocationScope allocationScope) {
  return static_cast<allocator*>(pUserData)->myAllocate(size,
							alignment,
							allocationScope);
}

void* VKAPI_CALL allocator::reallocateMem(void* pUserData,
					  void* pOriginal,
					  size_t size,
					  size_t alignment,
					  VkSystemAllocationScope allocationScope) {
  return static_cast<allocator*>(pUserData)->myReallocate(pOriginal,
							  size,
							  alignment,
							  allocationScope);
}

void VKAPI_CALL allocator::freeMem(void* pUserData,
				   void* pMemory) {
  static_cast<allocator*>(pUserData)->myFree(pMemory);
}

void VKAPI_CALL allocator::logInternalAllocate(void* pUserData,
					       size_t size,
					       VkInternalAllocationType allocationType,
					       VkSystemAllocationScope allocationScope) {
  static_cast<allocator*>(pUserData)->myLogInternalAllocate(size,
							    allocationType,
							    allocationScope);
}

void VKAPI_CALL allocator::logInternalFree(void* pUserData,
					   size_t size,
					   VkInternalAllocationType allocationType,
					   VkSystemAllocationScope allocationScope) {
  static_cast<allocator*>(pUserData)->myLogInternalFree(size,
							allocationType,
							allocationScope);
}


void* allocator::myAllocate(size_t size,
			    size_t alignment,
			    VkSystemAllocationScope allocationScope) {
  return aligned_alloc(alignment, size);
}

void* allocator::myReallocate(void* pOriginal,
			      size_t size,
			      size_t alignment,
			      VkSystemAllocationScope allocationScope) {
  return realloc(pOriginal, size);
}

void allocator::myFree(void* pMemory) {
  free(pMemory);
}

void allocator::myLogInternalAllocate(size_t size,
			      VkInternalAllocationType allocationType,
			      VkSystemAllocationScope allocationScope) {
  std::cout << "Vulkan allocated " << size << " bytes" << std::endl;
}

void allocator::myLogInternalFree(size_t size,
			  VkInternalAllocationType allocationType,
			  VkSystemAllocationScope allocationScope) {
  std::cout << "Vulkan freed " << size << " bytes" << std::endl;
}
