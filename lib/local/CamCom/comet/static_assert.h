/** \file
   Provide compile-time asserts.
    See <a href="http://www.boost.org">www.boost.org</a> for most recent version including documentation.
    \author John Maddock
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

#ifndef COMET_STATIC_ASSERT_H
#define COMET_STATIC_ASSERT_H

#include <comet/config.h>

//  (C) Copyright John Maddock 2000.
//  Permission to copy, use, modify, sell and
//  distribute this software is granted provided this copyright notice appears
//  in all copies. This software is provided "as is" without express or implied
//  warranty, and with no claim as to its suitability for any purpose.

//  See http://www.boost.org for most recent version including documentation.

/*
 Revision history:
     02 August 2000
         Initial version.
*/

namespace comet {
    namespace impl {

        template <bool> struct COMPILE_TIME_ASSERTION_FAILURE;

        template <> struct COMPILE_TIME_ASSERTION_FAILURE<true>{};

        template<int> struct ct_assert_test{};

    }
}

#define COMET_STATIC_ASSERT( B ) typedef ::comet::impl::ct_assert_test<sizeof(::comet::impl::COMPILE_TIME_ASSERTION_FAILURE< ( B ) >)> comet_static_assert_typedef_

#endif
