/** \file
 * Provide logging for calls.
 */
/*
 * Copyright © 2003 Michael Geddes
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

#ifndef COMET_CALLLOG_H
#define COMET_CALLLOG_H

#include <comet/config.h>
#include <comet/tstring.h>
#include <comet/variant.h>
#include <comet/currency.h>
#include <comet/server.h>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <comet/handle_except.h>

/** \page cometcalllogging Call Logging
    This is a feature of the comet interfaces that allows the server to log all
    calls to any of the interface methods.  It logs parameter values, and also allows
    logging of errors (see \ref comethandleexception ).

    Call logging in comet is enabled by using <kbd>tlb2h -L</kbd> (see \ref tlb2husage).

    To enable logging to a specified file, define  COMET_LOGFILE and define
    COMET_LOGFILE_DEFAULT to be the required logfile (see comet::stream_call_logger_t).

    To override logging, specialise comet::call_logger_<true> and implement all the interfaces
    defined by the default comet::call_logger_.

    How various parameter types are output can be modified by specialising the
    function comet::comet_log.
  */

/** \defgroup CallLog Call-logging
 *@{
 */
#ifdef COMET_DOXYGEN // Only for doxygen
/** \def COMET_LOGFILE
  * Define to get simple interface logfiles.
  *  Must use tlb2h -L to enable interface logging.  See \ref tlb2husage.
  *  \sa comet_log comet_log_interface comet_log_array stream_call_logger_t tofstream_comet
  */

#define COMET_LOGFILE

/** \def COMET_LOGFILE_DEFAULT
 * If COMET_LOGFILE is defined, define to set the default logfile name.
 * \code
        #define COMET_LOGFILE_DEFAULT "C:\\log\\comet.log"
 * \endcode
 * \relates call_logger_
 */
#define COMET_LOGFILE_DEFAULT "C:\\log\\comet.log"
#endif // COMET_DOXYGEN

    //@}

namespace comet
{
/*!\addtogroup CallLog
 */
//@{
    /** \struct call_logger_  calllog.h comet/calllog.h
      *  Default NULL logger and description of 'call logger' concept.
      *  Specialise to \b true to override.
      *  \code
        template<>
        struct call_logger_<true> : stream_call_logger_t<tofstream_comet>
        {
        };
      * \endcode
      *  \sa stream_call_logger_t tofstream_comet
      */
    template<bool OVERRIDE>
    struct call_logger_
    {
        /** Should the call be logged?
         * \return Return true to allow call to be logged.
         */
        static inline bool can_log_call() { return false; }
        /** Should the return values be logged?
         * \return Return true to allow call return values to be logged.
         */
        static inline bool can_log_return() { return false; }

        /** Called by interfaces to provide call logging.
         */
        static inline void log_call( const tstring &iface, const tstring &funcname, const tstring &log){}

        /// Called by interfaces to provide call return logging.
        static inline void log_return( const tstring &iface, const tstring &funcname, const tstring &log, const tstring &retval){}


        ///\name Exception handling redirection.
        //@{

        /** Should the error be logged?
         * \return Return true to allow error to be logged.
         */
        static inline bool can_log_exception() { return false; }

        /// Called by exception handlers to provide logging for errors.
        static inline void log_exception(const tstring &type, const tstring &desc, const source_info_t &errorSource ,const source_info_t &callSource ){}
        //@}
    };

