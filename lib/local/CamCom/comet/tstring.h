/** \file
  * C++ Std. lib TCHAR mappings.
  */
/*
 * Copyright © 2002 Sofus Mortensen
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

#ifndef COMET_TSTRING_H
#define COMET_TSTRING_H

#include <tchar.h>
#include <strsafe.h>
#include <string>
#include <iosfwd>

namespace comet {

    /*! \addtogroup WinUtil
     */
    //@{

#if defined(_MBCS) && !defined(_UNICODE) && !defined(COMET_NO_MBCS_WARNING)
#pragma message( "Warning: _MBCS is defined. Be aware that tstring does not support multi-byte character strings" )
#endif
    /** \page comettstring Comet tstring type.
     *   In order for comet projects to more easily support unicode via the
     *   windows file "tchar.h", many comet classes support the type #comet::tstring which is
     *   a convenience typedef to basic_string.  Associated with
     *   \link comet::tstring tstring \endlink are all or most of the other STL
     *   classes that have a \p char argument to the templates.
     *
     *   In addition to being used by comet, this is a generally useful series
     *   of typedefs to be used when combining STL types with tchar.h compatible
     *   projects.
     *
     *   There are two area to be particularly wary of with STL and tchar.h.
     *
     *   The first is that the filename argument for file streams are always
     *   narrow char *.  I would suggest using a #comet::bstr_t which will
     *   convert for you, especially in UNICODE (actually UCS2 little-endien) projects.
     *
     *   The second is that tstring support for multi-byte character strings is
     *   very minimal. Embedded NULLs in the stream are not a problem for
     *   copying around, however none of the parsing/searching/comparison
     *   methods cope with multi-byte character sets.
     *
     *   I believe that part of the reason for this is the platform specific
     *   nature of multi-byte, and the internal support for a variety of
     *   different MBCS implementations by Microsoft.
     */

    /** TCHAR version of std::basic_string.
     * See \ref comettstring
     */
    typedef std::basic_string< TCHAR > tstring;

    /** TCHAR version of std::basic_ios.
     * \relates tstring
     */
    typedef std::basic_ios<TCHAR, std::char_traits<TCHAR> > tios;
    /** TCHAR version of std::basic_streambuf.
     * \relates tstring
     */
    typedef std::basic_streambuf<TCHAR, std::char_traits<TCHAR> >    tstreambuf;
    /** TCHAR version of std::basic_istream.
     * \relates tstring
     */
    typedef std::basic_istream<TCHAR, std::char_traits<TCHAR> > tistream;
    /** TCHAR version of std::basic_ostream.
     * \relates tstring
     */
    typedef std::basic_ostream<TCHAR, std::char_traits<TCHAR> > tostream;
    /** TCHAR version of std::basic_iostream.
     * \relates tstring
     */
    typedef std::basic_iostream<TCHAR, std::char_traits<TCHAR> > tiostream;
    /** TCHAR version of std::basic_stringbuf.
     * \relates tstring
     */
    typedef std::basic_stringbuf<TCHAR, std::char_traits<TCHAR>, std::allocator<TCHAR> > tstringbuf;
    /** TCHAR version of std::basic_istringstream.
     * \relates tstring
     */
    typedef std::basic_istringstream<TCHAR, std::char_traits<TCHAR>, std::allocator<TCHAR> > tistringstream;
    /** TCHAR version of std::basic_ostringstream.
     * \relates tstring
     */
    typedef std::basic_ostringstream<TCHAR, std::char_traits<TCHAR>, std::allocator<TCHAR> > tostringstream;
    /** TCHAR version of std::basic_stringstream.
     * \relates tstring
     */
    typedef std::basic_stringstream<TCHAR, std::char_traits<TCHAR>, std::allocator<TCHAR> > tstringstream;
    /** TCHAR version of std::basic_filebuf.
     * \relates tstring
     */
    typedef std::basic_filebuf<TCHAR, std::char_traits<TCHAR> > tfilebuf;
    /** TCHAR version of std::basic_ifstream.
     * \relates tstring
     */
    typedef std::basic_ifstream<TCHAR, std::char_traits<TCHAR> > tifstream;
    /** TCHAR version of std::basic_ofstream.
     * \relates tstring
     */
    typedef std::basic_ofstream<TCHAR, std::char_traits<TCHAR> > tofstream;
    /** TCHAR version of std::basic_fstream.
     * \relates tstring
     */
    typedef std::basic_fstream<TCHAR, std::char_traits<TCHAR> > tfstream;

    //@}
}


#endif
