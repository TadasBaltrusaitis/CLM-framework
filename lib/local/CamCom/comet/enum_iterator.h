/** \file
  * Standard C++ iterators wrapping any COM enumerator
  */
/*
 * Copyright © 2000 Sofus Mortensen
 * Copyright © 2011 Alexander Lamaison
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

#ifndef COMET_ENUM_ITERATOR_H
#define COMET_ENUM_ITERATOR_H

#include <comet/config.h>

#include <comet/enum_common.h>
#include <comet/ptr.h>

#include <stdexcept>

namespace comet {

    /**
     * STL style iterator for COM enumerator interfaces
     */
    template<typename E, typename T=enumerated_type_of<E>::is>
    class enum_iterator : public std::iterator<std::input_iterator_tag, T>
    {
        typedef E                                   enumerator_type;
        typedef typename enumerated_type_of<E>::is  element_type;
        typedef impl::type_policy<element_type>     policy;

        com_ptr<enumerator_type> enum_;

        static value_type policy_init(const element_type& element)
        {
            value_type out;
            policy::init(out, element);
            return out;
        }

        static value_type copy_value_from_other(const enum_iterator& other)
        {
            if (other.is_value_set_)
            {
                value_type v;
                policy::init(v, other.value_);
                return v;
            }
            else
            {
                return value_type();
            }
        }

        value_type value_;

        /**
         * Flag that ensures the value only gets cleared if it's been set.
         *
         * Clearing an uninitialised value could be disastrous as it could
         * contain any random bits which the clearing code could interpret as
         * pointers.
         *
         * This could happen in the situation where the enumerator has no
         * items so the value never gets set.
         */
        bool is_value_set_;

    public:
        enum_iterator(const com_ptr<enumerator_type>& e) :
          enum_(e), is_value_set_(false)
        {
            next();
        }

        enum_iterator() : is_value_set_(false) {}

        enum_iterator(const enum_iterator& other) :
            enum_(other.enum_), value_(copy_value_from_other(other)),
            is_value_set_(other.is_value_set_) {}

        enum_iterator& operator=(const enum_iterator& other)
        {
            enum_iterator copy(other);
            swap(copy);
            return *this;
        }

        void swap(enum_iterator& other)
        {
            enum_.swap(other.enum_);
            std::swap(value_, other.value_);
        }

        /** Move to next element. */
        enum_iterator& operator++()
        {
            next();
            return *this;
        }

        /** Move to next element (post increment). */
        enum_iterator operator++(int)
        {
            enum_iterator t(*this);
            operator++();
            return t;
        }

        /**
         * Compare against end.
         * Comparisons against a non-end iterator throw an exception.
         * \todo Look into doing element comparisons.
         */
        bool operator!=(const enum_iterator& other)
        {
            if (enum_ && other.enum_)
                throw std::logic_error(
                    "enum_iterator comparison does not work");

            return enum_ || other.enum_;
        }

        /** Current element. */
        value_type& operator*()
        {
            return value_;
        }

        value_type* operator->()
        {
            return &value_;
        }

    private:

        void next()
        {
            if (enum_)
            {
                unsigned long fetched = 0;
                element_type pod;

                enum_->Next(1, &pod, &fetched) | raise_exception;
                if (fetched == 0)
                {
                    enum_ = NULL;
                    return;
                }

                try
                {
                    if (is_value_set_)
                        policy::clear(value_);
                    value_ = policy_init(pod);
                    is_value_set_ = true;
                }
                catch (...)
                {
                    policy::clear(pod);
                    throw;
                }
                policy::clear(pod);
            }
        }
    };
}

#endif
