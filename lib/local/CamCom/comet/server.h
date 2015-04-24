/** \file
  * Main functionality for providing a COM server dll.
  */
/*
 * Copyright © 2000-2002 Sofus Mortensen, Paul Hollingsworth, Michael Geddes, Mikael Lindgren
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

#ifndef COMET_SERVER_H
#define COMET_SERVER_H

#include <comet/config.h>
#ifdef COMET_GCC_HEADERS
#include <windows.h>
#else // COMET_GCC_HEADERS
#include <olectl.h>
#endif // COMET_GCC_HEADERS

#include <malloc.h>
#include <string>
#include <vector>

#include <comet/impqi.h>
#include <comet/interface.h>
#include <comet/regkey.h>
#include <comet/threading.h>
#include <comet/tstring.h>
#include <comet/handle_except.h>
#include <comet/module.h>

/** \page cometclassfactory Comet Class Factories
 * Comet currently has support for \ref cometclassfactorystandard (non-aggregating), \ref cometclassfactoryaggregating and
 * \ref cometclassfactorysingleton class factories.  As there has been no demand for custom
 * class factories yet, there is currently no support for them.
 *
 * The different class-factories are enabled by specialising the
 * coclass_implementation to inherit off different classes.
 *
 * \section cometclassfactorystandard Standard
 *
 * The trigger classes for standard class-factory are either comet::coclass, or
 * comet::simple_object.
 *
 * There is nothing much more noteworthy about this form except that it will
 * return CLASS_E_NOAGGREGATION if aggregation is specified.
 *
 * \section cometclassfactoryaggregating Aggregating
 *
 * The trigger classes for the aggregating class-factory are comet::aggregateable_coclass or
 * comet::aggregateable_object.
 *
 * The implementation of aggregation in comet is similar to the
 * poly-aggregateable implementations found in ATL; there is no
 * aggregation-only coclass implementation.
 *
 * The aggregating class factory will hook up aggegation if an IUnknown is
 * supplied.
 *
 * \section cometclassfactorysingleton Singleton
 *
 * The trigger classes for the singleton class-factory  are
 * comet::singleton_coclass or comet::singleton_object.
 *
 * The singleton class-factory will cause all CreateObject calls for this class to
 * return the same object. The life-time of the object is from first call till
 * just before the dll/exe is unloaded.
 *
 * More complex requirements are anticipated, however they have not been
 * implemented.
 */
    /*! \defgroup Server Server implementation details.
     */
    //@{

/** Template class for specifying your own custom registration.

    The default template does nothing - define a specialization of ::custom_registration
    for each Coclass that you wish to have custom registration.

    on_register is called \em after the standard registration.
    on_unregister is called \em before the standard unregistration.

    Common initialization and cleanup code for both registration and unregistration
    can be put in the constructor and destructor.

    All exceptions thrown by on_unregister are caught and discarded.

    Example usage:
    \code
    class custom_registration<comet::CoPerson>
    {
        comet::regkey key_;
        public:
        custom_registration<comet::CoPerson>()
        {
            key_ = comet::regkey(HKEY_LOCAL_MACHINE).open(_T("Software\\AcmeCorp\\AcmePayroll"));
        };
        void on_register(const TCHAR *filename)
        {
            key_.create(_T("Bill Bloggs"));
        }
        void on_unregister(const TCHAR *filename)
        {
            key_.delete_subkey(_T("Bill Bloggs"));
        }
    };
    \endcode
*/
template<typename CLASS>
class custom_registration
{
public:
    void on_register(const TCHAR *) {}
    void on_unregister(const TCHAR *) {}
};
//@}

namespace comet {


    namespace impl {

        enum factory_type_t { ft_standard, ft_aggregateable, ft_singleton };

        inline void create_record_info( const IID& lib_guid, const IID& rec_guid, unsigned short major_version, unsigned short minor_version, IRecordInfo*& ri )
        {
            auto_cs lock( module().cs() );

            if (!ri) {
                com_ptr<ITypeLib> tl;
                LoadRegTypeLib(lib_guid,
                        major_version,
                        minor_version,
                        GetUserDefaultLCID(),
                        tl.out()) | raise_exception;
                com_ptr<ITypeInfo> ti;
                tl->GetTypeInfoOfGuid(rec_guid, ti.out()) | raise_exception;
                GetRecordInfoFromTypeInfo(ti.in(), &ri) | raise_exception;
                module().add_object_to_dispose( impl::create_itf_releaser( ri ) );
            }
        }

        template<typename T> struct interface_wrapper : public T
        {
            typedef T interface_is;
        };

        template<typename T> class ATL_NO_VTABLE simple_object_aux :
            public implement_qi< typelist::append< T,
        make_list<impl::interface_wrapper<ISupportErrorInfo> >::result > >
        {
            public:
//                enum { factory_type = ft_standard };

                STDMETHOD_(ULONG, AddRef)()
                {
                    LONG rc = InterlockedIncrement(&rc_);
                    if (rc_ == 1) module().lock();
                    return rc;
                }

                STDMETHOD_(ULONG, Release)()
                {
                    LONG rc = InterlockedDecrement(&rc_);
                    if (rc == 0) {
                        try {
                            delete this;
                        } COMET_CATCH_UNKNOWN( L"Release", IID_IUnknown, bstr_t());
                        module().unlock();
                    }
                    return rc;
                }

                STDMETHOD(InterfaceSupportsErrorInfo)(REFIID)
                {
                    return S_OK;
                }

                void so_internal_addref() { ++rc_; }
                void so_internal_release() { --rc_; }

            protected:
                simple_object_aux() : rc_(0) {}
                virtual ~simple_object_aux() {}
            private:
                long rc_;

                // non-copyable
                simple_object_aux(const simple_object_aux&);
                simple_object_aux& operator=(const simple_object_aux&);
        };

    }
    /*!\defgroup Objects Classes used as bases for COM objects.
     */
    //@{
    /** Provide an inner unknown for aggregation.
      * This is the unknown that handles lifetime of an aggregateable object.
      */
    template<class C>
    struct aggregate_inner_unknown : IUnknown
    {
        aggregate_inner_unknown() : rc_(0) {}

        STDMETHOD_(ULONG, AddRef)()
        {
            if (rc_ == 0) module().lock();
            return InterlockedIncrement(&rc_);
        }

