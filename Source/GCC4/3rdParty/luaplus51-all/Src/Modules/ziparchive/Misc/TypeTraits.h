#ifndef MISC_TYPETRAITS_H
#define MISC_TYPETRAITS_H

#include <new>
#include <string.h>

namespace Misc {

template <typename T>
class DefaultTypeTraits
{
public:
	typedef const T& INARGTYPE;
	typedef T& OUTARGTYPE;
	typedef const T& CONSTOUTARGTYPE;

	static void ConstructElements(T* element, size_t count)				{  while (count--)  ::new(element++) T;  }
	static void ConstructElements(T* element, INARGTYPE src, size_t count)	{  while (count--)  ::new(element++) T(src);  }
	static void DestructElements(T* element, size_t count)				{  (void)element;  while (count--)  (element++)->~T();  }
	static void CopyElements(T* dest, const T* src, size_t count)		{  while (count--)  *dest++ = *src++;  }

	static void RelocateElements(T* dest, T* src, size_t count)			{  memmove(dest, src, count * sizeof(T));  }
	static void CopyConstructElements(T* dest, T* src, size_t count)	{  memmove(dest, src, count * sizeof(T));  }
	static size_t Hash(const T& element)								{  return (unsigned int)element;  }
	static bool CompareElements(const T& element1, const T& element2)	{  return element1 == element2;  }
	static int CompareElementsOrdered(const T& element1, const T& element2)  {  return element1 < element2 ? -1 : (element1 == element2 ? 0 : 1);  }
};

template <typename T>
class TypeTraits : public DefaultTypeTraits< T >
{
};

template< typename T >
class PrimitiveTypeTraits : public DefaultTypeTraits< T >
{
public:
	typedef T INARGTYPE;

	static void ConstructElements(T*, size_t)								{  }
	static void ConstructElements(T* element, INARGTYPE src, size_t count)  {  while (count--)  *element = src;  }
	static void DestructElements(T*, size_t)								{  }
	static void CopyElements(T* dest, const T* src, size_t count)			{  memcpy(dest, src, count * sizeof(T));  }
	static void RelocateElements(T* dest, T* src, size_t count)				{  memmove(dest, src, count * sizeof(T));  }
	static void CopyConstructElements(T* dest, T* src, size_t count)		{  memmove(dest, src, count * sizeof(T));  }
};

template<> class TypeTraits<unsigned char> : public PrimitiveTypeTraits<unsigned char> {};
template<> class TypeTraits<unsigned short> : public PrimitiveTypeTraits<unsigned short> {};
template<> class TypeTraits<unsigned int> : public PrimitiveTypeTraits<unsigned int> {};
template<> class TypeTraits<unsigned long> : public PrimitiveTypeTraits<unsigned long> {};
#if _MSC_VER  &&  _MSC_VER <= 1300
template<> class TypeTraits<unsigned __int64> : public PrimitiveTypeTraits<unsigned __int64> {};
#else
template<> class TypeTraits<unsigned long long> : public PrimitiveTypeTraits<unsigned long long> {};
#endif
template<> class TypeTraits<signed char> : public PrimitiveTypeTraits<signed char> {};
template<> class TypeTraits<char> : public PrimitiveTypeTraits<char> {};
template<> class TypeTraits<short> : public PrimitiveTypeTraits<short> {};
template<> class TypeTraits<int> : public PrimitiveTypeTraits<int> {};
template<> class TypeTraits<long> : public PrimitiveTypeTraits<long> {};
#if _MSC_VER  &&  _MSC_VER <= 1300
template<> class TypeTraits<__int64> : public PrimitiveTypeTraits<__int64> {};
#else
template<> class TypeTraits<long long> : public PrimitiveTypeTraits<long long> {};
#endif
template<> class TypeTraits<float> : public PrimitiveTypeTraits<float> {};
template<> class TypeTraits<double> : public PrimitiveTypeTraits<double> {};
template<> class TypeTraits<bool> : public PrimitiveTypeTraits<bool> {};
template<> class TypeTraits<void*> : public PrimitiveTypeTraits<void*> {};

} // namespace Misc

#endif // TYPETRAITS_H
