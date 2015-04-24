/** \file
* Wrapper for Win32 API HANDLE.
*/
/*
* Copyright © 2001,2004 Sofus Mortensen, Michael Geddes
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

#ifndef COMET_HANDLE_H
#define COMET_HANDLE_H

#include <comet/config.h>
#include <comet/static_assert.h>
#include <comet/error_fwd.h>
#include <comet/common.h>
#include <comet/assert.h>

#include <algorithm>

namespace comet {

    /*! \addtogroup COMType
     */
    //@{

    /** Base and policy class for auto_handle_wrap_t type.
     *  This provides the destroy/detach operations for auto_handle_wrap_t as well as
     *  providing a base class to contain the handle and to add functions to
     *  that are specific to the handle type.
     *  \relates auto_handle_wrap_t
     *  \sa auto_handle_wrap_t
     */
    template< typename H, long INVALID_HANDLE_ >
    class handle_policy_base_t
    {
        public:
            /// Default constructor.
            handle_policy_base_t() : handle_(reinterpret_cast<H>(INVALID_HANDLE_)) {}

            explicit handle_policy_base_t( H handle) : handle_(handle) {}

            typedef H value_type;

            //! Implicit conversion to HANDLE
            operator H() const throw()
            {
                // This function seemed not to work with returning a namespaced
                // type.
                return handle_;
            }
            static inline H invalid_handle() { return reinterpret_cast<H>(INVALID_HANDLE_) ;}

            /// Is the handle valid?
            bool valid() const
            {
                return handle_ != invalid_handle();
            }

            /// Detach a raw handle
            H detach_handle()
            {
                return swap_handle(invalid_handle());
            }

            //! Detaches currently held handle without closing it.
            static inline value_type detach( handle_policy_base_t &handle )throw()
            {
                return handle.detach_handle();
            }

        protected:
            /// Destructor to prevent one of these from being created by itself.
            ~handle_policy_base_t()
            { }

            /// Detach the contained handle to the passed in.
            void detach_to(handle_policy_base_t &rhs) throw()
            {
                value_type handle(handle_);
                handle_= invalid_handle();
                rhs.handle_ = handle;
            }

            H get_handle() const { return handle_; }
            H *get_handle_ptr() { return &handle_;}

            H swap_handle(H new_handle)
            {
                H old = handle_;
                handle_ = new_handle;
                return old;
            }

        private:
            H handle_;
    };

    /** \class handle_nothrow_error_policy_t handle.h comet/handle.h
     * Nothrow Error policy.
     * \relates auto_handle_wrap_t
     */
    struct handle_nothrow_error_policy_t
    {
        static void on_error()
        { }
    };

    /** \class handle_throw_error_policy_t handle.h comet/handle.h
     * Throwing Error policy.
     * \relates auto_handle_wrap_t
     */
    struct handle_throw_error_policy_t
    {
        static void on_error()
        {
            DWORD err = GetLastError();
            raise_exception(HRESULT_FROM_WIN32(err));
        }
    };

    /** \class auto_handle_wrap_t  handle.h comet/handle.h
      * Wrapper for a Win32 API/GDI HANDLE.
      * Behaves similarly to an auto_ptr in that it implements single-reference,
      * reference counting, with reference-transferring assignment and
      * copy-construction.
      */

    template<typename C_, typename H, long INVALID_HANDLE_, typename ERROR_POLICY>
    class auto_handle_wrap_t : public handle_policy_base_t<H, INVALID_HANDLE_>
    {
        typedef typename handle_policy_base_t<H, INVALID_HANDLE_>::value_type value_type;
        protected:

            /// Call destroy_handle
            static inline bool destroy_( value_type h)
            {
                return C_::destroy_handle(h);
            }

            /// Destroy the handle passed in.
            /** Must be implemented by the parent class.
             */
            static bool destroy_handle(value_type h);

            typedef handle_policy_base_t<H, INVALID_HANDLE_> policy_base ;

            void set_handle( value_type h ) throw()
            {
                destroy_(this->swap_handle( h));
            }

            static bool expect_nonzero( BOOL value)
            {
                bool is_ok = (value != FALSE);
                if (!is_ok)
                    ERROR_POLICY::on_error();
                return is_ok;
            }
        public:

            /// default constructor.
            auto_handle_wrap_t() throw()
            {}

            /// A reference to a handle.
            typedef const C_ &ref;

            /** Assign by auto_attach.
            */
            auto_handle_wrap_t( const impl::auto_attach_t<typename policy_base::value_type> &handle ) throw()
                : policy_base(handle.get())
                {}

            /** Non-const copy constructor - takes control of the reference.
             * \param rhs Handle to detach from.
            */
            auto_handle_wrap_t( auto_handle_wrap_t &rhs) throw()
            {
                rhs.detach_to(*this);
            }

            /** Non-const copy constructor - takes control of the reference.
             * \param rhs Handle to detach from.
            */
            auto_handle_wrap_t( policy_base &rhs) throw()
            {
                rhs.detach_to(*this);
            }

            /** Destructor - closes the handle.
            */
            ~auto_handle_wrap_t() throw()
            {
                destroy_(get());
            }


            //! Assignment from similar handles.
            /** Typically, there might be a different error policy.
             */
            template<typename EP>
                auto_handle_wrap_t<C_, H, INVALID_HANDLE_, EP> & operator=(auto_handle_wrap_t<C_, H, INVALID_HANDLE_, EP>& rhs) throw()
                {
                    close_handle();
                    rhs.detach_to(*this);
                    return *this;
                }

            //! Assign by auto_attach
            C_ &operator=(const impl::auto_attach_t<value_type> &handle )
            {
                close_handle();
                set_handle(handle.get());
                return *static_cast<C_ *>(this);
            }

            //! Closes the currently held handle (if any).
            bool close() throw()
            {
                return destroy_(policy_base::detach(*this));
            }

            //! \name Accessor methods
            //@{

            //! Fitter method.
            /*!
              Used when calling a function that take a HANDLE* argument.

              \code
              auto_handle read_pipe, write_pipe;

              CreatePipe(read_pipe.out(), write_pipe.out(), 0, 0));
              \endcode
              */
            typename policy_base::value_type* out() throw()
            {
                close_handle();
                return this->get_handle_ptr();
            }

            typename policy_base::value_type get() const throw()
            { return this->get_handle(); }
            typename policy_base::value_type in() const throw()
            { return this->get_handle(); }
            typename policy_base::value_type *inout() throw()
            { return this->get_handle_ptr(); }
            //@}

            static inline const C_ &create_const_reference(const value_type &val)
            { return *reinterpret_cast<const C_ *>(&val); }

            static inline C_ &create_reference(value_type &val)
            { return *reinterpret_cast<C_ *>(&val); }

            /// Destroy a reference.
            static bool destroy_reference(value_type h)
            {
                return true;
            }

        private:

            //! Closes the currently held handle (if any).
            inline void close_handle()
            {
                if (!close())
                    ERROR_POLICY::on_error();
            }
    };

    namespace impl
    {
        /** \internal
         */
        class THIS_IS_NOT_ALLOWED
        {
            ~THIS_IS_NOT_ALLOWED() {}
        };
    }

    /// Disallow closing of a const handle.
    template< typename H, long INVALID_HANDLE_ >
    inline impl::THIS_IS_NOT_ALLOWED CloseHandle(const handle_policy_base_t<H,INVALID_HANDLE_>&);

    /// Make sure closing of an auto_handle_wrap_t detaches first.
    /** \return true if CloseHandle was successful.
     */
    template< typename H, long INVALID_HANDLE_ >
    bool CloseHandle(handle_policy_base_t<H,INVALID_HANDLE_> &rhs)
    {
        return rhs.close();
    }

    /// Wrapper for HANDLE type
    template< typename ERROR_POLICY = handle_nothrow_error_policy_t >
    struct auto_handle_t : auto_handle_wrap_t< auto_handle_t<ERROR_POLICY>, HANDLE, 0, ERROR_POLICY >
    {
        typedef auto_handle_wrap_t< auto_handle_t, HANDLE, 0, ERROR_POLICY > handle_base;

        /// Default constructor.
        auto_handle_t() {}
        /// Copy constructor.
        auto_handle_t( auto_handle_t &rhs)
            : auto_handle_wrap_t< auto_handle_t, HANDLE, 0, ERROR_POLICY >(rhs)
        {}

        /// Auto_attach constructor.
        auto_handle_t( const impl::auto_attach_t< HANDLE > &rhs )
            : auto_handle_wrap_t< auto_handle_t, HANDLE, 0, ERROR_POLICY >(rhs)
        { }
        /// Default assignment.
        auto_handle_t &operator=(auto_handle_t &rhs)
        {
            handle_base::operator=(rhs);
            return *this;
        }
        auto_handle_t  &operator=( const impl::auto_attach_t<HANDLE> &rhs)
        {
            handle_base::operator=(rhs);
            return *this;
        }

        /// Destroy a handle
        static bool destroy_handle( HANDLE h)
        {
            return ::CloseHandle(h) != FALSE;
        }

    };

    /// Auto handle - wrapper for HANLDE.
    typedef auto_handle_t<> auto_handle;
    /// Auto handle - throwing wrapper for HANDLE.
    typedef auto_handle_t< handle_throw_error_policy_t > auto_handle_throw;

    /// Create a reference object to a handle that doesn't destroy it's contents.
    template <typename T>
    struct auto_reference_t : T
    {
        auto_reference_t( const comet::impl::auto_attach_t<typename T::value_type> &rhs ) : T(rhs)
        {}
        ~auto_reference_t()
        {
            destroy_reference(this->detach_handle());
        }
    };
    //@}
}

using comet::CloseHandle;

#endif
