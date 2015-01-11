/** \file
  * Simple uncopyable buffer class.
  */
 /*
 * Copyright © 2004, Michael Geddes, Lijun Qin.
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

#ifndef INCLUDE_COMET_AUTO_BUFFER_H
#define INCLUDE_COMET_AUTO_BUFFER_H

#ifdef _SHOW_INC
#pragma message("   #Include " __FILE__)
#endif

namespace comet
{
    /*!\addtogroup Misc
     */
    //@{

    /** \class auto_buffer_t auto_buffer.h comet/auto_buffer.h
     * Simle auto-deleting buffer class.
     * Non-copyable /returnable.
     */
    template <typename T>
    class auto_buffer_t
    {
        public:
            typedef size_t size_type;
            /** Create a buffer of the given size.
             *  This is the only valid constructor.
             */
            explicit auto_buffer_t(size_type size)
            {
                begin_ = new T[size];
            }
            /// Auto-delete the buffer.
            ~auto_buffer_t() throw() { delete[] begin_; }

            /// Clear the buffer.
            void clear()  throw()
            {
                delete[] begin_;
                begin_ =  0;
            }
            /// Is the buffer empty.
            bool empty() const throw()
            {
                return begin_ != NULL;
            }
            /// Resize the buffer.
            void resize( size_type newSize) throw()
            {
                delete[] begin_;
                begin_ =  new T[newSize];
            }

            /// Return a reference to the specifed element.
            T & at( size_type t) throw() { return begin_[t]; }
            T & operator[]( size_type t) throw() { return begin_[t]; }
            /// Return a const reference to the specifed element.
            const T & at( size_type t) const throw() { return begin_[t]; }
            const T & operator[]( size_type t) const throw() { return begin_[t]; }

            /// Detatch the memory.
            T *detach()
            {
                T *val = begin_;
                begin_ = NULL;
                return val;
            }
            /// Return the memory.
            T *get() { return begin_; }
            const T *get()const { return begin_; }

        private:
            /// Can't assign.
            auto_buffer_t &operator=(const auto_buffer_t &);
            /// can't copy.
            auto_buffer_t(const auto_buffer_t&);

            /// Pointer to memory.
            T *begin_;
    };
    //@}
}

#endif /* INCLUDE_COMET_AUTO_BUFFER_H */
