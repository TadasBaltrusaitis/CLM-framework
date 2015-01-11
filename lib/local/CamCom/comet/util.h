/** \file
  * Various utilities.
  */
/*
 * Copyright © 2000, 2001 Sofus Mortensen
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

#ifndef COMET_UTIL_H
#define COMET_UTIL_H

#include <comet/config.h>

#include <stdexcept>

#include <comet/error.h>
#include <comet/server.h>

namespace comet {

    /*! \addtogroup WinUtil
     */
    //@{
    //! Automatic handling of CoInitialize / CoUninitialize.
    class auto_coinit {
    public:
#if (_WIN32_WINNT >= 0x0400 ) || defined(_WIN32_DCOM) // DCOM
        explicit auto_coinit(COINIT ci) {
            ::CoInitializeEx(0, ci) | raise_exception;
        }
#endif

        auto_coinit() { ::CoInitialize(0) | raise_exception;     }

        ~auto_coinit() { ::CoUninitialize(); }

    private:
        auto_coinit& operator=(const auto_coinit&);
        auto_coinit(const auto_coinit&);
    };

    // For backward compatibility.
    typedef auto_coinit auto_CoInitialize;

    /// Returns the class id of the free threaded marshaler
    inline CLSID get_clsid_of_ftm()
    {
        CLSID rv;
        class Dummy : public static_object<> {} dummy;

        IUnknown* pUnk = 0;
        ::CoCreateFreeThreadedMarshaler(&dummy, &pUnk) | raise_exception;

        com_ptr<IMarshal> marshal = try_cast(pUnk);

        marshal.raw()->GetUnmarshalClass(IID_IUnknown, &dummy, MSHCTX_INPROC,0,MSHLFLAGS_NORMAL, &rv) | raise_exception;
        return rv;
    }

    /**
     * Returns true if and only if object is aggregating the free threaded marshaler.
     */
    inline bool is_object_aggregating_ftm(const com_ptr<IUnknown>& p)
    {
        com_ptr<IMarshal> marshal = com_cast(p);
        if (marshal == 0) return false;

        class Dummy : public static_object<> {} dummy;

        CLSID clsid = CLSID_NULL;
        marshal.raw()->GetUnmarshalClass(IID_IUnknown, &dummy, MSHCTX_INPROC,0,MSHLFLAGS_NORMAL, &clsid);

        CLSID ftm_clsid = get_clsid_of_ftm();
        return clsid == ftm_clsid ? true : false;
    }

#if (_WIN32_WINNT >= 0x0400 ) || defined(_WIN32_DCOM) // DCOM
    /**
     * Returns true if and only if current apartment is the MTA.
     */
    inline bool is_mta()
    {
        if (SUCCEEDED(::CoInitializeEx(0, COINIT_MULTITHREADED)))
        {
            ::CoUninitialize();
            return true;
        }
        return false;
    }
#endif
    /**
     * Returns true if and only if current apartment is an STA.
     */
    inline bool is_sta()
    {
        if (SUCCEEDED(::CoInitialize(0)))
        {
            ::CoUninitialize();
            return true;
        }
        return false;
    }

    /**
     * Returns true if and only if specified ptr is a standard proxy.
     */
    inline bool is_std_proxy(const com_ptr<IUnknown>& unk)
    {
        com_ptr<IUnknown> pm;
        return SUCCEEDED(unk.raw()->QueryInterface(IID_IProxyManager, reinterpret_cast<void**>(pm.out())));
    }

} // namespace

#endif
