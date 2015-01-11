/** \file
  * Threading support.
  */
/*
 * Copyright © 2001-2002 Sofus Mortensen, Mikael Lindgren
 *
 * This material is provided "as is", with absolutely no warranty
 * expressed or implied. Any use is at your own risk. Permission to
 * use or copy this software for any purpose is hereby granted without
 * fee, provided the above notices are retained on all copies.
 * Permission to modify the code and to distribute modified code is
 * granted, provided the above notices are retained, and a notice that
 * the code was modified is included with the above copyright notice.
 *
 * This header is part of Comet version 2.
 * https://github.com/alamaison/comet
 */

#ifndef COMET_THREADING_H
#define COMET_THREADING_H

#include <comet/config.h>
#include <comet/handle.h>

#include <comet/error.h>

namespace comet {

/** \class critical_section  threading.h comet/threading.h
  *  wrapper for Win32 CRITICAL_SECTION.
  */
class critical_section {
public:
    critical_section()  {
        ::InitializeCriticalSection(&cs_);
    }

    ~critical_section() {
        ::DeleteCriticalSection(&cs_);
    }

    void enter() const {
        ::EnterCriticalSection(&cs_);
    }

    void leave() const {
        ::LeaveCriticalSection(&cs_);
    }

private:
    critical_section(const critical_section&);
    critical_section& operator=(const critical_section&);

    mutable CRITICAL_SECTION cs_;
};

/** \class auto_cs  threading.h comet/threading.h
  * Stack based critical-section resource obtaining and releasing.
  */
class auto_cs {
public:
    explicit auto_cs(const critical_section& cs) : cs_(cs) {
        cs_.enter();
    }

    ~auto_cs() {
        cs_.leave();
    }

private:
    auto_cs& operator=(const auto_cs&);
    auto_cs(const auto_cs&);

    const critical_section& cs_;
};

/** \class locking_ptr  threading.h comet/threading.h
    Locking pointer.
     Based on an article on the volatile keyword by Andrei Alexandrescu
     See <a href=:http://www.cuj.com/experts/1902/alexandr.htm?topic=experts"> article</a>.
  */
template <typename T> class locking_ptr {
public:
    locking_ptr(volatile T& obj, critical_section& cs)    :
      pointer_(const_cast<T*>(&obj)),
      cs_(cs)
    {
          cs_.enter();
    }

    ~locking_ptr()
    {
        cs_.leave();
    }

    T& operator*()
    {
        return *pointer_;
    }

    T* operator->()
    {
        return pointer_;
    }

private:
    T* pointer_;
    critical_section& cs_;

    locking_ptr(const locking_ptr&);
    locking_ptr& operator=(const locking_ptr&);
};

class thread
{
private:
    DWORD id_;
    auto_handle    handle_;

    // declare non-copyable
    thread(thread const& );
    thread& operator=(thread const& );
public:
    thread() : id_(0)
    {}

    const auto_handle& start()
    {
        handle_ = auto_attach(::CreateThread(0, 0, &thread::start_proc, this, 0, &id_));
        return handle_;
    }

    const auto_handle& handle() const
    {
        return handle_;
    }

    void exit(DWORD exit_code)
    {
        ::ExitThread(exit_code);
    }

    void suspend()
    {
        ::SuspendThread(handle_);
    }

    void resume()
    {
        ::ResumeThread(handle_);
    }

    bool running()
    {
        DWORD code;
        return handle_ &&
                    ::GetExitCodeThread(handle_, &code) &&
                    code == STILL_ACTIVE;
    }

    bool wait(DWORD timeout = INFINITE)
    {
        return ::WaitForSingleObject(handle_, timeout) == WAIT_OBJECT_0;
    }

private:
    static DWORD _stdcall start_proc(void* param)
    {
        return static_cast<thread*>(param)->thread_main();
    }

    virtual DWORD thread_main() = 0;
};

class event
{
private:
    auto_handle    handle_;

    // declare non-copyable
    event(event const& );
    event& operator=(event const& );
public:
    explicit event(bool initial_state = false)
    {
        handle_ = auto_attach(::CreateEvent(0, false, initial_state, 0));
    }

    explicit event(const TCHAR* name, bool initial_state = false)
    {
        handle_ = auto_attach(::CreateEvent(0, false, initial_state, name));
    }

    void set()
    {
        if (0 == ::SetEvent(handle_))
        {
            DWORD err = GetLastError();
            raise_exception(HRESULT_FROM_WIN32(err));
        }
    }

    void reset()
    {
        if (0==::ResetEvent(handle_) )
        {
            DWORD err = GetLastError();
            raise_exception(HRESULT_FROM_WIN32(err));
        }
    }

    bool wait(DWORD timeout = INFINITE)
    {
        return ::WaitForSingleObject(handle_, timeout) == WAIT_OBJECT_0;
    }

    bool is_set()
    {
        return ::WaitForSingleObject(handle_, 0) == WAIT_OBJECT_0;
    }

    operator HANDLE() const
    { return handle_; }
};

}

#endif
