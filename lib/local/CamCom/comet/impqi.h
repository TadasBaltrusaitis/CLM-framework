/** \file
  * Implementation of QueryInterface.
  */
/*
 * Copyright © 2000-2002 Sofus Mortensen, Michael Geddes
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

#ifndef COMET_IMPQI_H
#define COMET_IMPQI_H

#include <wtypes.h>

#include <comet/config.h>

#include <comet/ptr.h>
#include <comet/typelist.h>
#include <comet/common.h>
#include <comet/type_traits.h>
#include <comet/uuid_fwd.h>
#include <comet/threading.h>
#include <comet/module.h>

namespace comet {
    /*! \addtogroup Interfaces
     */
    //@{



    namespace impl {

        /** Base class for recognising qi hook.
         * \internal
         */
        class qi_hook_itf_tag {};

    }

    class qi_hook {};

    template<typename Itf> class qi_hook_itf : public impl::qi_hook_itf_tag{
    public:
        typedef Itf exposes;
        virtual com_ptr<Itf> get_interface_ptr(const com_ptr< ::IUnknown>&) throw() = 0;
    };

    namespace impl {

        template<typename Itf> COMET_FORCEINLINE bool is_interface_compatible(const uuid_t& iid, Itf*)
        {
            if (iid == uuidof<Itf>()) return true;
            else return is_interface_compatible<comtype<Itf>::base>(iid, 0);
        }

        template<> COMET_FORCEINLINE bool is_interface_compatible< ::IUnknown >(const uuid_t&, ::IUnknown*)
        {
            return false;
        }

        template<> COMET_FORCEINLINE bool is_interface_compatible<nil>(const uuid_t&, nil*)
        {
            return false;
        }
        enum use_cast_t {uc_false=0, uc_static, uc_static_op, uc_qi_hook_itf, uc_qi_hook };


        // remove enum for VC2005B2
        template</*enum*/ use_cast_t>
        struct find_compatibility_aux
        {
            template<typename T>
            struct with
            {
                enum { is = false };

                template<typename P> inline static bool qi(P *, const uuid_t& , com_ptr< ::IUnknown>& )
                {
                    return false;
                }
            };
        };

        template<>
        struct find_compatibility_aux<uc_static>
        {
            template<typename Itf> struct with {
                template<typename T> static bool qi(T *This, const uuid_t& iid, com_ptr< ::IUnknown>& unk)
                {
                    if (is_interface_compatible<Itf>(iid, 0))
                    {
                        unk = static_cast< ::IUnknown* >(static_cast<Itf*>(This));
                        return true;
                    }
                    return false;
                }
            };
        };

        template<>
        struct find_compatibility_aux<uc_qi_hook_itf>
        {
            template<typename Itf> struct with {
                template<typename T> static bool qi(T *This, const uuid_t& iid, com_ptr< ::IUnknown>& unk)
            {
                    if (is_interface_compatible<typename Itf::exposes>(iid,0)) {
                        unk = static_cast<qi_hook_itf<typename Itf::exposes>*>(This)->get_interface_ptr( cast_to_unknown(This) );
                        return true;
                    }
                    return false;
                }
            };
        };

        template<>
        struct find_compatibility_aux<uc_qi_hook>
        {
            template<typename Itf> struct with {
                template<typename T> static bool qi(T *This, const uuid_t& iid, com_ptr< ::IUnknown>& unk)
            {
                    if ( static_cast<Itf*>(This)->qi(This, iid, unk) )
                        return true;
                    else
                        return false;
                }
            };
        };

/*        template<>
        struct find_compatibility_aux<uc_qi_hook_itf>
        {
            template<typename U>
            struct with
            {
                static bool is( const uuid_t &iid){ return is_interface_compatible<U::exposes>(iid,0);}
                template<class T>
                static com_ptr< ::IUnknown> cast_from(T *This)
                {
#ifndef NDEBUG
                    try {
#endif
                    return static_cast<qi_hook_itf<Itf::exposes>*>(This)->get_interface_ptr( cast_to_unknown(This) );
#ifndef NDEBUG
                    } catch (...) {
                        // get_interface_ptr is not allowed to throw. Return null pointer on failure
                        COMET_ASSERT(0);
                        return 0;
                    }
#endif
                }
            };
        };

        template<>
        struct find_compatibility_aux<uc_qi_hook>
        {
            template<typename U>
            struct with
            {
                static bool is( const uuid_t &iid){ return true; }

                template<class T>
                static com_ptr< ::IUnknown> cast_from(T *This)
                {
                    Itf::
                }
            };
        };*/

        template< typename Itf>
        struct use_cast_aux
        {
            enum { is_static = (type_traits::conversion<Itf*, ::IUnknown*>::exists) };
            enum { is_static_op =(type_traits::is_cast_operator_compatible<Itf, ::IUnknown>::is)};
            enum { is_qi_hook_itf = (type_traits::conversion<Itf*, qi_hook_itf_tag*>::exists) };
            enum { is_qi_hook = (type_traits::conversion<Itf*, qi_hook*>::exists) };
            // GCC Doesn't handle evaluation of ?: opeators in templates yet.
//            enum { is = (int)( is_static ? uc_static: ( is_static_op ? uc_static_op : uc_false))
            enum { is = is_static      * uc_static +
                        is_qi_hook_itf * uc_qi_hook_itf +
                        is_qi_hook     * uc_qi_hook +
                        is_static_op   * uc_static_op  };
        };

        template<typename Itf>
        struct find_compatibility
        {
            enum { needs_cast_ = use_cast_aux<Itf>::is };
            typedef find_compatibility_aux< (use_cast_t)needs_cast_ > compatible;

            COMET_FORCEINLINE static bool with(const uuid_t &iid)
            { return compatible::template with<Itf>::is(iid); };
            template<typename T>
            COMET_FORCEINLINE static com_ptr< ::IUnknown> cast_from( T *This)
            { return compatible::template with<Itf>::cast_from(This); }
        };

        template<typename ITF_LIST>    struct interface_finder
        {
            template<typename T> COMET_FORCEINLINE static bool find_interface(T* This, const uuid_t& iid, com_ptr< ::IUnknown>& rv)
            {
                typedef typename find_compatibility_aux< (use_cast_t)use_cast_aux< COMET_STRICT_TYPENAME ITF_LIST::head >::is >::template with<COMET_STRICT_TYPENAME ITF_LIST::head> fc;
                if ( fc::qi(This, iid, rv) )
                    return true;
                else
                    return interface_finder< COMET_STRICT_TYPENAME ITF_LIST::tail>::find_interface(This, iid, rv);
            }

            COMET_FORCEINLINE static bool find_interface_2(const uuid_t& iid)
            {
                if (is_interface_compatible<COMET_STRICT_TYPENAME ITF_LIST::head>(iid, 0)) return true;
                return interface_finder< COMET_STRICT_TYPENAME ITF_LIST::tail>::find_interface_2(iid);
            }
        };

        template<> struct interface_finder<nil>
        {
            template<typename T> COMET_FORCEINLINE static bool find_interface(T*, const uuid_t&, com_ptr< ::IUnknown>&)
            { return false;    }

            COMET_FORCEINLINE static bool find_interface_2(const uuid_t&)
            {
                return false;
            }
        };

/*        template<> struct interface_finder<make_list<> >
        {
            template<typename T> COMET_FORCEINLINE static ::IUnknown* find_interface(T*, const uuid_t&)
            {
                return 0;
            }
        };*/

    }

    /** \struct typelibrary_loader impqi.h comet/impqi.h
      * Type Library Loader.
      * Allow provision for different means of loading a type-library.
      * \param TL A \e Comet type-library.
      */
    template<typename TL>
    struct typelibrary_loader
    {
        //! Load the type-library.
        /** Create a different template instantiation of this to load
          * type-libraries from a different source (example - from a second
          * resource in the dll).
          */
        static inline HRESULT load( ITypeLib **pTypeLib)
        { return LoadRegTypeLib(uuidof<TL>(), TL::major_version, TL::minor_version, LANG_NEUTRAL, pTypeLib); }
    };

    /** \struct implement_qi  impqi.h comet/impqi.h
      * Implementation of QueryInterface.  Inherits from all the types defined
      * in \p ITF_LIST.
      * \param ITF_LIST interface implementation list.
      */
    template<typename ITF_LIST> class ATL_NO_VTABLE implement_qi : public typelist::inherit_all<ITF_LIST>
    {
    private:
        // Hide qi
        void qi();
    public:
        /** Get at the unknown for this class. Is here for compatibility when  using
          * implement_internal_qi via aggregateable_coclass for getting at a
          * pointer from which to QueryInterface from.
          */
        ::IUnknown* get_unknown()const
        { return static_cast< typename ITF_LIST::head * >(const_cast<implement_qi<ITF_LIST> *>(this)); }

        STDMETHOD(QueryInterface)(REFIID riid, void** ppv)
        {
            const uuid_t& iid = uuid_t::create_const_reference(riid);
            com_ptr< ::IUnknown> p;

            impl::interface_finder<ITF_LIST>::find_interface(this, iid, p);

            if (!p) {
                if (riid != IID_IUnknown) {
                    *ppv = 0;
                    return E_NOINTERFACE;
                }
                p = get_unknown();
//                p = static_cast< ::IUnknown* >(static_cast< typename ITF_LIST::head * >(this));
            }

            *ppv = reinterpret_cast<void*>(p.detach());

            return S_OK;
        }
    };

      /** \struct implement_internal_qi  impqi.h comet/impqi.h
        * Implementation of QueryInterfaceInternal.  Inherits from all the types defined
        * in \p ITF_LIST. This implementation is used in aggregation.
        * \param ITF_LIST interface implementation list.
        */
    template<typename ITF_LIST> class ATL_NO_VTABLE implement_internal_qi : public typelist::inherit_all<ITF_LIST>
    {
    private:
        void qi();
    public:
        /** Get at the unknown for this class. Is especially useful using
          * aggregateable_coclass in getting at a pointer from which to
          * QueryInterface from.
          */
        ::IUnknown* get_unknown()const
        { return static_cast< typename ITF_LIST::head * >( const_cast<implement_internal_qi<ITF_LIST> *>(this)); }

        HRESULT QueryInterfaceInternal(REFIID riid, void** ppv)
        {
            const IID& iid = riid;
            com_ptr< ::IUnknown> p;

            impl::interface_finder<ITF_LIST>::find_interface(this, iid, p);

            if (!p) {
                if (riid != IID_IUnknown) {
                    *ppv = 0;
                    return E_NOINTERFACE;
                }
//                p = cast_to_unknown(this);
                p = static_cast< ::IUnknown* >(static_cast<typename ITF_LIST::head*>(this));
            }

            *ppv = reinterpret_cast<void*>(p.detach());

            return S_OK;
        }
    };

    namespace impl {
        template<typename ITF_LIST> ::IUnknown* cast_to_unknown(implement_qi<ITF_LIST>* iq)
        { return static_cast< typename ITF_LIST::head*>(iq); }
    }

    /** \class impl_dispatch  impqi.h comet/impqi.h
      * Implement IDispatch via type-library.
      */
    template<typename BASE, typename TL> class ATL_NO_VTABLE impl_dispatch : public BASE
    {
    protected:
        /// \name IDispatch Interface
        //@{
        STDMETHOD(GetTypeInfo)(UINT, LCID, ITypeInfo** ti)
        {
            *ti = get_ti();
            if (*ti)
            {
                (*ti)->AddRef();
                return S_OK;
            }
            return E_NOTIMPL;
        }

        STDMETHOD(GetTypeInfoCount)(UINT *it)
        { *it = 1; return S_OK;    }

        STDMETHOD(GetIDsOfNames)(REFIID, OLECHAR** pNames, UINT cNames, LCID, DISPID* pdispids)
        {
            ITypeInfo* ti = get_ti();
            if (ti)
                return ti->GetIDsOfNames(pNames, cNames, pdispids);
            else
                return E_NOTIMPL;
        }

        STDMETHOD(Invoke)(DISPID id, REFIID, LCID, WORD wFlags, DISPPARAMS *pd, VARIANT* pVarResult, EXCEPINFO* pe, UINT* pu)
        {
            ITypeInfo* ti = get_ti();
            if (ti)
            {
                void* pThis = static_cast<BASE*>(this);
                return ti->Invoke(pThis, id, wFlags, pd, pVarResult, pe, pu);
            }
            else
                return E_NOTIMPL;
        }
        //@}
    private:
        ITypeInfo* get_ti()
        {
            static ITypeInfo* ti_;
            if (ti_ == 0)
            {
                auto_cs lock(module().cs());
                if (ti_ == 0)
                {
                    com_ptr<ITypeLib> ptl;

                    typelibrary_loader<TL>::load(ptl.out());
                    if (ptl) ptl->GetTypeInfoOfGuid(uuidof<BASE>(), &ti_);

                    if (ti_ != 0) module().add_object_to_dispose( impl::create_itf_releaser(ti_) );
                }
            }
            return ti_;
        }

//        static ITypeInfo* ti_;
    };

    template<typename Itf> class com_ptr;

