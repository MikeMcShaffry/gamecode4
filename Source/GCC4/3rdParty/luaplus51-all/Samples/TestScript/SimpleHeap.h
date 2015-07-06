#ifndef SIMPLEHEAP_H
#define SIMPLEHEAP_H

#include <stdio.h>

#define SIMPLEHEAP_STATS

namespace SimpleMem {

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


class SimpleHeap
{
public:
	enum AllocStyle
	{
		FIRSTFIT_BOTTOM,
		FIRSTFIT_TOP,
	};

	// Constructors
	SimpleHeap(const char* name, bool multithreaded);
	~SimpleHeap();

	void Initialize(unsigned long size);
	void Destroy();

	void* Alloc(unsigned long size, AllocStyle allocStyle = FIRSTFIT_BOTTOM,
			const char* allocName = 0, const char* fileName = 0, int lineNumber = -1);
	void* Realloc(void* ptr, unsigned long size, AllocStyle allocStyle = FIRSTFIT_BOTTOM,
			const char* allocName = 0, const char* fileName = 0, int lineNumber = -1);
	void Free(void* ptr);
	bool IsValidHeapAddress(void* ptr);

	const char* GetName() const;
	unsigned long GetAlignment();
	void SetAlignment(unsigned long alignmentBytes);

	void IntegrityCheck();

	void LogStats(int checkpointBegin = 0, int checkpointEnd = 0);
	void HeapDump(FILE* file, int checkpointBegin = 0, int checkpointEnd = 0);

protected:
	struct BlockHeader;
	struct FreeBlockHeader;

	static const long MINIMUM_DATA_SIZE_FOR_FREE_BLOCK;

	void Reset();
	void DebugIntegrityCheck();
	bool CheckBlock(BlockHeader* block);
	void CheckAllLinks(BlockHeader* block);

	void FirstFitFromBottom(long size, BlockHeader*& block, FreeBlockHeader*& freeBlock);
	void FirstFitFromTop(long size, BlockHeader*& block, FreeBlockHeader*& freeBlock);

	unsigned char* m_buffer;
	unsigned long m_bufferSize;
	unsigned long m_alignment;

	FreeBlockHeader* m_bottomBlock;
	FreeBlockHeader* m_topBlock;

	char m_name[100];

#ifdef SIMPLEHEAP_STATS
	unsigned long m_usedBlockCount;
	unsigned long m_freeBlockCount;
	unsigned long m_usedBytesRequested;
	unsigned long m_usedBytesTotal;
	unsigned long m_usedBytesPeak;
	unsigned long m_freeBytesAvailableTotal;
	unsigned long m_allocIndex;
#endif // SIMPLEHEAP_STATS

	bool m_multithreaded;
	unsigned char m_criticalSection[32];
};


} // namespace SimpleMem

#endif // SIMPLEHEAP_H
