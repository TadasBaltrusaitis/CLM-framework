/** \file
  * Implementation of Lists of types.
  * Type lists are rather integral to the operation of Comet, and provide us
  * with many of the mechanisms that allow us to all but remove the need for
  * \#define macros in code.
  */
/*
 * Copyright © 2000-2002 Sofus Mortensen, Michael Geddes
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

#ifndef COMET_TYPELIST_H
#define COMET_TYPELIST_H

#include <comet/config.h>

namespace comet {

    /** \struct nil  typelist.h comet/typelist.h
      * End of list type.
      */
    struct nil {};

    template<typename T, typename U>
        struct tl_t {
        typedef T head;
        typedef U tail;
    };

#ifndef COMET_TL_TRUNC
#ifndef __BORLANDC__
#define COMET_TL_LIST( X01,X02,X03,X04,X05,X06,X07,X08,X09, \
    X10,X11,X12,X13,X14,X15,X16,X17,X18,X19, \
    X20,X21,X22,X23,X24,X25,X26,X27,X28,X29, \
    X30,X31,X32,X33,X34,X35,X36,X37,X38,X39) \
    X01,X02,X03,X04,X05,X06,X07,X08,X09, \
    X10,X11,X12,X13,X14,X15,X16,X17,X18,X19, \
    X20,X21,X22,X23,X24,X25,X26,X27,X28,X29, \
    X30,X31,X32,X33,X34,X35,X36,X37,X38,X39
#else
#define COMET_TL_LIST( X01,X02,X03,X04,X05,X06,X07,X08,X09, \
    X10,X11,X12,X13,X14,X15,X16,X17,X18,X19, \
    X20,X21,X22,X23,X24,X25,X26,X27,X28,X29, \
    X30,X31,X32,X33,X34,X35,X36,X37,X38,X39) \
    X01,X02,X03,X04,X05,X06,X07,X08,X09, \
    X10,X11,X12,X13,X14,X15,X16,X17,X18,X19, \
    X20,X21,X22,X23,X24,X25,X26,X27,X28,X29
#endif
#else
#define COMET_TL_LIST( X01,X02,X03,X04,X05,X06,X07,X08,X09, \
    X10,X11,X12,X13,X14,X15,X16,X17,X18,X19, \
    X20,X21,X22,X23,X24,X25,X26,X27,X28,X29, \
    X30,X31,X32,X33,X34,X35,X36,X37,X38,X39) \
    X01,X02,X03,X04,X05,X06,X07,X08,X09,X10
#endif



#define COMET_LIST_TEMPLATE \
    typename X00=comet::nil, COMET_TL_LIST( typename X01=comet::nil, typename X02=comet::nil, typename X03=comet::nil, typename X04=comet::nil, \
    typename X05=comet::nil, typename X06=comet::nil, typename X07=comet::nil, typename X08=comet::nil, typename X09=comet::nil, \
    typename X10=comet::nil, typename X11=comet::nil, typename X12=comet::nil, typename X13=comet::nil, typename X14=comet::nil, \
    typename X15=comet::nil, typename X16=comet::nil, typename X17=comet::nil, typename X18=comet::nil, typename X19=comet::nil, \
    typename X20=comet::nil, typename X21=comet::nil, typename X22=comet::nil, typename X23=comet::nil, typename X24=comet::nil, \
    typename X25=comet::nil, typename X26=comet::nil, typename X27=comet::nil, typename X28=comet::nil, typename X29=comet::nil, \
    typename X30=comet::nil, typename X31=comet::nil, typename X32=comet::nil, typename X33=comet::nil, typename X34=comet::nil, \
    typename X35=comet::nil, typename X36=comet::nil, typename X37=comet::nil, typename X38=comet::nil, typename X39=comet::nil)

