/** \file
  * Connection-point implementations.
  * The thought behind the current implementation was to try and maintain
  * connections points accessible from the coclass through the member
  * connection_point.
  *
  * Where multiple connection-points are defined, the member is accessed thus:
  * \code
  *     connection_point_for<IEventInterface>::connection_point
  * \endcode
  * The default connection point wrapper implementations have a prefix of
  * \b Fire_ before the event name.
  */
/*
 * Copyright © 2000 Sofus Mortensen
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

#ifndef COMET_CP_H
#define COMET_CP_H

#include <comet/config.h>

#include <map>

#include <comet/server.h>
#include <comet/enum.h>

#pragma warning( push )
#pragma warning( disable : 4355 )

/** \page cometconnectionpoints Connection Points.
\section cometconnectionpointssource Connction Point Source
Connection points provide events for comet, and rely on tlb2h (see \ref tlb2husage) to generate
the implementation for each method on the interface.

Then comet::implement_cpc is used to provide IConnectionPointContainer
(which provides the method for finding a particular connection point.

The default implementation of a coclass will inherit of this method by
default, however they can be explicitly referenced to provide
alternate implementations.

The class comet::implement_cpc is templated to an interface list, which
can be constructed with comet::make_list.

\code
    class my_class : public simple_object< IFooImpl<my_class>, implement_cpc< make_list<IFooEvent, IBarEvent>::result > >
\endcode

This causes each of the interface in the list of interfaces to be wrapped by a
comet::connection_point_for class which provides the access syntax.

\code
    connection_point_for<IFooEvent>::connection_point.Fire_FooMethod( args );
\endcode

Note that \b Fire_ is prepended to each method name.

If there is only one connection point in the list, then the
connection_point_for namespace segregator is not required.

If there is no connection point implementation for the interface, then you should make
sure that it is referenced with a [source] tag by a coclass definition
in the library you are including, and that you are generating server
implementations or forcing it by prepending '*'  in a symbol file, see
the \ref wrappergensymbolformat.

\section cometconnectionpointsink Connction Point Sink

In order to prevent the circular references inherent in connection points, the
reference loop must be broken somehow.  This is best done by using a contained
member that calls back on the parent class.

The class sink_impl is best used in this circumstance as it contains code for calling
Advise and Unadvise. It inherits from static_object which is designed to be embedded in
a class, and does reference counting on the module and does not destruct itself.

This class is templated to the interface implementation desired, which will
mostly be the 'Impl' class for the interface, but might be a raw COM interface.

In this example we have a class 'MyCoclass' that is sinking the event interface IMyEventImpl which
has a single 'EventHappened' method.
\code
    class coclass_MyCoclass : public coclass<MyCoclass>
    {
        // Private Embedded sink implementation.
        struct sink_t  : sink_impl<IMyEventImpl>
        {
            sink_t(coclass_MyCoclass *Parent) :m_parent(Parent) {}
            inline void EventHappened( long param_Here ) { m_parent->sink_EventHappened(param_Here); }
            coclass_MyCoclass *m_parent; // Non-addref reference.
        } m_sink;
        public:
            coclass_MyCoclass() : m_sink(this) {}

            // Set the event source.
            void SetEventSource( const com_ptr<IUnknown> &unknown)
            {
                if (m_sink.is_advised())
                    m_sink.unadvise();
                m_sink.advise( unknown );
            }

            inline void sink_EventHappened( long param_Here )
            {
                // Event code goes here
            }
    };
\endcode

*/

namespace comet {

    template<typename Itf> class connection_point;

    /*! \addtogroup Interfaces
     */
    //@{
    /** \class simple_cpc cp.h comet/cp.h
      * Implement a simple connection-point container with a single
      * connection-point.
      * \param Itf The single connection-point interface.
      * \sa implement_cpc
      */
    template<typename Itf> class ATL_NO_VTABLE simple_cpc : public IConnectionPointContainer
    {
    public:
        typedef IConnectionPointContainer interface_is;

    protected:
        /// \name IConnectionPointContainer interface
        //@{
        STDMETHOD(EnumConnectionPoints)(IEnumConnectionPoints**)
        {
            return E_NOTIMPL;
        }

        STDMETHOD(FindConnectionPoint)(REFIID riid, IConnectionPoint** ppCP)
        {
            if (!ppCP) return E_POINTER;
            if (riid == uuidof<Itf>()) {
                *ppCP = &connection_point;
                (*ppCP)->AddRef();
                return S_OK;
            }
            return CONNECT_E_NOCONNECTION;
        }
        //@}

    protected:
        simple_cpc() : connection_point(this) {}

        connection_point<Itf> connection_point;
    };


