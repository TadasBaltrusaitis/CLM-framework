/** \file
* Array wrapper.
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
*
*/

/*
* comet::array_t is adapted from class array by Nicolai M. Josuttis
*
* (C) Copyright Nicolai M. Josuttis 2001.
* Permission to copy, use, modify, sell and distribute this software
* is granted provided this copyright notice appears in all copies.
* This software is provided "as is" without express or implied
* warranty, and with no claim as to its suitability for any purpose.
*
*/

#ifndef COMET_ARRAY_H
#define COMET_ARRAY_H

#include <cstddef>
#include <stdexcept>
#include <iterator>
#include <algorithm>
#include <vector>

namespace comet {

#pragma pack(push)
#pragma pack(1)

    /*!\addtogroup Misc
     */
    //@{

    template<typename T, size_t SZ> class array_t
    {
        T a_[SZ];
    public:
        typedef T value_type;
        typedef typename std::vector<T>::iterator iterator;
        typedef typename std::vector<T>::const_iterator const_iterator;

        typedef typename std::vector<T>::reverse_iterator reverse_iterator;
        typedef typename std::vector<T>::const_reverse_iterator const_reverse_iterator;

        typedef typename std::vector<T>::size_type size_type;
        typedef typename std::vector<T>::difference_type difference_type;

        typedef T& reference;
        typedef const T& const_reference;

        //    reference operator[](size_type i) { return a_[i]; }
        //    const_reference operator[](size_type i) const { return a_[i]; }

        iterator begin() { return iterator(a_); }
        iterator end() { return iterator(a_ + SZ); }
        const_iterator begin() const { return const_iterator(a_); }
        const_iterator end() const { return const_iterator(a_ + SZ); }

        reverse_iterator rbegin() { return reverse_iterator(a_); }
        reverse_iterator rend() { return reverse_iterator(a_ + SZ); }
        const_reverse_iterator rbegin() const { return const_reverse_iterator(a_); }
        const_reverse_iterator rend() const { return const_reverse_iterator(a_ + SZ); }

        operator const T*() const { return a_; }
        operator T*() { return a_; }

        static size_type size() { return SZ; }
        static bool empty() { return false; }
        static size_type max_size() { return SZ; }
        enum { static_size = SZ };

        reference front() { return a_[0]; }
        const_reference front() const { return a_[0]; }
        reference back() { return a_[SZ-1]; };
        const_reference back() const { return a_[SZ-1]; }

        // swap (note: linear complexity)
        void swap (array_t<T,SZ>& y) {
            std::swap_ranges(begin(),end(),y.begin());
        }

        // assignment with type conversion
        template <typename T2>
        array_t<T,SZ>& operator= (const array_t<T2,SZ>& rhs) {
            std::copy(rhs.begin(),rhs.end(), begin());
            return *this;
        }

        // assign one value to all elements
        void assign (const T& value)
        {
            std::fill_n(begin(),size(),value);
        }

        reference at(size_type i) { rangecheck(i); return a_[i]; }
        const_reference at(size_type i) const { rangecheck(i); return a_[i]; }

    private:
        // check range (may be private because it is static)
        static void rangecheck (size_type i) {
            if (i >= size()) { throw std::range_error("array"); }
        }

    };
    //@}
#pragma pack(pop)

    // comparisons
    template<class T, size_t SZ>
    bool operator== (const array_t<T,SZ>& x, const array_t<T,SZ>& y) {
        return std::equal(x.begin(), x.end(), y.begin());
    }
    template<class T, size_t SZ>
    bool operator< (const array_t<T,SZ>& x, const array_t<T,SZ>& y) {
        return std::lexicographical_compare(x.begin(),x.end(),y.begin(),y.end());
    }
    template<class T, size_t SZ>
    bool operator!= (const array_t<T,SZ>& x, const array_t<T,SZ>& y) {
        return !(x==y);
    }
    template<class T, size_t SZ>
    bool operator> (const array_t<T,SZ>& x, const array_t<T,SZ>& y) {
        return y<x;
    }
    template<class T, size_t SZ>
    bool operator<= (const array_t<T,SZ>& x, const array_t<T,SZ>& y) {
        return !(y<x);
    }
    template<class T, size_t SZ>
    bool operator>= (const array_t<T,SZ>& x, const array_t<T,SZ>& y) {
        return !(x<y);
    }
}

#endif
