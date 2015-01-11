/** \file
  * Provide COM Error support.
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

#ifndef COMET_ERROR_H
#define COMET_ERROR_H

#include <comet/config.h>

#include <stdexcept>
#include <string>

#include <comet/error_fwd.h>
#include <comet/bstr.h>
#include <comet/ptr.h>
#include <comet/uuid_fwd.h>

#pragma warning(push)
#pragma warning(disable : 4290)

namespace comet {

    namespace impl {

        inline com_ptr<IErrorInfo> GetErrorInfo() throw()
        {
            IErrorInfo* ei = 0;
            ::GetErrorInfo(0, &ei);
            return com_ptr<IErrorInfo>(auto_attach(ei));
        }

        inline com_ptr<ICreateErrorInfo> CreateErrorInfo() throw()
        {
            ICreateErrorInfo* cei = 0;
            ::CreateErrorInfo(&cei);
            return com_ptr<ICreateErrorInfo>(auto_attach(cei));
        }

        template<typename Itf> inline bool supports_ErrorInfo(Itf* p)
        {
            com_ptr<ISupportErrorInfo> q = com_cast(com_ptr<Itf>(p));
            if (q == 0) return false;
            return S_OK == q->InterfaceSupportsErrorInfo(uuidof<Itf>());
        }

    }


    /** \class com_error  error.h comet/error.h
      * COM error.
      */
    class com_error : public std::runtime_error
    {
    public:
        //! Construct com_error from HRESULT.
        /*!
            \param hr
                HRESULT value of error.
        */
        explicit com_error(HRESULT hr) : hr_(hr), std::runtime_error("")
        {}

        //! Construct com_error from HRESULT and textual description.
        /*!
            \param msg
                Description of error.
            \param hr
                HRESULT value of error.
        */
        explicit com_error(const bstr_t& msg, HRESULT hr = E_FAIL) : hr_(hr), std::runtime_error("")
        {
            com_ptr<ICreateErrorInfo> cei(impl::CreateErrorInfo());
            if ( !cei.is_null() ) {
                try {
                    cei->SetDescription(msg.in());
                    ei_ = com_ptr<IErrorInfo>( com_cast(cei)) ;
                } catch (std::bad_alloc&)
                {}
            }
        }
        //! Construct com_error from HRESULT, textual description, source, iid, help.
        /*!
            \param msg          Description of error.
            \param hr           HRESULT value of error.
            \param src          Description of source line
            \param iid          Interface the error was on
            \param helpFile     Name of help file
            \param helpContext  Name of help Context
        */
        explicit com_error(const bstr_t &msg, HRESULT hr, const bstr_t &src, const uuid_t &iid = uuid_t(),
                const bstr_t &helpFile=bstr_t(), DWORD helpContext = -1)
            : hr_(hr), std::runtime_error("")
        {
            com_ptr<ICreateErrorInfo> cei(impl::CreateErrorInfo());
            if ( ! cei.is_null() )
            {
                try {
                    cei->SetDescription(msg.in());
                    if (!src.is_null() )
                        cei->SetSource( src.in() );
                    if (iid != uuid_t())
                        cei->SetGUID( iid );
                    if (!helpFile.is_null())
                    {
                        cei->SetHelpFile( helpFile.in() );
                        cei->SetHelpContext( helpContext );
                    }
                    ei_ = com_ptr<IErrorInfo>( com_cast(cei)) ;
                } catch (std::bad_alloc&)
                {}
            }
        }

        /// Construct with an error-info and hresult.
        explicit com_error(HRESULT hr, const com_ptr<IErrorInfo>& ei)
            : hr_(hr), ei_(ei), std::runtime_error("")
        {}

    public:
        //! Return a string with a description of the error
        /*!
            what() uses Description from IErrorInfo if such is present, otherwise FormatMessage is used
            to create a description of the HRESULT error value.

            \retval
                A const char* string with a textual description of the error.
        */
        const char* what() const throw()
        {
            try {
                if (what_.empty()) {
                    what_ = s_str();
                }
            } catch (std::bad_alloc&) {
                return 0;
            }

            return what_.c_str();
        }
        /// Returns a std::string with a description of the error.
        std::string s_str() const
        {
            std::string ret;
            get_str(ret);
            return ret;
        }
        /// Returns a std::wstring with a description of the error.
        std::wstring w_str() const
        {
            std::wstring ret;
            get_str(ret);
            return ret;
        }
        /// Returns a tstring with a description of the error.
        tstring t_str() const
        {
            tstring ret;
            get_str(ret);
            return ret;
        }

    private:
        void get_str(std::string& ret) const
        {
            if (ei_.is_null() == false)
            {
                bstr_t bs;
                if (SUCCEEDED(ei_->GetDescription(bs.out())) && !bs.is_empty())
                {
                    ret= bs.s_str();
                    return;
                }
            }

            char* lpMsgBuf;
            if (FormatMessageA(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS, NULL, hr_,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                reinterpret_cast<char*>(&lpMsgBuf), 0, NULL))
            {
                char* lpEnd = lpMsgBuf;
                while (*lpEnd != '\0')
                    lpEnd++;

                while (lpEnd > lpMsgBuf && // Trim trailing newlines
                    (*(lpEnd - 1) == '\n' || *(lpEnd - 1) == '\r'))
                    lpEnd--;

                ret.assign(lpMsgBuf, lpEnd - lpMsgBuf);
                LocalFree(lpMsgBuf);
            }
        }

        void get_str(std::wstring& ret) const
        {
            if (ei_.is_null() == false)
            {
                bstr_t bs;
                if (SUCCEEDED(ei_->GetDescription(bs.out())) && !bs.is_empty())
                {
                     ret = bs.w_str();
                     return;
                }
            }

            wchar_t* lpMsgBuf;
            if (FormatMessageW(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS, NULL, hr_,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                reinterpret_cast<wchar_t*>(&lpMsgBuf), 0, NULL))
            {
                wchar_t* lpEnd = lpMsgBuf;
                while (*lpEnd != L'\0')
                    lpEnd++;

                while (lpEnd > lpMsgBuf && // Trim trailing newlines
                    (*(lpEnd - 1) == L'\n' || *(lpEnd - 1) == L'\r'))
                    lpEnd--;

                ret.assign(lpMsgBuf, lpEnd - lpMsgBuf);
                LocalFree(lpMsgBuf);
            }
        }

    public:

        /** Return the HRESULT for the error.
          */
        HRESULT hr() const throw()
        {
            return hr_;
        }

        /// \name wrappers for IErrorInfo
        //@{

        /// Description of the error.
        bstr_t description() const
        {
            bstr_t rv;
            if (!ei_.is_null()) ei_->GetDescription( rv.out() ) | raise_exception;
            return rv;
        }

        /// The error source description.
        bstr_t source() const
        {
            bstr_t rv;
            if (!ei_.is_null()) ei_->GetSource( rv.out() ) | raise_exception;
            return rv;
        }

        /// Interface IID.
        GUID guid() const
        {
            GUID rv;
            if (!ei_.is_null()) ei_->GetGUID( &rv ) | raise_exception;
            else ZeroMemory(&rv, sizeof(rv));
            return rv;
        }

        /// Helpfile name.
        bstr_t help_file() const
        {
            bstr_t rv;
            if (!ei_.is_null()) ei_->GetHelpFile( rv.out() ) | raise_exception;
            return rv;
        }


        /// Help conext.
        DWORD help_context() const
        {
            DWORD rv = 0;
            if (!ei_.is_null()) ei_->GetHelpContext( &rv ) | raise_exception;
            return rv;
        }
        //@}

        /// Return the error-info object.
        com_ptr<IErrorInfo> get_ei() const
        {
            return ei_;
        }

    private:
        mutable std::string what_;
        com_ptr<IErrorInfo> ei_;
        HRESULT hr_;
    };


    namespace impl {

        inline HRESULT  return_com_error(HRESULT hr, const bstr_t &desc, const bstr_t &src = auto_attach(BSTR(NULL)),
                const uuid_t &iid=CLSID_NULL, const bstr_t &helpFile=bstr_t(), DWORD helpContext = -1)
        {
            com_ptr<ICreateErrorInfo> cei(impl::CreateErrorInfo());
            if (cei.is_null() == false) {
                try {
                    cei->SetDescription(desc.in());
                    if (!src.is_null() )
                        cei->SetSource( src.in() );
                    if (iid != uuid_t())
                        cei->SetGUID( iid );
                    if (!helpFile.is_null())
                    {
                        cei->SetHelpFile( helpFile.in() );
                        cei->SetHelpContext( helpContext );
                    }
                    com_ptr<IErrorInfo> ei = com_cast(cei);
                    ::SetErrorInfo(0, ei.in());
                } catch (std::bad_alloc&)
                {}
            }
            return hr;
        }
        inline HRESULT  return_com_error(const std::exception& err, const bstr_t &src = auto_attach(BSTR(NULL)),
                const uuid_t &iid=CLSID_NULL, const bstr_t &helpFile=bstr_t(), DWORD helpContext = -1)
        {
            return return_com_error( E_FAIL, bstr_t(err.what()),src,iid,helpFile, helpContext );
        }
        inline HRESULT return_com_error(const com_error& err, const bstr_t &src = bstr_t(), const uuid_t &iid = CLSID_NULL) throw()
        {
            const bstr_t &cursrc =err.source();
            const uuid_t &curiid =err.guid();
            // Return error info with more info.
            return return_com_error( err.hr(), err.description(), cursrc.is_null()?src:cursrc,
                    curiid.is_null()?iid:curiid, err.help_file(), err.help_context());
        }


        inline void throw_com_error_(HRESULT hr, const com_ptr<IErrorInfo>& ei)
        {
            throw_error_handler<true>::throw_error(hr, ei);
        }

        // Raising an HRESULT with a message
        inline void raise_exception_t::operator()(const bstr_t& msg, HRESULT hr/* = E_FAIL*/) const
        {
            throw com_error(msg, hr);
        }

        // Raising an HRESULT
        inline void raise_exception_t::operator()(HRESULT hr) const
        {
            throw com_error(hr);
        }

    } // namespace impl

        /*! \addtogroup ErrorHandling
     */
    //@{

    /** Throw COM error using com_error, using HRESULT and IErrorInfo.
      */
    template<typename Itf> inline void throw_com_error(Itf* p, HRESULT hr)
    {
        if (impl::supports_ErrorInfo(p))
        {
            com_ptr<IErrorInfo> ei = impl::GetErrorInfo();
            if (ei.is_null() == false) impl::throw_com_error_(hr, ei);
        }
        throw com_error(hr);
    }

    template<bool OVERRIDE>
    inline void throw_error_handler<OVERRIDE>::throw_error(HRESULT hr, const com_ptr<IErrorInfo> &ei)
    {
        throw com_error(hr, ei);
    }

    /**
     * Construct exception for a method called on a raw COM interface.
     *
     * This method aims to imbue the com_error with as much information about
     * the failure as possible:
     * - If the interface supports IErrorInfo, the information is taken from
     *   the last ErrorInfo set on the current thread.
     * - If not, the HRESULT alone determines the message.
     *
     * Use this constructor immediately after an interface returns a
     * failure code before any other code can call SetErrorInfo and
     * overwrite the error.
     *
     * \param failure_source  Interface operator method returned a failure code.
     * \param hr              HRESULT value of error.
     *
     * \todo  Can we add an optional user-defined message to this?
     */
    template<typename Itf>
    com_error com_error_from_interface(Itf* failure_source, HRESULT hr)
    {
        if (impl::supports_ErrorInfo(failure_source))
            return com_error(hr, impl::GetErrorInfo());
        else
            return com_error(hr);
    }

    template<typename Itf>
    com_error com_error_from_interface(
        com_ptr<Itf> failure_source, HRESULT hr)
    {
        return com_error_from_interface(failure_source.get(), hr);
    }

    //@}
}

#pragma warning(pop)

#endif
