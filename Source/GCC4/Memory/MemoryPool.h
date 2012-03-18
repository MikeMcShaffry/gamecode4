#pragma once
//========================================================================
// MemoryPool.h : 
//
// Part of the GameCode4 Application
//
// GameCode4 is the sample application that encapsulates much of the source code
// discussed in "Game Coding Complete - 4th Edition" by Mike McShaffry and David
// "Rez" Graham, published by Charles River Media. 
// ISBN-10: 1133776574 | ISBN-13: 978-1133776574
//
// If this source code has found it's way to you, and you think it has helped you
// in any way, do the authors a favor and buy a new copy of the book - there are 
// detailed explanations in it that compliment this code well. Buy a copy at Amazon.com
// by clicking here: 
//    http://www.amazon.com/gp/product/1133776574/ref=olp_product_details?ie=UTF8&me=&seller=
//
// There's a companion web site at http://www.mcshaffry.com/GameCode/
// 
// The source code is managed and maintained through Google Code: 
//    http://code.google.com/p/gamecode4/
//
// (c) Copyright 2012 Michael L. McShaffry and David Graham
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser GPL v3
// as published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See 
// http://www.gnu.org/licenses/lgpl-3.0.txt for more details.
//
// You should have received a copy of the GNU Lesser GPL v3
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
//========================================================================

//--------------------------------------------------------------------------------------------------
// This class represents a single memory pool.  A memory pool is pool of memory that's split into 
// chunks of equal size, each with a 4-byte header.  The header is treated as a pointer that points
// to the next chunk, making the pool a singly-linked list of memory chunks.
// 
// When the pool is first initialized (via the Init() function), you must pass in a chunk size and
// the number of chunks you want created.  These two values are immutable unless you destroy and
// reinitialize the entire pool.  The chunk size is the size of each chunk, minus the header, in 
// bytes.  The memory pool will allocate the appropriate amount of memory and set up the data
// structure in the Init() call.  Thus, total memory usage will be N * (S + 4) + O, where N is the
// number of chunks, S is the size of each chunk, and O is the overhead for the class (currently 
// 18 + (number of reallocations * 4).
// 
// Call the Alloc() function to retrieve a chunk from the memory pool.  The Alloc() function removes
// the head of the linked list, sets the new head to the next chunk, and returns a pointer to the 
// data section of the old head.  If there aren't anymore chunks left when Alloc() is called, it 
// will allocate another block of N chunks, where N is the number of chunks you passed into Init().
// While Alloc() is typically a very fast function, this reallocation will certainly cost you so 
// choose your initial sizes carefully.
// 
// Call the Free() function to release a chunk of memory back into the memory pool for reuse.  This
// will cause the chunk to the inserted to the front of the list, ready for the next bit.
//--------------------------------------------------------------------------------------------------
class MemoryPool
{
	unsigned char** m_ppRawMemoryArray;  // an array of memory blocks, each split up into chunks and connected
	unsigned char* m_pHead;  // the front of the memory chunk linked list
	unsigned int m_chunkSize, m_numChunks;  // the size of each chunk and number of chunks per array, respectively
	unsigned int m_memArraySize;  // the number elements in the memory array
	bool m_toAllowResize;  // true if we resize the memory pool when it fills up

    // tracking variables we only care about for debug
#ifdef _DEBUG
    std::string m_debugName;
    unsigned long m_allocPeak, m_numAllocs;
#endif

public:
	// construction
	MemoryPool(void);
	~MemoryPool(void);
	bool Init(unsigned int chunkSize, unsigned int numChunks);
	void Destroy(void);
	
	// allocation functions
	void* Alloc(void);
	void Free(void* pMem);
	unsigned int GetChunkSize(void) const { return m_chunkSize; }
	
	// settings
	void SetAllowResize(bool toAllowResize) { m_toAllowResize = toAllowResize; }
	
	// debug functions
#ifdef _DEBUG
    void SetDebugName(const char* debugName) { m_debugName = debugName; }
	std::string GetDebugName(void) const { return m_debugName; }
#else
	void SetDebugName(const char* debugName) { }
	//std::string GetDebugName(void) const { return (std::string("<No Name>")); }
	const char* GetDebugName(void) const { return "<No Name>"; }
#endif
	
private:
	// resets internal vars
	void Reset(void);

	// internal memory allocation helpers
	bool GrowMemoryArray(void);
	unsigned char* AllocateNewMemoryBlock(void);
	
	// internal linked list management
	unsigned char* GetNext(unsigned char* pBlock);
	void SetNext(unsigned char* pBlockToChange, unsigned char* pNewNext);
	
	// don't allow copy constructor
	MemoryPool(const MemoryPool& memPool) {}
};
