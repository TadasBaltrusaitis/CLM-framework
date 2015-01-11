/** \file
  *  Connection-point traits.
  */

/* Copyright © 2002 Michael Geddes
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

#ifndef INCLUDE_COMET_CPTRAITS_H
#define INCLUDE_COMET_CPTRAITS_H

#ifdef _SHOW_INC
#pragma message("   #Include " __FILE__)
#endif

#include <comet/config.h>
#include <comet/error_fwd.h> // raise_exception

#include <Windows.h> // HRESULT

namespace comet
{
    /*! \struct cp_throw cptraits.h comet/cptraits.h
     * Default 'throw' traits for connection-points.
     * \code Usage
     *      connection_point_for<IMySourceInterface>::connection_point.Fire_MyEvent( arg1, comet::cp_throw());
     * \endcode
     */
    struct cp_throw
    {

        /// Constructor - called before iterating over the connection-points.
        cp_throw() : _hr_(S_OK) { }

        /// Destructor - called after iterating over all the connection-points.
        ~cp_throw() { _hr_ | raise_exception ; }

        /*! Called when a connection point fails.
         * Can also be used to remember the hresult for the destructor.
         *
         * \retval true Cause the on_fail method to be called.
         * \retval false Ignore the failure.
         */
        bool check_fail(HRESULT hr)
        {
            if( FAILED(hr) )
                 _hr_ = hr;
            return false;
        }

        /*! Called when check_fail returns true.
         * \param par_connects Reference to the STL container containing the connection-points.
         * \param par_it The iterator of the failed connection-point.
         */
        template<typename CONNECTIONS>
        static bool on_fail(CONNECTIONS &par_connects, const typename CONNECTIONS::iterator & par_it)
        {
            COMET_NOTUSED(par_it);
            COMET_NOTUSED(par_connects);
            return false;
        }

    protected:
        HRESULT _hr_;

    };

    /*! \struct cp_nothrow_remove cptraits.h comet/cptraits.h
     * Traits for connection-points, errors cause the connection-point to auto-remove.
     * \code
     *      connection_point_for<IMySourceInterface>::connection_point.Fire_MyEvent( arg1, comet::cp_nothrow_remove());
     * \endcode
     * \sa cp_throw
     */
    struct cp_nothrow_remove
    {
        //! Called when a connection point fails.
        bool check_fail( HRESULT _hr_) throw() { return FAILED(_hr_); }

        //! Called when check_fail returns true.
        template<typename CONNECTIONS>
        static bool on_fail(CONNECTIONS &par_connects, const typename CONNECTIONS::iterator &par_it ) throw()
        {
            try{ par_connects.erase(par_it); }catch(...) {}
            return false;
        }
    };


    /*! \struct cp_nothrow cptraits.h comet/cptraits.h
     * Tratis for connection-points, errors are ignored.
     * \code
     *      connection_point_for<IMySourceInterface>::connection_point.Fire_MyEvent( arg1, comet::cp_nothrow());
     * \endcode
     * \sa cp_throw
     */
    struct cp_nothrow
    {
        //! Called when a connection point fails.
        bool check_fail( HRESULT _hr_) throw() { return FAILED(_hr_); }

        //! Called when check_fail returns true.
        template<typename CONNECTIONS>
        static bool on_fail(CONNECTIONS &par_connects, const typename CONNECTIONS::iterator &par_it ) throw() { return false; }
    };
}
#endif /* INCLUDE_COMET_CPTRAITS_H */
