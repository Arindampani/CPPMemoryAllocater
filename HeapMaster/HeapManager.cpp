#include "HeapManager.h"
#include <assert.h>
#include <stdio.h>

HeapManager* HeapManager::create(void* i_pHeapMemory, size_t i_HeapMemorySize, unsigned int i_numDescriptors) {

	HeapManager* heapManager = static_cast<HeapManager*>(i_pHeapMemory);
	BlockDescriptors* FreeMemoryBlockDescriptor = reinterpret_cast<BlockDescriptors*>(static_cast<char *>(i_pHeapMemory) + sizeof(HeapManager));
	FreeMemoryBlockDescriptor->m_pBLockStartAddr = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(i_pHeapMemory) + sizeof(heapManager) + i_numDescriptors*sizeof(BlockDescriptors));
	FreeMemoryBlockDescriptor->m_sizeBLock = i_HeapMemorySize - sizeof(heapManager) + i_numDescriptors * sizeof(BlockDescriptors);
	FreeMemoryBlockDescriptor->next = nullptr;
	heapManager->pFreeMemoryList = FreeMemoryBlockDescriptor;

	BlockDescriptors* FreeBlockDescriptorList = FreeMemoryBlockDescriptor + 1;

	BlockDescriptors* startNode = FreeBlockDescriptorList;
	size_t i = i_numDescriptors-1;
	while (i) {

		BlockDescriptors* newBlock = startNode + 1;
		newBlock->m_pBLockStartAddr = nullptr;
		newBlock->m_sizeBLock = 0;
		newBlock->next = nullptr;
		startNode->next = newBlock;
		startNode = startNode->next;
		i--;
	}
	heapManager->pOutstandingAllocationList = nullptr;
	heapManager->pFreeDescriptorList = FreeBlockDescriptorList;
	heapManager->m_NumDescriptors = i_numDescriptors;
	heapManager->m_MemorySizeAllocatable = heapManager->pFreeMemoryList->m_sizeBLock;
	heapManager->pAllocatableStart = heapManager->pFreeMemoryList->m_pBLockStartAddr;

	return heapManager;
}

void* HeapManager::_alloc(size_t i_bytes) {
	//printf("alloc is called for size %zd\n",i_bytes);
	BlockDescriptors* startNode = pFreeMemoryList;

	if (pFreeDescriptorList) {
		while (startNode) {
			if (startNode->m_sizeBLock >= i_bytes)
			{
				BlockDescriptors* newBlock = pFreeDescriptorList;

				//printf("count is %d \n", count);
				pFreeDescriptorList = pFreeDescriptorList->next;
				newBlock->m_pBLockStartAddr = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(startNode->m_pBLockStartAddr) + startNode->m_sizeBLock - i_bytes);
				newBlock->m_sizeBLock = i_bytes;
				newBlock->next = nullptr;
				insertInOutstandingList(newBlock);
				startNode->m_sizeBLock = startNode->m_sizeBLock - i_bytes;
				//printf("alloc is finished for %p and size %zd\n", newBlock->m_pBLockStartAddr,i_bytes);
				return newBlock->m_pBLockStartAddr;//startAddrToReturn;
			}

			startNode = startNode->next;
		}
	}
	else {
		if (startNode) {
			return nullptr;
		}
		else if (startNode != nullptr && startNode->m_sizeBLock == i_bytes) {
			BlockDescriptors* newBLock = startNode;
			startNode = startNode->next;
			insertInOutstandingList(newBLock);
			//printf("it ran for start %p",newBLock->m_pBLockStartAddr);
			return newBLock->m_pBLockStartAddr;
		}
		else
		{
			while (startNode) {
				if (startNode->next->m_sizeBLock == i_bytes) {

					BlockDescriptors* newBLock = startNode->next;
					startNode->next = startNode->next->next;
					insertInOutstandingList(newBLock);
					//printf("it ran for %p", newBLock->m_pBLockStartAddr);
					return newBLock->m_pBLockStartAddr;
				}
				startNode = startNode->next;
			}
		}

		startNode = pFreeMemoryList;
		while (startNode) {
			if (startNode->next->m_sizeBLock > i_bytes) {

				BlockDescriptors* newBLock = startNode->next;
				startNode->next = startNode->next->next;
				insertInOutstandingList(newBLock);
				//printf("it ran for free %p", newBLock->m_pBLockStartAddr);
				return newBLock->m_pBLockStartAddr;
			}
			startNode = startNode->next;
		}

	}

	return NULL;
}

void* HeapManager::_alloc(size_t i_bytes, unsigned int i_alignment) {
	_alloc(i_bytes);
	return NULL;
}

void HeapManager::insertInOutstandingList(BlockDescriptors* newBlock) {
	assert(newBlock);
	BlockDescriptors* startNode = pOutstandingAllocationList;
	
	if (startNode==nullptr || startNode->m_pBLockStartAddr >= newBlock->m_pBLockStartAddr) {
		//printf("inserted first\n");
		newBlock->next = startNode;
		pOutstandingAllocationList = newBlock;
	}
	else {
		while (startNode->next != NULL && startNode->next->m_pBLockStartAddr < newBlock->m_pBLockStartAddr) {
				startNode = startNode->next;
			
		}
	//	printf("inserted again\n");
		newBlock->next = startNode->next;
		startNode->next = newBlock;
	}
	return;
}

