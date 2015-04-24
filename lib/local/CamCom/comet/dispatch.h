/** \file
  * Provides dispatch driver via wrap_t< ::IDispatch>
  * Provides dynamic implementation of IDispatch via dynamic_dispatch.
  */
/*
 * Copyright © 2001 Sofus Mortensen
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

#ifndef COMET_DISPATCH_H
#define COMET_DISPATCH_H

#include <comet/ptr.h>

#include <map>

namespace comet {
    /*! \addtogroup Interfaces
     */
    //@{

    /** Specialisation of wrap_t for IDispatch.
     * Implements wrappers for the call-by name and call-by dispid for IDispatch methods
     * and properties. The wrapper supports properties with up to 3 arguments and methods
     * with up to 4 arguments.
     * \code
            com_ptr<IDispatch> disp( my_dual_interface);
             variant_t val = disp->get(L"Name");
     * \endcode
     * See \ref cometcomptrsmartwrapper for details on wrap_t.
     */
    template<> struct wrap_t< ::IDispatch>
    {
        /** Get property by dispid.
         */
        variant_t get(DISPID id)
        {
            VARIANT result;
            VARIANT* vars = 0;
            DISPPARAMS disp = { vars, 0, 0, 0 };
            HRESULT hr = raw(this)->Invoke(id, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &disp, &result, 0, 0);
            if (FAILED(hr)) throw_com_error(raw(this), hr);
            return auto_attach(result);
        }

        /** Get property by name.
         */
        variant_t get(const wchar_t* name)
        {
            DISPID id;
            HRESULT hr = raw(this)->GetIDsOfNames(IID_NULL, const_cast<OLECHAR**>(&name), 1, LOCALE_USER_DEFAULT, &id);
            if (FAILED(hr)) throw_com_error(raw(this), hr);
            return get(id);
        }

        /** Get property by dispid with 1 argument.
         */
        variant_t get(DISPID id, const variant_t& a0)
        {
            VARIANT result;
            VARIANT vars[1]; vars[0] = a0.in();
            DISPPARAMS disp = { vars, 0, 1, 0 };
            HRESULT hr = raw(this)->Invoke(id, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &disp, &result, 0, 0);
            if (FAILED(hr)) throw_com_error(raw(this), hr);
            return auto_attach(result);
        }

        /** Get property by name with 1 argument.
         */
        variant_t get(const wchar_t* name, const variant_t& a0)
        {
            DISPID id;
            HRESULT hr = raw(this)->GetIDsOfNames(IID_NULL, const_cast<OLECHAR**>(&name), 1, LOCALE_USER_DEFAULT, &id);
            if (FAILED(hr)) throw_com_error(raw(this), hr);
            return get(id, a0);
        }

        /** Get property by dispid with 2 arguments.
         */
        variant_t get(DISPID id, const variant_t& a1, const variant_t& a0)
        {
            VARIANT result;
            VARIANT vars[2]; vars[0] = a0.in();    vars[1] = a1.in();
            DISPPARAMS disp = { vars, 0, 2, 0 };
            HRESULT hr = raw(this)->Invoke(id, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &disp, &result, 0, 0);
            if (FAILED(hr)) throw_com_error(raw(this), hr);
            return auto_attach(result);
        }

        /** Get property by name with 2 arguments.
         */
        variant_t get(const wchar_t* name, const variant_t& a1, const variant_t& a0)
        {
            DISPID id;
            HRESULT hr = raw(this)->GetIDsOfNames(IID_NULL, const_cast<OLECHAR**>(&name), 1, LOCALE_USER_DEFAULT, &id);
            if (FAILED(hr)) throw_com_error(raw(this), hr);
            return get(id, a1, a0);
        }

        /** Get property by dispid with 3 arguments.
         */
        variant_t get(DISPID id, const variant_t& a2, const variant_t& a1, const variant_t& a0)
        {
            VARIANT result;
            VARIANT vars[3]; vars[0] = a0.in(); vars[1] = a1.in(); vars[2] = a2.in();
            DISPPARAMS disp = { vars, 0, 3, 0 };
            HRESULT hr = raw(this)->Invoke(id, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &disp, &result, 0, 0);
            if (FAILED(hr)) throw_com_error(raw(this), hr);
            return auto_attach(result);
        }

        /** Get property by name with 3 arguments.
         */
        variant_t get(const wchar_t* name, const variant_t& a2, const variant_t& a1, const variant_t& a0)
        {
            DISPID id;
            HRESULT hr = raw(this)->GetIDsOfNames(IID_NULL, const_cast<OLECHAR**>(&name), 1, LOCALE_USER_DEFAULT, &id);
            if (FAILED(hr)) throw_com_error(raw(this), hr);
            return get(id, a2, a1, a0);
        }

        /** Put property by dispid.
         */
        void put(DISPID id, const variant_t& val)
        {
            VARIANT vars[1]; vars[0] = val.in();
            DISPID did = DISPID_PROPERTYPUT;
            DISPPARAMS disp = { vars, &did, 1, 1 };
            HRESULT hr = raw(this)->Invoke(id, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUT, &disp, 0, 0, 0);
            if (FAILED(hr)) throw_com_error(raw(this), hr);
        }

        /** Put property by name.
         */
        void put(const wchar_t* name, const variant_t& val)
        {
            DISPID id;
            HRESULT hr = raw(this)->GetIDsOfNames(IID_NULL, const_cast<OLECHAR**>(&name), 1, LOCALE_USER_DEFAULT, &id);
            if (FAILED(hr)) throw_com_error(raw(this), hr);
            put(id, val);
        }

        /** Put property with 1 argument by dispid.
         */
        void put(DISPID id, const variant_t& a1, const variant_t& val)
        {
            VARIANT vars[2]; vars[0] = val.in(); vars[1] = a1.in();
            DISPID did = DISPID_PROPERTYPUT;
            DISPPARAMS disp = { vars, &did, 2, 1 };
            HRESULT hr = raw(this)->Invoke(id, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUT, &disp, 0, 0, 0);
            if (FAILED(hr)) throw_com_error(raw(this), hr);
        }

        /** Put property with 1 argument by name.
         */
        void put(const wchar_t* name, const variant_t& a1, const variant_t& val)
        {
            DISPID id;
            HRESULT hr = raw(this)->GetIDsOfNames(IID_NULL, const_cast<OLECHAR**>(&name), 1, LOCALE_USER_DEFAULT, &id);
            if (FAILED(hr)) throw_com_error(raw(this), hr);
            put(id, a1, val);
        }


        /** Put property with 2 arguments by dispid.
         */
        void put(DISPID id, const variant_t& a2, const variant_t& a1, const variant_t& val)
        {
            VARIANT vars[3]; vars[0] = val.in(); vars[1] = a1.in(); vars[2] = a2.in();
            DISPID did = DISPID_PROPERTYPUT;
            DISPPARAMS disp = { vars, &did, 3, 1 };
            HRESULT hr = raw(this)->Invoke(id, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUT, &disp, 0, 0, 0);
            if (FAILED(hr)) throw_com_error(raw(this), hr);
        }

        /** Put property with 2 arguments by name.
         */
        void put(const wchar_t* name, const variant_t& a2, const variant_t& a1, const variant_t& val)
        {
            DISPID id;
            HRESULT hr = raw(this)->GetIDsOfNames(IID_NULL, const_cast<OLECHAR**>(&name), 1, LOCALE_USER_DEFAULT, &id);
            if (FAILED(hr)) throw_com_error(raw(this), hr);
            put(id, a2, a1, val);
        }

        /** Put property with 3 arguments by dispid.
         */
        void put(DISPID id, const variant_t& a3, const variant_t& a2, const variant_t& a1, const variant_t& val)
        {
            VARIANT vars[4]; vars[0] = val.in(); vars[1] = a1.in(); vars[2] = a2.in(); vars[3] = a3.in();
            DISPID did = DISPID_PROPERTYPUT;
            DISPPARAMS disp = { vars, &did, 4, 1 };
            HRESULT hr = raw(this)->Invoke(id, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUT, &disp, 0, 0, 0);
            if (FAILED(hr)) throw_com_error(raw(this), hr);
        }

        /** Put property with 3 arguments by name.
         */
        void put(const wchar_t* name, const variant_t& a3, const variant_t& a2, const variant_t& a1, const variant_t& val)
        {
            DISPID id;
            HRESULT hr = raw(this)->GetIDsOfNames(IID_NULL, const_cast<OLECHAR**>(&name), 1, LOCALE_USER_DEFAULT, &id);
            if (FAILED(hr)) throw_com_error(raw(this), hr);
            put(id, a3, a2, a1, val);
        }

        /** Put property by reference by dispid.
         */
        void putref(DISPID id, const variant_t& val)
        {
            VARIANT vars[1]; vars[0] = val.in();
            DISPID did = DISPID_PROPERTYPUT;
            DISPPARAMS disp = { vars, &did, 1, 1 };
            HRESULT hr = raw(this)->Invoke(id, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUTREF, &disp, 0, 0, 0);
            if (FAILED(hr)) throw_com_error(raw(this), hr);
        }

        /** Put property by reference by name.
         */
        void putref(const wchar_t* name, const variant_t& val)
        {
            DISPID id;
            HRESULT hr = raw(this)->GetIDsOfNames(IID_NULL, const_cast<OLECHAR**>(&name), 1, LOCALE_USER_DEFAULT, &id);
            if (FAILED(hr)) throw_com_error(raw(this), hr);
            putref(id, val);
        }

        void putref(DISPID id, const variant_t& a1, const variant_t& val)
        {
            VARIANT vars[2]; vars[0] = val.in(); vars[1] = a1.in();
            DISPID did = DISPID_PROPERTYPUT;
            DISPPARAMS disp = { vars, &did, 2, 1 };
            HRESULT hr = raw(this)->Invoke(id, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUTREF, &disp, 0, 0, 0);
            if (FAILED(hr)) throw_com_error(raw(this), hr);
        }

        void putref(const wchar_t* name, const variant_t& a1, const variant_t& val)
        {
            DISPID id;
            HRESULT hr = raw(this)->GetIDsOfNames(IID_NULL, const_cast<OLECHAR**>(&name), 1, LOCALE_USER_DEFAULT, &id);
            if (FAILED(hr)) throw_com_error(raw(this), hr);
            putref(id, a1, val);
        }

        void putref(DISPID id, const variant_t& a2, const variant_t& a1, const variant_t& val)
        {
            VARIANT vars[3]; vars[0] = val.in(); vars[1] = a1.in(); vars[2] = a2.in();
            DISPID did = DISPID_PROPERTYPUT;
            DISPPARAMS disp = { vars, &did, 3, 1 };
            HRESULT hr = raw(this)->Invoke(id, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUTREF, &disp, 0, 0, 0);
            if (FAILED(hr)) throw_com_error(raw(this), hr);
        }

        void putref(const wchar_t* name, const variant_t& a2, const variant_t& a1, const variant_t& val)
        {
            DISPID id;
            HRESULT hr = raw(this)->GetIDsOfNames(IID_NULL, const_cast<OLECHAR**>(&name), 1, LOCALE_USER_DEFAULT, &id);
            if (FAILED(hr)) throw_com_error(raw(this), hr);
            putref(id, a2, a1, val);
        }

        void putref(DISPID id, const variant_t& a3, const variant_t& a2, const variant_t& a1, const variant_t& val)
        {
            VARIANT vars[4]; vars[0] = val.in(); vars[1] = a1.in(); vars[2] = a2.in(); vars[3] = a3.in();
            DISPID did = DISPID_PROPERTYPUT;
            DISPPARAMS disp = { vars, &did, 4, 1 };
            HRESULT hr = raw(this)->Invoke(id, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUTREF, &disp, 0, 0, 0);
            if (FAILED(hr)) throw_com_error(raw(this), hr);
        }

        void putref(const wchar_t* name, const variant_t& a3, const variant_t& a2, const variant_t& a1, const variant_t& val)
        {
            DISPID id;
            HRESULT hr = raw(this)->GetIDsOfNames(IID_NULL, const_cast<OLECHAR**>(&name), 1, LOCALE_USER_DEFAULT, &id);
            if (FAILED(hr)) throw_com_error(raw(this), hr);
            putref(id, a3, a2, a1, val);
        }

        /** Call method by dispid.
         */
        variant_t call(DISPID id)
        {
            VARIANT result;
            VARIANT* vars = 0;
            DISPPARAMS disp = { vars, 0, 0, 0 };
            HRESULT hr = raw(this)->Invoke(id, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &result, 0, 0);
            if (FAILED(hr)) throw_com_error(raw(this), hr);
            return auto_attach(result);
        }

        /** Call method by name.
         */
        variant_t call(const wchar_t* name)
        {
            DISPID id;
            HRESULT hr = raw(this)->GetIDsOfNames(IID_NULL, const_cast<OLECHAR**>(&name), 1, LOCALE_USER_DEFAULT, &id);
            if (FAILED(hr)) throw_com_error(raw(this), hr);
            return call(id);
        }

        variant_t call(DISPID id, const variant_t& a0)
        {
            VARIANT result;
            VARIANT vars[1]; vars[0] = a0.in();
            DISPPARAMS disp = { vars, 0, 1, 0 };
            HRESULT hr = raw(this)->Invoke(id, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &result, 0, 0);
            if (FAILED(hr)) throw_com_error(raw(this), hr);
            return auto_attach(result);
        }

        variant_t call(const wchar_t* name, const variant_t& a0)
        {
            DISPID id;
            HRESULT hr = raw(this)->GetIDsOfNames(IID_NULL, const_cast<OLECHAR**>(&name), 1, LOCALE_USER_DEFAULT, &id);
            if (FAILED(hr)) throw_com_error(raw(this), hr);
            return call(id, a0);
        }

        variant_t call(DISPID id, const variant_t& a1, const variant_t& a0)
        {
            VARIANT result;
            VARIANT vars[2]; vars[0] = a0.in(); vars[1] = a1.in();
            DISPPARAMS disp = { vars, 0, 2, 0 };
            HRESULT hr = raw(this)->Invoke(id, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &result, 0, 0);
            if (FAILED(hr)) throw_com_error(raw(this), hr);
            return auto_attach(result);
        }

        variant_t call(const wchar_t* name, const variant_t& a1, const variant_t& a0)
        {
            DISPID id;
            HRESULT hr = raw(this)->GetIDsOfNames(IID_NULL, const_cast<OLECHAR**>(&name), 1, LOCALE_USER_DEFAULT, &id);
            if (FAILED(hr)) throw_com_error(raw(this), hr);
            return call(id, a1, a0);
        }

        variant_t call(DISPID id, const variant_t& a2, const variant_t& a1, const variant_t& a0)
        {
            VARIANT result;
            VARIANT vars[3]; vars[0] = a0.in(); vars[1] = a1.in(); vars[2] = a2.in();
            DISPPARAMS disp = { vars, 0, 3, 0 };
            HRESULT hr = raw(this)->Invoke(id, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &result, 0, 0);
            if (FAILED(hr)) throw_com_error(raw(this), hr);
            return auto_attach(result);
        }

        variant_t call(const wchar_t* name, const variant_t& a2, const variant_t& a1, const variant_t& a0)
        {
            DISPID id;
            HRESULT hr = raw(this)->GetIDsOfNames(IID_NULL, const_cast<OLECHAR**>(&name), 1, LOCALE_USER_DEFAULT, &id);
            if (FAILED(hr)) throw_com_error(raw(this), hr);
            return call(id, a2, a1, a0);
        }

        variant_t call(DISPID id, const variant_t& a3, const variant_t& a2, const variant_t& a1, const variant_t& a0)
        {
            VARIANT result;
            VARIANT vars[4]; vars[0] = a0.in(); vars[1] = a1.in(); vars[2] = a2.in(); vars[3] = a3.in();
            DISPPARAMS disp = { vars, 0, 4, 0 };
            HRESULT hr = raw(this)->Invoke(id, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &result, 0, 0);
            if (FAILED(hr)) throw_com_error(raw(this), hr);
            return auto_attach(result);
        }

        variant_t call(const wchar_t* name, const variant_t& a3, const variant_t& a2, const variant_t& a1, const variant_t& a0)
        {
            DISPID id;
            HRESULT hr = raw(this)->GetIDsOfNames(IID_NULL, const_cast<OLECHAR**>(&name), 1, LOCALE_USER_DEFAULT, &id);
            if (FAILED(hr)) throw_com_error(raw(this), hr);
            return call(id, a3, a2, a1, a0);
        }
        variant_t call(DISPID id, const variant_t& a4, const variant_t& a3, const variant_t& a2, const variant_t& a1, const variant_t& a0)
        {
            VARIANT result;
            VARIANT vars[5]; vars[0] = a0.in(); vars[1] = a1.in(); vars[2] = a2.in(); vars[3] = a3.in(); vars[4] = a4.in();
            DISPPARAMS disp = { vars, 0, 5, 0 };
            HRESULT hr = raw(this)->Invoke(id, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &disp, &result, 0, 0);
            if (FAILED(hr)) throw_com_error(raw(this), hr);
            return auto_attach(result);
        }
        variant_t call(const wchar_t* name, const variant_t& a4, const variant_t& a3, const variant_t& a2, const variant_t& a1, const variant_t& a0)
        {
            DISPID id;
            HRESULT hr = raw(this)->GetIDsOfNames(IID_NULL, const_cast<OLECHAR**>(&name), 1, LOCALE_USER_DEFAULT, &id);
            if (FAILED(hr)) throw_com_error(raw(this), hr);
            return call(id, a4, a3, a2, a1, a0);
        }
    };
    /** \class dynamic_dispatch dispatch.h comet/dispatch.h
     * Implementation of a dynamic IDispatch, allowing methods to be added to
      * an IDispatch implementation.
      * The class needs to be inherited from to be able to add methods.
      * Each method can have up to 4 parameters.
      */

