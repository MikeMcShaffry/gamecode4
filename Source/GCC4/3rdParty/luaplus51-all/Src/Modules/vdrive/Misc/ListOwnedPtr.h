#ifndef MISC_LISTOWNEDPTR_H
#define MISC_LISTOWNEDPTR_H

#include "List.h"
#include "OwnedPtr.h"

namespace Misc {

template<typename T>
class ListOwnedPtrTypeTraits : public DefaultTypeTraits<OwnedPtr<T> >
{
public:
	typedef OwnedPtr<T>& INARGTYPE;
	typedef T* OUTARGTYPE;
	typedef const T* CONSTOUTARGTYPE;
};


template <typename T, class NodeTraits = ListNodeTraits<OwnedPtr<T>, ListOwnedPtrTypeTraits<T> > >
class ListOwnedPtr : public List<OwnedPtr<T>, ListOwnedPtrTypeTraits<T>, NodeTraits >
{
public:
	ListOwnedPtr()
	{
	}

private:
	ListOwnedPtr(const ListOwnedPtr&);
	ListOwnedPtr& operator=(const ListOwnedPtr&);
};

} // namespace Misc

#endif // MISC_LISTOWNEDPTR_H