    /** \struct stream_call_logger_t calllog.h comet/calllog.h
      * Log calls to a tostream.
      * A class/struct that subscribes to and implements the 'call logger' concept for a steam.
      * Template paramter is expected to be a class that has a method that creates a tostream
      * instance as follows:
      * \code
      * static tostream *create()
      * \endcode
      * \param CREATESTREAM Class with a static function \p create() that returns a <b> tostream * </b>
      * \sa tofstream_comet call_logger_
      */
    template<typename CREATESTREAM>
    struct stream_call_logger_t
#ifdef COMET_DOXYGEN // For documentation
        : call_logger_
#endif
    {
        static inline bool can_log_call()
        {
            tostream *ofs = logger();
            return ofs != NULL && ofs->good();
        }
        static inline bool can_log_return() { return can_log_call(); }
        static inline bool can_log_exception() { return can_log_call(); }

        static inline void log_call( const tstring &iface, const tstring &funcname, const tstring &log)
        {
            log_( false, iface, funcname, log, tstring());
        }
        static inline void log_return( const tstring &iface, const tstring &funcname, const tstring &log, const tstring &retval)
        {
            log_( true, iface, funcname, log, retval);
        }
        static inline void log_exception(const tstring &type, const tstring &desc, const source_info_t &errorSource ,const source_info_t &callSource )
        {
            COMET_NOTUSED(errorSource);
            tostream *ofs= logger();
            if (ofs==NULL) return; // Should never be NULL, as can_log_call() should have been obeyed before calling.
            *ofs << _T("Err ") << callSource.source().t_str();
            if ( !desc.empty() )
            {
                *ofs << type << _T(": ") << desc ;
            }
            *ofs << std::endl;
        }

    protected:
        // Log a function call/return.
        static void log_( bool return_val, const tstring &iface, const tstring &funcname, const tstring &log, const tstring &retval)
        {
            tostream *ofs= logger();
            if (ofs==NULL) return; // Should never be NULL, as can_log_call() should have been obeyed before calling.

            *ofs << (return_val?_T("Out "):_T("In  ")) << iface << _T("::") << funcname;
            if (!return_val || !log.empty())
                *ofs << _T("(") << log << _T(")");

            if (return_val && !retval.empty())
                *ofs  << _T(" returned ") << retval;
            *ofs << std::endl;

        }
        // Access to the static logger instance without needing to initialise a
        // member.
        static tostream *logger()
        {
            static tostream *ofs_= NULL;
            if (ofs_ ==NULL)
            {
                ofs_ = CREATESTREAM::create();
                if (ofs_ != NULL)
                {
                    // Safely clean up static pointer on module destruct
                    module().add_object_to_dispose( create_pointer_deleter( ofs_ ) );
                }
            }
            return ofs_;
        }
    };

    namespace impl
    {
        /** Redirect default logging to calllog.
         * \internal
         */
        template<>
        struct call_logger_redirect_<true>
        {
            // This trick allows the user to be able to still override the call logging.
            // Without this, log_exception has to call
            // call_logger_<true>::log_exception which causes the template to be
            // instantiated, and the user is no longer able to specialise
            // call_logger_
            template<bool OVERRIDE>
            struct exe
            {
                static inline void log_exception(const tstring &type, const tstring &desc, const source_info_t &errorSource, const source_info_t &callSource )
                {
                    call_logger_<OVERRIDE>::log_exception(type,desc,errorSource, callSource);
                }
                static inline bool can_log_exception()
                {
                    return call_logger_<OVERRIDE>::can_log_exception();
                }
            };
        };
    }

#ifdef COMET_LOGFILE
#ifndef COMET_LOGFILE_DEFAULT
#define COMET_LOGFILE_DEFAULT NULL
#endif
#include <fstream>
    /** \class tofstream_comet calllog.h comet/calllog.h
      * Provides a filestream creator as well as the implementation of an output filestream logger.
      * Allows overriding of file name.
      */
    class tofstream_comet : public tofstream
    {
        static const char *&filename_()
        {
            static const char *default_filename= COMET_LOGFILE_DEFAULT;
            return default_filename;
        }
        public:
            /// Get the default filename.
            static const char *get_default_filename() { return filename_(); }
            /// Set the default filename to a new value.
            void set_default_filename( const char *filename) { filename_() = filename; }

            tofstream_comet( const char *fname) : tofstream(fname) {}

            /** Create an instance of the logger.
             * Returns NULL if the file doesn't exist.
             */
            static tostream *create()
            {
                const char *fname =  filename_();
                if (fname == NULL)
                    return NULL;
                return new tofstream_comet(fname);
            }
        private:
            tofstream_comet(const tofstream_comet &);
            tofstream_comet &operator=(const tofstream_comet &);

    };

    /** Override specialisation of the call_logger_.
     * \relates call_logger_
     */
    template<>
    struct call_logger_<true> : public stream_call_logger_t<tofstream_comet>
    {
    };

#endif // FILE_LOG

