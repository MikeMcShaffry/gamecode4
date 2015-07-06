#ifndef MISC_LIST_H
#define MISC_LIST_H

#include "TypeTraits.h"
#include "FixedSizeAllocator.h"

#include <new>

namespace Misc {

template <typename T>
class InPlaceNodeTraits
{
public:
	typedef T Node;

	T& Value(T* base)											{  return *base;  }
	const T& Value(T* base) const								{  return *base;  }

	void SetAllocSize(size_t)									{  }
	void SetBlockSize(size_t)									{  }
	void FreeAll()												{  }

	Node* CreateNode();											// Not implemented.
	Node* CreateNode(const T& element)							{  return (Node*)&element;  }
	void DestroyNode(Node* node)								{  }
};


template <typename T, typename TTraits, typename AllocatorPolicy = FixedSizeAllocator >
class ListNodeTraits : public AllocatorPolicy
{
public:
	typedef AllocatorPolicy Allocator;

	ListNodeTraits()								{}

	class Node
	{
	public:
		Node() {}
		Node(typename TTraits::INARGTYPE inElement) : m_element(inElement) {}

	protected:
		friend class ListNodeTraits<T, TTraits, AllocatorPolicy>;
		Node* m_previousNode;
		Node* m_nextNode;
		T m_element;

		Node(const Node&);
		Node& operator=(const Node&);
	};

	typename TTraits::OUTARGTYPE Value(Node* node)				{  return node->m_element;  }
	typename TTraits::CONSTOUTARGTYPE Value(Node* node) const	{  return node->m_element;  }

protected:
	Node*& PreviousNode(Node* node) const						{  return node->m_previousNode;  }
	Node*& NextNode(Node* node) const							{  return node->m_nextNode;  }

	Node* CreateNode()
	{
		Node* insertNode = (Node*)Allocator::Alloc(sizeof(Node));
#if defined(_MSC_VER)
#pragma push_macro("new")
#undef new
#endif
		::new(insertNode) Node;
#if defined(_MSC_VER)
#pragma pop_macro("new")
#endif
		return insertNode;
	}

	Node* CreateNode(typename TTraits::INARGTYPE element)
	{
		Node* insertNode = (Node*)Allocator::Alloc(sizeof(Node));
#if defined(_MSC_VER)
#pragma push_macro("new")
#undef new
#endif
		::new(insertNode) Node(element);
#if defined(_MSC_VER)
#pragma pop_macro("new")
#endif
		return insertNode;
	}

	void DestroyNode(Node* node)								{  node->~Node();  Allocator::Free(node);  }
};


////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
template <typename T, class TTraits = TypeTraits<T>, class NodeTraits = ListNodeTraits<T, TTraits> >
class List : public NodeTraits
{
public:
	typedef typename TTraits::INARGTYPE INARGTYPE;
	typedef typename NodeTraits::Node Node;

	List() : m_headNode(NULL), m_tailNode(NULL), m_count(0) {  NodeTraits::SetAllocSize(sizeof(Node));  }
	~List()											{  Clear();  }

	size_t Count() const								{  return m_count;  }
	bool IsEmpty() const								{  return m_count == 0;  }

	Node* Head() const									{  return m_headNode;  }
	Node* Tail() const									{  return m_tailNode;  }
	Node* Next(Node* node) const						{  return NodeTraits::NextNode(node);  }
	Node* Previous(Node* node) const					{  return NodeTraits::PreviousNode(node);  }

	Node* FindIndex(size_t index) const
	{
		Node* node = Head();
		while (index--  &&  node)
		{
			node = Next(node);
		}
		return node;
	}

	Node* InsertNodeBefore(Node* beforeNode, Node* insertNode)
	{
		Node* beforePreviousNode = NodeTraits::PreviousNode(beforeNode);
		NodeTraits::PreviousNode(insertNode) = beforePreviousNode;
		NodeTraits::NextNode(insertNode) = beforeNode;

		if (beforePreviousNode)
			NodeTraits::NextNode(beforePreviousNode) = insertNode;
		else
			m_headNode = insertNode;

		NodeTraits::PreviousNode(beforeNode) = insertNode;
		m_count++;
		return insertNode;
	}

	Node* InsertBefore(Node* beforeNode)
	{
		return InsertNodeBefore(beforeNode, NodeTraits::CreateNode());
	}

	Node* InsertBefore(Node* beforeNode, INARGTYPE element)
	{
		return InsertNodeBefore(beforeNode, NodeTraits::CreateNode(element));
	}

	Node* InsertNodeBeforeHead(Node* insertNode)
	{
		NodeTraits::PreviousNode(insertNode) = NULL;
		NodeTraits::NextNode(insertNode) = m_headNode;

		if (m_headNode)
			NodeTraits::PreviousNode(m_headNode) = insertNode;
		else
			m_tailNode = insertNode;

		m_headNode = insertNode;
		m_count++;
		return insertNode;
	}

	Node* InsertBeforeHead()
	{
		return InsertNodeBeforeHead(NodeTraits::CreateNode());
	}

	Node* InsertBeforeHead(INARGTYPE element)
	{
		return InsertNodeBeforeHead(NodeTraits::CreateNode(element));
	}

	Node* InsertNodeAfter(Node* afterNode, Node* insertNode)
	{
		Node* afterNextNode = NodeTraits::NextNode(afterNode);
		NodeTraits::PreviousNode(insertNode) = afterNode;
		NodeTraits::NextNode(insertNode) = afterNextNode;

		if (afterNextNode)
			NodeTraits::PreviousNode(afterNextNode) = insertNode;
		else
			m_tailNode = insertNode;

		NodeTraits::NextNode(afterNode) = insertNode;
		m_count++;
		return insertNode;
	}