        STDMETHOD_(ULONG, Release)()
        {
            size_t rc = InterlockedDecrement(&rc_);
            if (rc == 0) {
                try {
                    delete static_cast<C *>(this);
                }
                COMET_CATCH_UNKNOWN( L"Release", IID_IUnknown, bstr_t());
                module().unlock();
            }
            return rc;
        }

        STDMETHOD(QueryInterface)(REFIID riid, void **pv)
        {
            if(riid == IID_IUnknown)
            {
                *pv=get_inner();
                AddRef();
                return S_OK;
            }
            return static_cast<C *>(this)->QueryInterfaceInternal(riid, pv);
        }

        IUnknown *get_inner() { return  static_cast<IUnknown *>(this); }

        private:
            long rc_;
    };

    /** Provides the outer-unknown for aggregation.
     * This unkonwn points to the agregating unknown if aggregation occured, or
     * to the aggregate_inner_unknown class if it didn't.
     * This also implements the interfaces and QueryInterface.
     */
    template<typename T>
    class aggregate_outer_unknown : public implement_internal_qi<
        typelist::append<T, make_list<impl::interface_wrapper<ISupportErrorInfo> >::result >
            >
    {
    public:
        aggregate_outer_unknown(): outer_(NULL) {}

        STDMETHOD_(ULONG, AddRef)()
        {
            //assert(outer_!=NULL);
            return outer_->AddRef();
        }

        STDMETHOD_(ULONG, Release)()
        {
            //assert(outer_!=NULL);
            return outer_->Release();
        }
        STDMETHOD(QueryInterface)(REFIID riid, void** ppv)
        {
            //assert(outer_!=NULL);
            return outer_->QueryInterface(riid, ppv);
        }
        STDMETHOD(InterfaceSupportsErrorInfo)(REFIID)
        {
            return S_OK;
        }
        void set_outer_(IUnknown *outer){ outer_ = outer;}


    private:
        IUnknown *outer_;
    };
    //@}

    namespace impl {

        template<typename T> class ATL_NO_VTABLE aggregateable_object_aux : public aggregate_outer_unknown<T>, public aggregate_inner_unknown<aggregateable_object_aux<T> >
        {
        public:
            aggregateable_object_aux()
            {
                set_outer_(static_cast<IUnknown *>(static_cast< aggregate_inner_unknown<impl::aggregateable_object_aux<T> > *>(this)));
            }

        protected:
            virtual ~aggregateable_object_aux() {}
            friend struct aggregate_inner_unknown<aggregateable_object_aux<T> >;
        private:
            // non-copyable
            aggregateable_object_aux(const aggregateable_object_aux&);
            aggregateable_object_aux& operator=(const aggregateable_object_aux&);
        };

    }

    /*!\addtogroup Objects
     */
    //@{
    /*! \class aggregateable_object server.h comet/server.h
        Implement a user aggregateable object.
        \code
            class my_class : public aggregateable_object< IFooImpl<my_class>, IBarImpl<myclass> >
            {
               ...
            };
        \endcode
        \sa handle_exception_default IProvideClassInfoImpl simple_object
     */
    template<COMET_LIST_TEMPLATE> class ATL_NO_VTABLE aggregateable_object : public impl::aggregateable_object_aux< typename make_list<COMET_LIST_ARG_1>::result >
    {
        public:
        enum { factory_type = impl::ft_aggregateable };
    };

    /*! \class simple_object  server.h comet/server.h
       A simple reference counted COM object.
        \code
            class my_class : public simple_object< IFooImpl<my_class>, IBarImpl<myclass> >
            {
               ...
            };
        \endcode
        \sa handle_exception_default IProvideClassInfoImpl aggregateable_object
    */
    template<COMET_LIST_TEMPLATE> class ATL_NO_VTABLE simple_object : public impl::simple_object_aux< typename make_list<COMET_LIST_ARG_1>::result >
    {
        public:
        enum { factory_type = impl::ft_standard };
    };
    //@}

    namespace impl {



    template<typename T> class ATL_NO_VTABLE static_object_aux :
        public implement_qi< typelist::append< T, make_list<impl::interface_wrapper<ISupportErrorInfo> >::result > >
    {
    public:
        STDMETHOD_(ULONG, AddRef)()
        {
            module().lock();
            return 2;
        }

        STDMETHOD_(ULONG, Release)()
        {
            module().unlock();
            return 1;
        }

        STDMETHOD(InterfaceSupportsErrorInfo)(REFIID)
        {
            return S_OK;
        }
    protected:
        static_object_aux() {}
        virtual ~static_object_aux() {}
    };

    }

    /*!\addtogroup Objects
     */
    //@{
    /*! \class static_object  server.h comet/server.h
        A simple static allocated COM object.
        \code
            class my_class : public static_object< IFooImpl<my_class>, IBarImpl<myclass> >
            {
               ...
            };
        \endcode
     *    \sa handle_exception_default IProvideClassInfoImpl
    */
    template<COMET_LIST_TEMPLATE> class ATL_NO_VTABLE static_object : public impl::static_object_aux< typename make_list<COMET_LIST_ARG_1>::result >
    {
    };

    /*! \class singleton_object  server.h comet/server.h
        A simple singleton allocated COM object.
        \code
            class my_class : public singleton_object< IFooImpl<my_class>, IBarImpl<myclass> >
            {
               ...
            };
        \endcode
     *    \sa handle_exception_default IProvideClassInfoImpl
    */
    template<COMET_LIST_TEMPLATE> class ATL_NO_VTABLE singleton_object : public impl::static_object_aux< typename make_list<COMET_LIST_ARG_1>::result >
    {
        public:
        enum { factory_type = impl::ft_singleton };
        void set_dispose_command_( impl::cmd_t *) { }
    };


    /*! \class embedded_object server.h comet/server.h
     *
     *    \sa handle_exception_default IProvideClassInfoImpl
     *    \todo documentation
     */
    template<typename PARENT, COMET_LIST_TEMPLATE> class ATL_NO_VTABLE embedded_object :
        public implement_qi< typelist::append< typename make_list<COMET_LIST_ARG_1>::result, typename make_list<impl::interface_wrapper<ISupportErrorInfo> >::result > >
    {
    public:
        STDMETHOD_(ULONG, AddRef)()
        {
            return parent_->AddRef();
        }

        STDMETHOD_(ULONG, Release)()
        {
            return parent_->Release();
        }

        STDMETHOD(InterfaceSupportsErrorInfo)(REFIID)
        {
            return S_OK;
        }

    protected:
        explicit embedded_object(PARENT* parent) : parent_(parent) {}

        PARENT* get_parent() const { return parent_; }

        typedef embedded_object base_class;
    private:
        PARENT* parent_;

        // non-copyable
        embedded_object(const embedded_object&);
        embedded_object& operator=(const embedded_object&);
    };

