#ifndef MISC_ARRAYOWNEDPTR_H
#define MISC_ARRAYOWNEDPTR_H

#include "Array.h"
#include "OwnedPtr.h"

namespace Misc {

template<typename T>
class ArrayOwnedPtrTypeTraits : public DefaultTypeTraits<OwnedPtr<T> >
{
public:
	typedef OwnedPtr<T>& INARGTYPE;
	typedef T* OUTARGTYPE;
	typedef const T* CONSTOUTARGTYPE;
};


template <typename T>
class ArrayOwnedPtr : public Array<OwnedPtr<T>, ArrayOwnedPtrTypeTraits<T> >
{
public:
	ArrayOwnedPtr()
	{
	}

private:
	ArrayOwnedPtr(const ArrayOwnedPtr&);
	ArrayOwnedPtr& operator=(const ArrayOwnedPtr&);
};

} // namespace Misc

#endif // MISC_ARRAYOWNEDPTR_H
