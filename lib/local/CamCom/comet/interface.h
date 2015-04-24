/** \file
  *  Provide comtype which supplies information about UUIDs & inheritance,
  *  potentially from Interfaces not defined using a COMET type-library.  Also
  *  defines specialisations for some such standard interfaces.
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

#ifndef COMET_INTERFACE_H
#define COMET_INTERFACE_H

#include <comet/config.h>

#include <ocidl.h>
#include <string>
#include <stdexcept>

#include <comet/typelist.h>
#include <comet/static_assert.h>
#include <comet/uuid_fwd.h>

#pragma warning(push, 4)
// NB: coclass_implementation _must_ have no data members.
// The comet framework uses the test
// sizeof coclass_implementation<T> == sizeof coclass_implementation<nil>
// in order to determine whether the user has specialized based on T or not.
// The logic here is that any real implementation will have a size that is at least
// sizeof IUnknown, because any real coclass_implementation must at least derive
// off IUnknown and have a "vtable" pointer.
/** \class coclass_implementation interface.h comet/interface.h
  * Utility class to make the implementation of a coclass accessible from the
  * servers.  Implementations of coclasses defined in the type library should
  * all be specialisations of this class if they are to be exposed by the
  * comet server.
  */
template<typename T> class coclass_implementation {};

#pragma warning(pop)

namespace comet {

    /** Provide a means for defining new comtype definitions.
     */
    template<typename ITF, const UUID *ItfID, typename BASE = ::IUnknown> struct uuid_comtype
    {
        static const uuid_t& uuid() { return uuid_t::create_const_reference(*ItfID); }
        typedef BASE base;
    };


    /** Provide access to uuid and base type of objects.
      * Specializations allow access to information relevant to non-comet
      * defined interfaces.
      */
    template<typename T> struct comtype {
        static const IID& uuid() throw() { return comtype<COMET_STRICT_TYPENAME T::interface_is>::uuid(); }
        typedef typename comtype<COMET_STRICT_TYPENAME T::interface_is>::base base;
    };

    template<> struct comtype<nil> {
//        static const IID& uuid() throw() { throw std::logic_error("interface.h:35"); return IID_NULL; }
        typedef nil base;
    };

    template<> struct comtype< ::IUnknown >
    {
        static const IID& uuid() { return IID_IUnknown; }
        typedef nil base;
    };

    template<> struct comtype<IConnectionPoint>
    {
        static const IID& uuid() { return IID_IConnectionPoint; }
        typedef ::IUnknown base;
    };

    template<> struct comtype<IConnectionPointContainer>
    {
        static const IID& uuid() { return IID_IConnectionPointContainer; }
        typedef ::IUnknown base;
    };

    template<> struct comtype<IEnumConnections>
    {
        static const IID& uuid() { return IID_IEnumConnections; }
        typedef ::IUnknown base;
    };

    template<> struct comtype<IDispatch>
    {
        static const IID& uuid() { return IID_IDispatch; }
        typedef ::IUnknown base;
    };

    template<> struct comtype<IEnumVARIANT>
    {
        static const IID& uuid() { return IID_IEnumVARIANT; }
        typedef ::IUnknown base;
    };

    template<> struct comtype<ISupportErrorInfo>
    {
        static const IID& uuid() { return IID_ISupportErrorInfo; }
        typedef ::IUnknown base;
    };

    template<> struct comtype<IErrorInfo>
    {
        static const IID& uuid() { return IID_IErrorInfo; }
        typedef ::IUnknown base;
    };

    template<> struct comtype<IProvideClassInfo>
    {
        static const IID& uuid() throw() { return IID_IProvideClassInfo; }
        typedef ::IUnknown base;
    };

    template<> struct comtype<IPersist>
    {
        static const IID& uuid() throw() { return IID_IPersist; }
        typedef ::IUnknown base;
    };

    template<> struct comtype<IPersistFile>
    {
        static const IID& uuid() throw() { return IID_IPersistFile; }
        typedef ::IPersist base;
    };

    template<> struct comtype<IPersistStream>
    {
        static const IID& uuid() throw() { return IID_IPersistStream; }
        typedef ::IPersist base;
    };

    template<> struct comtype<IPersistStreamInit>
    {
        static const IID& uuid() throw() { return IID_IPersistStreamInit; }
        typedef ::IPersist base;
    };

    template<> struct comtype<IMessageFilter>
    {
        static const IID& uuid() throw() { return IID_IMessageFilter; }
        typedef ::IUnknown base;
    };

    template<> struct comtype<IProvideClassInfo2>
    {
        static const IID& uuid() throw() { return IID_IProvideClassInfo2; }
        typedef ::IUnknown base;
    };

    template<> struct comtype<IMarshal>
    {
        static const IID& uuid() throw() { return IID_IMarshal; }
        typedef ::IUnknown base;
    };

    template<> struct comtype<IFontDisp>
    {
        static const IID& uuid() throw() { return IID_IFontDisp; }
        typedef ::IDispatch base;
    };

    template<> struct comtype<IPictureDisp>
    {
        static const IID& uuid() throw() { return IID_IPictureDisp; }
        typedef ::IDispatch base;
    };
    template<> struct comtype<IGlobalInterfaceTable>
    {
        static const IID& uuid() throw() { return IID_IGlobalInterfaceTable; }
        typedef ::IUnknown base;
    };

    template<> struct comtype<IClassFactory>
    {
        static const IID& uuid() throw() { return IID_IClassFactory; }
        typedef ::IUnknown base;
    };

    template<> struct comtype<IStream>
    {
        static const IID& uuid() throw() { return IID_IStream; }
        typedef ::IUnknown base;
    };

    template<> struct comtype<ISequentialStream>
    {
        static const IID& uuid() throw() { return IID_ISequentialStream; }
        typedef ::IUnknown base;
    };



    //! C++ replacement of VC's __uuidof()
    /*!
        Use this function to an IID to an interface or coclass.
    */
    template<typename Itf> inline const uuid_t& uuidof(Itf * = 0) throw()
    { return uuid_t::create_const_reference(comtype<Itf>::uuid()); }

    namespace impl {

        template<typename Itf> struct interface_lookup
        {
            static bool supports(const uuid_t& iid)
            {
                if (iid == uuidof<Itf>())
                    return true;
                else
                    return interface_lookup< typename comtype<Itf>::base >::supports();
            }

            template<typename T> static Itf* cast(T* t)
            {
                return static_cast<Itf*>(t);
            }
        };

        template<> struct interface_lookup<nil>
        {
            static bool supports(const uuid_t&)
            {
                return false;
            }

        };

/*        template<> struct interface_lookup<make_list<> >
        {
            static bool supports(const uuid_t&)
            {
                return false;
            }

        };*/
    }

} // namespace

#endif
