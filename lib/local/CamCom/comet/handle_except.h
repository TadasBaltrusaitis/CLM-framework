/** \file
  * Handle Comet exceptions.
  */
/*
 * Copyright 2003 Atex Media Command.  All rights reserved.
 * Copyright (C) 2013 Alexander Lamaison <alexander.lamaison@gmail.com>
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
#ifndef  COMET_HANDLE_EXCEPT_H
#define  COMET_HANDLE_EXCEPT_H

#ifdef _SHOW_INC
#pragma message("   #Include " __FILE__)
#endif

#include <comet/error.h>

/** \page comethandleexception Custom Exception Handling
    This allows the user to define how exception handling happens for a particular library.

    Unfortunately due to a compiler bug in MSVC 6  (fixed in 7), some of the
    functionality won't be available for that compiler (this has also limited
    the customisability originally designed for).

    To override the default behavior, specialise the struct
    comet::comet_exception_handler<true> and implement the functions defined
    in the default implementation comet::comet_exception_handler.
    \code
    comet {
    template<>
    struct comet_exception_handler<true> : comet_exception_handler<false>
    {
        static inline HRESULT catcher_all( const source_info_t &info)
        {
            throw;
        }
    };
    }
    \endcode

    With a suitable compiler, the method comet::comet_exception_handler::rethrow can be
    overridden to provide support for custom exceptions.

    Don't try putting a try catch round a throw in \p catcher_all as the resulting destruct-twice
    of the exception when using MSV6 will cause very hard to trace bugs.
 */

#ifdef COMET_DISABLE_EXCEPTION_RETHROW_CATCH
#define COMET_CATCH_CLASS_EX(funcName,iid) \
    catch ( const com_error &err) { \
        return comet_exception_handler<true>::catcher_com_error(err, impl::do_get_source_info_t<_B>::exec( static_cast<_B*>(this), source_info_t(funcName, iid))); \
    } catch ( const std::exception &err) { \
        return comet_exception_handler<true>::catcher_exception(err, impl::do_get_source_info_t<_B>::exec( static_cast<_B*>(this), source_info_t(funcName, iid))); \
    } catch ( HRESULT hr ) { \
        return comet_exception_handler<true>::catcher_hr(hr, impl::do_get_source_info_t<_B>::exec( static_cast<_B*>(this), source_info_t(funcName, iid))); \
    } catch (...) { \
        return comet_exception_handler<true>::catcher_all( impl::do_get_source_info_t<_B>::exec( static_cast<_B*>(this), source_info_t(funcName, iid))); \
    }

#define COMET_CATCH_UNKNOWN(funcName, iid, clsname) \
    catch ( const com_error &err) { \
        comet_exception_handler<true>::catcher_com_error(err,  source_info_t(funcName, iid, clsname)); \
    } catch ( const std::exception &err) { \
        comet_exception_handler<true>::catcher_exception(err, source_info_t(funcName, iid, clsname)); \
    } catch ( HRESULT hr ) { \
        comet_exception_handler<true>::catcher_hr(hr, source_info_t(funcName, iid, clsname)); \
    } catch (...) { \
        comet_exception_handler<true>::catcher_all( source_info_t(funcName, iid, clsname)); \
    }


#else // COMET_DISABLE_EXCEPTION_RETHROW_CATCH

#define COMET_CATCH_CLASS_EX( funcName, iid) \
    catch ( ... ) { \
        return comet_exception_handler<true>::rethrow( impl::do_get_source_info_t<_B>::exec( static_cast<_B*>(this), source_info_t(funcName, iid))); \
    }

#define COMET_CATCH_UNKNOWN(funcName, iid, clsname) \
    catch ( ... ) { \
        comet_exception_handler<true>::rethrow( source_info_t(funcName, iid, clsname)); \
    }

#endif

/** Macro used in implementation wrappers to hide a bug with catching rethrown
 * classes.
 */

#define COMET_CATCH_CLASS(funcName) COMET_CATCH_CLASS_EX(funcName, comtype<interface_is>::uuid())