    /*! \class embedded_object2 server.h comet/server.h
     *
     *    \sa handle_exception_default IProvideClassInfoImpl
     *    \todo documentation
     */
    template<typename PARENT, COMET_LIST_TEMPLATE> class ATL_NO_VTABLE embedded_object2 :
        public implement_qi< typelist::append< typename make_list<COMET_LIST_ARG_1>::result, make_list<impl::interface_wrapper<ISupportErrorInfo> >::result > >
    {
    public:
        STDMETHOD_(ULONG, AddRef)()
        {
            if (rc_ == 0) module().lock();
            long r = InterlockedIncrement(&rc_);
            if (is_connected_) return parent_->AddRef();
            return r;
        }

        STDMETHOD_(ULONG, Release)()
        {
            size_t rc = InterlockedDecrement(&rc_);

            if (is_connected_) return parent_->Release();

            if (rc == 0) {
                try {
                    delete this;
                }
                COMET_CATCH_UNKNOWN( L"Release", IID_IUnknown, bstr_t());
                module().unlock();
            }
            return rc;
        }

        STDMETHOD(InterfaceSupportsErrorInfo)(REFIID)
        {
            return S_OK;
        }

        void disconnect()
        {
            // todo make thread safe!

            if (is_connected_) {
                is_connected_ = false;

                for (long i=0; i<rc_; ++i) parent_->Release();

                // Provoke destruction if necessary.
                AddRef();
                Release();
            }
        }

    protected:
        explicit embedded_object2(PARENT* parent) : parent_(parent), rc_(0), is_connected_(true) {}

        virtual ~embedded_object2() {}

        PARENT* get_parent() const { return parent_; }

        typedef embedded_object2 base_class;
    private:
        PARENT* parent_;
        bool is_connected_;
        long rc_;

        // non-copyable
        embedded_object2(const embedded_object2&);
        embedded_object2& operator=(const embedded_object2&);
    };
    //@}

    /// Base class for class factories.
    template<typename T, bool LOCK_MODULE> class class_factory_base : public IClassFactory
    {
    public:
        STDMETHOD_(ULONG, AddRef)()
        {
            if (LOCK_MODULE) module().lock();
            return 2;
        }

        STDMETHOD_(ULONG, Release)()
        {
            if (LOCK_MODULE) module().unlock();
            return 1;
        }

        STDMETHOD(QueryInterface)(REFIID riid, void **pv)
        {
            if (!pv) return E_POINTER;
            *pv = 0;
            if (riid == IID_IClassFactory) *pv = static_cast<IClassFactory*>(this);
            if (riid == IID_IUnknown) *pv = this;

            if (*pv == NULL) return E_NOINTERFACE;

            AddRef();
            return S_OK;
        }

        STDMETHOD(LockServer)(BOOL bLock)
        {
            if (bLock)
                module().lock();
            else
                module().unlock();
            return S_OK;
        }
    protected:
        HRESULT handle_exception( const bstr_t &src )
        {
#ifndef COMET_DISABLE_EXCEPTION_RETHROW_CATCH
            return comet_exception_handler<true>::rethrow( source_info_t( src, IID_IUnknown, L"ClassFactory") );
#else // COMET_DISABLE_EXCEPTION_RETHROW_CATCH
            return comet_exception_handler<true>::catcher_hr(E_FAIL, source_info_t(src, IID_IUnknown, L"ClassFactory") );
#endif // COMET_DISABLE_EXCEPTION_RETHROW_CATCH
        }
    };

    //! Basic class-factory.
    template<typename T, bool LOCK_MODULE> class class_factory : public class_factory_base<T, LOCK_MODULE>
    {
    public:
        STDMETHOD(CreateInstance)(::IUnknown *pUnkOuter, REFIID riid, void **ppv)
        {
            if (pUnkOuter) return CLASS_E_NOAGGREGATION;
            *ppv = 0;

            if (!LOCK_MODULE) module().lock();

            T::coclass_type* t;
            try {
                t = new T;
            }
            catch (...)
            {
                if (!LOCK_MODULE) module().unlock();
                return handle_exception( bstr_t(L"CreateInstance(") + bstr_t(uuid_t::create_const_reference(riid),true ) + ")" );
            }

            t->AddRef();
            HRESULT hr = t->QueryInterface(riid, ppv);
            t->Release();

            if (!LOCK_MODULE) module().unlock();

            return hr;
        }

    };

    //! Class factory for aggregateable objects.
    template<typename T, bool LOCK_MODULE> class class_factory_agg : public class_factory_base<T, LOCK_MODULE>
    {
        STDMETHOD(CreateInstance)(::IUnknown *pUnkOuter, REFIID riid, void **ppv)
        {
            *ppv = 0;

            if (!LOCK_MODULE) module().lock();

            T* t;
            try {
                t = new T;
            }
            catch (...)
            {
                if (!LOCK_MODULE) module().unlock();
                return handle_exception( bstr_t(L"CreateInstance(") + bstr_t(uuid_t::create_const_reference(riid),true ) + ")" );
            }
            if(pUnkOuter!=NULL)
            {
                if(riid!=IID_IUnknown)
                {
                    if (!LOCK_MODULE) module().unlock();
                    return CLASS_E_NOAGGREGATION;
                }
                t->set_outer_(pUnkOuter);
            }

            t->get_inner()->AddRef();
            HRESULT hr = t->get_inner()->QueryInterface(riid, ppv);
            t->get_inner()->Release();

            if (!LOCK_MODULE) module().unlock();

            return hr;
        }
    };

