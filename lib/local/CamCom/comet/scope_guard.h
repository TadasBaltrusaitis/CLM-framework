/** \file
  * Scope-guards can be used to proivde transactional integrity.
  *
  * scope_guard and friends are adopted from source by Andrei Alexandrescu and Petru Marginean.
  *
  * See the <a href="http://www.cuj.com/experts/1812/alexandr.htm?topic=experts">article</a>.
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
 */


#ifndef COMET_SCOPE_GUARD_H
#define COMET_SCOPE_GUARD_H

#include <comet/config.h>

namespace comet {

    /*! \addtogroup Misc
     */
    //@{
    namespace impl {

        /** Base class providing dismission and safe execution primitives.
          */
        class scope_guard_impl_base
        {
            scope_guard_impl_base& operator =(const scope_guard_impl_base&);
        protected:
            ~scope_guard_impl_base()
            {
            }
            scope_guard_impl_base(const scope_guard_impl_base& other) throw()
                : dismissed_(other.dismissed_)
            {
                other.dismiss();
            }
            template <typename J> static void safe_execute(J& j) throw()
            {
                if (!j.dismissed_)
                try
                {
                    j.execute();
                }
                catch(...)
                {
                }
            }

            mutable bool dismissed_;
        public:
            scope_guard_impl_base() throw() : dismissed_(false)
            {
            }
            void dismiss() const throw()
            {
                dismissed_ = true;
            }
        };

        /** 0 parameter scope guard.
         * \internal
         */
        template <typename F> class scope_guard_impl_0 : public scope_guard_impl_base
        {
        public:
            ~scope_guard_impl_0() throw()
            {
                safe_execute(*this);
            }

            void execute()
            {
                fun_();
            }

            scope_guard_impl_0(F fun) : fun_(fun)
            {}

        private:
            F fun_;
        };

        /** 1 parameter scope guard.
         * \internal
         */
        template <typename F, typename P1> class scope_guard_impl_1 : public scope_guard_impl_base
        {
        public:
            ~scope_guard_impl_1() throw()
            {
                safe_execute(*this);
            }

            void execute()
            {
                fun_(p1_);
            }

            scope_guard_impl_1(F fun, P1 p1) : fun_(fun), p1_(p1)
            {}

        private:
            F fun_;
            const P1 p1_;
        };

        /** 2 parameter scope guard.
         * \internal
         */
        template <typename F, typename P1, typename P2>    class scope_guard_impl_2: public scope_guard_impl_base
        {
        public:
            ~scope_guard_impl_2() throw()
            {
                safe_execute(*this);
            }

            void execute()
            {
                fun_(p1_, p2_);
            }

            scope_guard_impl_2(F fun, P1 p1, P2 p2) : fun_(fun), p1_(p1), p2_(p2)
            {}

        private:
            F fun_;
            const P1 p1_;
            const P2 p2_;
        };

        /** 3 parameter scope guard.
         * \internal
         */
        template <typename F, typename P1, typename P2, typename P3> class scope_guard_impl_3 : public scope_guard_impl_base
        {
        public:
            ~scope_guard_impl_3() throw()
            {
                safe_execute(*this);
            }

            void execute()
            {
                fun_(p1_, p2_, p3_);
            }

            scope_guard_impl_3(F fun, P1 p1, P2 p2, P3 p3) : fun_(fun), p1_(p1), p2_(p2), p3_(p3)
            {}

        private:
            F fun_;
            const P1 p1_;
            const P2 p2_;
            const P3 p3_;
        };

        /** 0 parameter object scope guard.
         * \internal
         */
        template <class Obj, typename MemFun> class obj_scope_guard_impl_0 : public scope_guard_impl_base
        {
        public:
            ~obj_scope_guard_impl_0() throw()
            {
                safe_execute(*this);
            }

            void execute()
            {
                (obj_.*memFun_)();
            }

            obj_scope_guard_impl_0(Obj& obj, MemFun memFun)
                : obj_(obj), memFun_(memFun) {}

        private:
            Obj& obj_;
            MemFun memFun_;
        };

        /** 1 parameter object scope guard.
         * \internal
         */
        template <class Obj, typename MemFun, typename P1> class obj_scope_guard_impl_1 : public scope_guard_impl_base
        {
        public:
            ~obj_scope_guard_impl_1() throw()
            {
                safe_execute(*this);
            }

            void execute()
            {
                (obj_.*memFun_)(p1_);
            }

            obj_scope_guard_impl_1(Obj& obj, MemFun memFun, P1 p1)
                : obj_(obj), memFun_(memFun), p1_(p1) {}

        private:
            Obj& obj_;
            MemFun memFun_;
            const P1 p1_;
        };