    /** Default interface (com_ptr) logging.
      * Specialise to provide custom logging for com_ptr.
        \code
        template<>
        void comet_log_interface<IMyInterface>(tostream &os, const com_ptr<IMyInterface> &iface)
        {
            os << iface->GetName();
        }
        \endcode
        Specialisations for IUnknown and IDispatch are used by the variant loggers.
      * \sa comet_log comet_log_array
      * \relates call_logger_
      */
    template< typename IFACE>
    void comet_log_interface(tostream &os, const com_ptr<IFACE> &iface)
    {
        os << _T("0x") << std::hex << reinterpret_cast<unsigned long>(iface.get()) << std::dec;
    }

    // Forward declarations.
    template<typename T> void comet_log(tostream &os, const T &value);

    namespace impl
    {
        // Forward declarations.
        template<typename T> inline void default_comet_log(tostream &os, const T &value);
        static void comet_log_array_raw(tostream &os, SAFEARRAY *value);

        // The default variant handler.
        template< bool B>
        static inline void default_comet_log_variant(tostream &os, const variant_t &value, bool out_type)
        {
            VARTYPE vt = value.get_vt();
            if ((vt & VT_ARRAY) != 0)
            {
                comet_log_array_raw(os, value.get().parray);
            }
            else
            {
                VARIANT varcopy=value.in();
                if (vt == (VT_BYREF | VT_VARIANT)) // Dereference variant by reference
                    varcopy = *V_VARIANTREF(&varcopy);
                const VARIANT *var=&varcopy;

#define __VARIANT_LOGPOINTER_TYPE_CAST(vartype,cast) \
                case VT_##vartype:\
                    if(out_type) os << _T("VT_")_T(#vartype)_T(":");\
                    comet_log_interface(os, cast(V_##vartype(var))); \
                    break

#define __VARIANT_LOGPOINTER_REFTYPE_CAST(vartype,cast) \
                case VT_BYREF|VT_##vartype:\
                    if(out_type) os << _T("BYREF VT_")_T(#vartype)_T(":");\
                    comet_log_interface(os, cast(*V_##vartype##REF(var)));\
                    break
#define __VARIANT_LOG_TYPE_CAST(vartype,cast) \
                case VT_##vartype: \
                    if(out_type) os << _T("VT_")_T(#vartype)_T(":"); \
                    comet_log(os, cast(V_##vartype(var))); \
                    break

#define __VARIANT_LOG_REFTYPE_CAST(vartype,cast)\
                case VT_BYREF|VT_##vartype:\
                    if(out_type) os << _T("BYREF VT_")_T(#vartype)_T(":");\
                    comet_log(os, cast(*V_##vartype##REF(var)));\
                    break

#define __VARIANT_LOG_TYPE(vartype) \
                case VT_##vartype: \
                    if(out_type) os << _T("VT_")_T(#vartype)_T(":");\
                    comet_log(os, V_##vartype(var));\
                    break

#define __VARIANT_LOG_REFTYPE(vartype)\
                case VT_BYREF|VT_##vartype: \
                if(out_type) os << _T("BYREF VT_")_T(#vartype)_T(":"); \
                comet_log(os, *V_##vartype##REF(var)); \
                break

                switch (vt)
                {
                    __VARIANT_LOG_TYPE(UI1);
                    __VARIANT_LOG_TYPE(UI2);
                    __VARIANT_LOG_TYPE(UINT);
                    __VARIANT_LOG_TYPE(UI4);
                    __VARIANT_LOG_TYPE(I1);
                    __VARIANT_LOG_TYPE(I2);
                    __VARIANT_LOG_TYPE(INT);
                    __VARIANT_LOG_TYPE(I4);
                    __VARIANT_LOG_TYPE(R4);
                    __VARIANT_LOG_TYPE(R8);
                    __VARIANT_LOG_REFTYPE(UI1);
                    __VARIANT_LOG_REFTYPE(UI2);
                    __VARIANT_LOG_REFTYPE(UINT);
                    __VARIANT_LOG_REFTYPE(UI4);
                    __VARIANT_LOG_REFTYPE(I1);
                    __VARIANT_LOG_REFTYPE(I2);
                    __VARIANT_LOG_REFTYPE(INT);
                    __VARIANT_LOG_REFTYPE(I4);
                    __VARIANT_LOG_REFTYPE(R4);
                    __VARIANT_LOG_REFTYPE(R8);

                    case VT_BOOL:
                        if(out_type) os << _T("VT_BOOL:");
                        os << (V_BOOL(var)==VARIANT_FALSE)?_T("true"):_T("false");
                        break;
                    case VT_BYREF|VT_BOOL:
                        if(out_type) os << _T("BYREF VT_BOOL:");
                        os << (V_BOOL(var)==VARIANT_FALSE)?_T("true"):_T("false");
                        break;
                    __VARIANT_LOG_TYPE_CAST( CY, currency_t::create_const_reference);
                    __VARIANT_LOG_REFTYPE_CAST( CY, currency_t::create_const_reference);
                    __VARIANT_LOG_TYPE_CAST( DATE, datetime_t::create_const_reference);
                    __VARIANT_LOG_REFTYPE_CAST( DATE, datetime_t::create_const_reference);
                    __VARIANT_LOG_TYPE_CAST( BSTR, bstr_t::create_const_reference);
                    __VARIANT_LOG_REFTYPE_CAST( BSTR, bstr_t::create_const_reference);

                    __VARIANT_LOGPOINTER_TYPE_CAST( UNKNOWN, com_ptr<IUnknown>::create_const_reference);
                    __VARIANT_LOGPOINTER_TYPE_CAST( DISPATCH, com_ptr<IDispatch>::create_const_reference);
                    __VARIANT_LOGPOINTER_REFTYPE_CAST( UNKNOWN, com_ptr<IUnknown>::create_const_reference);
                    __VARIANT_LOGPOINTER_REFTYPE_CAST( DISPATCH, com_ptr<IDispatch>::create_const_reference);

                    case VT_DECIMAL:
                        if(out_type) os << _T("BYREF VT_DECIMAL:");
                        os << _T("?");
                        break;
                    case VT_ERROR:
                        if(out_type) os << _T("VT_ERROR:");
                        os <<_T("0x") << std::hex << V_ERROR(var) << std::dec;
                        break;
                    case VT_BYREF|VT_ERROR:
                        if(out_type) os << _T("BYREF VT_ERROR:");
                        os <<_T("0x") << std::hex << *V_ERRORREF(var) << std::dec;
                        break;
                    default:
                        os << _T("???");
                }
#undef __VARIANT_LOG_TYPE_CAST
#undef __VARIANT_LOG_REFTYPE_CAST
#undef __VARIANT_LOG_TYPE
#undef __VARIANT_LOG_REFTYPE
#undef __VARIANT_LOGPOINTER_TYPE_CAST
#undef __VARIANT_LOGPOINTER_REFTYPE_CAST
            }
        }

        /* Logging for raw safearrays.
         * For vector arrays of size <= 16, this fakes a variant and then uses the default variant logging to log the elements
         */
        static void comet_log_array_raw(tostream &os, SAFEARRAY *value)
        {
            UINT dims = SafeArrayGetDim(value);
            VARTYPE vt ;
            if( FAILED(SafeArrayGetVartype(value, &vt  ) ) )
            {
                os << "???";
                return;
            }

            long ubound=-1,lbound=0; // Outside so it can be used for the '1' case below.
            for (UINT d = 1; d<= dims ; ++d)
            {
                if( SUCCEEDED(SafeArrayGetUBound( value, d, &ubound) ) && SUCCEEDED(SafeArrayGetLBound(value, d, &lbound) ))
                {
                    if( lbound == 0)
                        os << _T("[") << ubound+1 << _T("]");
                    else
                        os << _T("(") << lbound << _T("..") << ubound << _T(")");
                }

            }
            if (dims == 1 )
            {
                long size = 1+ubound-lbound;
                if (size ==0)
                {
                    os << _T("{}");
                }
                else if( size > 0 && size <= 15)
                {
                    // For small arrays, print out the elements.
                    os << _T("{");
                    VARIANT v;
                    void *var;
                    bool first=true, is_variant= (vt==VT_VARIANT);
                    if ( is_variant)
                        var = (void *)&v;
                    else
                    {
                        V_VT(&v) = vt;
                        var = (void *)&V_UI1(&v);
                    }

                    for (long i = lbound; i <= ubound; ++i )
                    {
                        if(first)
                            first = false;
                        else
                            os << _T(",");
                        if( SUCCEEDED( SafeArrayGetElement( value, &i, var ) ))
                            default_comet_log_variant<true>(  os, variant_t(auto_attach(v)), first | is_variant);
                    }
                    os << _T("}");
                }
            }
        }

        // Default logging: stream the value.
        template<typename T>
        inline void default_comet_log(tostream &os, const T &value)
        {
            // ERRORS: If a compiler error occurs here, you may need to
            // specialise  comet_log<> to the type T here.
            os << value;
        }
        // Default logging for bstr_t
        template<>
        inline void default_comet_log<bstr_t>(tostream &os, const bstr_t &value)
        {
            // Put quotes round the string - simplistic, don't worry about
            // non-printables or escaping for the moment.
            os << _T("'") << value << _T("'");
        }

        // Deafult logging for variants.
        template<>
        static inline void default_comet_log<variant_t>(tostream &os, const variant_t &value)
        {
            default_comet_log_variant<true>( os, value, true);
        }
    }

    /** Type specific logger.
      * The default is to use internal logging.  This can be specialised to
      * provide type-specific logging.
      * \sa comet_log_array comet_log_interface
      * \relates call_logger_
      */
    template<typename T>
    void comet_log(tostream &os, const T &value)
    {
        impl::default_comet_log(os,value);
    }

    namespace impl
    {
        // trick to work out whether it is a safearray or com_ptr type (required because of no
        // partial specialisation). Needed to handle the special circumstances
        template<typename T>
        struct check_log_type_
        {
            static long check(...);

            template<typename S>
            static char check(const safearray_t<S> &);

            template<typename S>
            static short check( const com_ptr<S> &);

            static T createT();
            enum { is = sizeof( check( createT() ) ) };

        };
        template < int TYPE> struct choose_logger;

        // Default safearray_t logger where we know the type T.
        //
        template<typename T>
        inline void default_comet_log_array(tostream &os, const safearray_t<T> &value)
        {
            safearray_t<T>::index_type lbound=value.lower_bound(), size = value.size();
            if (lbound == 0)
                os << _T("[") << size << _T("]");
            else
                os << _T("(") << lbound << _T("..") << (lbound + size -1) << _T(")");

            if( value.size() <= 15)
            {
                os << _T("{");
                bool first = true;
                for (safearray_t<T>::const_iterator it = value.begin(); it != value.end(); ++it)
                {
                    choose_logger<check_log_type_<T>::is >::do_comet_log(os, *it);
                    if (first)
                        first=false;
                    else
                        os << _T(",");
                }
                os << _T("}");
            }
        }
    }
    /** Typed safearray_t logger.
      * The default is to use internal logging.  This can be specialised to
      * provide type-specific logging.
      * \sa comet_log comet_log_interface
      * \relates call_logger_
      */
    template<typename T>
    void comet_log_array(tostream &os, const safearray_t<T> &value)
    {
        impl::default_comet_log_array(os,value);
    }

    namespace impl
    {
        // choose the standard logger
        template < int TYPE>
        struct choose_logger
        {
            template<typename T>
            static inline void do_comet_log(tostream &os, const T &value)
            {
                comet_log(os,value);
            }
        };
        // Choose the array logger
        template<>
        struct choose_logger<sizeof(char)>
        {
            template <typename T>
            static inline void do_comet_log(tostream &os, const safearray_t<T> &value)
            {
                comet_log_array(os, value);
            }
        };
        // Choose the interface logger
        template<>
        struct choose_logger<sizeof(short)>
        {
            template <typename T>
            static inline void do_comet_log(tostream &os, const com_ptr<T> &value)
            {
                comet_log_interface(os, value);
            }
        };

        // Choose which of the comet_loggers to use.
        template<typename T>
        static inline tostream &do_comet_log(tostream &os, const T &value)
        {
            choose_logger<check_log_type_<T>::is >::do_comet_log(os, value);
            return os;
        }
    }
    //@}
}

#endif /* COMET_CALLLOG_H */
