#include "ThreadSafeQueue.h"
#include "ThreadConfig.h"

//*** Note, even though the source file is called 'ThreadSafeQueue' this implementation is not *ye* lock-free
//*** As long as your job size has any significant amount of 'work' to do, there is not real difference
//*** in performance.  I will be uploading a fully lock-free version relatively soon.


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

#ifdef MSVC
#pragma warning(disable:4239)
#pragma warning(disable:4127)
#endif

node_t                                    NodeNull;

struct TCount {
    node_t                                 *pNode;
    unsigned int                           count;
};

class MyThreadSafeQueue : public ThreadSafeQueue
{
public:

    MyThreadSafeQueue(void)
    {
        mMutex = SDL_CreateMutex();
        //
        Head.pNode=Tail.pNode=0;
        Head.count=Tail.count=0;
    }

    ~MyThreadSafeQueue(void)
    {
        SDL_DestroyMutex(mMutex);
    }

    virtual void enqueue(node_t *pNode)
    {
        SDL_mutexP(mMutex);
        //
        pNode->pNext = 0;
        //
        if ( Head.pNode == 0 )
        {
            Head.pNode = pNode;
            Tail.pNode = pNode;
        }
        else
        {
            Tail.pNode->pNext = pNode;
            Tail.pNode = pNode;
        }
       SDL_mutexV(mMutex);
    }

    virtual node_t * dequeue(void)
    {
        node_t *ret = 0;
        SDL_mutexP(mMutex);
        //
        if ( Head.pNode )
        {
            ret = Head.pNode;
            Head.pNode = ret->pNext;
            if ( Head.pNode == 0 )
            {
                Tail.pNode = 0;
            }
        }
        SDL_mutexV(mMutex);
        return ret;
    }

    virtual node_t * getHead()
    {
        return Head.pNode;
    }

private:
    SDL_mutex *mMutex;
    //
    TCount          Head;
    TCount          Tail;
};


ThreadSafeQueue * createThreadSafeQueue(void)
{
    MyThreadSafeQueue *m = MEMALLOC_NEW(MyThreadSafeQueue);
    return static_cast< ThreadSafeQueue *>(m);
}

void releaseThreadSafeQueue(ThreadSafeQueue *q)
{
    MyThreadSafeQueue *m = static_cast< MyThreadSafeQueue *>(q);
    MEMALLOC_DELETE(MyThreadSafeQueue,m);
}

};
