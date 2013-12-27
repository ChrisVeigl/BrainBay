/* SkinStyle - Win32 Skinning Library 
 * Author: John Roark <jroark@cs.usfca.edu>
 */

/*
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef _DYNARRAY_H_
#define _DYNARRAY_H_

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#pragma once

template< class TYPE >
class CDynArray
{
protected:
	struct CNode
	{
		CNode*	pNext;
		CNode*	pPrev;
		TYPE*	data;
	};

private:
	DWORD m_dwSize;
	CNode* m_pNodeHead;
	CNode* m_pNodeTail;
    	
public:
	CDynArray()
	{ 
		m_dwSize = 0; 
		m_pNodeTail = m_pNodeHead = NULL;
	}
	
	~CDynArray() { RemoveAll(); }

	void RemoveAll()
	{
		if(m_pNodeHead != NULL)
		{
			CNode *pTmp;
			for(DWORD i = 0; i < m_dwSize; i++) {
				delete m_pNodeHead->data;
				pTmp = m_pNodeHead;
				m_pNodeHead = m_pNodeHead->pNext;
				delete pTmp;
			}
			m_dwSize = 0; 
			m_pNodeTail = m_pNodeHead = NULL;
		}
	}
	
	TYPE *operator [](DWORD nIndex)
	{
		if((NULL != m_pNodeHead) && (nIndex < m_dwSize))
		{
			CNode *pTmp = m_pNodeHead;

			for(DWORD i = 0; i < nIndex; i++)
				pTmp = pTmp->pNext;

			return pTmp->data;
		}

		return NULL;
	}

	DWORD GetSize() { return m_dwSize; }

	bool Add(TYPE *Item)
	{
		if(m_pNodeHead == NULL)
		{
			m_pNodeHead = new CNode;
			m_pNodeTail = m_pNodeHead;
			m_pNodeHead->data = Item;
			m_pNodeHead->pPrev = NULL;
			m_pNodeHead->pNext = NULL;
		}
		else
		{
			CNode *pTmp;
			m_pNodeTail->pNext = new CNode;
			pTmp = m_pNodeTail;
			m_pNodeTail = m_pNodeTail->pNext;
			m_pNodeTail->data = Item;
			m_pNodeTail->pPrev = pTmp;
			m_pNodeTail->pNext = NULL;
		}

		m_dwSize++;
		return true;
	}

	bool RemoveAt(DWORD nIndex)
	{
		if((NULL != m_pNodeHead) && (nIndex < m_dwSize))
		{
			if(nIndex == 0)					// if remove the head
			{
				CNode *pTmp = m_pNodeHead;
				m_pNodeHead = m_pNodeHead->pNext;

				delete pTmp->data;
				delete pTmp;

				m_pNodeHead->pPrev = NULL;
			}
			else if(nIndex == (m_dwSize - 1))// else the tail
			{
				CNode *pTmp = m_pNodeTail;
				m_pNodeTail = m_pNodeTail->pPrev;

				delete pTmp->data;
				delete pTmp;

				m_pNodeTail->pNext = NULL;
			}
			else							// or any other item
			{
				CNode *pTmp = m_pNodeHead;

				for(DWORD i = 0; i < nIndex; i++)
					pTmp = pTmp->pNext;

				pTmp->pPrev->pNext = pTmp->pNext;
				pTmp->pNext->pPrev = pTmp->pPrev;
				
				delete pTmp->data;
				delete pTmp;
			}
			m_dwSize--;
			return true;
		}
		return false;
	}
};

#endif // _DYNARRAY_H_
