#ifndef MISC_ARRAY_H
#define MISC_ARRAY_H

#include "TypeTraits.h"
#include "HeapAllocator.h"
#include <assert.h>

namespace Misc {

template <typename T, class TTraits = TypeTraits<T>, class Allocator = HeapAllocator >
class Array : public Allocator
{
public:
	typedef typename TTraits::INARGTYPE INARGTYPE;
	typedef typename TTraits::OUTARGTYPE OUTARGTYPE;
	typedef typename TTraits::CONSTOUTARGTYPE CONSTOUTARGTYPE;

public:
	Array() : m_size(0), m_count(0), m_items(NULL)		{  }
	Array(const Array& src) : m_size(0), m_count(0), m_items(NULL)	{  Copy(src);  }

	Array& operator=(const Array& src)
	{
		if (this == &src)
			return *this;
		Copy(src);
		return *this;
	}

	~Array()
	{
		TTraits::DestructElements(m_items, m_count);
		Allocator::Free(m_items);
	}

	void Empty()					{  SetCount(0);  }		// Set the count to 0.
	void Clear()					{  Alloc(0);  }			// Set the count to 0 and free the memory.

	void Alloc(size_t newSize)
	{
		if(newSize == 0)
		{
			TTraits::DestructElements(m_items, m_count);
			Allocator::Free(m_items);
			m_size = 0;
			m_count = 0;
			m_items = NULL;
			return;
		}
		if (newSize > m_size)
		{
			Grow(newSize);
			return;
		}

		if (m_count > newSize)
		{
			T* newItems = static_cast< T* >(Allocator::Alloc(newSize * sizeof(T)));
			TTraits::CopyConstructElements(newItems, m_items, newSize);
			TTraits::DestructElements(m_items + newSize, m_count - newSize);
			Allocator::Free(m_items);
			m_size = newSize;
			m_count = newSize;
			m_items = newItems;
		}
	}

	size_t Size() const								{  return m_size;  }
	size_t Count() const							{  return m_count;  }

	void SetCount(size_t newCount)
	{
		if (newCount == 0)
		{
			TTraits::DestructElements(m_items, m_count);
			m_count = 0;
			return;
		}

		if (newCount > m_size)
		{
			Grow(newCount);
			TTraits::ConstructElements(m_items + m_count, newCount - m_count);
			m_count = newCount;
			return;
		}

		if (newCount > m_count)
			TTraits::ConstructElements(m_items + m_count, newCount - m_count);
		else if (m_count > newCount)
			TTraits::DestructElements(m_items + newCount, m_count - newCount);
		m_count = newCount;
	}

	bool IsEmpty() const								{  return m_count == 0;  }

	CONSTOUTARGTYPE Item(size_t index) const			{  assert(index < m_count);  return m_items[index];  }
	OUTARGTYPE Item(size_t index)						{  assert(index < m_count);  return m_items[index];  }
	CONSTOUTARGTYPE operator[](size_t index) const
	{
		if (index >= m_count)
		{
			int hi = 5; (void)hi;
		}
		assert(index < m_count);
		return m_items[index];
	}
	OUTARGTYPE operator[](size_t index)
	{
		if (index >= m_count)
		{
			int hi = 5; (void)hi;
		}
		assert(index < m_count);
		return m_items[index];
	}
	
	void Set(size_t index, INARGTYPE element)			{  assert(index < m_count);  m_items[index] = element;  }

	T* Items() const									{  return m_items;  }

	size_t Add()										{  SetCount(m_count + 1);  return m_count - 1;  }
	size_t Add(INARGTYPE element)						{  Grow(m_count + 1);  TTraits::ConstructElements(m_items + m_count, element, 1);  return m_count++;  }

	void Insert(size_t index, INARGTYPE element, size_t count = 1)
	{
		if (count == 0)
			return;

		if (index >= m_count)
		{
			Grow(index + count);
			TTraits::ConstructElements(m_items + index, element, count);
			m_count = index + count;
			return;
		}

		Grow(m_count + count);

		TTraits::RelocateElements(m_items + (index + count), m_items + index, m_count - index);
		TTraits::ConstructElements(m_items + index, element, count);

		m_count += count;
	}


	void Remove(size_t index, size_t count = 1)
	{
		if (count == 0  ||  index + count > m_count)
			return;

		size_t moveCount = m_count - (index + count);
		TTraits::DestructElements(m_items + index, count);
		TTraits::RelocateElements(m_items + index, m_items + (index + count), moveCount);
		m_count -= count;
	}

	void Copy(const Array<T, TTraits, Allocator>& src)
	{
		assert(this != &src);
		SetCount(src.m_count);
		TTraits::CopyElements(m_items, src.m_items, src.m_count);
	}

	typedef int (MISC_CDECL *CompareFunction)(const T*, const T*);
	void Sort(CompareFunction compareFunction)
	{
		qsort(m_items, m_count, sizeof(T), (int(MISC_CDECL*)(const void*, const void*))compareFunction);
	}
	
private:
	void Grow(size_t newSize)
	{
		if (newSize <= m_size)
			return;

		if (m_size == 0)
		{
			m_items = (T*)Allocator::Alloc(newSize * sizeof(T));
			m_size = newSize;
			return;
		}

		size_t growBy = m_size / 4;
		if (growBy < 16)
			growBy = 16;
		else if (growBy > 1024)
			growBy = 1024;

		if (newSize < m_size + growBy)
			newSize = m_size + growBy;

		T* newItems = (T*)Allocator::Alloc(newSize * sizeof(T));
		TTraits::CopyConstructElements(newItems, m_items, m_count);
		Allocator::Free(m_items);
		m_items = newItems;
		m_size = newSize;
	}

	size_t m_size;
	size_t m_count;
	T* m_items;
};

} // namespace Misc

#endif // MISC_ARRAY_H
