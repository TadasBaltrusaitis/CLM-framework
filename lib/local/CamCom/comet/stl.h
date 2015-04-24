/** \file
  Provide STL extensions.
  */
/*
 * Copyright © 2001 Sofus Mortensen
 * Copyright © 2013 Alexander Lamaison
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

#ifndef COMET_STL_H
#define COMET_STL_H

#include <comet/config.h>

#include <functional>

// add identity, select1st, and select2nd.
#if !defined(__SGI_STL_PORT) && !defined(__MINGW32__)
namespace std {
    /** \internal
     */
    template<typename T> struct identity : public unary_function<T, T> {
        T operator()(const T& t) { return t; }
    };

    /** \internal
     */
    template<typename T> struct select1st : public unary_function<typename T::first_type, T> {
        typename T::first_type operator()(const T& t) { return t.first; }
    };

    /** \internal
     */
    template<typename T> struct select2nd : public unary_function<typename T::second_type, T> {
        typename T::second_type operator()(const T& t) { return t.second; }
    };
}
#else
#include <functional>
#endif

template<typename T> class delete_fun : public std::unary_function<T, void> {
    public:
        void operator()(T& x) { delete x; }
};

template<typename T> class delete2nd_fun : public std::unary_function<T, void> {
    public:
        void operator()(T& x) { delete x.second; }
};

template<typename T> class delete1st_fun : public std::unary_function<T, void> {
    public:
        void operator()(T& x) { delete x.first; }
};



#endif
