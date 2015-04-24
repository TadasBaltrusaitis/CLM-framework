/** \file
   Provide run-time asserts.
  */
/*
 * Copyright © 2001 Sofus Mortensen
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

#ifndef COMET_ASSERT_H
#define COMET_ASSERT_H

#include <stdexcept>

# if defined(COMET_ASSERT_THROWS) || defined(COMET_ASSERT_THROWS_ALWAYS)
#  if defined(COMET_ASSERT_THROWS_ALWAYS) || !defined(NDEBUG)
namespace comet
{
    /*! \defgroup ErrorHandling Error handling.
     */
    //@{
//! Indicates a comet assertion failed.
/** This is enabled for debug builds if COMET_ASSERT_THROWS is defined and
 *  enabled for both debug and release if COMET_ASSERT_THROWS_ALWAYS is defined.
 */
struct assert_failed : std::runtime_error
{
    assert_failed( const char *val) : runtime_error(val) {}
};
    //@}
}
#   define COMET_ASSERT(x_) if (x_) ; else throw comet::assert_failed("Assert Failed: " #x_ );
#   define COMET_THROWS_ASSERT throw( comet::assert_failed)
#  else
#   define COMET_ASSERT(x_) ;
#  endif

# else
#  define COMET_THROWS_ASSERT throw()
#  ifndef __CYGWIN__

#   include <crtdbg.h>
#   define COMET_ASSERT _ASSERTE

#  else

#   include <assert.h>
#   define COMET_ASSERT assert
#  endif
# endif
/*! \addtogroup ErrorHandling
 */
//@{
/** \def COMET_ASSERT Assertion in commet.
 *  \sa COMET_THROWS_ASSERT COMET_ASSERT_THROWS_ALWAYS
 */
    //@}

#endif
