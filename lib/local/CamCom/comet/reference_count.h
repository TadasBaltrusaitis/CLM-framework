/** \file
  * Implements a class that maintains a reference count.
  */
/*
 * Copyright © 2000, 2001 Sofus Mortensen, Paul Hollingsworth
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

#ifndef COMET_REFERENCE_COUNT_H
#define COMET_REFERENCE_COUNT_H

#include <comet/config.h>
#include <algorithm>

namespace comet {
    /*! \addtogroup Misc
     */
    //@{
    /// Simple reference counter.
    class reference_count
    {
    public:
        reference_count() : rc_(0) {};
        explicit reference_count(size_t x) : rc_(reinterpret_cast<size_t*>(x)) {};

        reference_count(const reference_count& x) : rc_(x.rc_) {};

        enum { FLAGVALUE = 1 };
        bool is_flag() const {
            return reinterpret_cast<size_t>(rc_) == 1;
        }

        // is_null
        /** Implies that the there are currently no outstanding references
          * to this object.
          */
        bool is_null() const {
            return rc_ == 0;
        }

        /// Increment count.
        size_t operator++() {
            if (!rc_) {
                rc_ = new size_t(1);
            }
            return ++*rc_;
        }

        /// Decrement count.
        size_t operator--() {
            if (rc_) {
                if (--*rc_ == 0) {
                    delete rc_;
                    rc_ = 0;
                    return 0;
                }
                return *rc_;
            }
            return 0;
        }

        void clear() {
            rc_ = 0;
        }

        void swap(reference_count &rhs) throw() {
            std::swap(rc_, rhs.rc_);
        }

        reference_count& operator=(reference_count& x)
        {
            rc_ = x.rc_;
            return *this;
        }

    private:
        size_t* rc_;
    }; // class reference_count
    //@}
} // namespace comet

namespace std {
    //! Specialisation of std::swap for reference_count.
    template<> inline void swap(comet::reference_count &lhs, comet::reference_count &rhs) COMET_STD_SWAP_NOTHROW
    {
        lhs.swap(rhs);
    }
}

#endif
