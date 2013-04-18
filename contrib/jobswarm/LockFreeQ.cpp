#include "LockFreeQ.h"
#include "ThreadConfig.h"

//*** Note, even though the source file is called 'LockFreeQ' this implementation is not *ye* lock-free
//*** As long as your job size has any significant amount of 'work' to do, there is not real difference
//*** in performance.  I will be uploading a fully lock-free version relatively soon.

#if defined(__linux__)
// lock free not working on linux
#define USE_LOCK_FREE 0
#else
#define USE_LOCK_FREE 0 // set this to zero, to use locks
#endif

/*!
**
** Copyright (c) 2009 by John W. Ratcliff mailto:jratcliffscarab@gmail.com
**
** If you find this code useful or you are feeling particularily generous I would
** ask that you please go to http://www.amillionpixels.us and make a donation
** to Troy DeMolay.
**
** Skype ID: jratcliff63367
** Yahoo: jratcliff63367
** AOL: jratcliff1961
** email: jratcliffscarab@gmail.com
** Personal website: http://jratcliffscarab.blogspot.com
** Coding Website:   http://codesuppository.blogspot.com
** FundRaising Blog: http://amillionpixels.blogspot.com
** Fundraising site: http://www.amillionpixels.us
** New Temple Site:  http://newtemple.blogspot.com
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

namespace LOCK_FREE_Q
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

class MyLockFreeQ : public LockFreeQ
{
public:

    MyLockFreeQ(void)
    {
        mMutex = THREAD_CONFIG::tc_createThreadMutex();
        //
#if USE_LOCK_FREE
        NodeNull.pNext=0;
        //
        Head.pNode=Tail.pNode=&NodeNull;
        Head.count=Tail.count=0;
#else
        Head.pNode=Tail.pNode=0;
        Head.count=Tail.count=0;
#endif
    }

    ~MyLockFreeQ(void)
    {
        THREAD_CONFIG::tc_releaseThreadMutex(mMutex);
    }

#if USE_LOCK_FREE

    // [x] loop-spin
    // [ ] read write barrier
    // [ ] optimize functions (atomic operations (__*)

    ///////////////////////////////////////////////////////////////
    // enqueue
    ///////////////////////////////////////////////////////////////

    virtual void enqueue(node_t *item)
    {
        TCount   otail;
        //
        item->pNext=0;
        //
        for( ;; )
        {
            THREAD_CONFIG::tc_interlockedExchange( &otail , *(int64_t*)&Tail );

            // if tail next if 0 replace it with new item
            if( THREAD_CONFIG::tc_interlockedCompareExchange( &otail.pNode->pNext , *(int*)&item , 0 ) ) break;

            // else push tail back until it reaches end
            THREAD_CONFIG::tc_interlockedCompareExchange( &Tail , *(int*)&otail.pNode->pNext , otail.count+1 , *(int*)&otail.pNode , otail.count );
            //
            THREAD_CONFIG::tc_spinloop();
        }

        // try and change tail pointer (it is also fixed in Pop)
        THREAD_CONFIG::tc_interlockedCompareExchange( &Tail , *(int*)&item , otail.count+1 , *(int*)&otail.pNode , otail.count );
    }

    ///////////////////////////////////////////////////////////////
    // dequeue
    ///////////////////////////////////////////////////////////////

    virtual node_t *dequeue(void)
    {
        TCount   ohead;
        TCount   otail;
        //
        for( ;; )
        {
            THREAD_CONFIG::tc_interlockedExchange( &ohead , *(int64_t*)&Head );
            THREAD_CONFIG::tc_interlockedExchange( &otail , *(int64_t*)&Tail );
            //
            node_t      *next=ohead.pNode->pNext;

            // is queue empty
            if( ohead.pNode==otail.pNode )
            {
                // is it really empty
                if( !next ) return 0;

                // or is just tail falling behind...
                THREAD_CONFIG::tc_interlockedCompareExchange( &Tail , *(int*)&next , otail.count+1 , *(int*)&otail.pNode , otail.count );
            }
            else
            {
                if( THREAD_CONFIG::tc_interlockedCompareExchange( &Head , *(int*)&next , ohead.count+1 , *(int*)&ohead.pNode , ohead.count ) ) return next;
            }
            //
            THREAD_CONFIG::tc_spinloop();
        }
    }

#else
    virtual void enqueue(node_t *pNode)
    {
        mMutex->lock();
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
        mMutex->unlock();
    }

    virtual node_t * dequeue(void)
    {
        node_t *ret = 0;
        mMutex->lock();
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
        mMutex->unlock();
        return ret;
    }
#endif

    virtual node_t * getHead()
    {
        return Head.pNode;
    }



private:
    THREAD_CONFIG::ThreadMutex *mMutex;
    //
    TCount          Head;
    TCount          Tail;
};


LockFreeQ * createLockFreeQ(void)
{
    MyLockFreeQ *m = MEMALLOC_NEW(MyLockFreeQ);
    return static_cast< LockFreeQ *>(m);
}

void        releaseLockFreeQ(LockFreeQ *q)
{
    MyLockFreeQ *m = static_cast< MyLockFreeQ *>(q);
    MEMALLOC_DELETE(MyLockFreeQ,m);
}

};
