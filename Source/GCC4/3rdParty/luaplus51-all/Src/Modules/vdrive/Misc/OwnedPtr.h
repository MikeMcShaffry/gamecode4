#ifndef MISC_OWNEDPTR_H
#define MISC_OWNEDPTR_H

namespace Misc {

template <typename T>
class OwnedPtr
{
public:
	OwnedPtr() : _ownedPtr(NULL)					{ }
	OwnedPtr(const OwnedPtr<T>& src)				{  _ownedPtr = const_cast<OwnedPtr<T>& >(src)._ownedPtr;  const_cast<OwnedPtr<T>& >(src)._ownedPtr = NULL;  }
	~OwnedPtr()										{  delete _ownedPtr;  }

	void Own(T* src)								{  delete _ownedPtr;  _ownedPtr = src;  }
	void Destroy()									{  delete _ownedPtr;  _ownedPtr = NULL;  }

	OwnedPtr<T>& operator=(OwnedPtr<T>& src)
	{
		if (this == &src)
			return *this;

		delete _ownedPtr;
		_ownedPtr = src._ownedPtr;
		src._ownedPtr = NULL;

		return *this;
	}

	operator T*() const								{  return _ownedPtr;  }
	T* operator->() const							{  return _ownedPtr;  }
	T* Get() const									{  return _ownedPtr;  }

private:
	T* _ownedPtr;
};

} // namespace Misc

#endif // MISC_OWNEDPTR_H