    /// Class factory for singletons.
    template<typename T, bool LOCK_MODULE> class class_factory_singleton : public class_factory_base<T, LOCK_MODULE>
    {
        public:
            class_factory_singleton() : obj_(NULL), primed_(0)
            { }

            STDMETHOD(CreateInstance)(::IUnknown *pUnkOuter, REFIID riid, void **ppv)
            {
                *ppv = 0;
                if(pUnkOuter!=NULL)
                    return CLASS_E_NOAGGREGATION;

                if (!LOCK_MODULE) module().lock();

                // Add this to the list to get the object disposed on
                // moudle terminate.
                if (0==InterlockedExchange(&primed_, 1))
                {
                    module().add_object_to_dispose( create_object_disposer(this) );
                }

                if (obj_ == NULL )
                {

                    try {
                        // Create a new instance, but make sure only one is
                        // used if there is contention.
                        T *newVal = new T;
                        newVal->set_dispose_command_( create_object_disposer(this));
#ifndef InterlockedCompareExchangePointer
                        InterlockedCompareExchange( &(void *&)obj_, newVal, 0);
#else
                        InterlockedCompareExchangePointer( &(void *&)obj_, newVal, 0);
#endif
                    }
                    catch (...)
                    {
                        if (!LOCK_MODULE) module().unlock();
                        return handle_exception( bstr_t(L"CreateInstance(") + bstr_t(uuid_t::create_const_reference(riid),true ) + ")" );
                    }
                }

                HRESULT hr = obj_->QueryInterface(riid, ppv);

                if (!LOCK_MODULE) module().unlock();

                return hr;
            }

            // Called by object_disposer on module terminate.
            void object_dispose()
            {
                if (obj_ != NULL)
                {
                    T *t= obj_;
                    obj_ = NULL;
                    CoDisconnectObject(t->get_unknown(), 0);
                    delete t;
                }
            }

            T *obj_;
            long primed_;
        private:
    };


    /*!\addtogroup Objects
     */
    //@{
    /** Enumerate thread_model.
      */
    namespace thread_model {
        /** Enumerate thread_model.
          */
        enum thread_model_t {
            Apartment = 0, ///< Apartment threaded.
            Free      = 1, ///< Free threaded.
            Both      = 2, ///< Both threaded (either apartment or free)
            Neutral   = 3  ///< Netural threaded.
        };
    }
    //@}

    //! Provide static string description of thread models.
    template<long tm>
    struct tm_properties;

    template<> struct tm_properties<thread_model::Apartment>
    {
        COMET_FORCEINLINE static const TCHAR *string() { return _T("Apartment"); }
    };

    template<> struct tm_properties<thread_model::Free>
    {
        COMET_FORCEINLINE static const TCHAR *string() { return _T("Free"); }
    };

    template<> struct tm_properties<thread_model::Both>
    {
        COMET_FORCEINLINE static const TCHAR *string() { return _T("Both"); }
    };

    template<> struct tm_properties<thread_model::Neutral>
    {
        COMET_FORCEINLINE static const TCHAR *string() { return _T("Neutral"); }
    };

    /*! \defgroup Interfaces Interface implementations.
     */
    //@{
    /*! \class IProvideClassInfoImpl server.h comet/server.h
     *   This class provides IProvideClassInfo interfaces for simple, static, embeded  or
     *   other custom classes.
     *   IProvideClassInfoImpl is used by coclass and aggregateable_coclass by
     *   default.
     *   This class also provides class name information to the exception
     *   handler.
     *  \sa default_exception_handler_traits
     */
    template<typename COCLASS> class IProvideClassInfoImpl : public IProvideClassInfo, public handle_exception_default<COCLASS>
    {
    public:
        typedef IProvideClassInfo interface_is;
        STDMETHOD(GetClassInfo)(ITypeInfo **ppTypeInfo) throw()
        {
            if(!ppTypeInfo) return E_POINTER;
            *ppTypeInfo = 0;
            ITypeLib *pTypeLib;
            typedef typename COCLASS::type_library COCLASS_typelib;
            HRESULT hr = typelibrary_loader<COCLASS_typelib>::load(&pTypeLib);

            if(FAILED(hr)) return hr;
            hr = pTypeLib->GetTypeInfoOfGuid(uuidof<COCLASS>(), ppTypeInfo);
            pTypeLib->Release();
            return hr;
        }
    };
    /** \struct coclass server.h comet/server.h
      * Implement a standard coclass with interfaces defined in TypeLibrary and
      * implemented within the class \p T and thread model \p TM, followed by a
      * list of extra interfaces to implement.
      * Provides IProvideClassInfo and ISupportErrorInfo as standard.
      * \code
       template<>
       class coclass_implementation<CoMyClass> : public coclass<CoMyClass>
       {
          // ...
       };
      * \endcode
      * The thread model can be specified for the coclass, and as well, extra
      * interfaces can be implemented by specifying them at the end.
      * \code
       template<>
       class coclass_implementation<CoMyClass> : public coclass<CoMyClass, thread_model::Apartment, >
       {
          // ...
       };
      * \endcode
      * \sa FTM aggregates thread_model::thread_model_t
      */

/*    template<typename T, enum thread_model::thread_model_t TM = thread_model::Apartment> struct ATL_NO_VTABLE coclass : public impl::simple_object_aux< typelist::append<typename T::interface_impls,typename  make_list<IProvideClassInfoImpl<T> >::result > > {
        enum { thread_model = TM };
        static const TCHAR* get_progid() { return 0; }
    };*/
    //@}

    /*!\addtogroup Objects
     */
    //@{
    template<typename T, enum thread_model::thread_model_t TM = thread_model::Apartment, COMET_LIST_TEMPLATE> struct ATL_NO_VTABLE coclass : public impl::simple_object_aux< typelist::append< typelist::append< typename T::interface_impls, typename make_list<COMET_LIST_ARG_1>::result>, typename make_list<IProvideClassInfoImpl<T> >::result > >  {
        typedef coclass coclass_type;
        enum { factory_type = impl::ft_standard };
        enum { thread_model = TM };
        static const TCHAR* get_progid() { return 0; }
    };
    //@}

//    COMET_WRAP_EACH_DECLARE(aggregates_interface);

    namespace impl {
        template<typename T1, typename T2, typename T3>
            struct append3
        {
            typedef typelist::append<T1, T2> first_two;
            typedef typelist::append<first_two, T3> list;
        };
    }
#ifdef NOTNOW
    /** \struct coclass_aggregates  server.h comet/server.h
      * Implement a coclass with interfaces defined in TypeLibrary and
      * implemented by the class, as well as supporting specified aggregated interfaces.
      * Provides IProvideClassInfo and ISupportErrorInfo as standard.
      * \code
      * template<>
      * class coclass_implementation<CoMyClass>
      *     : public coclass_aggregates<CoMyClass,  make_list< IAggInterface1 >::result>
      * {
      *     public:
      *     coclass_implementation<CoMyClass>()
      *     {
      *         aggregates_interface<IAggInterface1>::set_aggregate(com_ptr<IUnknown>(CoClassForAggIFace,com_cast(this)));
      *     }
      *    // ...
      * };
      * \endcode
      * \note This should provide IProvideMultipleClassInfo.
      */

/*    template<typename T, typename AGG_LST, enum thread_model::thread_model_t TM = thread_model::Apartment>
        struct ATL_NO_VTABLE coclass_aggregates : public impl::simple_object_aux<impl::append3< typename T::interface_impls, COMET_WRAP_EACH(aggregates_interface, AGG_LST), make_list<IProvideClassInfoImpl<T> >::result > >
    {
        enum { thread_model = TM };
        static const TCHAR* get_progid() { return 0; }
    };*/
#endif  //0

