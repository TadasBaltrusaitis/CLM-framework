/** \file
  * ATL Module Compatability wrapper.
  */
/*
 * Copyright © 2000, 2001 Paul Hollingsworth, Michael Geddes
 * Copyright © 2013 Alexander Lamaison
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

#ifndef COMET_ATL_MODULE_H
#define COMET_ATL_MODULE_H

#include <comet/server.h>

#include <atlbase.h>

namespace comet {
    /*! \defgroup ATL ATL conversion.
     */
    //@{

    /** \struct declspec_comtype atl_module.h comet/atl_module.h
      * Helper to create a 'comtype' for a non-comet interface.
      * \code
      * namespace comet{
      * template<> struct comtype<IApplicationOption> :
      *    declspec_comtype<IApplicationOption, ::IDispatch >
      * { };
      * };
      * \endcode
      *
      */
    template<typename ITF, typename BASE = ::IUnknown> struct declspec_comtype
    {
        static const uuid_t& uuid() { return uuid_t::create_const_reference(__uuidof(ITF)); }
        typedef BASE base;
    };


    /// Placeholder for an empty comet typelibrary.
    struct empty_typelib
    {
        typedef nil coclasses;
    };

    template<typename COM_SERVER, typename ATL_MODULE = ATL::CComModule>
        class atl_module_ex : public ATL_MODULE
    {
        // ATL will take the responsibility of registering the embedded type library.
        public:
        HRESULT Init(_ATL_OBJMAP_ENTRY* p, HINSTANCE h, const GUID* plibid = NULL)
        {
            module().instance(h);
            return ATL_MODULE::Init(p, h, plibid);
        }

        HRESULT RegisterServer(BOOL bRegTypeLib = FALSE, const CLSID* pCLSID = NULL)
        {
            HRESULT hr = ATL_MODULE::RegisterServer(bRegTypeLib, pCLSID);
            if(SUCCEEDED(hr))
                hr = COM_SERVER::DllRegisterServer();
            return hr;
        }

        HRESULT UnregisterServer(BOOL bUnRegTypeLib, const CLSID* pCLSID = NULL)
        {
            COM_SERVER::DllUnregisterServer();
            return ATL_MODULE::UnregisterServer(bUnRegTypeLib, pCLSID);
        }

        HRESULT GetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
        {
            HRESULT hr = COM_SERVER::DllGetClassObject( rclsid, riid, ppv);
            if( hr == CLASS_E_CLASSNOTAVAILABLE )
            {
                hr = ATL_MODULE::GetClassObject(rclsid, riid,ppv);
            }
            return hr;
        }

        LONG GetLockCount()
        {
            return module().rc() + ATL_MODULE::GetLockCount();
        }
    };

    /** Wraps an ATL::CComModule to provide co-existing ATL/Comet co-classes.
      * ATL will take responsibility for registering the embedded type-library.
      *
      * Here is an example of how to use it:
      * \code
      *     struct typelib_subset
      *     {
      *         typedef COMET_STRICT_TYPENAME comet::typelist::make_list< CoClass1, CoClass2 > coclasses;
      *     };
      *     comet::atl_module<typelib_subset> _Module;
      * \endcode
      * And in std.h:
      * \code
      *     struct typelib_subset;
      *     extern comet::atl_module<typelib_subset> _Module;
      *  \endcode
      */
    template<typename TYPELIB = empty_typelib, typename ATL_MODULE = ATL::CComModule>
        class atl_module : public atl_module_ex< com_server<TYPELIB, com_server_traits<NO_EMBEDDED_TLB> >, ATL_MODULE >
    {
    };
    //@}
} // namespace comet

#endif