#define COMET_LIST_TEMPLATE_0 typename X00, COMET_TL_LIST(\
    typename X01, typename X02, typename X03, typename X04, \
    typename X05, typename X06, typename X07, typename X08, typename X09, \
    typename X10, typename X11, typename X12, typename X13, typename X14, \
    typename X15, typename X16, typename X17, typename X18, typename X19, \
    typename X20, typename X21, typename X22, typename X23, typename X24, \
    typename X25, typename X26, typename X27, typename X28, typename X29, \
    typename X30, typename X31, typename X32, typename X33, typename X34, \
    typename X35, typename X36, typename X37, typename X38, typename X39)

#define COMET_LIST_ARG_1 X00, COMET_TL_LIST(\
    X01,X02,X03,X04,X05,X06,X07,X08,X09, \
    X10,X11,X12,X13,X14,X15,X16,X17,X18,X19, \
    X20,X21,X22,X23,X24,X25,X26,X27,X28,X29, \
    X30,X31,X32,X33,X34,X35,X36,X37,X38,X39)

#define COMET_LIST_ARG_0 COMET_TL_LIST(\
    X01,X02,X03,X04,X05,X06,X07,X08,X09, \
    X10,X11,X12,X13,X14,X15,X16,X17,X18,X19, \
    X20,X21,X22,X23,X24,X25,X26,X27,X28,X29, \
    X30,X31,X32,X33,X34,X35,X36,X37,X38,X39)

