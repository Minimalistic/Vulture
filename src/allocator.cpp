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
  #if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
  return _aligned_malloc(size, alignment);
  #else
  return aligned_alloc(alignment, size);
  #endif
}

void* allocator::myReallocate(void* pOriginal,
			      size_t size,
			      size_t alignment,
			      VkSystemAllocationScope allocationScope) {
  #if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
  return _aligned_realloc(pOriginal, size, alignment);
  #else
  return realloc(pOriginal, size);
  #endif
}

void allocator::myFree(void* pMemory) {
  #if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
  _aligned_free(pMemory);
  #else
  free(pMemory);
  #endif
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
