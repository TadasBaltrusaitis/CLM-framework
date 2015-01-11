/** \file
  * Base regkey type and error-policy definition.
  */
/*
 * Copyright © 2000, 2001 Paul Hollingsworth
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

#ifndef COMET_REGKEY_H
#define COMET_REGKEY_H

#include <comet/config.h>

#include <comet/registry.h>
#include <comet/error.h>

namespace comet
{
    /** \struct reg_error regkey.h comet/regkey.h
      * Standard error policy, mainly for use by registry functions.
      */
    struct reg_error
    {
        static void on_error(LONG errcode)
        {
            throw com_error(HRESULT_FROM_WIN32(errcode));
        }

        static void on_typemismatch()
        {
            throw com_error(HRESULT_FROM_WIN32(ERROR_INVALID_DATATYPE));
        }
    };

    /** Standard type for use when dealing with registry keys.
      */
    typedef registry::key<reg_error> regkey;

}

namespace std {

    COMET_DECLARE_SWAP(comet::regkey)
    COMET_DECLARE_SWAP(comet::regkey::info_type)
    COMET_DECLARE_SWAP(comet::regkey::mapped_type)
    COMET_DECLARE_SWAP(comet::regkey::values_type)
    COMET_DECLARE_SWAP(comet::regkey::subkeys_type)
    COMET_DECLARE_SWAP(comet::regkey::value_names_type)
    COMET_DECLARE_SWAP(comet::regkey::values_type::iterator)
    COMET_DECLARE_SWAP(comet::regkey::values_type::const_iterator)
    COMET_DECLARE_SWAP(comet::regkey::subkeys_type::iterator)
    COMET_DECLARE_SWAP(comet::regkey::value_names_type::iterator)
}

#endif