    /** \class connection_point_for cp.h comet/cp.h
     * Provide access to implementation for a connection point.
     * \relates implement_cpc
     */
    template<typename Itf> class connection_point_for
    {
    public:
        connection_point_for(::IUnknown *self) : connection_point(self) {}
        connection_point<Itf> connection_point;
    };


    namespace impl {

        /** Find a connection point implementation for a given iid.
         * \internal
         */
        template<typename ITF_LIST> struct connection_point_finder
        {
            template<typename T> COMET_FORCEINLINE static ::IConnectionPoint* find_connection_point(T* This, const IID& iid)
            {
                typedef find_compatibility< COMET_STRICT_TYPENAME ITF_LIST::head > compatible;
                if (iid == uuidof<COMET_STRICT_TYPENAME ITF_LIST::head>())
                    return &((static_cast<connection_point_for< COMET_STRICT_TYPENAME ITF_LIST::head > *>(This))->connection_point)  ;
                else return connection_point_finder<COMET_STRICT_TYPENAME ITF_LIST::tail>::find_connection_point(This, iid);
            }
        };
        template<> struct connection_point_finder<nil>
        {
            template<typename T> COMET_FORCEINLINE static ::IConnectionPoint* find_connection_point(T*, const IID&)
            {
                return 0;
            }
        };

    }

    namespace impl {


            template<typename ITF_LIST>    struct ATL_NO_VTABLE inherit_all_ex_unknown;

#ifdef COMET_GOOD_RECURSIVE_STRUCT
            // Remove level of indirection. PC-lint cannot handle it, and MSVC7
            // should be ale to.
            template<typename ITF_LIST>    struct ATL_NO_VTABLE inherit_all_ex_unknown
                : public ITF_LIST::head, public inherit_all_ex_unknown<typename ITF_LIST::tail >
            {
                inherit_all_ex_unknown( ::IUnknown *initParam)
                    : ITF_LIST::head(initParam), inherit_all_ex_unknown< COMET_STRICT_TYPENAME ITF_LIST::tail >(initParam)
                {}
            };
            template<> struct inherit_all_ex_unknown<nil> { inherit_all_ex_unknown(::IUnknown *) {} };

#else // COMET_GOOD_RECURSIVE_STRUCT
            template<typename HEAD, typename ITF_TAIL>    struct ATL_NO_VTABLE inherit_all_ex_aux_unknown
                : public HEAD, public inherit_all_ex_unknown<ITF_TAIL>
            {
                inherit_all_ex_aux_unknown( ::IUnknown *initParam)
                    : HEAD(initParam), inherit_all_ex_unknown<ITF_TAIL>(initParam)
                {}
            };

// COMET_CONFIG_H is always defined! This is just a trick to get Doxygen to ignore the following declaration that
// otherwise seems to be cause an exception in Doxygen 1.2.8
#ifdef COMET_CONFIG_H
            template<typename ITF_LIST>    struct ATL_NO_VTABLE inherit_all_ex_unknown
                : public inherit_all_ex_aux_unknown<typename ITF_LIST::head,typename ITF_LIST::tail >
            {
                inherit_all_ex_unknown( ::IUnknown *initParam)
                    :inherit_all_ex_aux_unknown< COMET_STRICT_TYPENAME ITF_LIST::head, COMET_STRICT_TYPENAME ITF_LIST::tail >(initParam)
                {
                }
            };
            template<> struct inherit_all_ex_unknown<nil> { inherit_all_ex_unknown(::IUnknown *) {} };
#endif // COMET_CONFIG_H
#endif // COMET_GOOD_RECURSIVE_STRUCT



    }


    COMET_WRAP_EACH_DECLARE( connection_point_for)


    /** \struct implement_cpc  cp.h comet/cp.h
      * Implement a connection point container that can handle multiple
      * connection points.
      * This should be added to the \link comet::make_list list \endlink of implemented interfaces for a coclass as it
      * implements IConnectionPointContainer (which will be required for a qi).
      *
      * The class is used by the default coclass implementation to provide \ref cometconnectionpoints.
      *
      * \param ITF_LST \link comet::make_list List \endlink of connection points interfaces to implement.
      */
    template< typename ITF_LST> struct ATL_NO_VTABLE implement_cpc : public IConnectionPointContainer
    , public impl::inherit_all_ex_unknown< COMET_WRAP_EACH(connection_point_for, ITF_LST) >
    {
        typedef IConnectionPointContainer interface_is;
    public:
        implement_cpc()
            : impl::inherit_all_ex_unknown< COMET_WRAP_EACH(comet::connection_point_for, ITF_LST) >((::IUnknown *)this)
        {}

    protected:
        friend struct impl::connection_point_finder<ITF_LST>;
        /// \name IConnectionPointContainer interface
        //@{
        STDMETHOD(EnumConnectionPoints)(IEnumConnectionPoints**)
        {
            return E_NOTIMPL;
        }

        STDMETHOD(FindConnectionPoint)(REFIID riid, IConnectionPoint** ppCP)
        {
            if (!ppCP) return E_POINTER;

            const IID& iid = riid;

            *ppCP = impl::connection_point_finder<ITF_LST>::find_connection_point(this, iid);

            if ( *ppCP !=NULL)
            {
                (*ppCP)->AddRef();
                return S_OK;
            }
            return CONNECT_E_NOCONNECTION;
        }
        //@}
    };


