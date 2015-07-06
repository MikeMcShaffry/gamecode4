#ifndef MISC_HEAPALLOCATOR_H
#define MISC_HEAPALLOCATOR_H

namespace Misc {

class HeapAllocator
{
public:
	void* Alloc(size_t size)					{  return new unsigned char[size];  }
	void Free(void* ptr)						{  delete[] (unsigned char*)ptr;  }
	void FreeAll()								{  }
	void SetBlockSize(int)						{  }
	void SetObjectSize(int)						{  }
};

} // namespace Misc

#endif // MISC_HEAPALLOCATOR_H