    /*!\addtogroup Objects
     */
    //@{
    /** \struct aggregateable_coclass  server.h comet/server.h
      * Implement an aggregateable coclass with interfaces defined in TypeLibrary and
      * implemented within the class T.
      * Provides IProvideClassInfo and ISupportErrorInfo as standard.
      * \code
      * template<>
      * class coclass_implementation<CoMyClass>
      *     : public aggregateable_coclass<CoMyClass>
      * {
      *     // ....
      * };
      * \endcode
      */

    template<typename T, enum thread_model::thread_model_t TM = thread_model::Apartment>
    struct ATL_NO_VTABLE aggregateable_coclass : public impl::aggregateable_object_aux< typelist::append< typename T::interface_impls, typename make_list<IProvideClassInfoImpl<T> >::result > >
    {
        typedef aggregateable_coclass coclass_type;
        enum { thread_model = TM };
        enum { factory_type = impl::ft_aggregateable };
        static const TCHAR* get_progid() { return 0; }
        aggregateable_coclass() {}
    protected:
        ~aggregateable_coclass() {}
    private:
        aggregateable_coclass(const aggregateable_coclass&);
        aggregateable_coclass& operator=(const aggregateable_coclass&);
    };

    /** \class singleton_coclass  server.h comet/server.h
      * Implement a singleton coclass within interfaces defined in TypeLibrary
      * and implemented with the class T.
      * Gets cleaned up when dll unloads.
      * Provides IProvideClassInfo and ISupportErrorInfo as standard.
     \code
        class coclass_implementation<CoMyClass>
        : public singleton_coclass<CoMyClass>
        {
            // ....
        };
     \endcode
     */
    template< typename T, enum thread_model::thread_model_t TM = thread_model::Apartment>
    struct ATL_NO_VTABLE singleton_coclass : public impl::static_object_aux<
            typelist::append< typename T::interface_impls, typename make_list<typename IProvideClassInfoImpl<T> >::result >
        >
    {
        typedef singleton_coclass coclass_type;
        enum { thread_model = TM };
        enum { factory_type = impl::ft_singleton };
        void set_dispose_command_( impl::cmd_t *) { }
        static const TCHAR* get_progid() { return 0; }
        singleton_coclass() {}
        ~singleton_coclass() {}

    private:
        singleton_coclass(const singleton_coclass&);
        singleton_coclass& operator=(const singleton_coclass&);
    };

    template< typename T, enum thread_model::thread_model_t TM = thread_model::Apartment>
    struct ATL_NO_VTABLE singleton_autorelease_coclass : public impl::static_object_aux<
            typelist::append< typename T::interface_impls, typename make_list<typename IProvideClassInfoImpl<T> >::result >
        >
    {
        singleton_autorelease_coclass() : rc_(0), dispose_(0) {}
        ~singleton_autorelease_coclass()
        { delete dispose_; }

        STDMETHOD_(ULONG, AddRef)()
        {
            if (rc_ == 0) module().lock();
            return InterlockedIncrement(&rc_);
        }

        STDMETHOD_(ULONG, Release)()
        {
            LONG rc = InterlockedDecrement(&rc_);
            if (rc == 0) {
                try {
                    if (dispose_!=NULL)
                        dispose_->cmd();

                } COMET_CATCH_UNKNOWN( L"Release", IID_IUnknown, bstr_t());
                module().unlock();
            }
            return rc;
        }

        typedef singleton_autorelease_coclass coclass_type;
        enum { thread_model = TM };
        enum { factory_type = impl::ft_singleton };
        static const TCHAR* get_progid() { return 0; }

        void set_dispose_command_( impl::cmd_t *p)
        {
            delete dispose_;
            dispose_ = p;
        }

    private:
        singleton_autorelease_coclass(const singleton_autorelease_coclass&);
        singleton_autorelease_coclass& operator=(const singleton_autorelease_coclass&);
        long rc_;
        impl::cmd_t *dispose_;
    };
    //@}


    namespace impl {
        template<typename T>
        class reghelper_t
        {
            // convert_string is overloaded to select the correct
            // behaviour based on argument type.
            // dest_size is the number of bytes, not the number of characters.
            COMET_FORCEINLINE static void convert_string(wchar_t *dest, size_t dest_size, const wchar_t *src)
            {
                ::memcpy(dest, src, dest_size);
            }

            COMET_FORCEINLINE static void convert_string(char *dest, size_t dest_size, const wchar_t *src)
            {
                ::WideCharToMultiByte(CP_ACP, 0, src, -1, dest, dest_size, 0, 0);
            }

            static void removekey(const tstring& key);
            static void addkey(const tstring& key, const tstring& valueName, const tstring& value);
            static void addkey(const tstring& key, const tstring& value);
            static tstring StringFromUUID(REFCLSID rclsid);
            static void updatekey(bool unregister, const tstring &key, const tstring &valueName, const tstring &value);
            static void updatekey(bool unregister, const tstring &key, const tstring &value);
            public:
            static void update_coclass(bool unregister,
                const CLSID &rclsid,
                const TCHAR *filename,
                const TCHAR *thread_model,
                const TCHAR *coclass_name,
                const TCHAR *progid,
                unsigned long version,
                const GUID &rlibid,
                bool inproc_server,
                const GUID* appid);
        }; // reghelper_t

        template<typename T>
        void reghelper_t<T>::removekey(const tstring& key)
        {
            regkey rkey(HKEY_CLASSES_ROOT);
            rkey.delete_subkey_nothrow(key);
        }

        template<typename T>
        void reghelper_t<T>::addkey(const tstring& key, const tstring& valueName, const tstring& value)
        {
            regkey rkey(HKEY_CLASSES_ROOT);
            rkey.create(key)[valueName] = value;
        }

        template<typename T>
        void reghelper_t<T>::addkey(const tstring& key, const tstring& value)
        {
            regkey rkey(HKEY_CLASSES_ROOT);
            rkey.create(key)[_T("")] = value;
        }

