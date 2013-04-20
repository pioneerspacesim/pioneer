#ifndef LOCK_FREE_Q_H
#define LOCK_FREE_Q_H

#include <SDL.h>

#include "UserMemAlloc.h" // use this to optionally trap all memory allocations

/*!
**
** Copyright (c) 20011 by John W. Ratcliff mailto:jratcliffscarab@gmail.com
**
**
** The MIT license:
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is furnished
** to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all
** copies or substantial portions of the Software.

** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
** WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
** CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

namespace THREAD_SAFE_QUEUE
{

// reader-writer circular queue.. one thread writes, one thread reads only...
template<class T> class CQueue
{
private:
    volatile int m_Read;
    volatile int m_Write;
    volatile T *m_Data;
    int         m_Size;

public:
    CQueue()
    {
        m_Read = 0;
        m_Write = 0;
        m_Data = 0;
        m_Size = 0;
    }

    ~CQueue(void)
    {
        MEMALLOC_DELETE_ARRAY(T,m_Data);
    }

    void init(int size)
    {
        MEMALLOC_DELETE_ARRAY(T,m_Data);
        m_Read = 0;
        m_Write = 0;
        m_Size = size;
        m_Data = MEMALLOC_NEW_ARRAY(T,size)[size];
    }

    //push a new element in the circular queue
    bool push(T &Element)
    {
        int nextElement = (m_Write + 1) % m_Size;
        if(nextElement != m_Read)
        {
            m_Data[m_Write] = Element;
            m_Write = nextElement;
            return true;
        }
        else
            return false;
    }

    //remove the next element from the circualr queue
    bool pop(T &Element)
    {
        if(m_Read == m_Write)
            return false;

        int nextElement = (m_Read + 1) % m_Size;

        Element = m_Data[m_Read];
        m_Read = nextElement;
        return true;
    }

    bool isFull(void) const
    {
        bool ret = false;

        int nextElement = (m_Write + 1) % m_Size;

        if(nextElement == m_Read)
        {
            ret = true;
        }

        return ret;
    }

};

class node_t
{
public:
    node_t *pNext;
};

  class ThreadSafeQueue
{
public:
    virtual node_t * getHead() = 0;
    virtual void enqueue(node_t *pNode) = 0;
    virtual node_t * dequeue(void) = 0;

};



  ThreadSafeQueue * createThreadSafeQueue(void);
  void releaseThreadSafeQueue(ThreadSafeQueue *q);

};

#endif
