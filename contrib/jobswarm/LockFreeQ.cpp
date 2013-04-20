#include "LockFreeQ.h"
#include "ThreadConfig.h"

//*** Note, even though the source file is called 'LockFreeQ' this implementation is not *ye* lock-free
//*** As long as your job size has any significant amount of 'work' to do, there is not real difference
//*** in performance.  I will be uploading a fully lock-free version relatively soon.


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
        mMutex = SDL_CreateMutex();
        //
        Head.pNode=Tail.pNode=0;
        Head.count=Tail.count=0;
    }

    ~MyLockFreeQ(void)
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