#if 0
    /** \class aggregates_interface impqi.h comet/impqi.h
      * Used as an implementation for an interface to Aggregate the required
      * interface.
      */
    template< typename Itf >
    class aggregates_interface
    {
        public:
            /** Set the inner-unknown returned from the call to CoCreateInstance.
              */
            void set_aggregate(const com_ptr< ::IUnknown>& aggobj) { ag_object_ = com_cast(aggobj); }
            /** Used by QueryInteface algorithms to work out the interface
              * exposed by this class.
              */
            typedef Itf exposes;

            operator Itf*() throw()
            {
                com_ptr< ::IUnknown> test = ag_object_;
                return ag_object_.in();
//                return com_ptr<Itf>( com_cast(ag_object_) );
            }
        protected:
            com_ptr<Itf> ag_object_;

    };
#endif






/*    class FTM : public qi_hook_itf<IMarshal>
    {
    private:
        com_ptr<IMarshal> get_interface_ptr(const com_ptr< ::IUnknown>& This) throw()
        {
            ::IUnknown* ftm = 0;
            CoCreateFreeThreadedMarshaler(This.in(), &ftm);
            return com_cast(ftm);
        }
    };*/

    /** \struct FTM impqi.h comet/impqi.h
      * Aggregate the Free Threaded Marshaller.
      * \code
            class coclass_implementation<CoMyClass>
                    : public coclass<CoMyClass, thread_model::Free, FTM>
            {
               ...
            };
        \endcode
      */
    struct FTM : public qi_hook
    {
        template<typename T>
        bool qi(T *This, const uuid_t& iid, com_ptr< ::IUnknown>& unk)
        {
            if (iid != uuidof<IMarshal>()) return false;
            ::IUnknown* ftm = 0;
            CoCreateFreeThreadedMarshaler(impl::cast_to_unknown(This), &ftm);
            unk = com_ptr< ::IMarshal>( com_cast(ftm) );
            return unk != 0;
        }
    };

    /** \struct aggregates impqi.h comet/impqi.h
      * Aggregate an interface.
      * \code
            class coclass_implementation<CoMyClass>
                    : public coclass<CoMyClass, thread_model::Free, aggregates<IMyInterface> >
            {
               coclass_implementation()
               {
                    aggregates<IMyInterface>create_aggregate::create_aggregate(this, CLSCTX_ALL);
               }
            };
        \endcode

        \sa coclass
      */
    template<typename COCLASS, COMET_LIST_TEMPLATE> class aggregates : public qi_hook
    {
        com_ptr< ::IUnknown> inner_;
    public:
        template<typename T> bool qi(T *This, const uuid_t& iid, com_ptr< ::IUnknown>& unk)
        {
            typedef typename make_list<COMET_LIST_ARG_1>::result TL;
            if (typelist::length<TL>::value > 0) {
                if (impl::interface_finder<TL>::find_interface_2(iid) == false) return false;
            }
            if (inner_ == 0) return false;
            ::IUnknown* p;
            if (SUCCEEDED(inner_.raw()->QueryInterface(iid, reinterpret_cast<void**>(&p))))
            {
                unk = auto_attach(p);
                return true;
            }
            return false;
        }
    protected:
        template<typename T> void create_aggregate(T *This, DWORD dwClsContext = CLSCTX_ALL)
        { ::IUnknown* unk_this = impl::cast_to_unknown(This); inner_ = com_ptr< ::IUnknown>(uuidof<COCLASS>(), com_ptr< ::IUnknown>::create_reference(unk_this), dwClsContext); }
    };

/*    template<typename Itf> class aggregates_interface : public qi_hook_itf<Itf>
    {
    private:
        com_ptr< ::IUnknown> inner_;
        com_ptr<Itf> get_interface_ptr(const com_ptr< ::IUnknown>&)
        { return com_cast(inner_); }
    protected:
        template<typename ITF_LIST>    void create_aggregate(const CLSID& clsid, implement_qi<ITF_LIST>* This, DWORD dwClsContext = CLSCTX_ALL)
        {
            ::IUnknown* unk_this = static_cast< typename ITF_LIST::head*>(This);
            inner_ = com_ptr< ::IUnknown>(clsid, com_ptr< ::IUnknown>::create_reference(unk_this), dwClsContext);
        }

        template<typename ITF_LIST>    void create_aggregate(const wchar_t* progid, implement_qi<ITF_LIST>* This, DWORD dwClsContext = CLSCTX_ALL)
        {
            ::IUnknown* unk_this = static_cast< typename ITF_LIST::head*>(This);
            inner_ = com_ptr< ::IUnknown>(progid, com_ptr< ::IUnknown>::create_reference(unk_this), dwClsContext);
        }
    };*/

    //@}
}

#endif
