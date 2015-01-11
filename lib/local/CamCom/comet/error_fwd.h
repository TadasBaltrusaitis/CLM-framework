/** \file
  * Error forwarding - using operator|.
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

#ifndef COMET_ERROR_FWD_H
#define COMET_ERROR_FWD_H

#include <comet/config.h>

#include <stdexcept>
#include <wtypes.h>

#include <OAIdl.h>

#pragma warning(push)
#pragma warning(disable : 4290)

namespace comet {
    template<typename Itf> class com_ptr;
    class bstr_t;

    class com_error;
    /*! \addtogroup ErrorHandling
     */
    //@{
    namespace impl {
        class raise_exception_t
        {
        public:
            // Raising a std::exception
            // inline void operator()(const std::exception& err) const;

            // Raising an HRESULT with a message
            inline void operator()(const bstr_t& msg, HRESULT hr = E_FAIL) const;

            // Raising an HRESULT
            inline void operator()(HRESULT hr) const;
        };

    }

    //! Raise exception "function"
    /*!
        raise_exception can by used in two different ways:

        First example. The 'do or die' paradigm using overloaded operator|.
        Here try() is a function that returns an HRESULT. If try fails an exception will be raised.
        \code
        try() | raise_exception;
        \endcode

        Second example. Using raise_exception as an exception throwing function.
        \code
        raise_exception(E_POINTER);
        raise_exception(L"Ran out of toilet paper!", E_FAIL);
        \endcode

    */
    static impl::raise_exception_t raise_exception;

    //! Overload for the 'do or die' useage of raise_exception.
    /*!
        \code
        try() | raise_exception;
        \endcode
    */

    namespace impl{
    inline HRESULT operator|(HRESULT hr, const impl::raise_exception_t&)
    {if (FAILED(hr)) raise_exception(hr); return hr; }
    }
    using impl::operator|;

    /**! \struct throw_error_handler error_fwd.h comet\error_fwd.h
      * Overridable error info handling.
      * To override the error handling, use code like this, making sure
      * that you define the handler before including comet/error.h.
        \code
            #include <comet/error_fwd.h>
            template<> struct throw_error_handler<true>
            {
                static inline void throw_error(HRESULT hr, const com_ptr<IErrorInfo> &ei);
            };
            #include <comet/error.h>
            // #include <mycustomerror.h>
            template<>
            inline void throw_error_handler<true>::throw_error(HRESULT hr, const com_ptr<IErrorInfo> &ei)
            {
            .... Override here ....
            }
        \endcode
       */

    template<bool OVERRIDE>
    struct throw_error_handler
    {
        /// Called to throw the error.
        static inline void throw_error(HRESULT hr, const com_ptr<IErrorInfo> &ei);
    };
    //@}
}

#pragma warning(pop)

#endif