        template<typename T>
        tstring reghelper_t<T>::StringFromUUID(REFCLSID rclsid)
        {
            wchar_t *ws;
            ::StringFromCLSID(rclsid, &ws);
            size_t num_chars = wcslen(ws) + 1;
            size_t bytes = num_chars * sizeof (TCHAR);
            TCHAR *s = static_cast<TCHAR*>(_alloca(bytes));
            convert_string(s, bytes, ws);
            CoTaskMemFree(ws);
            return s;
        }

        template<typename T>
        void reghelper_t<T>::updatekey(bool unregister, const tstring &key, const tstring &valueName, const tstring &value)
        {
            if(unregister)
                removekey(key);
            else
                addkey(key, valueName, value);
        }

        template<typename T>
        void reghelper_t<T>::updatekey(bool unregister, const tstring &key, const tstring &value)
        {
            if(unregister)
                removekey(key);
            else
                addkey(key, value);
        }

        template<typename T>
            void reghelper_t<T>::update_coclass(bool unregister,
            const CLSID &rclsid,
            const TCHAR *filename,
            const TCHAR *thread_model,
            const TCHAR *coclass_name,
            const TCHAR *progid,
            unsigned long version,
            const GUID &rlibid,
            bool inproc_server,
            const GUID* /*appid*/)
        {
            tstring clsid = StringFromUUID(rclsid);
            tstring name = _T("CLSID\\") + clsid;
            tstring typelib = StringFromUUID(rlibid);
            // On WinNT/Win2000, subkeys must be deleted before parent keys.
            // Therefore, be sure to specify changes to subkeys before
            // changes to parent keys - otherwise the server will not
            // cleanly remove all keys when it is unregistered.
            if (inproc_server)
            {
                updatekey(unregister, name + _T("\\InprocServer32"), filename);
                updatekey(unregister, name + _T("\\InprocServer32"), _T("ThreadingModel"), thread_model);
            }
            else
            {
                updatekey(unregister, name + _T("\\LocalServer32"), filename);
                updatekey(unregister, name + _T("\\Programmable"), _T(""));
            }
            updatekey(unregister, name + _T("\\TypeLib"), typelib);
//            updatekey(unregister, name, coclass_name);

            if (progid != 0)
            {
                tstring prgid(progid);

                if (version != 0)
                {
                    TCHAR buffer[35];
#if _MSC_VER >= 1400
                    _ultot_s(version, buffer, 35, 10);
                    tstring prgid_ver(prgid + _T(".") + buffer);
#else
                    tstring prgid_ver(prgid + _T(".") + _ultot(version, buffer, 10));
#endif

                    updatekey(unregister, name + _T("\\ProgID"), prgid_ver);
                    updatekey(unregister, prgid + _T("\\CurVer"), prgid_ver);
                    updatekey(unregister, prgid_ver + _T("\\CLSID"), clsid);
                    updatekey(unregister, prgid_ver, coclass_name);
                }
                else
                {
                    updatekey(unregister, name + _T("\\ProgID"), prgid);
                }

                updatekey(unregister, prgid + _T("\\CLSID"), clsid);
                updatekey(unregister, prgid, coclass_name);
            }

            updatekey(unregister, name, coclass_name);

//            if (progid != 0) {
//                updatekey(unregister, name + _T("\\ProgID"), progid);
//                updatekey(unregister, tstring(progid) + _T("\\CLSID"), clsid);
//                updatekey(unregister, progid, coclass_name);
//            }
        }

        typedef reghelper_t<void> reghelper;

#ifndef COMET_PARTIAL_SPECIALISATION

        template<bool undefined>
        struct entry_builder;

        template<typename T> struct THE_FOLLOWING_COCLASS_HAS_NOT_BEEN_IMPLEMENTED;
        template<> struct THE_FOLLOWING_COCLASS_HAS_NOT_BEEN_IMPLEMENTED<nil> {};

        template<> struct entry_builder<true>
        {
            template<typename CLASS>
            struct registration
            {
                COMET_FORCEINLINE static void perform(const TCHAR*, bool, bool, const GUID* )
                {
#ifndef COMET_ALLOW_UNIMPLEMENTED_COCLASSES
                    THE_FOLLOWING_COCLASS_HAS_NOT_BEEN_IMPLEMENTED<CLASS> x;
#endif
                }
            };

            template<typename CLASS, bool LOCK_MODULE>
            struct factory
            {
                COMET_FORCEINLINE static ::IUnknown* get(const CLSID&)
                {
                    return 0;
                }
            };
        }; // entry_builder<true>

        template<> struct entry_builder<false>
        {
            template<typename CLASS>
            struct registration
            {
                static void perform(const TCHAR *filename, bool unregister, bool inproc_server, const GUID* appid)
                {
                    if(unregister) {
                        try {
                            custom_registration<CLASS>().on_unregister(filename);
                        }
                        catch(...)
                        {

                        }
                    }

                    reghelper::update_coclass(unregister,
                        uuidof<CLASS>(),
                        filename,
                        tm_properties<coclass_implementation<CLASS>::thread_model>::string(),
                        CLASS::name(),
                        coclass_implementation<CLASS>::get_progid(),
                        CLASS::major_version,
                        uuidof<CLASS::type_library>(),
                        inproc_server,
                        appid);

                    if(!unregister)
                        custom_registration<CLASS>().on_register(filename);
                } // perform
            }; // registration


            template<int>
            struct factory_builder { };

            template<> struct factory_builder<ft_standard>
            {
                template<typename CLASS, bool LOCK_MODULE>
                struct factory
                {
                    typedef class_factory<CLASS, LOCK_MODULE> is_factory;
                };
            };
            template<> struct factory_builder<ft_aggregateable>
            {
                template<typename CLASS, bool LOCK_MODULE>
                struct factory
                {
                    typedef class_factory_agg<CLASS, LOCK_MODULE> is_factory;
                };
            };
            template<> struct factory_builder<ft_singleton>
            {
                template<typename CLASS, bool LOCK_MODULE>
                struct factory
                {
                    typedef class_factory_singleton<CLASS, LOCK_MODULE> is_factory;
                };
            };

            template<typename CLASS, bool LOCK_MODULE>
            struct factory_type
            {
                typedef COMET_STRICT_TYPENAME factory_builder< CLASS::factory_type >::factory<CLASS, LOCK_MODULE>::is_factory factory;
            };
            template<typename CLASS, bool LOCK_MODULE>
            struct factory
            {
                typedef typename factory_type< coclass_implementation<CLASS>, LOCK_MODULE >::factory CLASS_FACTORY;

                COMET_FORCEINLINE static ::IUnknown* get(const CLSID& clsid)
                {
                    static CLASS_FACTORY class_factory_;

                    // NB: This might cause problems if CLASS_FACTORY
                    // implemented more than one interface. If so, this logic will
                    // have to be changed to inline the QueryInterface call instead.
                    if (clsid == uuidof<CLASS>())
                        return &class_factory_;
                    return 0;
                } // get
            }; // factory
        }; // entry_builder<false>

