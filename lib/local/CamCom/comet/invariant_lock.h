/** \file
  * Implementation of invariant_lock asserts.
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

#ifndef COMET_UTIL_H
#define COMET_UTIL_H

#include <comet/config.h>

#include <stdexcept>

#include <comet/error.h>

namespace comet {
    /*! \addtogroup Misc
     */
    //@{

    /** Contains implementation of invariant_lock asserts.
      */
    namespace invariant_lock_impl {

        /** Provides base class for invariant_locks.
          * \sa create_invariant_lock enforcer simple_enforcer
          */
        class invariant_lock
        {
        protected:
            invariant_lock() {}
            invariant_lock(const invariant_lock& /*rhs*/) {}
        };

        /** A version of invariant_lock able to call any void member of a class.
          * The member should provide an assertable condition that gets asserted
          * on creation and destruction of the lock.
          * \sa create_invariant_lock
          */
        template<typename CLASS> struct enforcer : public invariant_lock
        {
            void (CLASS::*m_passert) () const;
            const CLASS *m_pobj;

            enforcer(const CLASS *pobj, void (CLASS::*passert) () const
                ) : m_pobj(pobj), m_passert(passert)
            {
                (m_pobj->*m_passert)();
            }
            ~enforcer()
            {
                (m_pobj->*m_passert)();
            }
        };

        /** A version of invariant_lock that calls public method \b assert_valid on
          * construction and destruction of the lock.
          * \sa create_invariant_lock
          */
        template<typename CLASS> struct simple_enforcer : public invariant_lock
        {
            const CLASS *m_pobj;
            simple_enforcer(const CLASS *pobj) : m_pobj(pobj)
            {
                m_pobj->assert_valid();
            }

            ~simple_enforcer()
            {
                m_pobj->assert_valid();
            }
        };
    } // namespace invariant_lock_impl

    /*! Create an invariant_lock.
      * \param pobj The class with the invariant assert
      * \param assert_member The void member to call on construct & destruct
      * \author Paul Hollingsworth (Paul@PaulHollingsworth.com)
      * \relates invariant_lock_impl::invariant_lock
      * \sa enforcer
      */
    template<typename CLASS>
    invariant_lock_impl::enforcer<CLASS> create_invariant_lock(const CLASS *pobj, void (CLASS::*assert_member) () const)
    {
        return invariant_lock_impl::enforcer<CLASS>(pobj, assert_member);
    }

    /*! Create a simple invariant lock.
      *  This lock will expect assert_valid to be publicly defined on the object
      *  provided.
      * \param pobj The class with the invariant assert.
      * \relates invariant_lock_impl::invariant_lock
      * \sa simple_enforcer
      * \author Paul Hollingsworth (Paul@PaulHollingsworth.com)
      */
    template<typename CLASS>
    invariant_lock_impl::simple_enforcer<CLASS> create_invariant_lock(const CLASS *pobj)
    {
        return pobj;
    }

    /*! Pointer class to an invariant_lock.
      * Assigning a temporary to a const reference will cause the object to be
      * kept for the scope of the const reference.
      * \relates invariant_lock_impl::invariant_lock
      * \sa create_invariant_lock
      * \author Paul Hollingsworth (Paul@PaulHollingsworth.com)
    */
    typedef invariant_lock_impl::invariant_lock &invariant_lock;
    //@}

} // namespace

#endif
