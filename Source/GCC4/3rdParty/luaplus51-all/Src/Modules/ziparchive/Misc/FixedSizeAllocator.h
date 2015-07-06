#ifndef MISC_FIXEDSIZEALLOCATOR_H
#define MISC_FIXEDSIZEALLOCATOR_H

#include <assert.h>

namespace Misc {

struct MemoryPage
{
	MemoryPage* next;
	size_t size;

	void* data() { return this+1; }

	static MemoryPage* Create(MemoryPage*& head, size_t byteCount)
	{
		MemoryPage* p = (MemoryPage*)new unsigned char[sizeof(MemoryPage) + byteCount];
		p->next = head;
		p->size = sizeof(MemoryPage) + byteCount;
		head = p;
		return p;
	}

	static void Free(MemoryPage*& toFreePoolBlock)
	{
		MemoryPage* poolBlock = toFreePoolBlock;
		while (poolBlock)
		{
			MemoryPage* next = poolBlock->next;
			delete[] (unsigned char*)poolBlock;
			poolBlock = next;
		}
		toFreePoolBlock = 0;
	}
};


class FixedSizeAllocator
{
public:
	FixedSizeAllocator() :
		m_freeNode(0),
		m_pages(0),
		m_allocSize(4),
		m_blockSize(10)
	{
	}

	FixedSizeAllocator(size_t allocSize, size_t blockSize)
	{
		assert(allocSize >= sizeof(Node));
		assert(blockSize > 1);

		m_allocSize = allocSize;
		m_blockSize = blockSize;
		m_freeNode = 0;
		m_pages = 0;
	}

	~FixedSizeAllocator()
	{
		FreeAll();
	}

	size_t GetAllocSize() { return m_allocSize; }
	void SetAllocSize(size_t allocSize)				{  m_allocSize = allocSize;  }
	void SetBlockSize(size_t blockSize)				{  m_blockSize = blockSize;  }

	void* Alloc(size_t)
	{
		if (m_freeNode == 0)
		{
			size_t bytesToAllocate = m_blockSize * m_allocSize;
			MemoryPage* newMemoryPage = MemoryPage::Create(m_pages, bytesToAllocate);

			Node* startPtr = (Node*)(newMemoryPage + 1);
			Node* curPtr = (Node*)((unsigned char*)startPtr + bytesToAllocate);
			do
			{
				curPtr = (Node*)((unsigned char*)curPtr - m_allocSize);
				curPtr->next = m_freeNode;
				m_freeNode = curPtr;
			} while (curPtr != startPtr);
		}

		void* pNode = m_freeNode;
		m_freeNode = m_freeNode->next;

		return pNode;
	}

	void Free(void* ptr)
	{
		if (ptr == 0)
			return;

		Node* node = (Node*)ptr;
		node->next = m_freeNode;
		m_freeNode = node;
	}

	void FreeAll()
	{
		MemoryPage::Free(m_pages);
		m_freeNode = 0;
	}

protected:
	struct Node
	{
		Node* next;
	};

	Node* m_freeNode;
	MemoryPage* m_pages;
	size_t m_allocSize;
	size_t m_blockSize;
};

} // namespace Misc

#endif // MISC_FIXEDSIZEALLOCATOR_H