        template<typename CLASS, bool FACTORY_LOCK_MODULE>
        struct coclass_table_entry
        {
            // We use sizeof here to determine if there is a specialization of coclass_implementation.
            // It is relying on the fact that any real implementation of coclass_implementation
            // must be at least sizeof IUnknown, whereas the default sizeof implementation
            // is always just a dummy class.
            enum { is_undefined = sizeof (coclass_implementation<CLASS>) == sizeof( coclass_implementation<nil>) };

            typedef typename entry_builder<is_undefined>::factory<CLASS, FACTORY_LOCK_MODULE> factory;
            typedef typename entry_builder<is_undefined>::registration<CLASS> registration;
        };

#else // COMET_PARTIAL_SPECIALISATION
        template<bool undefined, typename CLASS, bool FACTORY_LOCK_MODULE>
        struct entry_builder
        {
            typedef nil factory;
            typedef nil registration;
        };

        template<typename CLASS, bool FACTORY_LOCK_MODULE> struct entry_builder<true, CLASS, FACTORY_LOCK_MODULE>
        {
            struct registration
            {
                COMET_FORCEINLINE static void perform(const TCHAR*, bool, bool, const GUID* ) {}
            };

            struct factory
            {
                COMET_FORCEINLINE static ::IUnknown* get(const CLSID&)
                {
                    return 0;
                }
            };
        }; // entry_builder<true>

        template<int, typename CLASS, bool LOCK_MODULE>
        struct factory_builder_aux { };

        template<typename CLASS, bool LOCK_MODULE> struct factory_builder_aux<ft_standard,CLASS, LOCK_MODULE>
        {
            typedef class_factory<CLASS, LOCK_MODULE> is_factory;
        };
        template<typename CLASS, bool LOCK_MODULE> struct factory_builder_aux<ft_aggregateable,CLASS, LOCK_MODULE>
        {
            typedef class_factory_agg<CLASS, LOCK_MODULE> is_factory;
        };
        template<typename CLASS, bool LOCK_MODULE> struct factory_builder_aux<ft_singleton,CLASS, LOCK_MODULE>
        {
            typedef class_factory_singleton<CLASS, LOCK_MODULE> is_factory;
        };


        template<typename CLASS, bool FACTORY_LOCK_MODULE> struct entry_builder<false, CLASS, FACTORY_LOCK_MODULE>
        {
            struct registration
            {
                static void perform(const TCHAR *filename, bool unregister, bool inproc_server, const GUID* appid)
                {
                    if(unregister) {
                        try {
                            custom_registration<CLASS>().on_unregister(filename);
                        } catch(...) {}
                    }

                    reghelper::update_coclass(unregister,
                        uuidof<CLASS>(),
                        filename,
                        tm_properties<coclass_implementation<CLASS>::thread_model>::string(),
                        CLASS::name(),
                        coclass_implementation<CLASS>::get_progid(),
                        CLASS::major_version,
                        uuidof<CLASS::type_library>(),
                        inproc_server,
                        appid);

                    if(!unregister)
                        custom_registration<CLASS>().on_register(filename);
                } // perform
            }; // registration

            struct factory_type
            {
                enum {type_ = coclass_implementation<CLASS>::factory_type };
                typedef typename factory_builder_aux< type_, coclass_implementation<CLASS>, FACTORY_LOCK_MODULE >::is_factory factory;
            };
            struct factory
            {
                typedef typename factory_type::factory CLASS_FACTORY;

                COMET_FORCEINLINE static ::IUnknown* get(const CLSID& clsid)
                {
                    static CLASS_FACTORY class_factory_;

                    // NB: This might cause problems if CLASS_FACTORY
                    // implemented more than one interface. If so, this logic will
                    // have to be changed to inline the QueryInterface call instead.
                    if (clsid == uuidof<CLASS>())
                        return &class_factory_;
                    return 0;
                } // get
            }; // factory
        }; // entry_builder<false>

        template<typename CLASS, bool FACTORY_LOCK_MODULE>
        struct coclass_table_entry
        {
            // We use sizeof here to determine if there is a specialization of coclass_implementation.
            // It is relying on the fact that any real implementation of coclass_implementation
            // must be at least sizeof IUnknown, whereas the default sizeof implementation
            // is always just a dummy class.
            enum { is_undefined = (sizeof(coclass_implementation<CLASS>) == sizeof(coclass_implementation<nil>)) };

            typedef typename entry_builder<is_undefined,CLASS, FACTORY_LOCK_MODULE>::factory factory;
            typedef typename entry_builder<is_undefined,CLASS, FACTORY_LOCK_MODULE>::registration registration;
        };
#endif // COMET_PARTIAL_SPECIALISATION

    } // namespace impl

    /*! \addtogroup Server
     */
    //@{
    template<typename CLS_LIST, bool FACTORY_SHOULD_LOCK_MODULE = true>
    class coclass_table
    {
    public:
        COMET_FORCEINLINE static ::IUnknown* find(const CLSID& clsid)
        {
            ::IUnknown *ret = impl::coclass_table_entry<COMET_STRICT_TYPENAME CLS_LIST::head, FACTORY_SHOULD_LOCK_MODULE>::factory::get(clsid);
            return ret ? ret : coclass_table<COMET_STRICT_TYPENAME CLS_LIST::tail, FACTORY_SHOULD_LOCK_MODULE>::find(clsid);
        }

        COMET_FORCEINLINE static void registration(const TCHAR* filename, bool unregister, bool inproc_server = true, const GUID* appid = 0)
        {
            impl::coclass_table_entry<COMET_STRICT_TYPENAME CLS_LIST::head, FACTORY_SHOULD_LOCK_MODULE>::registration::perform(filename, unregister, inproc_server, appid);
            coclass_table<COMET_STRICT_TYPENAME CLS_LIST::tail, FACTORY_SHOULD_LOCK_MODULE>::registration(filename, unregister, inproc_server, appid);
        }
    };