	Node* InsertAfter(Node* afterNode)
	{
		return InsertNodeAfter(afterNode, NodeTraits::CreateNode());
	}

	Node* InsertAfter(Node* afterNode, INARGTYPE element)
	{
		return InsertNodeAfter(afterNode, NodeTraits::CreateNode(element));
	}

	Node* InsertNodeAfterTail(Node* insertNode)
	{
		NodeTraits::PreviousNode(insertNode) = m_tailNode;
		NodeTraits::NextNode(insertNode) = NULL;

		if (m_tailNode)
			NodeTraits::NextNode(m_tailNode) = insertNode;
		else
			m_headNode = insertNode;

		m_tailNode = insertNode;
		m_count++;
		return insertNode;
	}

	Node* InsertAfterTail()
	{
		return InsertNodeAfterTail(NodeTraits::CreateNode());
	}

	Node* InsertAfterTail(INARGTYPE element)
	{
		return InsertNodeAfterTail(NodeTraits::CreateNode(element));
	}

	void RemoveHead()
	{
		Node* node = m_headNode;
		m_headNode = NodeTraits::NextNode(node);
		if (m_headNode)
			NodeTraits::PreviousNode(m_headNode) = NULL;
		else
			m_tailNode = NULL;
		NodeTraits::DestroyNode(node);
		m_count--;
	}

	void RemoveTail()
	{
		Node* node = m_tailNode;
		m_tailNode = NodeTraits::PreviousNode(node);
		if (m_tailNode)
			NodeTraits::NextNode(m_tailNode) = NULL;
		else
			m_headNode = NULL;
		NodeTraits::DestroyNode(node);
		m_count--;
	}

	void RemoveNode(Node* node, bool deleteNode = true)
	{
		if (node == m_headNode)
			m_headNode = NodeTraits::NextNode(node);
		else
			NodeTraits::NextNode(NodeTraits::PreviousNode(node)) = NodeTraits::NextNode(node);
		if( node == m_tailNode )
			m_tailNode = NodeTraits::PreviousNode(node);
		else
			NodeTraits::PreviousNode(NodeTraits::NextNode(node)) = NodeTraits::PreviousNode(node);
		if (deleteNode)
			NodeTraits::DestroyNode(node);
		m_count--;
	}

	void Empty()
	{
		Node* node = m_headNode;
		while (node)
		{
			Node* nextNode = NodeTraits::NextNode(node);
			NodeTraits::DestroyNode(node);
			node = nextNode;
		}

		m_count = 0;
		m_headNode = NULL;
		m_tailNode = NULL;
	}

	void Clear()
	{
		Empty();
		NodeTraits::FreeAll();
	}

	// Algorithm taken from http://www.chiark.greenend.org.uk/~sgtatham/algorithms/listsort.html by Simon Tatham.
	template <typename CompareT>
	void Sort(CompareT Compare)
	{
		Node *q, *e;

		if (m_headNode == NULL)
			return;

		int insize = 1;

		while (1)
		{
			Node* p = m_headNode;
			m_headNode = NULL;
			m_tailNode = NULL;

			int nmerges = 0;  /* count number of merges we do in this pass */

			while (p) {
				nmerges++;  /* there exists a merge to be done */
				/* step `insize' places along from p */
				q = p;
				int psize = 0;
				for (int i = 0; i < insize; i++)
				{
					psize++;
					q = NodeTraits::NextNode(q);
					if (!q)
						break;
				}

				/* if q hasn't fallen off end, we have two lists to merge */
				int qsize = insize;

				/* now we have two lists; merge them */
				while (psize > 0 || (qsize > 0 && q))
				{
					/* decide whether next element of merge comes from p or q */
					if (psize == 0)
					{
						/* p is empty; e must come from q. */
						e = q;
						q = NodeTraits::NextNode(q);
						qsize--;
					}
					else if (qsize == 0 || !q)
					{
						/* q is empty; e must come from p. */
						e = p;
						p = NodeTraits::NextNode(p);
						psize--;
					}
					else if (Compare(NodeTraits::Value(p), NodeTraits::Value(q)))
					{	// p < q
						/* First element of p is lower (or same);
						* e must come from p. */
						e = p;
						p = NodeTraits::NextNode(p);
						psize--;
					}
					else
					{
						/* First element of q is lower; e must come from q. */
						e = q;
						q = NodeTraits::NextNode(q);
						qsize--;
					}

					/* add the next element to the merged list */
					if (m_tailNode)
						NodeTraits::NextNode(m_tailNode) = e;
					else
						m_headNode = e;
					NodeTraits::PreviousNode(e) = m_tailNode;
					m_tailNode = e;
				}

				/* now p has stepped `insize' places along, and q has too */
				p = q;
			}

			NodeTraits::NextNode(m_tailNode) = NULL;

			/* If we have done only one merge, we're finished. */
			if (nmerges <= 1)   /* allow for nmerges==0, the empty list case */
			{
				return;
			}

			/* Otherwise repeat, merging lists twice the size */
			insize *= 2;
		}
	}


private:
	List( const List& );
	List& operator=( const List& );

	Node* m_headNode;
	Node* m_tailNode;
	size_t m_count;
};

} // namespace Misc

#endif // MISC_LIST_H