    /** \class connection_point_impl  cp.h comet/cp.h
      * Implements a connection point.
      * \param Itf Interface of connection point.
      */
    template<typename Itf> class ATL_NO_VTABLE connection_point_impl : public embedded_object< IUnknown, IConnectionPoint >
    {
    public:
        bool is_connected() const
        {    return !connections_.empty(); }
    protected:
        connection_point_impl(::IUnknown* pUnk) : next_cookie_(1), embedded_object< IUnknown, IConnectionPoint >(pUnk) {}

        /// \name IConnectionPoint interface
        //@{
        STDMETHOD(GetConnectionInterface)(IID* pIID)
        {
            *pIID = uuidof<Itf>();
            return S_OK;
        }

        STDMETHOD(GetConnectionPointContainer)(IConnectionPointContainer** ppCPC) {
            com_ptr<IConnectionPointContainer> p;
            p = try_cast(com_ptr< ::IUnknown >(get_parent()));
            *ppCPC = com_ptr<IConnectionPointContainer>::detach( p );
            return S_OK;
        }

        STDMETHOD(Advise)(::IUnknown* pUnkSink, DWORD* pdwCookie)
        {
            try {
                connections_[next_cookie_] = try_cast( com_ptr< ::IUnknown >(pUnkSink) );
            }
            catch (...) {
                return CONNECT_E_CANNOTCONNECT;
            }
            *pdwCookie = next_cookie_++;
            return S_OK;
        }
        STDMETHOD(Unadvise)(DWORD dwCookie)
        {
            CONNECTIONS::iterator it = connections_.find(dwCookie);
            if (it == connections_.end()) return CONNECT_E_NOCONNECTION;
            connections_.erase(it);
            return S_OK;
        }

        STDMETHOD(EnumConnections)(IEnumConnections** ppEnum)
        {
            try {
                *ppEnum = com_ptr<IEnumConnections>::detach( stl_enumeration<IEnumConnections>::create(connections_, get_unknown()) );
            } catch (...) {
                return E_FAIL;
            }
            return S_OK;
        }
        //@}

        typedef std::map<DWORD, com_ptr<Itf> > CONNECTIONS;
        CONNECTIONS connections_;
    private:
        UINT next_cookie_;
    };

    /** \class sink_impl  cp.h comet/cp.h
      * Implement a sink for a connection pointer.
      * \param Itf interface to implement.
      */
    template<typename Itf> class ATL_NO_VTABLE sink_impl : public static_object<Itf>
    {
    public:
        /** Advise this object as sinking connections from \p t.
          */
        void advise(const com_ptr< ::IUnknown>& t)
        {
            if (ptr_) throw std::runtime_error("Cannot double advise.");
            com_ptr<IConnectionPointContainer> cpc( try_cast(t) );
            IConnectionPoint* cp;
            cpc->FindConnectionPoint( uuidof<Itf>(), &cp) | raise_exception;

            HRESULT hr = cp->Advise(static_cast< ::IUnknown* >(static_cast<Itf*>(this)), &cookie_);

            cp->Release();

            hr | raise_exception;

            ptr_ = t;
        }

        /** Unadvise this interface from object \p t.
          */
        void unadvise()
        {
            if (ptr_) {
                com_ptr<IConnectionPointContainer> cpc( try_cast(ptr_) );
                IConnectionPoint* cp;
                cpc->FindConnectionPoint( uuidof<Itf>(), &cp) | raise_exception;

                HRESULT hr = cp->Unadvise(cookie_);
                cookie_ = 0;
                ptr_ = 0;

                cp->Release();

                hr | raise_exception;
            }
        }

        /** Get event object.
         */
        com_ptr< ::IUnknown> object()
        {
            return ptr_;
        }

        /** Return true if advised.
         */
        bool is_advised()
        {
            return !ptr_.is_null();
        }

    protected:
        sink_impl() : cookie_(0) {}
        ~sink_impl() { unadvise(); }
    private:
        DWORD cookie_;
        com_ptr< ::IUnknown> ptr_;
    };
    //@}

}

#pragma warning( pop )

#endif
