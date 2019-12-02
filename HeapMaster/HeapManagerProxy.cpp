#include "HeapManagerProxy.h"

#include <assert.h>
#include <stdio.h>

#include "HeapManager.h"

namespace HeapManagerProxy {

	HeapManager * HeapManagerProxy::CreateHeapManager(void * i_pMemory, size_t i_sizeMemory, unsigned int i_numDescriptors)
	{
		return HeapManager::create(i_pMemory, i_sizeMemory, i_numDescriptors);
	}

	void HeapManagerProxy::Destroy(HeapManager * i_pManager)
	{
		assert(i_pManager);

		i_pManager->destroy();
	}

	void* HeapManagerProxy::alloc(HeapManager * i_pManager, size_t i_size)
	{
		assert(i_pManager);

		return i_pManager->_alloc(i_size);
	}

	void* HeapManagerProxy::alloc(HeapManager * i_pManager, size_t i_size, unsigned int i_alignment)
	{
		assert(i_pManager);

		return i_pManager->_alloc(i_size, i_alignment);
	}

	bool HeapManagerProxy::free(HeapManager * i_pManager, void* i_ptr)
	{
		assert(i_pManager);

		return i_pManager->_free(i_ptr);
	}

	void HeapManagerProxy::Collect(HeapManager * i_pManager)
	{
		assert(i_pManager);

		i_pManager->collect();
	}


	bool HeapManagerProxy::Contains(const HeapManager * i_pManager, void* i_ptr)
	{
		assert(i_pManager);

		return i_pManager->Contains(i_ptr);
	}

	bool HeapManagerProxy::IsAllocated(const HeapManager * i_pManager, void* i_ptr)
	{
		assert(i_pManager);

		return i_pManager->IsAllocated(i_ptr);
	}

	size_t HeapManagerProxy::GetLargestFreeBlock(const HeapManager * i_pManager)
	{
		assert(i_pManager);

		return i_pManager->getLargestFreeBlock();
	}

	size_t HeapManagerProxy::GetTotalFreeMemory(const HeapManager * i_pManager)
	{
		assert(i_pManager);

		return i_pManager->getTotalFreeMemory();
	}

	void HeapManagerProxy::ShowFreeBlocks(const HeapManager * i_pManager)
	{
		assert(i_pManager);

		i_pManager->ShowFreeBlocks();
	}

	void HeapManagerProxy::ShowOutstandingAllocations(const HeapManager * i_pManager)
	{
		assert(i_pManager);

#ifdef __TRACK_ALLOCATIONS
		i_pManager->ShowOutstandingAllocations();
#else
		printf("HeapManager compiled without __TRACK_ALLOCATIONS defined.\n");
#endif

	}

}
