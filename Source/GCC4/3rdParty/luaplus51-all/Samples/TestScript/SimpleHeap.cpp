#include "SimpleHeap.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define SIMPLEHEAP_ALLOC_STATS
//#define SIMPLEHEAP_CHECK_BLOCK
//#define SIMPLEHEAP_EXTRA_CHECK_BLOCK

namespace SimpleMem {

#ifdef SIMPLEHEAP_STATS

void MemLog(const char* formatString, ...)
{
	const int BUF_SIZE = 800;
	char buf[BUF_SIZE];
	va_list arglist;

	va_start(arglist, formatString);
	size_t maxCount = BUF_SIZE - 1;
	int bufLen = _vsnprintf(buf, maxCount, formatString, arglist);
	if (bufLen == -1)
	{
		buf[maxCount - 2] = '\n';
		buf[maxCount - 1] = 0;
	}
	va_end(arglist);

#ifdef _WINDOWS_
//	OutputDebugString(buf);
	printf("%s", buf);
#endif // _WINDOWS_
}

#endif // SIMPLEHEAP_STATS


template <typename T>
inline T Align(unsigned long align, T value)
{
	return (T)(((unsigned long)value + (align - 1)) & (0xFFFFFFFF - (align - 1)));
}

template <typename T>
inline T AlignDown(unsigned long align, T value)
{
	return (T)((unsigned long)value & (0xFFFFFFFF - (align - 1)));
}

template <typename T>
inline bool InRange(T value, T minValue, T maxValue)
{
	return value >= minValue  &&  value < maxValue;
}

inline long DiffPtr(void* upper, void* lower)
{
	long result = (long)((unsigned char*)upper - (unsigned char*)lower);
//	assert(result >= 0);
	return result;
}


///////////////////////////////////////////////////////////////////////////////
// BlockHeader definition
///////////////////////////////////////////////////////////////////////////////
#ifdef SIMPLEHEAP_ALLOC_STATS
#define OVERRUN_BYTES 4
#define OVERRUN_CODE 0xABADCAFE
#else
#define OVERRUN_BYTES 0
#endif

#include <pshpack1.h>

/**
	The header for a block in the heap.  Minimally, the header is 8 bytes long, when
	SIMPLEHEAP_ALLOC_STATS is not #defined.  Otherwise, the header is quite large,
	as debug information such as the requested size, allocation index, allocation name,
	filename, and line number is stored.
**/
struct SimpleHeap::BlockHeader
{
	static BlockHeader* GetBlockHeader(void* dataPtr)
		{  return (BlockHeader*)((unsigned char*)dataPtr - sizeof(BlockHeader));  }

	void* GetData(long offset = 0) const		{  return (unsigned char*)(this + 1) + offset;  }
	long GetSize() const						{  return m_size < 0 ? -m_size : m_size;  }
	bool IsUsedBlock() const					{  return m_size < 0;  }
	bool IsFreeBlock() const					{  return m_size >= 0;  }
	BlockHeader* GetPreviousBlock() const		{  return m_prevBlock;  }
	BlockHeader* GetNextBlock() const			{  return (BlockHeader*)GetData(GetSize());  }

	long GetRequestedSize()
	{
#ifdef SIMPLEHEAP_ALLOC_STATS
		return IsFreeBlock() ? GetSize() : m_requestedSize;
#else // SIMPLEHEAP_ALLOC_STATS
		return GetSize();
#endif // SIMPLEHEAP_ALLOC_STATS
	}

	void SetPreviousBlock(BlockHeader* block)	{  m_prevBlock = block;  }
	void SetNextBlock(BlockHeader* nextBlock)
		{  SetSize(CalcDataSize(nextBlock), IsFreeBlock());  }
	void SetNextBlock(BlockHeader* nextBlock, bool isThisBlockFree)
		{  SetSize(CalcDataSize(nextBlock), isThisBlockFree);  }
	void SetPointers(BlockHeader* previousBlock, BlockHeader* nextBlock, bool isThisBlockFree)
		{  SetPreviousBlock(previousBlock);  SetNextBlock(nextBlock, isThisBlockFree);  }
	void SetSize(long size, bool isFree)		{  m_size = isFree ? size : -size;  }
	long CalcDataSize(BlockHeader* upperBlock)	{  return (long)((unsigned char*)upperBlock - (unsigned char*)GetData());  }

protected:
	BlockHeader* m_prevBlock;
	long m_size;

public:
#ifdef SIMPLEHEAP_ALLOC_STATS
	long m_requestedSize;
	int m_allocIndex;
	const char* m_allocName;
	const char* m_fileName;
	unsigned short m_lineNumber;
	unsigned short extra;
	unsigned int extra2;
#endif // SIMPLEHEAP_ALLOC_STATS
};


/**
	The FreeBlockHeader contains everything the base BlockHeader owns and adds two
	additional pointers to the next and previous free block headers.
**/
struct SimpleHeap::FreeBlockHeader : public BlockHeader
{
protected:
	FreeBlockHeader* m_prevFreeBlock;
	FreeBlockHeader* m_nextFreeBlock;

public:
	void SetFreePointers(FreeBlockHeader* prev, FreeBlockHeader* next)
	{
		m_prevFreeBlock = prev;
		m_nextFreeBlock = next;
	}