template<typename BASE> class ATL_NO_VTABLE dynamic_dispatch : public ::IDispatch {

        struct method_ptr {
            bool has_retval;
            union {
                void (BASE::*pm00)();
                void (BASE::*pm01)(const variant_t&);
                void (BASE::*pm02)(const variant_t&, const variant_t&);
                void (BASE::*pm03)(const variant_t&, const variant_t&, const variant_t&);
                void (BASE::*pm04)(const variant_t&, const variant_t&, const variant_t&, const variant_t&);

                variant_t (BASE::*pm10)();
                variant_t (BASE::*pm11)(const variant_t&);
                variant_t (BASE::*pm12)(const variant_t&, const variant_t&);
                variant_t (BASE::*pm13)(const variant_t&, const variant_t&, const variant_t&);
                variant_t (BASE::*pm14)(const variant_t&, const variant_t&, const variant_t&, const variant_t&);
            };
        };

        typedef std::map<std::wstring, DISPID> NAMEMAP;
        NAMEMAP name_map_;

        // Workaround VC internal compiler error
        struct wrap_map_t {
            std::map<unsigned, method_ptr> m;
        };
        typedef std::map<DISPID, wrap_map_t > MAP;
        MAP map_;

        void add_method(const wchar_t* name, method_ptr p, DISPID id, int type)
        {
            if (id == flag_value)
            {
                NAMEMAP::const_iterator it = name_map_.find(name);
                if (it == name_map_.end())
                {
                    id = 100000 + map_.size();
                    while (map_.find(id) != map_.end()) ++id;
                }
                else
                {
                    id = it->second;
                }
            }

            name_map_[name] = id;
            map_[id].m[type] = p;
        }
        enum { flag_value = MINLONG };
    public:
        typedef ::IDispatch interface_is;

    protected:

        void remove(const wchar_t* name)
        {
            NAMEMAP::iterator it = name_map_.find(name);
            if (it != name_map_.end()) {
                DISPID id = it->second;
                name_map_.erase(it);

                map_.erase(id);
            }
        }

        void add_method(const wchar_t* name, void (BASE::*pm)(), DISPID id = flag_value)
        { method_ptr p; p.has_retval = false; p.pm00 = pm; add_method( name, p, id, 0 << 16 | DISPATCH_METHOD ); }

        void add_method(const wchar_t* name, void (BASE::*pm)(const variant_t&), DISPID id = flag_value)
        { method_ptr p; p.has_retval = false; p.pm01 = pm; add_method( name, p, id, 1 << 16 | DISPATCH_METHOD ); }

        void add_method(const wchar_t* name, void (BASE::*pm)(const variant_t&, const variant_t&), DISPID id = flag_value)
        { method_ptr p; p.has_retval = false; p.pm02 = pm; add_method( name, p, id, 2 << 16 | DISPATCH_METHOD ); }

        void add_method(const wchar_t* name, void (BASE::*pm)(const variant_t&, const variant_t&, const variant_t&), DISPID id = flag_value)
        { method_ptr p; p.has_retval = false; p.pm03 = pm; add_method( name, p, id, 3 << 16 | DISPATCH_METHOD ); }

        void add_method(const wchar_t* name, void (BASE::*pm)(const variant_t&, const variant_t&, const variant_t&, const variant_t&), DISPID id = flag_value)
        { method_ptr p; p.has_retval = false; p.pm04 = pm; add_method( name, p, id, 4 << 16 | DISPATCH_METHOD ); }

        void add_method(const wchar_t* name, variant_t (BASE::*pm)(), DISPID id = flag_value)
        { method_ptr p; p.has_retval = true; p.pm10 = pm; add_method( name, p, id, 0 << 16 | DISPATCH_METHOD ); }

        void add_method(const wchar_t* name, variant_t (BASE::*pm)(const variant_t&), DISPID id = flag_value)
        { method_ptr p; p.has_retval = true; p.pm11 = pm; add_method( name, p, id, 1 << 16 | DISPATCH_METHOD ); }

        void add_method(const wchar_t* name, variant_t (BASE::*pm)(const variant_t&, const variant_t&), DISPID id = flag_value)
        { method_ptr p; p.has_retval = true; p.pm12 = pm; add_method( name, p, id, 2 << 16 | DISPATCH_METHOD ); }

        void add_method(const wchar_t* name, variant_t (BASE::*pm)(const variant_t&, const variant_t&, const variant_t&), DISPID id = flag_value)
        { method_ptr p; p.has_retval = true; p.pm13 = pm; add_method( name, p, id, 3 << 16 | DISPATCH_METHOD ); }

        void add_method(const wchar_t* name, variant_t (BASE::*pm)(const variant_t&, const variant_t&, const variant_t&, const variant_t&), DISPID id = flag_value)
        { method_ptr p; p.has_retval = true; p.pm14 = pm; add_method( name, p, id, 4 << 16 | DISPATCH_METHOD ); }

        void add_put_property(const wchar_t* name, void (BASE::*pm)(const variant_t&), DISPID id = flag_value)
        { method_ptr p; p.has_retval = false; p.pm01 = pm; add_method( name, p, id, 1 << 16 | DISPATCH_PROPERTYPUT ); }

        void add_put_property(const wchar_t* name, void (BASE::*pm)(const variant_t&, const variant_t&), DISPID id = flag_value)
        { method_ptr p; p.has_retval = false; p.pm02 = pm; add_method( name, p, id, 2 << 16 | DISPATCH_PROPERTYPUT ); }

        void add_put_property(const wchar_t* name, void (BASE::*pm)(const variant_t&, const variant_t&, const variant_t&), DISPID id = flag_value)
        { method_ptr p; p.has_retval = false; p.pm03 = pm; add_method( name, p, id, 3 << 16 | DISPATCH_PROPERTYPUT ); }

        void add_put_property(const wchar_t* name, void (BASE::*pm)(const variant_t&, const variant_t&, const variant_t&, const variant_t&), DISPID id = flag_value)
        { method_ptr p; p.has_retval = false; p.pm04 = pm; add_method( name, p, id, 4 << 16 | DISPATCH_PROPERTYPUT ); }

        void add_putref_property(const wchar_t* name, void (BASE::*pm)(const variant_t&), DISPID id = flag_value)
        { method_ptr p; p.has_retval = false; p.pm01 = pm; add_method( name, p, id, 1 << 16 | DISPATCH_PROPERTYPUTREF ); }

        void add_putref_property(const wchar_t* name, void (BASE::*pm)(const variant_t&, const variant_t&), DISPID id = flag_value)
        { method_ptr p; p.has_retval = false; p.pm02 = pm; add_method( name, p, id, 2 << 16 | DISPATCH_PROPERTYPUTREF ); }

        void add_putref_property(const wchar_t* name, void (BASE::*pm)(const variant_t&, const variant_t&, const variant_t&), DISPID id = flag_value)
        { method_ptr p; p.has_retval = false; p.pm03 = pm; add_method( name, p, id, 3 << 16 | DISPATCH_PROPERTYPUTREF ); }

        void add_putref_property(const wchar_t* name, void (BASE::*pm)(const variant_t&, const variant_t&, const variant_t&, const variant_t&), DISPID id = flag_value)
        { method_ptr p; p.has_retval = false; p.pm04 = pm; add_method( name, p, id, 4 << 16 | DISPATCH_PROPERTYPUTREF ); }

        void add_get_property(const wchar_t* name, variant_t (BASE::*pm)(), DISPID id = flag_value)
        { method_ptr p; p.has_retval = true; p.pm10 = pm; add_method( name, p, id, 0 << 16 | DISPATCH_PROPERTYGET ); }

        void add_get_property(const wchar_t* name, variant_t (BASE::*pm)(const variant_t&), DISPID id = flag_value)
        { method_ptr p; p.has_retval = true; p.pm11 = pm; add_method( name, p, id, 1 << 16 | DISPATCH_PROPERTYGET ); }

        void add_get_property(const wchar_t* name, variant_t (BASE::*pm)(const variant_t&, const variant_t&), DISPID id = flag_value)
        { method_ptr p; p.has_retval = true; p.pm12 = pm; add_method( name, p, id, 2 << 16 | DISPATCH_PROPERTYGET ); }

        void add_get_property(const wchar_t* name, variant_t (BASE::*pm)(const variant_t&, const variant_t&, const variant_t&), DISPID id = flag_value)
        { method_ptr p; p.has_retval = true; p.pm13 = pm; add_method( name, p, id, 3 << 16 | DISPATCH_PROPERTYGET ); }

        void add_get_property(const wchar_t* name, variant_t (BASE::*pm)(const variant_t&, const variant_t&, const variant_t&, const variant_t&), DISPID id = flag_value)
        { method_ptr p; p.has_retval = true; p.pm14 = pm; add_method( name, p, id, 4 << 16 | DISPATCH_PROPERTYGET ); }

    private:
        STDMETHOD(Invoke)(DISPID id, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pd, VARIANT* pVarResult, EXCEPINFO* pe, UINT* pu)
        {
            unsigned type = pd->cArgs << 16 | (wFlags & 15);
            typename MAP::const_iterator it2 = map_.find(id);
            if (it2 == map_.end()) return DISP_E_MEMBERNOTFOUND;

            typename std::map<unsigned, method_ptr>::const_iterator it = it2->second.m.find(type);
            if (it == it2->second.m.end()) return DISP_E_BADPARAMCOUNT;

            try {

                if (pd->cNamedArgs > 1) return DISP_E_NONAMEDARGS;

                if (pd->cNamedArgs == 1) {

                    if ((wFlags & 15) != DISPATCH_PROPERTYPUT && (wFlags & 15) != DISPATCH_PROPERTYPUTREF) return DISP_E_NONAMEDARGS;
                    if (pd->rgdispidNamedArgs[0] != DISPID_PROPERTYPUT) return DISP_E_NONAMEDARGS;

                    switch (pd->cArgs)
                    {
                    case 1:
                        (static_cast<BASE*>(this)->*it->second.pm01)(variant_t::create_reference(pd->rgvarg[0]));
                        break;
                    case 2:
                        (static_cast<BASE*>(this)->*it->second.pm02)(variant_t::create_reference(pd->rgvarg[1]), variant_t::create_reference(pd->rgvarg[0]));
                        break;
                    case 3:
                        (static_cast<BASE*>(this)->*it->second.pm03)(variant_t::create_reference(pd->rgvarg[2]), variant_t::create_reference(pd->rgvarg[1]), variant_t::create_reference(pd->rgvarg[0]));
                        break;
                    case 4:
                        (static_cast<BASE*>(this)->*it->second.pm04)(variant_t::create_reference(pd->rgvarg[3]), variant_t::create_reference(pd->rgvarg[2]), variant_t::create_reference(pd->rgvarg[1]), variant_t::create_reference(pd->rgvarg[0]));
                        break;
                    default:
                        return DISP_E_MEMBERNOTFOUND;
                    }

                }
                else
                {

                variant_t rv;
                if (it->second.has_retval)
                {
                    switch (pd->cArgs)
                    {
                    case 0:
                        rv = (static_cast<BASE*>(this)->*it->second.pm10)();
                        break;
                    case 1:
                        rv = (static_cast<BASE*>(this)->*it->second.pm11)(variant_t::create_reference(pd->rgvarg[0]));
                        break;
                    case 2:
                        rv = (static_cast<BASE*>(this)->*it->second.pm12)(variant_t::create_reference(pd->rgvarg[1]), variant_t::create_reference(pd->rgvarg[0]));
                        break;
                    case 3:
                        rv = (static_cast<BASE*>(this)->*it->second.pm13)(variant_t::create_reference(pd->rgvarg[2]), variant_t::create_reference(pd->rgvarg[1]), variant_t::create_reference(pd->rgvarg[1]));
                        break;
                    case 4:
                        rv = (static_cast<BASE*>(this)->*it->second.pm14)(variant_t::create_reference(pd->rgvarg[3]), variant_t::create_reference(pd->rgvarg[1]), variant_t::create_reference(pd->rgvarg[2]), variant_t::create_reference(pd->rgvarg[0]));
                        break;
                    default:
                        return DISP_E_MEMBERNOTFOUND;
                    }
                }
                else
                {
                    switch (pd->cArgs)
                    {
                    case 0:
                        (static_cast<BASE*>(this)->*it->second.pm00)();
                        break;
                    case 1:
                        (static_cast<BASE*>(this)->*it->second.pm01)(variant_t::create_reference(pd->rgvarg[0]));
                        break;
                    case 2:
                        (static_cast<BASE*>(this)->*it->second.pm02)(variant_t::create_reference(pd->rgvarg[1]), variant_t::create_reference(pd->rgvarg[0]));
                        break;
                    case 3:
                        (static_cast<BASE*>(this)->*it->second.pm03)(variant_t::create_reference(pd->rgvarg[2]), variant_t::create_reference(pd->rgvarg[1]), variant_t::create_reference(pd->rgvarg[0]));
                        break;
                    case 4:
                        (static_cast<BASE*>(this)->*it->second.pm04)(variant_t::create_reference(pd->rgvarg[3]), variant_t::create_reference(pd->rgvarg[2]), variant_t::create_reference(pd->rgvarg[1]), variant_t::create_reference(pd->rgvarg[0]));
                        break;
                    default:
                        return DISP_E_MEMBERNOTFOUND;
                    }
                }
                if (pVarResult) *pVarResult = rv.detach();
                }


            } catch (com_error& err) {
                return impl::return_com_error(err);
            } catch (const std::exception& err) {
                return impl::return_com_error(err);
            } catch (HRESULT hr) {
                return hr;
            } catch (...) {
                return E_FAIL;
            }

            return S_OK;

        }

        STDMETHOD(GetIDsOfNames)(REFIID, WCHAR** names, unsigned int c, LCID, DISPID* dispid)
        {
            bool failed = false;
            for (size_t i=0; i<c; ++i)
            {
                NAMEMAP::const_iterator it = name_map_.find(names[i]);
                if (it == name_map_.end())
                {
                    failed = true;
                    dispid[i] = DISPID_UNKNOWN;
                }
                else
                {
                    dispid[i] = it->second;
                }
            }
            return failed ? DISP_E_UNKNOWNNAME : S_OK;
        }

        STDMETHOD(GetTypeInfo)(UINT, LCID, ITypeInfo**)
        { return E_NOTIMPL; }

        STDMETHOD(GetTypeInfoCount)(UINT *it)
        { *it = 0; return S_OK;    }

    };
//@}
}

#endif
