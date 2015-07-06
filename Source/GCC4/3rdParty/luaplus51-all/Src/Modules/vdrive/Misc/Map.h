#ifndef MISC_MAP_H
#define MISC_MAP_H

#include "Misc.h"
#include "TypeTraits.h"
#include "FixedSizeAllocator.h"
#include <new>

namespace Misc {

template <typename KEY, typename VALUE, class KeyTraits = TypeTraits<KEY>, class ValueTraits = TypeTraits<VALUE>, class Allocator = FixedSizeAllocator>
class Map : public Allocator
{
public:
	typedef typename KeyTraits::INARGTYPE KINARGTYPE;
	typedef typename ValueTraits::INARGTYPE VINARGTYPE;

	class Node
	{
		friend class Map;

		Node* m_next;
		size_t m_hash;
		const KEY m_key;
		VALUE m_value;

		Node(KINARGTYPE key, size_t hash) : m_hash(hash), m_key(key)	{ }
		Node(const Node& src);					// Not implemented.
		Node& operator=(const Node& src);		// Not implemented.
	};

public:
	Map() : m_buckets(NULL), m_hashTableSize(20), m_count(0)	{  Allocator::SetAllocSize(sizeof(Node));  }
	~Map()											{  Clear();  }

	size_t GetHashTableSize() const					{  return m_hashTableSize;  }
	void SetHashTableSize(size_t hashTableSize)		{  m_hashTableSize = hashTableSize > 0 ? hashTableSize : 20;  }

	size_t Count() const							{  return m_count;  }
	bool IsEmpty() const							{  return m_count == 0;  }

	const KEY& Key(Node* node) const				{  return node->m_key;  }
	const VALUE& Value(Node* node) const			{  return node->m_value;  }
	VALUE& Value(Node* node)						{  return node->m_value;  }

	Node* Head() const
	{
		if (m_count == 0)
			return NULL;

		for (size_t bucket = 0; bucket < m_hashTableSize; ++bucket)
		{
			Node* node = m_buckets[bucket];
			if (node)
				return node;
		}

		return NULL;
	}

	Node* Next(Node* node) const
	{
		if (node->m_next)
			return node->m_next;

		for (size_t bucket = (node->m_hash % m_hashTableSize) + 1; bucket < m_hashTableSize; ++bucket)
		{
			if (m_buckets[bucket])
				return m_buckets[bucket];
		}

		return NULL;
	}

	Node* Find(KINARGTYPE key) const
	{
		if (m_buckets)
		{
			size_t hash = KeyTraits::Hash(key);
			for (Node* node = m_buckets[hash % m_hashTableSize]; node; node = node->m_next)
			{
				if (node->m_hash == hash  &&  KeyTraits::CompareElements(node->m_key, key))
					return node;
			}
		}

		return NULL;
	}

	VALUE& operator[](KINARGTYPE key)
	{
		return Insert(key)->m_value;
	}

	void Empty()
	{
		if (m_buckets)
		{
			for (size_t index = 0; index < m_hashTableSize; ++index)
			{
				Node* nextNode = NULL;
				for (Node* curNode = m_buckets[index]; curNode; curNode = nextNode)
				{
					nextNode = curNode->m_next;
					curNode->~Node();
					Allocator::Free(curNode);
				}
			}
		}

		delete[] m_buckets;
		m_buckets = NULL;
		m_count = 0;
	}

	void Clear()
	{
		Empty();
		Allocator::FreeAll();
	}

	Node* Insert(KINARGTYPE key)
	{
		if (!m_buckets)
		{
			m_buckets = new Node*[m_hashTableSize];
			memset(m_buckets, 0, sizeof(Node*) * m_hashTableSize);
		}

		size_t hash = KeyTraits::Hash(key);
		size_t bucket = hash % m_hashTableSize;
		for (Node* node = m_buckets[bucket]; node; node = node->m_next)
		{
			if (node->m_hash == hash  &&  KeyTraits::CompareElements(node->m_key, key))
				return node;
		}

		Node* newNode = (Node*)Allocator::Alloc(sizeof(Node));
		::new(newNode) Node(key, hash);
		newNode->m_next = m_buckets[bucket];
		m_buckets[bucket] = newNode;
		m_count++;
		return newNode;
	}

	bool Remove(KINARGTYPE key)
	{
		if (m_buckets == NULL)
			return false;

		size_t hash = KeyTraits::Hash(key);
		Node** prev = &m_buckets[hash % m_hashTableSize];
		for (Node* node = *prev; node; node = node->m_next)
		{
			if (node->m_hash == hash  &&  KeyTraits::CompareElements(node->m_key, key))
			{
				*prev = node->m_next;
				node->~Node();
				Allocator::Free(node);
				m_count--;
				return true;
			}
			prev = &node->m_next;
		}
		return false;
	}

	void RemoveNode(Node* node)
	{
		size_t bucket = node->m_hash % m_hashTableSize;

		if (node == m_buckets[bucket])
			m_buckets[bucket] = node->m_next;
		else
		{
			Node* prev = m_buckets[bucket];
			while (prev->m_next != node)
				prev = prev->m_next;
			prev->m_next = node->m_next;
		}

		node->~Node();
		Allocator::Free(node);
		m_count--;
	}

private:
	Node** m_buckets;
	size_t m_hashTableSize;
	size_t m_count;

	Map( const Map& );
	Map& operator=( const Map& );
};

} // namespace Misc

#endif // MISC_MAP_H