// We define these new catch macros because the ones above don't seem to work.
// They use a mystery _B to get the class name.  It may have been part of the
// old code-generation mechanism which we don't support any more.
#define COMET_CATCH_INTERFACE_BOUNDARY(funcName) \
    catch ( ... ) { \
        return ::comet::comet_exception_handler<true>::rethrow( \
            ::comet::source_info_t( \
                funcName, ::comet::comtype<interface_is>::uuid())); \
    }

#define COMET_CATCH_CLASS_INTERFACE_BOUNDARY(funcName, clsName) \
    catch ( ... ) { \
        return ::comet::comet_exception_handler<true>::rethrow( \
            ::comet::source_info_t( \
                funcName, ::comet::comtype<interface_is>::uuid(), clsName)); \
    }

namespace comet {

    /*! \addtogroup ErrorHandling
     */
    //@{
    /// Specifies the source of an exception.
    struct source_info_t
    {
        /// Default Constructor
        source_info_t() {}
        /// Constructor to fill in a few elements.
        source_info_t(const bstr_t &par_function, const uuid_t &par_interface_uuid = uuid_t(), const bstr_t &par_coclass_name = bstr_t() )
            : function_name(par_function), interface_uuid(par_interface_uuid), coclass_name(par_coclass_name) {}

        bstr_t function_name;       ///< Name of the function being called.
        bstr_t coclass_name;        ///< Coclass the method belongs to.
        uuid_t coclass_uuid;        ///< CLSID of the coclass.
        uuid_t interface_uuid;      ///< IID of the interface

        bstr_t source_override;     ///< Used to override the source description.

        /** Returns the 'source' description, either 'source_overrride' or the
         * concatenation of the coclass and function_name.
         */
        bstr_t source() const
        {
            if ( !source_override.is_empty())
            {
                return source_override;
            }
            if (coclass_name.is_empty())
            {
                return function_name;
            }
            if (function_name.is_empty())
            {
                return coclass_name;
            }
            return coclass_name + L"." + function_name;
        }

    };

    namespace impl
    {
        /** Redirect logging to calllog if enabled.
         * Used so that if call-logging is loaded, the default action is to
         * redirect the logging to the call-logger.
         * \internal
         */
        template<bool>
        struct call_logger_redirect_
        {
            template<bool>
            struct exe
            {
                static inline void log_exception(
                    const tstring& /*type*/, const tstring& /*desc*/,
                    const source_info_t& /*errorSource*/,
                    const source_info_t& /*callSource*/)
                { }
                static inline bool can_log_exception()
                { return false; }
            };
        };
    }

    /** \struct error_logger_ handle_except.h comet/handle_except.h
      * Default to NULL logger - and description of 'error logger' concept.
      * Specialise to \b true to override.
      */
    template<bool OVERRIDE>
    struct error_logger_
    {
        /** Should the error be logged?
         * \return Return true to allow error to be logged.
         */
        static inline bool can_log_exception()
        {
            return impl::call_logger_redirect_<OVERRIDE>::template exe<OVERRIDE>::can_log_exception();
        }

        /// Called by exception handlers to provide logging for errors.
        static inline void log_exception(const tstring &type, const tstring &desc, const source_info_t &errorSource ,const source_info_t &callSource )
        {
            impl::call_logger_redirect_<OVERRIDE>::template exe<OVERRIDE>::log_exception(type,desc,errorSource, callSource);
        }

    };

    /** Common exception handler for comet.
     * Specialize to \c true to overide the behaviour. It is usually convenient
     * to inherit the comet_exception_handler<true> from
     * comet_exception_handler<false>.
     */
    template< bool OVERRIDE>
    struct comet_exception_handler
    {
#ifndef COMET_DISABLE_EXCEPTION_RETHROW_CATCH
        /** Override to modify which exceptions are caught.
          * Note that due to a severe bug in MSVC 6, this will not be called in that
          * case.
          */
        static inline HRESULT rethrow(const source_info_t &info)
        {
            try {
                throw;
            } catch ( const com_error &err)
            {
                return catcher_com_error( err, info);
            }
            catch ( const std::exception &err)
            {
                return catcher_exception( err, info);
            }
            catch ( HRESULT hr )
            {
                return catcher_hr( hr, info );
            }
            catch (...)
            {
                return catcher_all(info);
            }

        }
#endif // COMET_DISABLE_EXCEPTION_RETHROW_CATCH

