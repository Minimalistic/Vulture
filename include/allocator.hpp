#ifndef ALLOCATOR_HPP_
#define ALLOCATION_HPP_

#include <vulkan/vulkan.h>

class allocator {
public:
  inline operator VkAllocationCallbacks() const {
    VkAllocationCallbacks result;

    result.pUserData = (void*) this;
    result.pfnAllocation = &allocateMem;
    result.pfnReallocation = &reallocateMem;
    result.pfnFree = &freeMem;
    result.pfnInternalAllocation = &logInternalAllocate;
    result.pfnInternalFree = &logInternalFree;

    return result;
  }

private:
  static void* VKAPI_CALL allocateMem(void*  pUserData,
				      size_t size,
				      size_t alignment,
				      VkSystemAllocationScope allocationScope);

  static void* VKAPI_CALL reallocateMem(void* pUserData,
					void* pOriginal,
					size_t size,
					size_t alignment,
					VkSystemAllocationScope allocationScope);

  static void VKAPI_CALL freeMem(void* pUserData,
				 void* pMemory);

  static void VKAPI_CALL logInternalAllocate(void* pUserData,
					     size_t size,
					     VkInternalAllocationType allocationType,
					     VkSystemAllocationScope allocationScope);

  static void VKAPI_CALL logInternalFree(void* pUserData,
					 size_t size,
					 VkInternalAllocationType allocationType,
					 VkSystemAllocationScope allocationScope);

  void* myAllocate(size_t size,
		   size_t alignment,
		   VkSystemAllocationScope allocationScope);

  void* myReallocate(void* pOriginal,
		     size_t size,
		     size_t alignment,
		     VkSystemAllocationScope allocationScope);

  void myFree(void* pMemory);

  void myLogInternalAllocate(size_t size,
			     VkInternalAllocationType allocationType,
			     VkSystemAllocationScope allocationScope);

  void myLogInternalFree(size_t size,
			 VkInternalAllocationType allocationType,
			 VkSystemAllocationScope allocationScope);

};

#endif