        /** 2 parameter object scope guard.
         * \internal
         */
        template <class Obj, typename MemFun, typename P1, typename P2>    class obj_scope_guard_impl_2 : public scope_guard_impl_base
        {
        public:
            ~obj_scope_guard_impl_2() throw()
            {
                safe_execute(*this);
            }

            void execute()
            {
                (obj_.*memFun_)(p1_, p2_);
            }

            obj_scope_guard_impl_2(Obj& obj, MemFun memFun, P1 p1, P2 p2)
                : obj_(obj), memFun_(memFun), p1_(p1), p2_(p2) {}

        private:
            Obj& obj_;
            MemFun memFun_;
            const P1 p1_;
            const P2 p2_;
        };

        /** Implementation to allow argument to be passed by reference.
         * \internal
         * \sa make_guard
         */
        template <class T> class ref_holder
        {
            T& ref_;
        public:
            ref_holder(T& ref) : ref_(ref) {}
            operator T& () const
            {
                return ref_;
            }
        private:
            // Disable assignment - not implemented
            ref_holder& operator=(const ref_holder&);
        };

    }

    /** Allow an argument to be passed by reference.
      * \code
      *     scope_guard guard = make_guard( fun, by_ref(long) );
      * \endcode
      * \relates comet::scope_guard
      * \internal
      */
    template <class T> inline impl::ref_holder<T> by_ref(T& t)
    {
        return impl::ref_holder<T>(t);
    }

    /** Implements a scope guard with 0 parameters.
      * \param fun Function or \link functor comet::functor \endlink
      * \relates comet::scope_guard
      * \internal
      */
    template <typename F> inline impl::scope_guard_impl_0<F> make_guard(F fun)
    {
        return impl::scope_guard_impl_0<F>(fun);
    }

    /** Implements a scope guard with 1 parameter.
      * \param fun Function or \link functor comet::functor \endlink
      * \relates comet::scope_guard
      * \sa by_ref
      * \internal
      */
    template <typename F, typename P1> inline impl::scope_guard_impl_1<F, P1> make_guard(F fun, P1 p1)
    {
        return impl::scope_guard_impl_1<F, P1>(fun, p1);
    }

    /** Implements a scope guard with 2 parameters.
      * \param fun Function or \link functor comet::functor \endlink
      * \relates comet::scope_guard
      * \sa by_ref
      * \internal
      */
    template <typename F, typename P1, typename P2> inline impl::scope_guard_impl_2<F, P1, P2> make_guard(F fun, P1 p1, P2 p2)
    {
        return impl::scope_guard_impl_2<F, P1, P2>(fun, p1, p2);
    }

    /** Implements a scope guard with 3 parameters.
      * \param fun Function or \link functor comet::functor \endlink
      * \relates comet::scope_guard
      * \sa by_ref
      * \internal
      */
    template <typename F, typename P1, typename P2, typename P3> inline impl::scope_guard_impl_3<F, P1, P2, P3> make_guard(F fun, P1 p1, P2 p2, P3 p3)
    {
        return impl::scope_guard_impl_3<F, P1, P2, P3>(fun, p1, p2, p3);
    }

    /** Implements a scope guard with 0 parameters, calling a memeber function on an object.
      * \param fun Function or \link functor comet::functor \endlink
      * \relates comet::scope_guard
      */
    template <class Obj, typename MemFun> inline impl::obj_scope_guard_impl_0<Obj, MemFun> make_obj_guard(Obj& obj, MemFun memFun)
    {
        return impl::obj_scope_guard_impl_0<Obj, MemFun>(obj, memFun);
    }

    /** Implements a scope guard with 1 parameter, calling a memeber function on an object.
      * \param fun Function or \link functor comet::functor \endlink
      * \relates comet::scope_guard
      * \sa by_ref
      * \internal
      */
    template <class Obj, typename MemFun, typename P1> inline impl::obj_scope_guard_impl_1<Obj, MemFun, P1> make_obj_guard(Obj& obj, MemFun memFun, P1 p1)
    {
        return impl::obj_scope_guard_impl_1<Obj, MemFun, P1>(obj, memFun, p1);
    }

    /** Implements a scope guard with 2 parameters, calling a memeber function on an object.
      * \param fun Function or \link functor comet::functor \endlink
      * \relates comet::scope_guard
      * \sa by_ref
      * \internal
      */
    template <class Obj, typename MemFun, typename P1, typename P2> inline impl::obj_scope_guard_impl_2<Obj, MemFun, P1, P2> make_obj_guard(Obj& obj, MemFun memFun, P1 p1, P2 p2)
    {
        return impl::obj_scope_guard_impl_2<Obj, MemFun, P1, P2>(obj, memFun, p1, p2);
    }

    /** Pointer to a scope guard.
      * Relies on const references holding on to an assigned stack object for
      * the scope of the reference.
      * \sa scope_guard_impl_0 obj_scope_guard_impl_0  scope_guard_impl_1 obj_scope_guard_impl_1  scope_guard_impl_2 obj_scope_guard_impl_2
      */
    typedef const impl::scope_guard_impl_base& scope_guard;

    //@}

} // namespace

#endif
