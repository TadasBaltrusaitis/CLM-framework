/** \file
  * Standard C++ iterators wrapping IEnumVARIANT objects.
  */
/*
 * Copyright © 2000 Sofus Mortensen
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

#ifndef COMET_VARIANT_ITERATOR_H
#define COMET_VARIANT_ITERATOR_H

#include <comet/config.h>

#include <comet/ptr.h>
#include <comet/variant.h>

#include <stdexcept>

namespace comet {

    /** \class variant_iterator enum.h comet/enum.h
      * STL style iterator for IEnumVariant interface.
      */
    class variant_iterator
    {
        com_ptr<IEnumVARIANT> enum_;
        variant_t ce_;
    public:
        /** Constructor.
          */
        variant_iterator( const com_ptr<IEnumVARIANT>& e ) : enum_(e) {
            next();
        }

        variant_iterator() {}

        /** Move to next element.
          */
        variant_iterator& operator++() {
            next();
            return *this;
        }

        bool operator!=(const variant_iterator& v) {
            if (!v.enum_.is_null())
                throw std::logic_error(
                    "variant_iterator comparison does not work");

            return !enum_.is_null();
        }

        /** Move to next element (post increment).
          */
        variant_iterator operator++(int) {
            variant_iterator t(*this);
            operator++();
            return t;
        }

        /** Current element.
          */
        variant_t& operator*() {
            return ce_;
        }
    private:
        void next() {
            if (enum_) {
                unsigned long x = 0;
                enum_->Next(1, ce_.out(), &x) | raise_exception;
                if (x == 0) enum_ = 0;
            }
        }
    };

    /** \class itf_iterator enum.h comet/enum.h
      * STL style Iterator for IEnumVARIANT interface returning a contained
      * interface pointer.
      */
    template<typename Itf> class itf_iterator
    {
        com_ptr<IEnumVARIANT> enum_;
        com_ptr<Itf> p_;
    public:
        /** Constructor.
          */
        itf_iterator( const com_ptr<IEnumVARIANT>& e ) : enum_(e) {
            next();
        }

        itf_iterator() {}

        /** Move to next element.
          */
        itf_iterator& operator++() {
            next();
            return *this;
        }

        bool operator!=(const itf_iterator& v) {
            if (v.enum_)
                throw std::logic_error(
                    "itf_iterator comparison does not work");

            return enum_ != 0;
        }

        /** Move to next element.
          */
        itf_iterator operator++(int) {
            itf_iterator t(*this);
            operator++();
            return t;
        }

        /** Access element.
          */
        com_ptr<Itf>& operator*() {
            return p_;
        }
    private:
        void next() {
            if (enum_) {
                unsigned long x = 0;
                variant_t v;
                enum_->Next(1, v.out(), &x) | raise_exception;
                if (x == 0) {
                    enum_ = 0;
                    p_ = 0;
                }
                else {
                    p_ = try_cast(v);
                }
            }
        }
    };

}

#endif