bool HeapManager::_free(void* i_ptr) {
	assert(i_ptr);
	//printf("Test free %p\n",i_ptr);
	BlockDescriptors* startNode = pOutstandingAllocationList;
	BlockDescriptors* prevNode = startNode;
	if (startNode == nullptr) {
		return false;
	}
	if (startNode->m_pBLockStartAddr <= i_ptr && (reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(startNode->m_pBLockStartAddr) + startNode->m_sizeBLock) > i_ptr))
	{
		BlockDescriptors* newBlock = startNode;
		pOutstandingAllocationList = newBlock->next;
		newBlock->next = nullptr;
		return insertInFreeList(newBlock);
	}
	startNode = startNode->next;
	while (startNode) {
		if (startNode->m_pBLockStartAddr <= i_ptr && (reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(startNode->m_pBLockStartAddr) + startNode->m_sizeBLock) > i_ptr))
		{
			BlockDescriptors* newBlock = startNode;
			prevNode->next = newBlock->next;
			newBlock->next = nullptr;
			return insertInFreeList(newBlock);
		}
		else {
			prevNode = prevNode->next;
			startNode = startNode->next;
		}
	}
	return false;
	
}

bool HeapManager::insertInFreeList(BlockDescriptors* newBlock) {
	assert(newBlock);
	BlockDescriptors* startNode = pFreeMemoryList;

	if (startNode == nullptr || startNode->m_pBLockStartAddr >= newBlock->m_pBLockStartAddr) {
		//printf("inserted first\n");
		newBlock->next = startNode;
		pFreeMemoryList = newBlock;
		return true;
	}
	else {
		while (startNode->next != NULL && startNode->next->m_pBLockStartAddr < newBlock->m_pBLockStartAddr) {
			startNode = startNode->next;

		}
		//printf("inserted again\n");
		newBlock->next = startNode->next;
		startNode->next = newBlock;
		return true;
	}
}


void HeapManager::collect() {
	//printf("Test collect\n");
	BlockDescriptors* startNode = pFreeMemoryList;

	while (startNode) {
		if (startNode->next != nullptr && reinterpret_cast<void*>((reinterpret_cast<uintptr_t>(startNode->m_pBLockStartAddr) + startNode->m_sizeBLock)) == startNode->next->m_pBLockStartAddr)
		{
			startNode->m_sizeBLock = startNode->m_sizeBLock + startNode->next->m_sizeBLock;
			BlockDescriptors* newblock = startNode->next;
			startNode->next = newblock->next;
			newblock->m_pBLockStartAddr = nullptr;
			newblock->m_sizeBLock = 0;
			newblock->next = nullptr;
			insertInFreeDescriptorList(newblock);
		}
		else {
			startNode = startNode->next;
		}
	
	}

	return;
}

 void HeapManager::insertInFreeDescriptorList(BlockDescriptors* newBlock) {
	assert(newBlock);
	BlockDescriptors* startNode = pFreeDescriptorList;
	//assert(startNode);
	if (startNode == NULL) {
		pFreeDescriptorList = newBlock;
	}
	else {
		newBlock->next = startNode->next;
		startNode->next = newBlock;
	}
	return;
}


size_t HeapManager::getLargestFreeBlock() const {
	//printf("Test freeblock\n");
	BlockDescriptors* startNode = pFreeMemoryList;
	size_t largest = 0;
	assert(startNode);
	
	while (startNode) {
		if (startNode->m_sizeBLock > largest)
			largest = startNode->m_sizeBLock;

		startNode = startNode->next;
	}
	return largest;
}

size_t HeapManager::getTotalFreeMemory() const {
	//printf("Test freeMemory\n");
	BlockDescriptors* startNode = pFreeMemoryList;
	size_t freeMemory = 0;

	while (startNode)
	{
		freeMemory = freeMemory + startNode->m_sizeBLock;
		startNode = startNode->next;
	}
	return freeMemory;
}


bool HeapManager::Contains(void* i_ptr) const {
	return (i_ptr >= pAllocatableStart && i_ptr < reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(pAllocatableStart) + m_MemorySizeAllocatable));
}

bool HeapManager::IsAllocated(void* i_ptr) const {
	BlockDescriptors* startNode = pOutstandingAllocationList;
	//printf("startCheck \n");
	while (startNode) {
		//printf("checking for %p %zd %p\n",startNode->m_pBLockStartAddr, startNode->m_sizeBLock,i_ptr);
		if (startNode->m_pBLockStartAddr == i_ptr)
			return true;

		startNode = startNode->next;
	}
	//printf("Allocation check failed for %p",i_ptr);
	return false;
}

void HeapManager::ShowFreeBlocks() const {
	BlockDescriptors* startNode = pFreeMemoryList;

	while (startNode) {
		startNode = startNode->next;
	}
}

void HeapManager::destroy() {
	BlockDescriptors* startNode = pFreeDescriptorList;
	while (startNode)
	{
		BlockDescriptors* temp = startNode;
		temp->next = nullptr;
		temp = nullptr;
		startNode = startNode->next;
	}
	pFreeDescriptorList = nullptr;
	startNode = pFreeMemoryList;
	while (startNode)
	{
		BlockDescriptors* temp = startNode;
		temp->next = nullptr;
		temp = nullptr;
		startNode = startNode->next;
	}
	pFreeMemoryList = nullptr;
	pOutstandingAllocationList = nullptr;
}

void HeapManager::ShowOutstandingAllocations() const {
	//printf("Test showoout\n");
	BlockDescriptors* startNode = pOutstandingAllocationList;

	while (startNode) {
		//printf("Allocated : %p %zd \n ", startNode->m_pBLockStartAddr, startNode->m_sizeBLock);
		startNode = startNode->next;
	}
}
void* HeapManager::roundUp(void* i_ptr, size_t i_align) {

	//return ((i_ptr + (i_align - 1)) & ~(i_align - 1)) );

	return i_ptr;
}

