/** \file
  * Global Interface Table wrapper.
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

#ifndef COMET_GIT_H
#define COMET_GIT_H

#include <comet/config.h>

#include <comet/ptr.h>

namespace comet {
    /*! \addtogroup COMType
     */
    //@{

    class GIT;

    /// Type-safe GIT Cookie.
    template<typename Itf> class GIT_cookie
    {
        friend class GIT;
    public:
        GIT_cookie(const GIT_cookie& c) : cookie_(c.cookie_) {}
        explicit GIT_cookie(DWORD c) : cookie_(c) {}
        GIT_cookie() : cookie_(0) {}

        DWORD get_cookie() const { return cookie_; }
    private:
        DWORD cookie_;
    };

    /// Global Interface Table wrapper.
    class GIT {
    public:
        GIT() : git_(CLSID_StdGlobalInterfaceTable)
        {}

        /** Register Interface in the GIT.
          * \param ptr Interface to register.
          * \return Type-safe cookie.
          */
        template<typename Itf>
        GIT_cookie<Itf> register_interface(com_ptr<Itf> const & ptr)
        {
            DWORD cookie;
            git_->RegisterInterfaceInGlobal(ptr.get(), uuidof<Itf>(), &cookie) | raise_exception;
            return GIT_cookie<Itf>(cookie);
        }

        /** Retrieve Interface in the GIT.
          * \param c Cookie
          * \return Marshalled interface.
          */
        template<typename Itf>
        com_ptr<Itf> get_interface(GIT_cookie<Itf> const& c)
        {
            Itf* itf;
            git_->GetInterfaceFromGlobal(c.get_cookie(), uuidof<Itf>(), reinterpret_cast<void**>(&itf)) | raise_exception;
            return auto_attach(itf);
        }

        /** Revoke the cookie from the GIT.
          * \param c Cookie.
          */
        template<typename Itf>
        void revoke_interface(GIT_cookie<Itf> const& c)
        {
            HRESULT hr = git_->RevokeInterfaceFromGlobal(c.get_cookie());
            hr;
            assert(SUCCEEDED(hr));
        }

    private:
        com_ptr< ::IGlobalInterfaceTable> git_;
    };
    //@}

} // namespace comet

#endif
