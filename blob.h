/*
  Revisions:

  2004-06-30 carlc
    - oops - default GetNewSize() doesnt work when current size is 0.
    - oops - all GetNewSize() calls were supposed to be * sizeof(Tel)

  2004-07-09 carlc
    - fixed realloc bug... it was supposed to return true if no allocation needed to happen
*/

#pragma once

#include <windows.h>


class default_blob_traits
{
public:
  // return the new size, in elements
  static long GetNewSize(long current_size, long requested_size)
  {
    if(!current_size)
    {
      return requested_size;
    }

    // same as (current_size * 1.5)
    while(current_size < requested_size)
    {
      current_size += (current_size >> 1);
    }
    return current_size;
  }
};

// manages a simple memory blob.
// an unallocated state will have m_p = 0 and m_size = 0
// traits is like blob_traits
//
// if the class is "not lockable" that means in exchange for removing all the "lockable" checks (for performance), this class
// will allow direct access to the buffer via GetLockedBuffer()).
template<typename Tel, bool TLockable = true, bool TStaticBufferSupport = true, typename Ttraits = default_blob_traits, long TStaticBufferSize = 4096>
class Blob
{
public:

  typedef Tel _El;
  typedef Ttraits _Traits;

  Blob() :
    m_p(TStaticBufferSupport ? m_StaticBuffer : 0),
    m_size(TStaticBufferSupport ? TStaticBufferSize : 0),
    m_locked(false),
    m_hHeap(GetProcessHeap())
  {
  }

  ~Blob()
  {
    Free();
  }

  long size() const
  {
    return m_size;
  }

  // these are just to break up some if() statements.
  inline bool CurrentlyUsingStaticBuffer() const
  {
    // is static buffer support enabled, and are we using the static buffer right now?
    return TStaticBufferSupport && (m_p == m_StaticBuffer);
  }
  inline bool CompletelyUnallocated() const
  {
    // is static buffer support disabled, and do we have NOTHING allocated now?
    return (!TStaticBufferSupport) && (m_p == 0);
  }
  inline bool CurrentlyLocked() const
  {
    return TLockable && m_locked;
  }
  inline bool LockableAndUnlocked() const
  {
    return TLockable && (!m_locked);
  }

  // frees memory if its not locked.
  bool Free()
  {
    bool r = false;
    // why do i check both TLockable and m_locked?  Because if the compiler is smart enough,
    // it will totally eliminate the comparison if TLockable is always false (in the template param).
    // msvc 7.1 has been verified to successfully remove the compares.
    if(!CurrentlyLocked())
    {
      if(TStaticBufferSupport)
      {
        if(m_StaticBuffer != m_p)
        {
          HeapFree(m_hHeap, 0, m_p);
          m_p = m_StaticBuffer;
          m_size = TStaticBufferSize;
        }
      }
      else
      {
        if(m_p)
        {
          HeapFree(m_hHeap, 0, m_p);
          m_p = 0;
          m_size = 0;
        }
      }
      r = true;
    }
    return r;
  }

  bool Alloc(long n)
  {
    bool r = false;
    if(Free())
    {
      if(CurrentlyUsingStaticBuffer())
      {
        if(m_size < n)
        {
          // we need to allocate on the heap.
          Tel* pNew;
          long nNewSize = Ttraits::GetNewSize(0, n);
          pNew = static_cast<Tel*>(HeapAlloc(m_hHeap, 0, sizeof(Tel) * nNewSize));
          if(pNew)
          {
            m_p = pNew;
            m_size = nNewSize;
            r = true;
          }
        }
      }
      else
      {
        // we need to allocate on the heap.
        Tel* pNew;
        long nNewSize = Ttraits::GetNewSize(0, n);
        pNew = static_cast<Tel*>(HeapAlloc(m_hHeap, 0, sizeof(Tel) * nNewSize));
        if(pNew)
        {
          m_p = pNew;
          m_size = nNewSize;
          r = true;
        }
      }
    }
    return r;
  }

  bool Realloc(long n)
  {
    bool r = false;
    if(!CurrentlyLocked())
    {
      if(m_size >= n)
      {
        // no need to allocate;
        r = true;
      }
      else
      {
        // we definitely need to allocate now.
        long nNewSize = Ttraits::GetNewSize(m_size, n);
        Tel* pNew;

        if(CurrentlyUsingStaticBuffer() || CompletelyUnallocated())
        {
          // allocate for the first time.
          pNew = static_cast<Tel*>(HeapAlloc(m_hHeap, 0, sizeof(Tel) * nNewSize));
          if(pNew)
          {
            if(CurrentlyUsingStaticBuffer())
            {
              // copy the contents of the static buffer into the new heap memory.
              CopyMemory(pNew, m_p, TStaticBufferSize);
            }

            m_p = pNew;
            m_size = nNewSize;

            r = true;
          }
        }
        else
        {
          // realloc, because we already have a heap buffer.
          pNew = static_cast<Tel*>(HeapReAlloc(m_hHeap, 0, m_p, sizeof(Tel) * nNewSize));
          if(pNew)
          {
            m_p = pNew;
            m_size = nNewSize;
            r = true;
          }
        }
      }
    }
    return r;
  }

  // locks the buffer so it can be written to.  this will allow GetBuffer(void) to be used.
  bool Lock()
  {
    bool r = false;
    // if its not lockable or the size is 0, you cant lock!
    if(TLockable || m_size)
    {
        m_locked = true;
        r = true;
    }
    return r;
  }
  bool Unlock()
  {
    if(TLockable) m_locked = false;// once again this is necessary for optimization.
    return true;
  }

  // returns a const buffer
  const Tel* GetBuffer() const
  {
    return m_p;
  }

  // returns 0 if its not allowed (buffer must be locked!)
  Tel* GetLockedBuffer() const
  {
    Tel* r = 0;
    if((!TLockable) || (CurrentlyLocked()))// look at Lock() to see why i have this.
    {
      r = m_p;
    }
    return r;
  }

  // combination of Alloc(), Lock() and GetLockedBuffer(void)
  // call Unlock() after this!
  Tel* GetBuffer(long n)
  {
    Tel* r = 0;
    if(LockableAndUnlocked())
    {
      if(Alloc(n))
      {
        if(Lock())
        {
          r = m_p;
        }
      }
    }
    return r;
  }

  //bool Move()
  //{
  //}
  //bool Copy()
  //{
  //}
  //bool Compare(const This_T& r)
  //{
  //}
  //bool Assign(const This_T& r)
  //{
  //}
  //bool Assign(const Tel* p, long len)
  //{
  //}

private:
  long m_size;
  Tel* m_p;

  bool m_locked;

  Tel m_StaticBuffer[TStaticBufferSize];

  HANDLE m_hHeap;
};