        /// Override to modify handling of com_error
        static inline HRESULT catcher_com_error( const com_error &err,const source_info_t &info)
        {
            if ( error_logger_<OVERRIDE>::can_log_exception() )
            {
                source_info_t errorInfo;
                errorInfo.interface_uuid = err.guid();
                errorInfo.source_override = err.source();
                error_logger_<OVERRIDE>::log_exception( _T("comet::com_error"), err.t_str(), errorInfo, info);
            }
            return impl::return_com_error(err, info.source(), info.interface_uuid);
        }
        /// Override to modify handling of std::exception
        static inline HRESULT catcher_exception(const std::exception& err,const source_info_t &info)
        {
            if ( error_logger_<OVERRIDE>::can_log_exception() )
            {
                error_logger_<OVERRIDE>::log_exception( _T("std::exception"),
                        bstr_t(err.what()).t_str(),
                        source_info_t(), info);
            }
            return impl::return_com_error(err, info.source(), info.interface_uuid);
        }
        /// Override to modify handling of HRESULT
        static inline HRESULT catcher_hr( HRESULT hr,const source_info_t &info)
        {
            return impl::return_com_error(hr, bstr_t(), info.source(), info.interface_uuid);
        }
        /// This can be overridden to provide handling of other exceptions.
        static inline HRESULT catcher_all( const source_info_t &info)
        {
            COMET_NOTUSED(info);
            throw;
        }
    };


    namespace impl
    {
        /** Marker for using the coclass information.
          * \relates handle_exception_default
          */
        struct handle_exception_default_marker {};

        /** Call the coclasses exception handler if it is marked.
          * \relates handle_exception_default
          */
        template <typename O>
        struct do_get_source_info_t
        {
            /** Call the classes exception handler.
              * \relates handle_exception_default
              */
            template <bool USETHIS>
            struct execute_handle
            {
                inline static void get_source_info(O * pThis, source_info_t &info)
                {
                    pThis->get_source_info_( info );
                }
            };
            /** Call the default global exception handler.
              * \relates handle_exception_default
              */
            template <>
            struct execute_handle<false>
            {
                inline static void get_source_info(O *pThis, const source_info_t &info )
                {
                    COMET_NOTUSED(pThis);
                    COMET_NOTUSED(info);
                    // return impl::do_handle_exception<nil, IFACE>(src);
                }
            };

            /** Called by the interface wrappers to call the exception handlers.
              * \relates handle_exception_default
              */
            inline static source_info_t exec( O *pThis, source_info_t info)
            {
                // Either call the coclass exception handler, or the project default.
                execute_handle<type_traits::conversion<O&, handle_exception_default_marker>::exists>::get_source_info( pThis, info);
                /* ERRORS: If the conversion is ambiguous here, then there are probably two ways of
                 * getting to handle_exception_default_marker, usually via handle_exception_default<COCLASS>,
                 * which is already provided by by IProvideClassInfoImpl<COCLASS>.
                 */
                return info;
            }
        };
    }

    /** \struct handle_exception_default server.h comet/server.h
      * Used by the exception handler to provide source information for the specified CoClass.
      * Inherit from this class or IProvideClassInfoImpl to make sure that the
      * coclass name is known about for custom coclasses.
      */
    template<typename COCLASS>
    struct handle_exception_default
        : impl::handle_exception_default_marker // Required to mark the class as being able to handle exceptions with coclass information
    {
        void get_source_info_( source_info_t &info)
        {
            info.coclass_name = COCLASS::name();
            info.coclass_uuid = comtype<COCLASS>::uuid();
        }
    };
    // Handles the case where there is no coclass, and therefore no coclass name.
    template<>
    struct handle_exception_default<nil>  : impl::handle_exception_default_marker
    {
        void get_source_info_( source_info_t &info)
        {
            COMET_NOTUSED(info);
        }
    };
    //@}
}
#endif /* COMET_HANDLE_EXCEPT_H */