	void SetFreePointersAndLinks(FreeBlockHeader* prev, FreeBlockHeader* next)
	{
		m_prevFreeBlock = prev;
		prev->m_nextFreeBlock = this;
		m_nextFreeBlock = next;
		next->m_prevFreeBlock = this;
		if (IsUsedBlock())
			m_size = -m_size;
	}

	FreeBlockHeader* GetNextFreeBlock() const			{  return m_nextFreeBlock;  }
	FreeBlockHeader* GetPreviousFreeBlock() const		{  return m_prevFreeBlock;  }
	void SetNextFreeBlock(FreeBlockHeader* block)		{  m_nextFreeBlock = block;  }
	void SetPreviousFreeBlock(FreeBlockHeader* block)	{  m_prevFreeBlock = block;  }
};

#include <poppack.h>


const long SimpleHeap::MINIMUM_DATA_SIZE_FOR_FREE_BLOCK = sizeof(FreeBlockHeader) - sizeof(BlockHeader);


///////////////////////////////////////////////////////////////////////////////
// SimpleHeap
///////////////////////////////////////////////////////////////////////////////
/**
	Constructor.
**/
SimpleHeap::SimpleHeap(const char* name, bool multithreaded)
	: m_buffer(NULL)
	, m_bufferSize(0)
	, m_bottomBlock(NULL)
	, m_topBlock(NULL)
	, m_alignment(MINIMUM_DATA_SIZE_FOR_FREE_BLOCK)
	, m_multithreaded(multithreaded)
#ifdef SIMPLEHEAP_STATS
	, m_usedBlockCount(0)
	, m_freeBlockCount(0)
	, m_usedBytesRequested(0)
	, m_usedBytesTotal(0)
	, m_usedBytesPeak(0)
	, m_freeBytesAvailableTotal(0)
	, m_allocIndex(1)
#endif // SIMPLEHEAP_STATS
{
	if (multithreaded)
	{
		assert(sizeof(m_criticalSection) >= sizeof(CRITICAL_SECTION));
		::InitializeCriticalSection((CRITICAL_SECTION*)m_criticalSection);
	}

#ifdef SIMPLEHEAP_STATS
	strcpy(m_name, name);
#endif // SIMPLEHEAP_STATS
}


/**
	Destructor.
**/
SimpleHeap::~SimpleHeap()
{
	Destroy();

	if (m_multithreaded)
	{
		::DeleteCriticalSection((CRITICAL_SECTION*)m_criticalSection);
	}
}


/**
	Initializes the heap, using malloc() to allocate memory of [size] bytes.
**/
void SimpleHeap::Initialize(unsigned long size)
{
	m_bufferSize = size + sizeof(FreeBlockHeader) * 3;
	m_buffer = (unsigned char*)malloc(m_bufferSize);
	if (!m_buffer)
		assert(0);
	
	Reset();
}


/**
	Destroys the heap.
**/
void SimpleHeap::Destroy()
{
	if (m_buffer)
	{
		free(m_buffer);
		m_buffer = NULL;
		m_bufferSize = 0;

		m_bottomBlock = NULL;
		m_topBlock = NULL;
	}
}


/**
	Allocates memory of [size] bytes, based on the [allocStyle] argument.
**/
void* SimpleHeap::Alloc(unsigned long size, AllocStyle allocStyle,
		const char* allocName, const char* fileName, int lineNumber)
{
	if (m_multithreaded)
	{
		::EnterCriticalSection((CRITICAL_SECTION*)m_criticalSection);
	}

	// The requested size must be at least 1 byte, according to malloc() rules.
	if (size == 0)
		size = 1;

	// The total size to allocate includes overrun bytes.
	long totalSize = Align(m_alignment, size + OVERRUN_BYTES);

	// Find a free block for the requested allocation style.
	BlockHeader* newBlock;
	FreeBlockHeader* foundFreeBlock;
	if (allocStyle == FIRSTFIT_BOTTOM)
		FirstFitFromBottom(totalSize, newBlock, foundFreeBlock);
	else if (allocStyle == FIRSTFIT_TOP)
		FirstFitFromTop(totalSize, newBlock, foundFreeBlock);

	if (!foundFreeBlock)
	{
		assert(0);
#ifdef SIMPLEHEAP_STATS
		MemLog("SimpleHeap::Alloc(): Out of heap space allocating %d bytes.\n", size);
#endif // SIMPLEHEAP_STATS
		if (m_multithreaded)
		{
			::LeaveCriticalSection((CRITICAL_SECTION*)m_criticalSection);
		}
		return NULL;
	}

	// Look up a couple pointers.
	FreeBlockHeader* previousFreeBlock;
	FreeBlockHeader* nextFreeBlock = foundFreeBlock->GetNextFreeBlock();
	BlockHeader* previousBlock;
	BlockHeader* nextBlock = foundFreeBlock->GetNextBlock();
	long foundFreeBlockSize = foundFreeBlock->GetSize();

	// Determine if there is enough room for a free block to exist before the
	// to-be-allocated block.  We have to have at least MINIMUM_DATA_SIZE_FOR_FREE_BLOCK bytes,
	// so the FreeBlockHeader will fit.
	long leftOverData = foundFreeBlock->CalcDataSize(newBlock);
	if (leftOverData >= MINIMUM_DATA_SIZE_FOR_FREE_BLOCK)
	{
		// There is enough space.
#ifdef SIMPLEHEAP_STATS
		m_freeBytesAvailableTotal -= foundFreeBlockSize - leftOverData;
#endif // SIMPLEHEAP_STATS

		previousFreeBlock = foundFreeBlock;
		previousBlock = foundFreeBlock;

		foundFreeBlock->SetNextBlock(newBlock);
	}
	else
	{
		// There is not enough space.  Remove the free block.
#ifdef SIMPLEHEAP_STATS
		m_freeBlockCount--;
		m_freeBytesAvailableTotal -= foundFreeBlockSize;
		m_usedBytesTotal += DiffPtr(newBlock, foundFreeBlock);
#endif // SIMPLEHEAP_STATS

		previousFreeBlock = foundFreeBlock->GetPreviousFreeBlock();
		previousBlock = foundFreeBlock->GetPreviousBlock();

		previousBlock->SetNextBlock(nextBlock);
		nextBlock->SetPreviousBlock(previousBlock);
		previousFreeBlock->SetNextFreeBlock(nextFreeBlock);
		nextFreeBlock->SetPreviousFreeBlock(previousFreeBlock);
	}

	// Can we make a free block following the to-be-allocated block?
	FreeBlockHeader* newFreeBlock = (FreeBlockHeader*)newBlock->GetData(totalSize);
	if (DiffPtr(nextBlock, newFreeBlock) >= sizeof(FreeBlockHeader))
	{
		// Yes.  Make the new block.
		newFreeBlock->SetPointers(newBlock, nextBlock, true);
		newFreeBlock->SetFreePointersAndLinks(previousFreeBlock, nextFreeBlock);

		nextBlock->SetPreviousBlock(newFreeBlock);
		nextBlock = newFreeBlock;

#ifdef SIMPLEHEAP_ALLOC_STATS
		nextBlock->m_requestedSize = 0;
		nextBlock->m_allocIndex = -1;
		nextBlock->m_allocName = NULL;
		nextBlock->m_fileName = NULL;
		nextBlock->m_lineNumber = (unsigned short)-1;
#endif // SIMPLEHEAP_ALLOC_STATS

#ifdef SIMPLEHEAP_STATS
		m_freeBlockCount++;
		m_freeBytesAvailableTotal += newFreeBlock->GetSize();
#endif // SIMPLEHEAP_STATS
	}

	// Create the block the user requested.
	newBlock->SetPreviousBlock(previousBlock);
	newBlock->SetNextBlock(nextBlock, false);
	previousBlock->SetNextBlock(newBlock);
	nextBlock->SetPreviousBlock(newBlock);

#ifdef SIMPLEHEAP_ALLOC_STATS
	newBlock->m_requestedSize = size;
	newBlock->m_allocIndex = m_allocIndex++;
	newBlock->m_allocName = allocName;
	newBlock->m_fileName = fileName;
	newBlock->m_lineNumber = lineNumber;

	*(unsigned long*)newBlock->GetData(size) = OVERRUN_CODE;
#endif // SIMPLEHEAP_ALLOC_STATS

#ifdef SIMPLEHEAP_STATS
	m_usedBlockCount++;
#ifdef SIMPLEHEAP_ALLOC_STATS
	m_usedBytesRequested += size;
#else
	m_usedBytesRequested += Align(m_alignment, size);
#endif
	m_usedBytesTotal += newBlock->GetSize() + sizeof(BlockHeader);
	if (m_usedBytesTotal > m_usedBytesPeak)
		m_usedBytesPeak = m_usedBytesTotal;
#endif // SIMPLEHEAP_STATS
	
#ifdef SIMPLEHEAP_EXTRA_CHECK_BLOCK
	// Integrity checks.
	CheckAllLinks(newBlock);
	CheckAllLinks(newBlock->GetNextBlock());

	FreeBlockHeader* sbNextFreeBlock = m_bottomBlock->GetNextFreeBlock();
	if (sbNextFreeBlock)
	{
		assert(sbNextFreeBlock->IsFreeBlock());
		CheckAllLinks(sbNextFreeBlock);
	}
	
	DebugIntegrityCheck();

    memset(newBlock->GetData(), 0xcd, size);
#endif // SIMPLEHEAP_EXTRA_CHECK_BLOCK

	if (m_multithreaded)
	{
		::LeaveCriticalSection((CRITICAL_SECTION*)m_criticalSection);
	}

	return newBlock->GetData();
}


/**
	Reallocate a previous block.
**/
void* SimpleHeap::Realloc(void* ptr, unsigned long newSize, AllocStyle allocStyle,
			const char* allocName, const char* fileName, int lineNumber)
{
	if (!ptr)
	{
		return Alloc(newSize, allocStyle, allocName, fileName, lineNumber);
	}

	if (newSize == 0)
	{
		Free(ptr);
		return NULL;
	}

	if (m_multithreaded)
	{
		::EnterCriticalSection((CRITICAL_SECTION*)m_criticalSection);
	}

	BlockHeader* block = BlockHeader::GetBlockHeader(ptr);
	long originalSize = block->GetRequestedSize();
	if ((long)newSize <= originalSize)
	{
		if (m_multithreaded)
		{
			::LeaveCriticalSection((CRITICAL_SECTION*)m_criticalSection);
		}
		return ptr;
	}

	void* newPtr = Alloc(newSize, allocStyle, allocName, fileName, lineNumber);
	memcpy(newPtr, ptr, originalSize < (long)newSize ? originalSize : newSize);
    if ((long)newSize > originalSize)
        memset((unsigned char*)newPtr + originalSize, 0xcd, newSize - originalSize);
	Free(ptr);
	if (m_multithreaded)
	{
		::LeaveCriticalSection((CRITICAL_SECTION*)m_criticalSection);
	}
	
	return newPtr;
}


/**
**/
void SimpleHeap::Free(void* ptr)
{
	// Skip NULL pointers.
	if (!ptr)
		return;

	if (m_multithreaded)
	{
		::EnterCriticalSection((CRITICAL_SECTION*)m_criticalSection);
	}

	// Retrieve the block header and do a couple integrity checks.
	FreeBlockHeader* toFreeBlock = (FreeBlockHeader*)BlockHeader::GetBlockHeader(ptr);
	if (toFreeBlock->IsFreeBlock())
	{
		assert(0);
#ifdef SIMPLEHEAP_STATS
		MemLog("%08x: Block is being double freed.\n", ptr);
#endif // SIMPLEHEAP_STATS
		if (m_multithreaded)
		{
			::LeaveCriticalSection((CRITICAL_SECTION*)m_criticalSection);
		}

		return;
	}

#ifdef SIMPLEHEAP_ALLOC_STATS
	if (*(unsigned long*)toFreeBlock->GetData(toFreeBlock->GetRequestedSize()) != OVERRUN_CODE)
	{
		assert(0);
#ifdef SIMPLEHEAP_STATS
		MemLog("%08x: Overrun check code is damaged.\n", ptr);
#endif // SIMPLEHEAP_STATS
		if (m_multithreaded)
		{
			::LeaveCriticalSection((CRITICAL_SECTION*)m_criticalSection);
		}

		return;
	}
#endif // SIMPLEHEAP_ALLOC_STATS

	// Retrieve a few pointers that will get clobbered.
	FreeBlockHeader* previousBlock = (FreeBlockHeader*)toFreeBlock->GetPreviousBlock();
	FreeBlockHeader* nextBlock = (FreeBlockHeader*)toFreeBlock->GetNextBlock();

#ifdef SIMPLEHEAP_STATS
	unsigned long originalRequestedSize = toFreeBlock->GetRequestedSize();
	unsigned long originalTotalSize = Align(m_alignment, toFreeBlock->GetSize());
#endif // SIMPLEHEAP_STATS

	// If the used block about to be freed is not large enough to hold a
	// FreeBlockHeader structure, then just free the block but don't
	// chain it into the free list.  The one exception is if the next
	// block is a free block.  If so, there will be plenty of space.
	if (toFreeBlock->GetSize() < MINIMUM_DATA_SIZE_FOR_FREE_BLOCK  &&  nextBlock->IsUsedBlock())
	{
		// Unlink the block.  There isn't enough space for it to be a free block.
		previousBlock->SetNextBlock(nextBlock);
		nextBlock->SetPreviousBlock(previousBlock);

#ifdef SIMPLEHEAP_EXTRA_CHECK_BLOCK
		CheckAllLinks(previousBlock);
		CheckAllLinks(nextBlock);

		FreeBlockHeader* sbNextFreeBlock = m_bottomBlock->GetNextFreeBlock();
		if (sbNextFreeBlock)
		{
			assert(sbNextFreeBlock->IsFreeBlock());
			CheckAllLinks(sbNextFreeBlock);
		}

		DebugIntegrityCheck();
#endif // SIMPLEHEAP_EXTRA_CHECK_BLOCK

		if (m_multithreaded)
		{
			::LeaveCriticalSection((CRITICAL_SECTION*)m_criticalSection);
		}

		return;
	}

	toFreeBlock->SetPreviousFreeBlock(NULL);

	// A SimpleHeap will never have a free block next to another free block.  Check
	// if the previous block is a free block.  If it is, merge with it.
	if (previousBlock->IsFreeBlock()  &&  previousBlock != m_bottomBlock)
	{
		// The previous block is free, so merge with it.
#ifdef SIMPLEHEAP_STATS
		m_freeBlockCount--;
		m_freeBytesAvailableTotal -= previousBlock->GetSize();
#endif // SIMPLEHEAP_STATS

		// Relink.
		previousBlock->SetNextBlock(nextBlock);
		nextBlock->SetPreviousBlock(previousBlock);

		// The new toFreeBlock is now the previous block.
		toFreeBlock = previousBlock;
	}

	// Is there a free block as the next block?
	if (nextBlock->IsFreeBlock()  &&  nextBlock != m_topBlock)
	{
		// The next block is free, so merge with it.
#ifdef SIMPLEHEAP_STATS
		m_freeBlockCount--;
		m_freeBytesAvailableTotal -= nextBlock->GetSize();
#endif // SIMPLEHEAP_STATS

		// Wedge it into the normal block list.
		toFreeBlock->SetNextBlock(nextBlock->GetNextBlock(), true);
		toFreeBlock->GetNextBlock()->SetPreviousBlock(toFreeBlock);

		// Wedge it into the free list, but bypassing the next block.
		toFreeBlock->SetNextFreeBlock(nextBlock->GetNextFreeBlock());
		if (!toFreeBlock->GetPreviousFreeBlock())
			toFreeBlock->SetPreviousFreeBlock(nextBlock->GetPreviousFreeBlock());

		toFreeBlock->GetPreviousFreeBlock()->SetNextFreeBlock(toFreeBlock);
		toFreeBlock->GetNextFreeBlock()->SetPreviousFreeBlock(toFreeBlock);
	}

	// Did the toFreeBlock become a free block in the above operations?
	if (!toFreeBlock->IsFreeBlock())
	{
		// No, it didn't.  Since the free block list is in ascending order,
		// it will have to be searched to find the right position.  This could be
		// optimized by caching some previous free blocks and starting the search
		// from them.
		for (FreeBlockHeader* block = m_bottomBlock; block != m_topBlock;
				block = block->GetNextFreeBlock())
		{
			if (toFreeBlock > block &&  toFreeBlock < block->GetNextFreeBlock())
			{
				toFreeBlock->SetFreePointersAndLinks(block, block->GetNextFreeBlock());
				break;
			}
		}
	}

#ifdef SIMPLEHEAP_STATS
	m_freeBlockCount++;
	m_freeBytesAvailableTotal += toFreeBlock->GetSize();
	m_usedBlockCount--;
	m_usedBytesRequested -= originalRequestedSize;
	m_usedBytesTotal -= originalTotalSize + sizeof(BlockHeader);
#endif // SIMPLEHEAP_STATS
	
#ifdef SIMPLEHEAP_CHECK_BLOCK
	// Integrity checks.
	assert(toFreeBlock->IsFreeBlock());
	assert(toFreeBlock->GetSize() == toFreeBlock->CalcDataSize(toFreeBlock->GetNextBlock()));
#endif // SIMPLEHEAP_CHECK_BLOCK

#ifdef SIMPLEHEAP_EXTRA_CHECK_BLOCK
	memset(toFreeBlock + 1, 0xfd, toFreeBlock->GetSize() - (sizeof(FreeBlockHeader) - sizeof(BlockHeader)));

	CheckAllLinks(toFreeBlock);

	FreeBlockHeader* sbNextFreeBlock = m_bottomBlock->GetNextFreeBlock();
	if (sbNextFreeBlock)
	{
		assert(sbNextFreeBlock->IsFreeBlock());
		CheckAllLinks(sbNextFreeBlock);
	}

	DebugIntegrityCheck();
#endif // SIMPLEHEAP_EXTRA_CHECK_BLOCK

	if (m_multithreaded)
	{
		::LeaveCriticalSection((CRITICAL_SECTION*)m_criticalSection);
	}
}


bool SimpleHeap::IsValidHeapAddress(void* ptr)
{
	return (ptr >= m_buffer  &&  ptr < m_buffer + m_bufferSize);
}


const char* SimpleHeap::GetName() const
{
	return m_name;
}

	
unsigned long SimpleHeap::GetAlignment()
{
	return m_alignment;
}

	
void SimpleHeap::SetAlignment(unsigned long alignment)
{
	m_alignment = alignment;
}


void SimpleHeap::IntegrityCheck()
{
#ifdef SIMPLEHEAP_STATS
	for (BlockHeader* currentBlock = m_bottomBlock->GetNextBlock(); currentBlock != m_topBlock;
			currentBlock = currentBlock->GetNextBlock())
	{
		if (!CheckBlock(currentBlock))
			break;
	}
#endif // SIMPLEHEAP_STATS
}


void SimpleHeap::LogStats(int checkpointBegin, int checkpointEnd)
{
	if (m_multithreaded)
	{
		::EnterCriticalSection((CRITICAL_SECTION*)m_criticalSection);
	}

#ifdef SIMPLEHEAP_STATS
	unsigned long usedBlockCount = 0;
	unsigned long freeBlockCount = 0;
	unsigned long usedBytesRequested = 0;
	unsigned long usedBytesTotal = 0;
	unsigned long freeBytesAvailableTotal = 0;

	if (checkpointBegin == 0)
	{
		usedBlockCount = m_usedBlockCount;
		usedBytesRequested = m_usedBytesRequested;
		usedBytesTotal = m_usedBytesTotal;
		freeBlockCount = m_freeBlockCount;
		freeBytesAvailableTotal = m_freeBytesAvailableTotal;
	}
	else
	{
		for (BlockHeader* currentBlock = m_bottomBlock->GetNextBlock();
				currentBlock != m_topBlock; currentBlock = currentBlock->GetNextBlock())
		{
#ifdef SIMPLEHEAP_ALLOC_STATS
			if (checkpointBegin != 0  &&  !InRange(currentBlock->m_allocIndex, checkpointBegin, checkpointEnd))
				continue;
#endif // SIMPLEHEAP_ALLOC_STATS

			if (currentBlock->IsUsedBlock())
			{
				usedBlockCount++;
				usedBytesRequested += currentBlock->GetSize();
				usedBytesTotal += currentBlock->GetSize() + sizeof(BlockHeader);
			}
			else
			{
				freeBlockCount++;
				freeBytesAvailableTotal += currentBlock->GetSize();
			}
		}
	}

	MemLog("%s:\n", GetName());
	MemLog("Used block count:           %u\n", usedBlockCount);
	MemLog("Free block count:           %u\n", freeBlockCount);
	MemLog("Used bytes requested:       %u\n", usedBytesRequested);
	MemLog("Used bytes total:           %u\n", usedBytesTotal);
	if (checkpointBegin != 0)
		MemLog("Used bytes peak:            %u\n", m_usedBytesPeak);
	MemLog("Free bytes available total: %u\n", m_freeBytesAvailableTotal);
	MemLog("-------------------------------------\n");
#endif // SIMPLEHEAP_STATS

	if (m_multithreaded)
	{
		::LeaveCriticalSection((CRITICAL_SECTION*)m_criticalSection);
	}
}


void SimpleHeap::HeapDump(FILE* file, int checkpointBegin, int checkpointEnd)
{
	if (m_multithreaded)
	{
		::EnterCriticalSection((CRITICAL_SECTION*)m_criticalSection);
	}

#ifdef SIMPLEHEAP_STATS
	for (BlockHeader* currentBlock = m_bottomBlock->GetNextBlock(); currentBlock != m_topBlock;
			currentBlock = currentBlock->GetNextBlock())
	{
#ifdef SIMPLEHEAP_ALLOC_STATS
		if (checkpointBegin != 0  &&  !InRange(currentBlock->m_allocIndex, checkpointBegin, checkpointEnd))
			continue;
#endif // SIMPLEHEAP_ALLOC_STATS

		if (!IsValidHeapAddress(currentBlock))
		{
			assert(0);
			fprintf(file, "Block [%08x] does not reside within the heap.\n", currentBlock);
			break;
		}

		fprintf(file, "%s\t", m_name);
		fprintf(file, "%s\t", currentBlock->IsFreeBlock() ? "FREE" : "USED");
		fprintf(file, "%08x\t", currentBlock);
		fprintf(file, "%10u\t", currentBlock->GetRequestedSize());
		fprintf(file, "%10u\t", currentBlock->GetSize() + sizeof(BlockHeader));

		int allocIndex = 0;
		const char* allocName = "Unknown";
		const char* fileName = "Unknown";
		int lineNumber = 0;
#ifdef SIMPLEHEAP_ALLOC_STATS
		if (currentBlock->IsUsedBlock())
		{
			allocIndex = currentBlock->m_allocIndex;
			allocName = currentBlock->m_allocName ? currentBlock->m_allocName : "Unknown";
			fileName = currentBlock->m_fileName ? currentBlock->m_fileName : "Unknown";
			lineNumber = currentBlock->m_lineNumber;
		}
#endif // SIMPLEHEAP_ALLOC_STATS
		fprintf(file, "%7d\t%s\t%s\t%d\n",
				allocIndex, allocName, fileName, lineNumber);
	}

	fprintf(file, "*************************************\n");
	fflush(file);
#endif // SIMPLEHEAP_STATS
	
	if (m_multithreaded)
	{
		::LeaveCriticalSection((CRITICAL_SECTION*)m_criticalSection);
	}
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// Protected:
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void SimpleHeap::Reset()
{
	if (m_multithreaded)
	{
		::EnterCriticalSection((CRITICAL_SECTION*)m_criticalSection);
	}

	// The heap has a bottom block header and a top block header, both of which
	// are made to appear as free blocks.  This is so block relinking algorithms
	// don't have to do special case work for these two blocks.  Neither block
	// has any room for data.
	m_bottomBlock = (FreeBlockHeader*)m_buffer;
	m_topBlock = (FreeBlockHeader*)(m_buffer + m_bufferSize - sizeof(FreeBlockHeader));

	// The free block is the actual usable free data space.
	FreeBlockHeader* freeBlock = m_bottomBlock + 1;

	// Link the three blocks.
	m_bottomBlock->SetPointers(NULL, freeBlock, true);
	m_bottomBlock->SetFreePointers(NULL, freeBlock);
	
	m_topBlock->SetPointers(freeBlock, (BlockHeader*)m_topBlock + 1, true);
	m_topBlock->SetFreePointers(freeBlock, NULL);

	freeBlock->SetPointers(m_bottomBlock, m_topBlock, true);
	freeBlock->SetFreePointers(m_bottomBlock, m_topBlock);

#ifdef SIMPLEHEAP_CHECK_BLOCK
	CheckBlock(m_bottomBlock);
	CheckBlock(m_topBlock);
	CheckBlock(freeBlock);
#endif // SIMPLEHEAP_CHECK_BLOCK

#ifdef SIMPLEHEAP_STATS
	m_freeBlockCount = 1;
	m_freeBytesAvailableTotal = freeBlock->GetSize();
#endif // SIMPLEHEAP_STATS

	DebugIntegrityCheck();

	if (m_multithreaded)
	{
		::LeaveCriticalSection((CRITICAL_SECTION*)m_criticalSection);
	}
}


void SimpleHeap::DebugIntegrityCheck()
{
#ifdef SIMPLEHEAP_ALLOC_STATS
	static bool s_debugIntegrityCheckOn = false;

	if (s_debugIntegrityCheckOn)
		IntegrityCheck();
#endif // SIMPLEHEAP_ALLOC_STATS
}


#ifdef SIMPLEHEAP_STATS
#define CB_ASSERT(condition, errorStr) \
	if (!(condition)) \
	{ \
		assert(0); \
		MemLog("%s\n", errorStr); \
		return false; \
	}
#else // SIMPLEHEAP_STATS
#define CB_ASSERT(condition, errorStr) \
	if (!(condition)) \
	{ \
		assert(0); \
		return false; \
	}
#endif // SIMPLEHEAP_STATS


/**
	Verifies the integrity of the block.
**/
bool SimpleHeap::CheckBlock(BlockHeader* block)
{
	if (!block)
		return true;

	CB_ASSERT(IsValidHeapAddress(block), "The block is not in the heap.\n");

	CB_ASSERT(block->GetPreviousBlock() != block  &&  block->GetNextBlock() != block,
			"The block's next or previous pointer refers back to the block.\n");
	
	if (block->GetPreviousBlock())
	{
		CB_ASSERT(IsValidHeapAddress(block->GetPreviousBlock()),
				"The previous block is not in the heap.\n");
		CB_ASSERT(block->GetPreviousBlock()->GetNextBlock() == block,
				"The previous block does not link forward to the block being tested.\n");
	}

	CB_ASSERT(IsValidHeapAddress(block->GetNextBlock()),
			"The next block is not in the heap.\n");
	CB_ASSERT(IsValidHeapAddress(block->GetData(block->GetSize())),
			"The next block's total size overruns the end of the heap.\n");
	if (block->GetSize() != 0)
		CB_ASSERT(block->GetNextBlock()->GetPreviousBlock() == block,
				"The next block does not link back to the block being tested.\n");

#ifdef SIMPLEHEAP_ALLOC_STATS
	if (block->IsUsedBlock())
	{
		unsigned long* oc = (unsigned long*)block->GetData(block->GetRequestedSize());
		CB_ASSERT(*oc == OVERRUN_CODE, "Overrun check code is damaged.\n");
	}
#endif // SIMPLEHEAP_ALLOC_STATS

	if (block->IsFreeBlock())
	{
		FreeBlockHeader* currentFreeBlock = (FreeBlockHeader*)block;

		FreeBlockHeader* previousFreeBlock = currentFreeBlock->GetPreviousFreeBlock();
		if (previousFreeBlock)
		{
			CB_ASSERT(IsValidHeapAddress(previousFreeBlock),
					"The previous free block is not in the heap.\n");
			CB_ASSERT(previousFreeBlock->IsFreeBlock(), "The previous 'free' block is not flagged as free.\n");
			CB_ASSERT(previousFreeBlock->GetNextFreeBlock() == currentFreeBlock,
					"The previous free block does not link forward to the block being tested.\n");
		}

		FreeBlockHeader* nextFreeBlock = currentFreeBlock->GetNextFreeBlock();
		if (nextFreeBlock)
		{
			CB_ASSERT(IsValidHeapAddress(nextFreeBlock),
					"The previous free block is not in the heap.\n");
			CB_ASSERT(nextFreeBlock->IsFreeBlock(), "The previous 'free' block is not flagged as free.\n");
			CB_ASSERT(nextFreeBlock->GetPreviousFreeBlock() == currentFreeBlock,
					"The next free block does not link back to the block being tested.\n");
		}
	}
		
	return true;
}


/**
	\internal CheckAllLinks() walks all the immediate block links of [block] and
		runs a full integrity check on each link.
**/
void SimpleHeap::CheckAllLinks(BlockHeader* block)
{
	if (!block)
		return;
	CheckBlock(block);

	if (block != m_topBlock)
		CheckBlock(block->GetNextBlock());
	if (block != m_bottomBlock)
		CheckBlock(block->GetPreviousBlock());

	if (block->IsFreeBlock())
	{
		FreeBlockHeader* freeBlock = (FreeBlockHeader*)block;
		if (freeBlock != m_topBlock)
			CheckBlock(freeBlock->GetNextFreeBlock());
		if (freeBlock != m_bottomBlock)
			CheckBlock(freeBlock->GetPreviousFreeBlock());
	}
}


/**
	\internal
	
	Retrieves the first block fitting [size] closest to the bottom of the heap.
**/
void SimpleHeap::FirstFitFromBottom(long size, BlockHeader*& block, FreeBlockHeader*& freeBlock)
{
	// Traverse the free list from the bottom to the top.
	for (freeBlock = m_bottomBlock->GetNextFreeBlock();
			freeBlock != m_topBlock; freeBlock = freeBlock->GetNextFreeBlock())
	{
		// Align the data as the user has requested.
		void* alignPtr = Align(m_alignment, freeBlock->GetData());

		// Make sure the block has enough bytes to do the operation.
		if (DiffPtr(freeBlock->GetNextBlock(), alignPtr) >= size)
		{
			// Found a suitable block.
			block = BlockHeader::GetBlockHeader(alignPtr);
#ifdef SIMPLEHEAP_CHECK_BLOCK
			CheckBlock(freeBlock);
#endif // SIMPLEHEAP_CHECK_BLOCK
			return;
		}
	}

	// No block was found.
	block = NULL;
	freeBlock = NULL;
}


/**
	\internal

	Retrieves the first block fitting [size], retrieved from the top of the heap.
**/
void SimpleHeap::FirstFitFromTop(long size, BlockHeader*& block, FreeBlockHeader*& freeBlock)
{
	// Traverse the free list from the top to the bottom.
	for (freeBlock = m_topBlock->GetPreviousFreeBlock();
			freeBlock != m_bottomBlock; freeBlock = freeBlock->GetPreviousFreeBlock())
	{
		// Align the data as the user has requested.
		void* alignPtr = AlignDown(m_alignment, (unsigned char*)freeBlock->GetNextBlock() - size);
		block = BlockHeader::GetBlockHeader(alignPtr);

		// Make sure the block has enough bytes to do the operation.
		if (DiffPtr(block, freeBlock) >= 0)
		{
			// Found a suitable block.
#ifdef SIMPLEHEAP_CHECK_BLOCK
			CheckBlock(freeBlock);
#endif // SIMPLEHEAP_CHECK_BLOCK
			return;
		}
	}

	// No block was found.
	block = NULL;
	freeBlock = NULL;
}


} // namespace SimpleMem
