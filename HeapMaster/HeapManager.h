#pragma once

struct BlockDescriptors {
	void* m_pBLockStartAddr;
	size_t m_sizeBLock;
	BlockDescriptors* next;
};

class HeapManager
{

public:

	BlockDescriptors* pFreeMemoryList = nullptr;
	BlockDescriptors* pOutstandingAllocationList = nullptr;
	BlockDescriptors* pFreeDescriptorList = nullptr;
	size_t m_NumDescriptors;
	size_t m_MemorySizeAllocatable;
	void* pAllocatableStart = nullptr;

	static HeapManager* create(void* i_pHeapMemory, size_t i_HeapMemorySize, unsigned int i_numDescriptors);

	// allocates memory. named with a underscore to resolve name clash 
	void* _alloc(size_t i_bytes);
	// allocation with alignment. returns a block of memory with a given alignment
	void* _alloc(size_t i_bytes, unsigned int i_alignment);

	//BlockDescriptors* searchAvailableDescriptorSpace();

	void insertInOutstandingList(BlockDescriptors* newBlock);

	bool insertInFreeList(BlockDescriptors* newBlock);
	// frees an allocation
	bool _free(void* i_ptr);

	// attempt to merge abutting blocks.
	void collect();

	void insertInFreeDescriptorList(BlockDescriptors* newBlock);

	// tells us the size of the largest free block
	size_t getLargestFreeBlock() const;
	// tells us how much total memory is available
	size_t getTotalFreeMemory() const;

	// tells us if a pointer is within this HeapManagers managed memory
	bool Contains(void* i_ptr) const;
	// tells us if this pointer is an outstanding allocation from this block
	bool IsAllocated(void* i_ptr) const;

	// a debugging helper function to show us all the free blocks.
	void ShowFreeBlocks() const;
	// a debugging helper function to show us all the outstanding blocks.
	void destroy();

	void ShowOutstandingAllocations() const;

	void* roundUp(void* i_ptr, size_t i_align);
};


