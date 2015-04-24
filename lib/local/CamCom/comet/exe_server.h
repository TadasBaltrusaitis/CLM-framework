/** \file
  * exe-server classes.
  */
/*
 * Copyright © 2001, 2002 Mikael Lindgren, Sofus Mortensen
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

#ifndef COMET_EXE_SERVER_H
#define COMET_EXE_SERVER_H

#include <comet/server.h>
#include <comet/static_assert.h>

namespace comet {

    namespace impl {
        template <typename CLS_LIST> struct register_class_entry
        {
            typedef COMET_STRICT_TYPENAME CLS_LIST::head  CLASS;
            typedef COMET_STRICT_TYPENAME CLS_LIST::tail  NEXT;

            template <typename CLS> struct register_info
            {
                DWORD id;
            };

            static register_info<CLASS>& get_register_info()
            {
                static register_info<CLASS> info;
                return info;
            }
            COMET_FORCEINLINE static HRESULT register_class_object(DWORD context, DWORD flags)
            {
                const IID& clsid = comtype<CLASS>::uuid();
                ::IUnknown* p = impl::coclass_table_entry<CLASS, false>::factory::get(clsid);
                if (p)
                {
                    HRESULT hr = ::CoRegisterClassObject(clsid, p, context, flags, &get_register_info().id);
                    p->Release();
                    if (hr != S_OK)
                        return hr;
                }
                return register_class_entry<NEXT>::register_class_object(context, flags);
            }

            COMET_FORCEINLINE static void revoke_class_object()
            {
                ::CoRevokeClassObject(get_register_info().id);
                register_class_entry<NEXT>::revoke_class_object();
            }
        };

        template <> struct register_class_entry<nil>
        {
            COMET_FORCEINLINE static HRESULT register_class_object(DWORD context, DWORD flags)
            {
                COMET_NOTUSED(context);
                COMET_NOTUSED(flags);
                return S_OK;
            }

            COMET_FORCEINLINE static void revoke_class_object()
            {
            }
        };
    };

    /*! \addtogroup Server
     */
    //@{

    //! Define an EXE server
    template<typename TYPELIB, bool FREE_THREADED = false, typename TRAITS = com_server_traits<0> > class exe_server : private thread
    {
#if  !(_WIN32_WINNT >= 0x0400 ) && !defined(_WIN32_DCOM)
        // Implementing a free threaded exe server requires NT 4.0 or better.
        COMET_STATIC_ASSERT(FREE_THREADED == false);
#endif

        typedef coclass_table<typename TYPELIB::coclasses, false> COCLASS_TABLE;

        enum { terminate_pause = 1000, idle_shutdown_time = 5000 };

    public:
        exe_server(HINSTANCE instance);
        exe_server(HINSTANCE instance, const GUID& appid, const tstring& appid_descr);
        ~exe_server();

        HRESULT run();

        HRESULT register_server();
        HRESULT unregister_server();
        HRESULT register_class_objects(DWORD context, DWORD flags);
        void revoke_class_objects();

    private:
        event shutdown_event_;
        DWORD main_thread_id_;
        const GUID* appid_;
        tstring appid_descr_;

        virtual DWORD thread_main();
    };
    //@}

    template<typename TYPELIB, bool FREE_THREADED, typename TRAITS>
    exe_server<TYPELIB, FREE_THREADED, TRAITS>::exe_server(HINSTANCE instance):
        main_thread_id_(::GetCurrentThreadId()),
        appid_(0)
    {
        module_t& m = module();

        m.instance(instance);
        m.set_shutdown_event(shutdown_event_);

        // initialize static variables in factory::get to avoid potential thread safety problem.
        ::IUnknown* cf = COCLASS_TABLE::find(IID_NULL);
    }

    template<typename TYPELIB, bool FREE_THREADED, typename TRAITS>
    exe_server<TYPELIB, FREE_THREADED, TRAITS>::exe_server(HINSTANCE instance, const GUID& appid, const tstring& appid_descr):
        main_thread_id_(::GetCurrentThreadId()),
        appid_(&appid), appid_descr_(appid_descr)
    {
        module_t& m = module();

        m.instance(instance);
        m.set_shutdown_event(shutdown_event_);

        // initialize static variables in factory::get to avoid potential thread safety problem.
        ::IUnknown* cf = COCLASS_TABLE::find(IID_NULL);
    }

    template<typename TYPELIB, bool FREE_THREADED, typename TRAITS>
    exe_server<TYPELIB, FREE_THREADED, TRAITS>::~exe_server()
    {
        module().clear_shutdown_event();
    }

    template<typename TYPELIB, bool FREE_THREADED, typename TRAITS>
    HRESULT exe_server<TYPELIB, FREE_THREADED, TRAITS>::run()
    {
        thread::start();

        HRESULT hr;
#if  (_WIN32_WINNT >= 0x0400 ) || defined(_WIN32_DCOM) // DCOM
        if (FREE_THREADED)
        {
            hr = register_class_objects(CLSCTX_LOCAL_SERVER, REGCLS_MULTIPLEUSE|REGCLS_SUSPENDED);
            if (SUCCEEDED(hr))
                hr = ::CoResumeClassObjects();
        }
        else
#endif
            hr = register_class_objects(CLSCTX_LOCAL_SERVER, REGCLS_MULTIPLEUSE);

        if (SUCCEEDED(hr))
        {
            MSG msg;
            while (::GetMessage(&msg, 0, 0, 0))
                ::DispatchMessage(&msg);
        }
        else if (thread::running())
        {
            shutdown_event_.set();
            thread::wait(0);
        }

        revoke_class_objects();
        Sleep(terminate_pause); //wait for any threads to finish

        module().shutdown();

        return hr;
    }

    template<typename TYPELIB, bool FREE_THREADED, typename TRAITS>
    HRESULT exe_server<TYPELIB, FREE_THREADED, TRAITS>::register_server()
    {
        TCHAR filename[MAX_PATH];

        GetModuleFileName(module().instance(), filename, MAX_PATH);

        {
            HRESULT hr = impl::typelibrary_registration<TRAITS::embedded_tlb>::perform(filename, false);
            if(FAILED(hr)) return SELFREG_E_TYPELIB;
        }

        try {
            if (appid_)
            {
                tstring key(_T("AppID\\{") + uuid_t(*appid_).str() + _T("}"));

                regkey rkey(HKEY_CLASSES_ROOT);
                rkey.create(key)[_T("")] = appid_descr_;
            }
            COCLASS_TABLE::registration(filename, false, false, appid_);
        }
        catch (const com_error &e)
        {
            unregister_server();
            return impl::return_com_error(e);
        }
        catch (const std::exception &e)
        {
            unregister_server();
            ::OutputDebugStringA(e.what());
            return E_FAIL;
        }

        return S_OK;
    }

    template<typename TYPELIB, bool FREE_THREADED, typename TRAITS>
    HRESULT exe_server<TYPELIB, FREE_THREADED, TRAITS>::unregister_server()
    {
        TCHAR filename[MAX_PATH];
        GetModuleFileName(module().instance(), filename, MAX_PATH);

        impl::typelibrary_registration<TRAITS::embedded_tlb>::perform(filename, true);

        if (appid_)
        {
            tstring key(_T("AppID\\{") + uuid_t(*appid_).str() + _T("}"));

            regkey rkey(HKEY_CLASSES_ROOT);
            rkey.delete_subkey_nothrow(key);
        }
        COCLASS_TABLE::registration(filename, true, false, appid_);
        return S_OK;
    }

    template<typename TYPELIB, bool FREE_THREADED, typename TRAITS>
    HRESULT exe_server<TYPELIB, FREE_THREADED, TRAITS>::register_class_objects(DWORD context, DWORD flags)
    {
        return impl::register_class_entry<TYPELIB::coclasses>::register_class_object(context, flags);
    }

    template<typename TYPELIB, bool FREE_THREADED, typename TRAITS>
    void exe_server<TYPELIB, FREE_THREADED, TRAITS>::revoke_class_objects()
    {
        impl::register_class_entry<TYPELIB::coclasses>::revoke_class_object();
    }

    template<typename TYPELIB, bool FREE_THREADED, typename TRAITS>
    DWORD exe_server<TYPELIB, FREE_THREADED, TRAITS>::thread_main()
    {
        module_t& m = module();

        while (1)
        {
            shutdown_event_.wait();
            do
            {
                m.reset_activity_flag();
            } while (shutdown_event_.wait(idle_shutdown_time));
            // timed out

            if (!m.has_activity()) // if no activity let's really bail
            {
#if  (_WIN32_WINNT >= 0x0400 ) || defined(_WIN32_DCOM) // DCOM
                if (FREE_THREADED)
                {
                    ::CoSuspendClassObjects();
                    if (!m.has_activity())
                        break;
                }
                else
                    break;
#else
                break;
#endif
            }
        }
        ::PostThreadMessage(main_thread_id_, WM_QUIT, 0, 0);
        return 0;
    }
};


#endif