#define COMET_LIST_NIL comet::nil, COMET_TL_LIST(\
    comet::nil,comet::nil,comet::nil,comet::nil,comet::nil,comet::nil,comet::nil,comet::nil,comet::nil, \
    comet::nil,comet::nil,comet::nil,comet::nil,comet::nil,comet::nil,comet::nil,comet::nil,comet::nil,comet::nil, \
    comet::nil,comet::nil,comet::nil,comet::nil,comet::nil,comet::nil,comet::nil,comet::nil,comet::nil,comet::nil, \
    comet::nil,comet::nil,comet::nil,comet::nil,comet::nil,comet::nil,comet::nil,comet::nil,comet::nil,comet::nil)

    /** \struct make_list typelist.h comet/typelist.h
      * Construct a 'type list' of up to 40 types.
      * A list is a 'head-tail' list terminated by a 'nil' tail struct.
      * It is most easily constructe by using make_list:
       \code
            comet::make_list< FooType,BarType>::result
        \endcode

      * The \e concept of a type-list is that it is either \link comet::nil nil \endlink
      * or a struct containing a \e typedef of \p head to the first element in the
      * list and a typedef of \p tail to a type-list that contains the rest of
      * the list.
      *
      * This means that for a type-list \e LST
      * \code
          LST::head
        \endcode
      * is the first element in the list (assuming it is not
      * \link comet::nil nil \endlink) and
      * \code
         LST::head::tail::head
        \endcode
        would be the second element in the list (assuming <kbd>LST::head::tail</KBD> is not
        \link comet::nil nil \endlink), and so on.

      * Two type lists can be appended together with typelist::append.

        \sa typelist::append
      */
    template<COMET_LIST_TEMPLATE> struct make_list
    {
        typedef tl_t<X00, typename make_list<COMET_LIST_ARG_0>::result> result;
    };

    template<> struct make_list<COMET_LIST_NIL>
    {
        typedef nil result;
    };

    namespace typelist {

    template<typename L> struct length
    {
        enum { value = 1 + length< typename L::tail>::value };
    };

    template<> struct length<nil>
    {
        enum { value = 0 };
    };

#ifdef COMET_PARTIAL_SPECIALISATION
    //! Find the type at the specified index.
    /** \param L Type List
     * \param idx Index
     */
    template<typename L, unsigned idx> struct type_at
    {
        typedef typename type_at<typename L::tail,idx-1>::result result;
    };
    template<typename L> struct type_at<L,0>
    {
        typedef typename L::head result;
    };
    template< unsigned idx> struct type_at<nil,idx>
    {
        typedef nil result;
    };
    template<> struct type_at<nil,0>
    {
        typedef nil result;
    };


    /** \internal
     */
    template<typename L, typename H, typename T> struct index_of_aux
    {
        enum { value = 1+(index_of_aux<typename L::tail,typename L::head, T>::value) };
    };
    template< typename L, typename T> struct index_of_aux<L,T,T>
    {
        enum { value = 0 };
    };
    /** Find the index of the type \a T in the type-list \a L.
     * The result is in index_of::value.
     */
    template<typename L, typename T> struct index_of
    {
        enum { value = index_of_aux<typename L::tail,typename L::head, T >::value };
    };


#else
    template<typename L, unsigned idx> struct type_at;

    template<typename L> struct type_at_aux
    {
        template<unsigned idx> struct X
        {
            typedef typename type_at<typename L::tail, idx-1>::result result;
//            typedef type_at_aux<typename L::tail>::X<idx-1>::result result;
        };

        template<> struct X<0>
        {
            typedef typename L::head result;
        };
    };

    template<> struct type_at_aux<nil>
    {
        template<unsigned idx> struct X
        {
            typedef nil result;
        };
    };


/*    template<> struct type_at_aux< make_list<> >
    {
        template<unsigned idx> struct X
        {
            typedef nil result;
        };
    };*/

    template<typename L, unsigned idx> struct type_at
    {
        typedef typename type_at_aux<L>::X<idx>::result result;
    };

    template<typename L, typename T> struct index_of;

    template<typename HEAD> struct index_of_aux
    {
        template<typename TAIL> struct X1
        {
            template<typename T> struct X2
            {
                enum { value = 1 + index_of<TAIL, T>::value };
            };

            template<> struct X2<HEAD>
            {
                enum { value = 0 };
            };
        };
    };

    template<typename L, typename T> struct index_of
    {
        enum { value = index_of_aux<typename L::head>::X1<typename L::tail>::X2<T>::value };
    };
#endif // COMET_PARTIAL_SPECIALISATION

#undef COMET_LIST_ARG

#ifdef COMET_PARTIAL_SPECIALISATION

    /** \struct append typelist.h comet/typelist.h
     * Appends two type-lists together.
     * Example:
     * \code
         comet::typelist::append<
             comet::make_list< Type1, Type2 >::result,
             comet::make_list< Type3, Type4 >::result
         >
       \endcode
       \sa make_list
     */
    template<typename L1, typename L2> struct append
    {
        typedef typename L1::head head;
        typedef append<typename L1::tail, L2> tail;
    };

    template<typename L2> struct append<nil,L2>
    {
        typedef typename L2::head head;
        typedef typename L2::tail tail;
    };
    template<typename L2> struct append<make_list<>,L2>
    {
        typedef typename L2::head head;
        typedef typename L2::tail tail;
    };
    template<> struct append<nil, nil>
    {
        typedef nil head;
        typedef nil tail;
    };
    template<typename L, typename T> struct append_element
    {
        typedef typename append<L, typename make_list<T>::result>::head head;
        typedef typename append<L, typename make_list<T>::result>::tail tail;
    };
#else // COMET_PARTIAL_SPECIALISATION

    template<typename L1, typename L2> struct append;

    namespace impl {

        template<typename L1> struct append_aux
        {
            template<typename L2> struct with
            {
                typedef typename L1::head head;
                typedef typename append<typename L1::tail, L2> tail;
//                typedef typename append_aux<L1::tail>::with<L2> tail;
            };

            template<> struct with<nil>
            {
                typedef typename L1::head head;
                typedef typename L1::tail tail;
            };
        };

        template<> struct append_aux<nil>
        {
            template<typename L2> struct with
            {
                typedef typename L2::head head;
                typedef typename L2::tail tail;
            };
        };

/*        template<> struct append_aux<make_list<> >
        {
            template<typename L2> struct with
            {
                typedef typename L2::head head;
                typedef typename L2::tail tail;
            };
        };*/

    }

    template<typename L1, typename L2> struct append
    {
        typedef typename impl::append_aux<L1>::with<L2>::head head;
        typedef typename impl::append_aux<L1>::with<L2>::tail tail;
    };

    template<typename L, typename T> struct append_element
    {
        typedef typename impl::append_aux<L>::with< typename make_list<T>::result >::head head;
        typedef typename impl::append_aux<L>::with< typename make_list<T>::result >::tail tail;
    };
#endif // COMET_PARTIAL_SPECIALISATION


#ifdef COMET_NESTED_TEMPLATES
    /** \struct wrap_each typelist.h comet/typelist.h
     * Wrap each of the elements of a list in the specified template class.
     * Due to lack of support for template parameter to templates by Microsoft,
     * macros have been defined as a necessary evil to get arround this.
     * The macros are #COMET_WRAP_EACH_DECLARE and #COMET_WRAP_EACH.
     * There is a propper implementation for compilers that support nested
     * interfaces, which will be selected by the macro as appropriate.
     * \sa COMET_WRAP_EACH_DECLARE COMET_WRAP_EACH
     */
    template<  template< typename > class T, typename L1> struct wrap_each;
    namespace impl
    {
        template< template< typename > class T, typename L1> struct wrap_each_aux
        {
            typedef wrap_each<T, L1> x;
        };
        template< template< typename > class T> struct wrap_each_aux< T, nil>
        {
            typedef nil x;
        };
    }

    template< template< typename > class T, typename L1> struct wrap_each
    {
        typedef T<typename L1::head> head;
        typedef typename impl::wrap_each_aux<T, typename L1::tail>::x tail;
    };

    template< template< typename > class T> struct wrap_each< T, ::comet::nil >
    {
    };
    /** \def COMET_WRAP_EACH_DECLARE(T)
     *  Declare a template for wrapping each element in the specified type.
     *  \relates wrap_each
     */
#define COMET_WRAP_EACH_DECLARE(T)
// MSVC7.1 doesn't like this:
//    template<  template< typename > class T1, typename L1> struct wrap_each;

    /** \def COMET_WRAP_EACH(T, L1)
     *  Produce a list L2 that consists of each member of list L1 wrapped in type T.
     *  This is very much a Lambda calculus concept for those who care.
     *  \relates wrap_each
     */
#define COMET_WRAP_EACH(T, L1) typelist::wrap_each<T, L1>

#else // COMET_NESTED_TEMPLATES
#define COMET_WRAP_EACH_DECLARE(T)                          \
    template< typename L1 > struct wrap_each_aux##T;        \
    namespace impl                                          \
    {                                                       \
        template< typename L1> struct p_wrap_each_aux##T    \
        {                                                   \
            typedef wrap_each_aux##T<L1> x;                 \
        };                                                  \
        template<> struct p_wrap_each_aux##T < ::comet::nil> \
        {                                                   \
        typedef ::comet::nil x;                             \
        };                                                  \
    };                                                      \
    template< typename L1> struct wrap_each_aux##T          \
    {                                                       \
        typedef T<typename L1::head> head;                  \
        typedef typename impl::p_wrap_each_aux##T < typename L1::tail>::x tail;\
    };                                                      \
    template<> struct wrap_each_aux##T< ::comet::nil> { };

#define COMET_WRAP_EACH(T, L1) wrap_each_aux##T<L1>

#endif // COMET_NESTED_TEMPLATES

    template<typename ITF_LIST>    struct ATL_NO_VTABLE inherit_all
        : public ITF_LIST::head, public inherit_all<typename ITF_LIST::tail>
    {};

    template<> struct inherit_all<nil> {};
//    template<> struct inherit_all<make_list<> > {};

    }

/*  MSVC7 chokes on this one - not actually used anywhere

    template<int idx, COMET_LIST_TEMPLATE> struct select
    {
        typedef typelist::type_at< make_list<COMET_LIST_ARG_1>::result, idx>::result result;
    };*/

} // namespace

#endif
