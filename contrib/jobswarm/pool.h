#ifndef POOL_H

#define POOL_H

#include "UserMemAlloc.h"

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



/** @file pool.h

 *  @brief A template class to handle high-speed iteration, allocation, and deallocation of fixed sized objects.
 *
 *  @author John W. Ratcliff
*/
#include <assert.h>
#include <string.h>

#ifdef MSVC_VER
#pragma warning(push)
#pragma warning(disable:4100)
#endif

class MPoolExtra
{
public:
    MPoolExtra(size_t mlen,const char *poolType,const char * /*file*/,int /*lineno*/)
    {
        mPoolType = poolType;
        mNext = 0;

        mData = (char *)MEMALLOC_MALLOC(sizeof(char)*mlen);

        memset(mData,0,mlen);
    }

    ~MPoolExtra(void)
    {
        MEMALLOC_FREE(mData);
    }

    MPoolExtra *mNext;  // the 'next' block allocated.
    char      *mData;
    const char *mPoolType;
};


template <class Type > class Pool
{
public:
    Pool(void)
    {
        mPoolType  = "GENERIC-POOL";
        mHead      = 0;
        mTail      = 0;
        mFree      = 0;
        mData      = 0;
        mCurrent   = 0;
        mFreeCount = 0;
        mUsedCount = 0;
        mMaxUsed   = 0;
        mMaxItems  = 0;
        mGrowCount = 0;
        mStartCount = 0;
        mCurrentCount = 0;
        mInitialized = false;
    };

    ~Pool(void)
    {
        Release();
    };


    void Release(void)
    {
        mHead = 0;
        mTail = 0;
        mFree = 0;

        // delete all of the memory blocks we allocated.
        MPoolExtra *extra = mData;
        while ( extra )
        {
            MPoolExtra *next = extra->mNext;
            MEMALLOC_DELETE(MPoolExtra,extra);
            extra = next;
        }

        mData = 0;
        mCurrent = 0;
        mFreeCount = 0;
        mUsedCount = 0;
        mMaxUsed = 0;
        mInitialized = false;
    };

    void Set(int startcount,int growcount,int maxitems,const char *poolType,const char *file,int lineno)
    {
        mPoolType = poolType;
        mFile     = file;
        mLineNo   = lineno;

        Release();

        mMaxItems = maxitems;
        mGrowCount = growcount;
        mStartCount = startcount;
        mCurrentCount = startcount;

        if ( mStartCount > 0 )
        {
            mData = MEMALLOC_NEW(MPoolExtra)(sizeof(Type)*mStartCount,mPoolType,mFile,mLineNo);
            Type *data = (Type *) mData->mData;
            {
                Type *t = (Type *)mData->mData;
                for (int i=0; i<mStartCount; i++)
                {
                    new ( t ) Type;
                    t++;
                }
            }
            mFree = data;
            mHead = 0;
            mTail = 0;
            int i;
            for (i=0; i<(startcount-1); i++)
            {
                data[i].SetNext( &data[i+1] );
                if ( i == 0 )
                    data[i].SetPrevious( 0 );
                else
                    data[i].SetPrevious( &data[i-1] );
            }

            data[i].SetNext(0);
            data[i].SetPrevious( &data[i-1] );
            mCurrent = 0;
            mFreeCount = startcount;
            mUsedCount = 0;
        }
        mInitialized = true;
    };


    Type * GetNext(bool &looped)
    {

        looped = false; // default value

        if ( !mHead ) return 0; //  there is no data to process.

        Type *ret;

        if ( !mCurrent )
        {
            ret = mHead;
            looped = true;
        }
        else
        {
            ret = mCurrent;
        }

        if ( ret ) mCurrent = ret->GetNext();


        return ret;
    };

    bool IsEmpty(void) const
    {
        if ( !mHead ) return true;
        return false;
    };

    int Begin(void)
    {
        mCurrent = mHead;
        return mUsedCount;
    };

	int GetUsedCount(void) const { return mUsedCount; };
	int GetFreeCount(void) const { return mFreeCount; };

    Type * GetNext(void)
    {
        if ( !mHead ) return 0; //  there is no data to process.

        Type *ret;

        if ( !mCurrent )
        {
            ret = mHead;
        }
        else
        {
            ret = mCurrent;
        }

        if ( ret ) mCurrent = ret->GetNext();


        return ret;
    };

    Type * Release(Type *t)
    {

        if ( t == mCurrent ) mCurrent = t->GetNext();

        if ( t == mTail )
        {
            mTail = t->GetPrevious(); // the new tail..
        }

        //  first patch old linked list.. his previous now points to his next
        Type *prev = t->GetPrevious();

        if ( prev )
        {
            Type *next = t->GetNext();
            prev->SetNext( next ); //  my previous now points to my next
            if ( next ) next->SetPrevious(prev);
            //  list is patched!
        }
        else
        {
            Type *next = t->GetNext();
            mHead = next;
            if ( mHead ) mHead->SetPrevious(0);
        }

        Type *temp = mFree; //  old head of free list.
        mFree = t; //  new head of linked list.
        t->SetPrevious(0);
        t->SetNext(temp);

        mUsedCount--;
        assert(mUsedCount >= 0);
        mFreeCount++;
        return mCurrent;
    };

    Type * GetFreeLink(void)
    {
        //  Free allocated items are always added to the head of the list
        if ( !mFree )
        {
            getMore();
        }

        Type *ret = mFree;

        if ( mFree )
        {
            mFree = ret->GetNext(); //  new head of free list
            Type *temp = mHead; //  current head of list
            if ( mHead == 0 )  // if it's the first item then this is the head of the list.
            {
                mTail = ret;
                mHead = ret;        //  new head of list is this free one
                if ( temp ) temp->SetPrevious(ret);
                mHead->SetNext(temp);
                mHead->SetPrevious(0);
            }
            else
            {
                assert( mTail );
                assert( mTail->GetNext() == 0 );
                mTail->SetNext( ret );
                ret->SetPrevious( mTail );
                ret->SetNext(0);
                mTail = ret;
            }
            mUsedCount++;
            if ( mUsedCount > mMaxUsed ) mMaxUsed = mUsedCount;
            mFreeCount--;
        }
        return ret;
    };

    Type * getMore(void) // ok, we need to see if we can grow some more.
    {
        Type *ret = mFree;
        if ( ret == 0 && (mCurrentCount+mGrowCount) < mMaxItems && mGrowCount > 0 ) // ok..we are allowed to allocate some more...
        {
            MPoolExtra *pe = mData; // the old one...
            mData  = MEMALLOC_NEW(MPoolExtra)(sizeof(Type)*mGrowCount,mPoolType,mFile,mLineNo);
            {
                Type *t = (Type *)mData->mData;
                for (int i=0; i<mGrowCount; i++)
                {
                    new ( t ) Type;
                    t++;
                }
            }
            mData->mNext = pe; // he points to the old one.
            // done..memory allocated and added to singly linked list.

            Type *data = (Type *) mData->mData;
            mFree = data;     // new head of free list.
            int i;
            for (i=0; i<(mGrowCount-1); i++)
            {
                data[i].SetNext( &data[i+1] );
                if ( i == 0 )
                    data[i].SetPrevious( 0 );
                else
                    data[i].SetPrevious( &data[i-1] );
            }
            data[i].SetNext(0);
            data[i].SetPrevious( &data[i-1] );

            mFreeCount+=mGrowCount; // how many new free entries we have added...
            mCurrentCount+=mGrowCount;
        }
        return ret;
    }

  bool isInitialized(void) const { return mInitialized; };

//private:
    bool         mInitialized;
    int        mMaxItems;
    int        mGrowCount;
    int        mStartCount;
    int        mCurrentCount; // this is total allocated, not free/used

    Type       *mCurrent;
    MPoolExtra *mData;
    Type        *mHead;
    Type        *mTail;
    Type        *mFree;
    int        mUsedCount;
    int        mFreeCount;
    int        mMaxUsed;
    const char *mPoolType;
    const char *mFile;
    int         mLineNo;
};

#ifdef MSVC_VER
#pragma warning(pop)
#endif

#endif
