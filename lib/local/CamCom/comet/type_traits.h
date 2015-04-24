/** \file
  * Interrogate traits of template types.
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

/*
 *    Partial copyright for is_const, is_volatile and is_reference.
 *
 *  (C) Copyright Steve Cleary, Beman Dawes, Howard Hinnant & John Maddock 2000.
 *  Permission to copy, use, modify, sell and
 *  distribute this software is granted provided this copyright notice appears
 *  in all copies. This software is provided "as is" without express or implied
 *  warranty, and with no claim as to its suitability for any purpose.
 */

#ifndef COMET_TYPE_TRAITS_H
#define COMET_TYPE_TRAITS_H

namespace comet {

    /** Provides structs to interrogate traits of template types.
      */
    namespace type_traits {

        //////////////////////////////////////////////////////////////////////////
        // From www.boost.org

        //* is a type T  declared const - is_const<T>
        namespace detail{
            typedef char yes_result;
            typedef char (&no_result)[8];
            yes_result is_const_helper(const volatile void*);
            no_result is_const_helper(volatile void *);
            yes_result is_volatile_helper(const volatile void*);
            no_result is_volatile_helper(const void *);

            template <typename T> struct helper { T t; };
        }

        template <typename T> struct is_const
        {
            enum{ result = (sizeof(detail::yes_result) == sizeof( detail::is_const_helper(
                            & ((reinterpret_cast<detail::helper<T> *>(0))->t))
                    ))
            };
        };

        template <typename T> struct is_volatile
        {
            enum{ result = (sizeof(detail::yes_result) == sizeof(detail::is_volatile_helper(
                            & ((reinterpret_cast<detail::helper<T> *>(0))->t))
                        ))
            };
        };

#  pragma warning(push)
#  pragma warning(disable: 4181)

        //* is a type T a reference type - is_reference<T>
        template <typename T> struct is_reference
        {
        private:
            typedef T const volatile cv_t;
        public:
            enum // dwa 10/27/00 - VC6.4 seems to choke on short-circuit (&&,||)
            {    // evaluations in constant expressions
                value = !is_const<cv_t>::result | !is_volatile<cv_t>::result
            };
        };

        template <> struct is_reference<void>
        {
            enum{ value = false };
        };

#  pragma warning(pop)


        //////////////////////////////////////////////////////////////////////////

        template<typename T> struct is_integer { enum { result = false }; };
        template<> struct is_integer<bool> { enum { result = true }; };
        template<> struct is_integer<char> { enum { result = true }; };
        template<> struct is_integer<signed char> { enum { result = true }; };
        template<> struct is_integer<unsigned char> { enum { result = true }; };
//        template<> struct is_integer<wchar_t> { enum { result = true }; };
        template<> struct is_integer<short> { enum { result = true }; };
        template<> struct is_integer<unsigned short> { enum { result = true }; };
        template<> struct is_integer<int> { enum { result = true }; };
        template<> struct is_integer<unsigned int> { enum { result = true }; };
        template<> struct is_integer<long> { enum { result = true }; };
        template<> struct is_integer<unsigned long> { enum { result = true }; };

        //! @if Internal
        /** Taken from STLPort.
         * \internal
         */
        struct _PointerShim {
          // Since the compiler only allows at most one non-trivial
          // implicit conversion we can make use of a shim class to
          // be sure that IsPtr below doesn't accept classes with
          // implicit pointer conversion operators
          _PointerShim(const volatile void*); // no implementation
        };
        //! @endif

        // These are the discriminating functions
        static char __cdecl _IsP(bool, _PointerShim); // no implementation is required
        static char* __cdecl _IsP(bool, ...);          // no implementation is required

        template <class _Tp>
        struct is_pointer {
          // This template meta function takes a type T
          // and returns true exactly when T is a pointer.
          // One can imagine meta-functions discriminating on
          // other criteria.
          static _Tp& __null_rep();
          enum { result = (sizeof(_IsP(false,__null_rep())) == sizeof(char)) };
        };

        // Work out whether the Type is something we can use operator-> on
        // (hopefully).  If we fail, the worst we get is a level 3 warinng.
        template <class _Tp>
        struct is_class_pointer
        {
            enum { result = (is_pointer<_Tp>::result && ! is_integer<_Tp>::result) };
        };

        template<int x> struct int_holder
        {
            enum { result = x };
        };

        template<typename T> struct type_holder
        {
            typedef T result;
        };

/*        template<typename T, typename U> class is_static_compatible
        {
            class yes { };
            class no  {char a[10]; };

            static yes test( U * );
            static no  test( ... );

        public:
            enum { is = sizeof(test(static_cast<T*>(0))) == sizeof(yes) ? true : false };
        };*/

        template<typename T, typename U> class is_cast_operator_compatible
        {
        protected:
            class yes { };
            class no  {char a[10]; };

            static yes test( U* );
            static no  test( ... );

        public:
            enum { is = sizeof(test(*static_cast<T*>(0))) == sizeof(yes) /*? 1 : 0 */};
        };

#ifndef COMET_PARTIAL_SPECIALISATION
        template<typename T> struct conversion_aux
        {
            template<typename U> class X
            {
            protected:
                class yes { };
                class no  {char a[10]; };
                static T makeT();

                static yes test(U);
                static no test(...);
            public:
                enum { exists = sizeof(test(makeT())) == sizeof(yes) };
                enum { same_type = false };
            };


            template<> class X<T>
            {
            public:
                enum { exists = true };
                enum { same_type = true };
            };

        };
        template<typename T, typename U> struct conversion
        {
            enum { exists = conversion_aux<T>::X<U>::exists };
            enum { same_type = conversion_aux<T>::X<U>::same_type };
            enum { exists_two_way = exists && conversion_aux<U>::X<T>::exists };
        };
#else
        template<typename T, typename U> struct conversion
        {
            protected:
                class yes { };
                class no  {char a[10]; };
                static T makeT();

                static yes test(U);
                static no test(...);
            public:
                enum { exists = sizeof(test(makeT())) == sizeof(yes) };
                enum { same_type = false };
                enum { exists_two_way = conversion<U,T>::exists};

        };
        template<typename T> struct conversion<T,T>
        {

            enum { exists = true };
            enum { same_type = true };
            enum { exists_two_way = true};
        };

#endif // COMET_PARTIAL_SPECIALISATION

        template<typename T, typename U> struct super_sub_class
        {
            enum { result = conversion<const U*, const T*>::exists && !conversion<const T*, const void*>::same_type };
        };

        inline bool is_null_vtable_entry(void *p,short n)
        {
            return ((n<0)?true:(0==(*reinterpret_cast<long *>(reinterpret_cast<char *>(*(reinterpret_cast<long *>(p)))+n))));
        }

    }

    namespace impl {

        // used by NutshellImpl's
        struct false_type {};
        struct true_type {};

        template<int T> struct is_one { typedef false_type val; };
        template<> struct is_one<1> { typedef true_type val; };

    }


}


#endif