    struct coclass_term
    {
        COMET_FORCEINLINE static ::IUnknown* find(const IID&)
        {
            return 0;
        }
        COMET_FORCEINLINE static void registration(const TCHAR*, bool, bool = true, const GUID* = 0) {}
    };

//    template<> class coclass_table<make_list<> > : public coclass_term {};
    template<> class coclass_table<nil, true> : public coclass_term {};
    template<> class coclass_table<nil, false> : public coclass_term {};

    enum { NO_EMBEDDED_TLB = 1};

    template<size_t FL>
    struct com_server_traits {
        enum { flags = FL };
        enum { embedded_tlb = !(flags & NO_EMBEDDED_TLB) };
    };
    /** Main COM DLL module implementation.
     * Called my the COMET_DECLARE_DLL_FUNCTIONS macro - provides implementation
     * for the DLL entry points.
     * \sa COMET_DECLARE_DLL_FUNCTIONS
     */
    template<typename TYPELIB, typename TRAITS = com_server_traits<0> > class com_server
    {
        typedef coclass_table<typename TYPELIB::coclasses, true> COCLASS_TABLE;
    public:
        static BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID);
        static HRESULT DllCanUnloadNow();
        static HRESULT DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv);
        static HRESULT DllRegisterServer();
        static HRESULT DllUnregisterServer();
    };
    //@}
    template<typename TYPELIB, typename TRAITS>
    BOOL WINAPI com_server<TYPELIB, TRAITS>::DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID)
    {
        if (dwReason == DLL_PROCESS_ATTACH)
        {
            module().instance(hInstance);
            DisableThreadLibraryCalls(hInstance);

            // initialize static variables in factory::get to avoid potential thread safety problem.
            ::IUnknown* cf = COCLASS_TABLE::find(IID_NULL);
            /* prevent warning */ cf;
        }
        else if (dwReason == DLL_PROCESS_DETACH)
        {
            module().shutdown();
        }
        return TRUE;
    }

    template<typename TYPELIB, typename TRAITS>
    HRESULT com_server<TYPELIB, TRAITS>::DllCanUnloadNow()
    {
        return module().rc() == 0 ? S_OK : S_FALSE;
    }

    template<typename TYPELIB, typename TRAITS>
    HRESULT com_server<TYPELIB, TRAITS>::DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
    {
        ::IUnknown* cf = COCLASS_TABLE::find(rclsid);
        if (cf == 0) return CLASS_E_CLASSNOTAVAILABLE;

        return cf->QueryInterface(riid, ppv);
    }

    namespace impl {
        template<size_t embedded>
            struct typelibrary_registration
        {
            static void convert(wchar_t *dest, const char *src)
            {
                size_t cs = 0;
#if _MSC_VER >=1400
                ::mbstowcs_s(&cs, dest, MAX_PATH, src, MAX_PATH);
#else
                cs = ::mbstowcs(dest,src, MAX_PATH);
#endif
            }
            static void convert(wchar_t *dest, const wchar_t *src)
            {
#if _MSC_VER >= 1400
                ::wcscpy_s(dest, MAX_PATH, src);
#else
                ::wcscpy(dest, src);
#endif
            }
            static HRESULT perform(const TCHAR *filename, bool unregister) throw()
            {
                wchar_t wfilename[MAX_PATH];
                convert(wfilename, filename);
                ITypeLib* tl;
                HRESULT hr = LoadTypeLib(wfilename, &tl);
                if (SUCCEEDED(hr)) {
                    if(unregister)    {
                        TLIBATTR *tla = 0;
                        hr = tl->GetLibAttr(&tla);
                        if (SUCCEEDED(hr)) {
                            hr = UnRegisterTypeLib(tla->guid, tla->wMajorVerNum, tla->wMinorVerNum, tla->lcid, tla->syskind);
                            tl->ReleaseTLibAttr(tla);
                        }
                    }
                    else
                        hr = RegisterTypeLib(tl, wfilename, 0);
                    tl->Release();
                }
                return hr;
            }
        };

        template<> struct typelibrary_registration<0>
        {
            COMET_FORCEINLINE static HRESULT perform(const TCHAR *, bool)
            {
                return S_OK;
            }
        };
    } // namespace impl

    template<typename TYPELIB, typename TRAITS>
    HRESULT com_server<TYPELIB, TRAITS>::DllRegisterServer()
    {
        TCHAR filename[MAX_PATH];

        GetModuleFileName(module().instance(), filename, MAX_PATH);

        {
            HRESULT hr = impl::typelibrary_registration<TRAITS::embedded_tlb>::perform(filename, false);
            if(FAILED(hr)) return SELFREG_E_TYPELIB;
        }

        try {
            COCLASS_TABLE::registration(filename, false);
        }
        catch (const com_error &e)
        {
            DllUnregisterServer();
            return impl::return_com_error(e);
        }
        catch (const std::exception &e)
        {
            DllUnregisterServer();
            ::OutputDebugStringA(e.what());
            return E_FAIL;
        }

        return S_OK;
    }

    template<typename TYPELIB, typename TRAITS>
    HRESULT com_server<TYPELIB, TRAITS>::DllUnregisterServer()
    {
        TCHAR filename[MAX_PATH];
        GetModuleFileName(module().instance(), filename, MAX_PATH);

        impl::typelibrary_registration<TRAITS::embedded_tlb>::perform(filename, true);

        COCLASS_TABLE::registration(filename, true);
        return S_OK;
    }
/** Declares the DLL Functions and passes them through to \a SERVER static
  * functions.
  * \sa com_server
  */

#define COMET_DECLARE_DLL_FUNCTIONS(SERVER)                                                     \
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID)                     \
{                                                                                               \
    return SERVER::DllMain(hInstance, dwReason, 0);                                             \
}                                                                                               \
                                                                                                \
STDAPI DllCanUnloadNow()                                                                        \
{                                                                                               \
    return SERVER::DllCanUnloadNow();                                                           \
}                                                                                               \
                                                                                                \
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)                             \
{                                                                                               \
    return SERVER::DllGetClassObject(rclsid, riid, ppv);                                        \
}                                                                                               \
                                                                                                \
STDAPI DllRegisterServer()                                                                      \
{                                                                                               \
    return SERVER::DllRegisterServer();                                                         \
}                                                                                               \
                                                                                                \
STDAPI DllUnregisterServer()                                                                    \
{                                                                                               \
    return SERVER::DllUnregisterServer();                                                       \
}

}

#endif
