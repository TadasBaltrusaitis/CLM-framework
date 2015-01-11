/** \file
  * Functor implementation.
  *
  * functor.h is based on the functor library presented in Chapter 5 of
  * <a href=http://cseng.aw.com/book/0,3828,0201704315,00.html>"Modern C++ Design"</a> by Andrei Alexandrescu.
  *
  */

/*
 * Copyright © 2001 Sofus Mortensen, Michael Geddes
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

#ifndef COMET_FUNCTOR_H
#define COMET_FUNCTOR_H

#include <comet/config.h>
#include <comet/typelist.h>
#include <comet/type_traits.h>
#include <memory>

namespace comet {
    /*! \addtogroup Misc
     */
    //@{

#ifdef COMET_PARTIAL_SPECIALISATION

namespace detail {

    template<bool, typename T> struct parameter_type_aux
    {
        typedef const T& result;
    };

    template<typename T> struct parameter_type_aux<true,T>
    {
        typedef T result;
    };

};

/** \struct parameter_type functor.h comet/functor.h
  * Gives us a reference to a type.  If the type is already a reference, then
  * just returns the type.
  */
template<typename T> struct parameter_type
{
    enum { is_ref = type_traits::is_reference<T>::value  };
    /// A reference to \p T or \p  T if it is already a reference.
    typedef typename detail::parameter_type_aux< (is_ref!=0) ,T >::result result;
};
#else // COMET_PARTIAL_SPECIALISATION
namespace detail {

    template<bool> struct parameter_type_aux
    {
        template<typename T> struct X
        {
            typedef const T& result;
        };
    };

    template<> struct parameter_type_aux<true>
    {
        template<typename T> struct X
        {
            typedef T result;
        };
    };

};

template<typename T> struct parameter_type
{
    enum { is_ref = type_traits::is_reference<T>::value };
    typedef detail::parameter_type_aux< (is_ref!=0) >::X<T>::result result;
};
#endif // COMET_PARTIAL_SPECIALISATION

#ifndef COMET_PARTIAL_SPECIALISATION

#define COMET_PARTIAL_NAME( name ) X
#define COMET_PARTIAL_NS_1(A) >::X<A>
#define COMET_PARTIAL_NS_2(A,B) >::X<A,B>
#define COMET_PARTIAL_NS_3(A,B,C) >::X<A,B,C>

#define COMET_DEFINE_PARTIAL( X1, X2 , name) \
template < X1 > struct name\
{ \
    template < X2 > class X

#define COMET_SPECIALISE_PARTIAL( SX1, X2 , name) \
template <> struct name<SX1>\
{ \
    template < X2 > class X

#define COMET_DEFINE_PARTIAL2( X1, X2, X3 , name) \
template < X1 > struct name\
{ \
    template < X2, X3 > class X

#define COMET_SPECIALISE_PARTIAL2( SX1, X2, X3 , name) \
template <> struct name<SX1>\
{ \
    template < X2, X3 > class X

#define COMET_DEFINE_PARTIAL3( X1, X2, X3, X4 , name) \
template < X1 > struct name\
{ \
    template < X2, X3, X4 > class X

#define COMET_SPECIALISE_PARTIAL3( SX1, X2, X3, X4 , name) \
template <> struct name<SX1>\
{ \
    template < X2, X3, X4 > class X


#define COMET_CLOSE_PARTIAL() };

#else

#define COMET_PARTIAL_NAME( name ) name
#define COMET_PARTIAL_NS_1(A) ,A>
#define COMET_PARTIAL_NS_2(A,B) ,A,B>
#define COMET_PARTIAL_NS_3(A,B,C) ,A,B,C>

#define COMET_CLOSE_PARTIAL()

#define COMET_PARTIAL_CONSTRUCTOR( name ) name

#define COMET_DEFINE_PARTIAL(TN1, X1, TN2, X2 , name) \
template <TN1 X1, TN2 X2 > class name

#define COMET_SPECIALISE_PARTIAL( SX1, TN2,X2 , name) \
template < TN2 X2 > class name<SX1,X2>

#define COMET_DEFINE_PARTIAL2( TN1,X1,TN2,X2,TN3,X3 , name) \
template <TN1 X1, TN2 X2, TN3 X3 > class name

#define COMET_SPECIALISE_PARTIAL2( SX1, TN2,X2, TN3,X3 , name) \
template < TN2 X2, TN3 X3 > class name<SX1,X2,X3>

#define COMET_DEFINE_PARTIAL3(TN1,X1, TN2,X2, TN3,X3, TN4,X4 , name) \
template < TN1 X1, TN2 X2, TN3 X3,TN4 X4 > class name

#define COMET_SPECIALISE_PARTIAL3( SX1, TN2,X2, TN3,X3, TN4,X4 , name) \
template < TN2 X2, TN3 X3, TN4 X4 > class name<SX1,X2,X3,X4>

#endif

/** \struct parameter_types functor.h comet/functor.h
  * Gives the parameter type of parameters 1 thru 16.
  * This gaurantees the arguments pass by reference, without breaking if it is already a
  * reference.
  * \sa parameter_type
  */
template<typename LIST> struct parameter_types
{
    typedef typename parameter_type< typename typelist::type_at<LIST, 0>::result >::result PARM_1;
    typedef typename parameter_type< typename typelist::type_at<LIST, 1>::result >::result PARM_2;
    typedef typename parameter_type< typename typelist::type_at<LIST, 2>::result >::result PARM_3;
    typedef typename parameter_type< typename typelist::type_at<LIST, 3>::result >::result PARM_4;
    typedef typename parameter_type< typename typelist::type_at<LIST, 4>::result >::result PARM_5;
    typedef typename parameter_type< typename typelist::type_at<LIST, 5>::result >::result PARM_6;
    typedef typename parameter_type< typename typelist::type_at<LIST, 6>::result >::result PARM_7;
    typedef typename parameter_type< typename typelist::type_at<LIST, 7>::result >::result PARM_8;
    typedef typename parameter_type< typename typelist::type_at<LIST, 8>::result >::result PARM_9;
    typedef typename parameter_type< typename typelist::type_at<LIST, 9>::result >::result PARM_10;
    typedef typename parameter_type< typename typelist::type_at<LIST, 10>::result >::result PARM_11;
    typedef typename parameter_type< typename typelist::type_at<LIST, 11>::result >::result PARM_12;
    typedef typename parameter_type< typename typelist::type_at<LIST, 12>::result >::result PARM_13;
    typedef typename parameter_type< typename typelist::type_at<LIST, 13>::result >::result PARM_14;
    typedef typename parameter_type< typename typelist::type_at<LIST, 14>::result >::result PARM_15;
    typedef typename parameter_type< typename typelist::type_at<LIST, 15>::result >::result PARM_16;
};

template<typename R, typename LIST> class functor_impl;


/** \struct functor_impl_aux functor.h comet/functor.h
  * Virtual interface for functor implementation.
  * Provide partial specialisation of functor_impl to parameter type.
  * \param L number of arguments.
  * \param R  Return type.
  * \param LIST Argument list.
  */
COMET_DEFINE_PARTIAL2( int,L,typename,R,typename,LIST,functor_impl_aux) : public parameter_types<LIST>
{
    /** Virtual override to call the function.
      * \param arguments Whatever arguments provided by \a LIST
      * \return Return type is \a R
      */
    virtual R operator()( nil arguments ) = 0;

    /** Clone the functor. (easiest way of managing memory - should only be
      * a light implementation anyway).
      */
    virtual functor_impl<R, LIST>* clone() const = 0;
    /** Provide virtual destruction.
      */
    virtual ~COMET_PARTIAL_NAME(functor_impl_aux)() {}
};
COMET_CLOSE_PARTIAL()


COMET_SPECIALISE_PARTIAL2( 0, typename,R, typename,LIST, functor_impl_aux) : public parameter_types<LIST>
{
public:
    virtual R operator()( ) = 0;
    virtual functor_impl<R, LIST>* clone() const = 0;
    virtual ~COMET_PARTIAL_NAME(functor_impl_aux)() {}
};
COMET_CLOSE_PARTIAL()

COMET_SPECIALISE_PARTIAL2(1, typename,R, typename,LIST, functor_impl_aux) : public parameter_types<LIST>
{
public:
    virtual R operator()( PARM_1 ) = 0;
    virtual functor_impl<R, LIST>* clone() const = 0;
    virtual ~COMET_PARTIAL_NAME(functor_impl_aux)() {}
};
COMET_CLOSE_PARTIAL()

COMET_SPECIALISE_PARTIAL2(2, typename,R, typename,LIST, functor_impl_aux) : public parameter_types<LIST>
{
public:
    virtual R operator()( PARM_1, PARM_2 ) = 0;
    virtual functor_impl<R, LIST>* clone() const = 0;
    virtual ~COMET_PARTIAL_NAME(functor_impl_aux)() {}
};
COMET_CLOSE_PARTIAL()

COMET_SPECIALISE_PARTIAL2(3, typename,R, typename,LIST, functor_impl_aux) : public parameter_types<LIST>
{
public:
    virtual R operator()( PARM_1, PARM_2, PARM_3 ) = 0;
    virtual functor_impl<R, LIST>* clone() const = 0;
    virtual ~COMET_PARTIAL_NAME(functor_impl_aux)() {}
};
COMET_CLOSE_PARTIAL()

COMET_SPECIALISE_PARTIAL2(4, typename,R, typename,LIST,functor_impl_aux) : public parameter_types<LIST>
{
public:
    virtual R operator()( PARM_1, PARM_2, PARM_3, PARM_4 ) = 0;
    virtual functor_impl<R, LIST>* clone() const = 0;
    virtual ~COMET_PARTIAL_NAME(functor_impl_aux)() {}
};
COMET_CLOSE_PARTIAL()

COMET_SPECIALISE_PARTIAL2(5,typename,R, typename,LIST,functor_impl_aux) : public parameter_types<LIST>
{
public:
    virtual R operator()( PARM_1, PARM_2, PARM_3, PARM_4, PARM_5 ) = 0;
    virtual functor_impl<R, LIST>* clone() const = 0;
    virtual ~COMET_PARTIAL_NAME(functor_impl_aux)() {}
};
COMET_CLOSE_PARTIAL()

COMET_SPECIALISE_PARTIAL2(6, typename,R, typename,LIST, functor_impl_aux) : public parameter_types<LIST>
{
public:
    virtual R operator()( PARM_1, PARM_2, PARM_3, PARM_4, PARM_5, PARM_6 ) = 0;
    virtual functor_impl<R, LIST>* clone() const = 0;
    virtual ~COMET_PARTIAL_NAME(functor_impl_aux)() {}
};
COMET_CLOSE_PARTIAL()

COMET_SPECIALISE_PARTIAL2(7, typename,R, typename,LIST, functor_impl_aux) : public parameter_types<LIST>
{
public:
    virtual R operator()( PARM_1, PARM_2, PARM_3, PARM_4, PARM_5, PARM_6, PARM_7 ) = 0;
    virtual functor_impl<R, LIST>* clone() const = 0;
    virtual ~COMET_PARTIAL_NAME(functor_impl_aux)() {}
};
COMET_CLOSE_PARTIAL()

COMET_SPECIALISE_PARTIAL2(8, typename,R, typename,LIST, functor_impl_aux) : public parameter_types<LIST>
{
public:
    virtual R operator()( PARM_1, PARM_2, PARM_3, PARM_4, PARM_5, PARM_6, PARM_7, PARM_8 ) = 0;
    virtual functor_impl<R, LIST>* clone() const = 0;
    virtual ~COMET_PARTIAL_NAME(functor_impl_aux)() {}
};
COMET_CLOSE_PARTIAL()

COMET_SPECIALISE_PARTIAL2(9, typename,R, typename,LIST, functor_impl_aux) : public parameter_types<LIST>
{
public:
    virtual R operator()( PARM_1, PARM_2, PARM_3, PARM_4, PARM_5, PARM_6, PARM_7, PARM_8, PARM_9 ) = 0;
    virtual functor_impl<R, LIST>* clone() const = 0;
    virtual ~COMET_PARTIAL_NAME(functor_impl_aux)() {}
};
COMET_CLOSE_PARTIAL()

COMET_SPECIALISE_PARTIAL2(10, typename,R, typename,LIST, functor_impl_aux) : public parameter_types<LIST>
{
public:
    virtual R operator()( PARM_1, PARM_2, PARM_3, PARM_4, PARM_5, PARM_6, PARM_7, PARM_8, PARM_9, PARM_10 ) = 0;
    virtual functor_impl<R, LIST>* clone() const = 0;
    virtual ~COMET_PARTIAL_NAME(functor_impl_aux)() {}
};
COMET_CLOSE_PARTIAL()

COMET_SPECIALISE_PARTIAL2(11, typename,R, typename,LIST, functor_impl_aux) : public parameter_types<LIST>
{
public:
    virtual R operator()( PARM_1, PARM_2, PARM_3, PARM_4, PARM_5, PARM_6, PARM_7, PARM_8, PARM_9, PARM_10, PARM_11 ) = 0;
    virtual functor_impl<R, LIST>* clone() const = 0;
    virtual ~COMET_PARTIAL_NAME(functor_impl_aux)() {}
};
COMET_CLOSE_PARTIAL()

COMET_SPECIALISE_PARTIAL2(12, typename,R, typename,LIST, functor_impl_aux) : public parameter_types<LIST>
{
public:
    virtual R operator()( PARM_1, PARM_2, PARM_3, PARM_4, PARM_5, PARM_6, PARM_7, PARM_8, PARM_9, PARM_10, PARM_11, PARM_12 ) = 0;
    virtual functor_impl<R, LIST>* clone() const = 0;
    virtual ~COMET_PARTIAL_NAME(functor_impl_aux)() {}
};
COMET_CLOSE_PARTIAL()

COMET_SPECIALISE_PARTIAL2(13, typename,R, typename,LIST, functor_impl_aux) : public parameter_types<LIST>
{
public:
    virtual R operator()( PARM_1, PARM_2, PARM_3, PARM_4, PARM_5, PARM_6, PARM_7, PARM_8, PARM_9, PARM_10, PARM_11, PARM_12, PARM_13 ) = 0;
    virtual functor_impl<R, LIST>* clone() const = 0;
    virtual ~COMET_PARTIAL_NAME(functor_impl_aux)() {}
};
COMET_CLOSE_PARTIAL()

COMET_SPECIALISE_PARTIAL2(14, typename,R, typename,LIST, functor_impl_aux) : public parameter_types<LIST>
{
public:
    virtual R operator()( PARM_1, PARM_2, PARM_3, PARM_4, PARM_5, PARM_6, PARM_7, PARM_8, PARM_9, PARM_10, PARM_11, PARM_12, PARM_13, PARM_14 ) = 0;
    virtual functor_impl<R, LIST>* clone() const = 0;
    virtual ~COMET_PARTIAL_NAME(functor_impl_aux)() {}
};
COMET_CLOSE_PARTIAL()

COMET_SPECIALISE_PARTIAL2(15, typename,R, typename,LIST, functor_impl_aux) : public parameter_types<LIST>
{
public:
    virtual R operator()( PARM_1, PARM_2, PARM_3, PARM_4, PARM_5, PARM_6, PARM_7, PARM_8, PARM_9, PARM_10, PARM_11, PARM_12, PARM_13, PARM_14, PARM_15 ) = 0;
    virtual functor_impl<R, LIST>* clone() const = 0;
    virtual ~COMET_PARTIAL_NAME(functor_impl_aux)() {}
};
COMET_CLOSE_PARTIAL()

COMET_SPECIALISE_PARTIAL2(16, typename,R, typename,LIST, functor_impl_aux) : public parameter_types<LIST>
{
public:
    virtual R operator()( PARM_1, PARM_2, PARM_3, PARM_4, PARM_5, PARM_6, PARM_7, PARM_8, PARM_9, PARM_10, PARM_11, PARM_12, PARM_13, PARM_14, PARM_15, PARM_16 ) = 0;
    virtual functor_impl<R, LIST>* clone() const = 0;
    virtual ~COMET_PARTIAL_NAME(functor_impl_aux)() {}
};
COMET_CLOSE_PARTIAL()


/** \struct functor_handler functor.h comet/functor.h
  * Provide implementation of functor_impl virtual type for functions.
  * The implementation is effectively 'partially specialised' to return type.
  */
COMET_DEFINE_PARTIAL2(typename,RT, typename,PF, typename,FUN, functor_handler)
    : public functor_impl< typename  PF::result_type, typename  PF::parm_list >
{
public:
    typedef  typename PF::result_type result_type;

    COMET_PARTIAL_NAME(functor_handler)(const FUN& fun) : fun_(fun) {}
    functor_impl< typename PF::result_type, typename PF::parm_list >* clone() const
    { return new COMET_PARTIAL_NAME(functor_handler)(*this); }

    result_type operator()( )
    { return fun_( ); }

    result_type operator()( typename PF::PARM_1 p1 )
    { return fun_( p1 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2 )
    { return fun_( p1, p2 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3 )
    { return fun_( p1, p2, p3 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3, typename PF::PARM_4 p4 )
    { return fun_( p1, p2, p3, p4 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3, typename PF::PARM_4 p4, typename PF::PARM_5 p5 )
    { return fun_( p1, p2, p3, p4, p5 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3, typename PF::PARM_4 p4, typename PF::PARM_5 p5, typename PF::PARM_6 p6 )
    { return fun_( p1, p2, p3, p4, p5, p6 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3, typename PF::PARM_4 p4, typename PF::PARM_5 p5, typename PF::PARM_6 p6, typename PF::PARM_7 p7 )
    { return fun_( p1, p2, p3, p4, p5, p6, p7 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3, typename PF::PARM_4 p4, typename PF::PARM_5 p5, typename PF::PARM_6 p6, typename PF::PARM_7 p7, typename PF::PARM_8 p8 )
    { return fun_( p1, p2, p3, p4, p5, p6, p7, p8 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3, typename PF::PARM_4 p4, typename PF::PARM_5 p5, typename PF::PARM_6 p6, typename PF::PARM_7 p7, typename PF::PARM_8 p8, typename PF::PARM_9 p9 )
    { return fun_( p1, p2, p3, p4, p5, p6, p7, p8, p9 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3, typename PF::PARM_4 p4, typename PF::PARM_5 p5, typename PF::PARM_6 p6, typename PF::PARM_7 p7, typename PF::PARM_8 p8, typename PF::PARM_9 p9, typename PF::PARM_10 p10 )
    { return fun_( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3, typename PF::PARM_4 p4, typename PF::PARM_5 p5, typename PF::PARM_6 p6, typename PF::PARM_7 p7, typename PF::PARM_8 p8, typename PF::PARM_9 p9, typename PF::PARM_10 p10, typename PF::PARM_11 p11 )
    { return fun_( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3, typename PF::PARM_4 p4, typename PF::PARM_5 p5, typename PF::PARM_6 p6, typename PF::PARM_7 p7, typename PF::PARM_8 p8, typename PF::PARM_9 p9, typename PF::PARM_10 p10, typename PF::PARM_11 p11, typename PF::PARM_12 p12 )
    { return fun_( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3, typename PF::PARM_4 p4, typename PF::PARM_5 p5, typename PF::PARM_6 p6, typename PF::PARM_7 p7, typename PF::PARM_8 p8, typename PF::PARM_9 p9, typename PF::PARM_10 p10, typename PF::PARM_11 p11, typename PF::PARM_12 p12, typename PF::PARM_13 p13 )
    { return fun_( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3, typename PF::PARM_4 p4, typename PF::PARM_5 p5, typename PF::PARM_6 p6, typename PF::PARM_7 p7, typename PF::PARM_8 p8, typename PF::PARM_9 p9, typename PF::PARM_10 p10, typename PF::PARM_11 p11, typename PF::PARM_12 p12, typename PF::PARM_13 p13, typename PF::PARM_14 p14 )
    { return fun_( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3, typename PF::PARM_4 p4, typename PF::PARM_5 p5, typename PF::PARM_6 p6, typename PF::PARM_7 p7, typename PF::PARM_8 p8, typename PF::PARM_9 p9, typename PF::PARM_10 p10, typename PF::PARM_11 p11, typename PF::PARM_12 p12, typename PF::PARM_13 p13, typename PF::PARM_14 p14, typename PF::PARM_15 p15 )
    { return fun_( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3, typename PF::PARM_4 p4, typename PF::PARM_5 p5, typename PF::PARM_6 p6, typename PF::PARM_7 p7, typename PF::PARM_8 p8, typename PF::PARM_9 p9, typename PF::PARM_10 p10, typename PF::PARM_11 p11, typename PF::PARM_12 p12, typename PF::PARM_13 p13, typename PF::PARM_14 p14, typename PF::PARM_15 p15, typename PF::PARM_16 p16 )
    { return fun_( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16 ); }

private:
    FUN fun_;

};
COMET_CLOSE_PARTIAL()

COMET_SPECIALISE_PARTIAL2( void, typename,PF, typename,FUN , functor_handler)
    : public functor_impl< typename PF::result_type, typename PF::parm_list >
{
public:
    typedef typename PF::result_type result_type;

    COMET_PARTIAL_NAME(functor_handler)(const FUN& fun) : fun_(fun) {}

    functor_impl< typename PF::result_type, typename PF::parm_list >* clone() const
    { return new COMET_PARTIAL_NAME(functor_handler)(*this); }
    result_type operator()( )
    { fun_( ); }

    result_type operator()( typename PF::PARM_1 p1 )
    { fun_( p1 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2 )
    { fun_( p1, p2 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3 )
    { fun_( p1, p2, p3 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3, typename PF::PARM_4 p4 )
    { fun_( p1, p2, p3, p4 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3, typename PF::PARM_4 p4, typename PF::PARM_5 p5 )
    { fun_( p1, p2, p3, p4, p5 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3, typename PF::PARM_4 p4, typename PF::PARM_5 p5, typename PF::PARM_6 p6 )
    { fun_( p1, p2, p3, p4, p5, p6 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3, typename PF::PARM_4 p4, typename PF::PARM_5 p5, typename PF::PARM_6 p6, typename PF::PARM_7 p7 )
    { fun_( p1, p2, p3, p4, p5, p6, p7 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3, typename PF::PARM_4 p4, typename PF::PARM_5 p5, typename PF::PARM_6 p6, typename PF::PARM_7 p7, typename PF::PARM_8 p8 )
    { fun_( p1, p2, p3, p4, p5, p6, p7, p8 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3, typename PF::PARM_4 p4, typename PF::PARM_5 p5, typename PF::PARM_6 p6, typename PF::PARM_7 p7, typename PF::PARM_8 p8, typename PF::PARM_9 p9 )
    { fun_( p1, p2, p3, p4, p5, p6, p7, p8, p9 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3, typename PF::PARM_4 p4, typename PF::PARM_5 p5, typename PF::PARM_6 p6, typename PF::PARM_7 p7, typename PF::PARM_8 p8, typename PF::PARM_9 p9, typename PF::PARM_10 p10 )
    { fun_( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3, typename PF::PARM_4 p4, typename PF::PARM_5 p5, typename PF::PARM_6 p6, typename PF::PARM_7 p7, typename PF::PARM_8 p8, typename PF::PARM_9 p9, typename PF::PARM_10 p10, typename PF::PARM_11 p11 )
    { fun_( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3, typename PF::PARM_4 p4, typename PF::PARM_5 p5, typename PF::PARM_6 p6, typename PF::PARM_7 p7, typename PF::PARM_8 p8, typename PF::PARM_9 p9, typename PF::PARM_10 p10, typename PF::PARM_11 p11, typename PF::PARM_12 p12 )
    { fun_( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3, typename PF::PARM_4 p4, typename PF::PARM_5 p5, typename PF::PARM_6 p6, typename PF::PARM_7 p7, typename PF::PARM_8 p8, typename PF::PARM_9 p9, typename PF::PARM_10 p10, typename PF::PARM_11 p11, typename PF::PARM_12 p12, typename PF::PARM_13 p13 )
    { fun_( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3, typename PF::PARM_4 p4, typename PF::PARM_5 p5, typename PF::PARM_6 p6, typename PF::PARM_7 p7, typename PF::PARM_8 p8, typename PF::PARM_9 p9, typename PF::PARM_10 p10, typename PF::PARM_11 p11, typename PF::PARM_12 p12, typename PF::PARM_13 p13, typename PF::PARM_14 p14 )
    { fun_( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3, typename PF::PARM_4 p4, typename PF::PARM_5 p5, typename PF::PARM_6 p6, typename PF::PARM_7 p7, typename PF::PARM_8 p8, typename PF::PARM_9 p9, typename PF::PARM_10 p10, typename PF::PARM_11 p11, typename PF::PARM_12 p12, typename PF::PARM_13 p13, typename PF::PARM_14 p14, typename PF::PARM_15 p15 )
    { fun_( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3, typename PF::PARM_4 p4, typename PF::PARM_5 p5, typename PF::PARM_6 p6, typename PF::PARM_7 p7, typename PF::PARM_8 p8, typename PF::PARM_9 p9, typename PF::PARM_10 p10, typename PF::PARM_11 p11, typename PF::PARM_12 p12, typename PF::PARM_13 p13, typename PF::PARM_14 p14, typename PF::PARM_15 p15, typename PF::PARM_16 p16 )
    { fun_( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16 ); }


private:
    FUN fun_;

};
COMET_CLOSE_PARTIAL()

/** \struct memfun_handler functor.h comet/functor.h
  * Provide implementation of functor_impl virtual type for calling a member on
  * an object.
  * The implementation is effectively 'partially specialised' to return type.
  * \param RT Return type
  * \param PF Parameter types
  * \param OBJ_PTR Object pointer type
  * \param MEMFN_PTR Function pointer type
  */
COMET_DEFINE_PARTIAL3(typename,RT,typename,PF, typename,OBJ_PTR, typename,MEMFN_PTR , memfun_handler)
    : public functor_impl<typename  PF::result_type,typename  PF::parm_list >
{
private:
    OBJ_PTR obj_;
    MEMFN_PTR memfn_;

public:
    typedef typename PF::result_type result_type;

    COMET_PARTIAL_NAME(memfun_handler)(const OBJ_PTR& obj, MEMFN_PTR memfn) : obj_(obj), memfn_(memfn) {}

    functor_impl<typename  PF::result_type,typename  PF::parm_list >* clone() const
    { return new COMET_PARTIAL_NAME(memfun_handler)(*this); }

    result_type operator()( )
    { return ((*obj_).*memfn_)( ); }

    result_type operator()( typename PF::PARM_1 p1 )
    { return ((*obj_).*memfn_)( p1 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2 )
    { return ((*obj_).*memfn_)( p1, p2 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3 )
    { return ((*obj_).*memfn_)( p1, p2, p3 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3, typename PF::PARM_4 p4 )
    { return ((*obj_).*memfn_)( p1, p2, p3, p4 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3, typename PF::PARM_4 p4, typename PF::PARM_5 p5 )
    { return ((*obj_).*memfn_)( p1, p2, p3, p4, p5 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3, typename PF::PARM_4 p4, typename PF::PARM_5 p5, typename PF::PARM_6 p6 )
    { return ((*obj_).*memfn_)( p1, p2, p3, p4, p5, p6 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3, typename PF::PARM_4 p4, typename PF::PARM_5 p5, typename PF::PARM_6 p6, typename PF::PARM_7 p7 )
    { return ((*obj_).*memfn_)( p1, p2, p3, p4, p5, p6, p7 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3, typename PF::PARM_4 p4, typename PF::PARM_5 p5, typename PF::PARM_6 p6, typename PF::PARM_7 p7, typename PF::PARM_8 p8 )
    { return ((*obj_).*memfn_)( p1, p2, p3, p4, p5, p6, p7, p8 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3, typename PF::PARM_4 p4, typename PF::PARM_5 p5, typename PF::PARM_6 p6, typename PF::PARM_7 p7, typename PF::PARM_8 p8, typename PF::PARM_9 p9 )
    { return ((*obj_).*memfn_)( p1, p2, p3, p4, p5, p6, p7, p8, p9 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3, typename PF::PARM_4 p4, typename PF::PARM_5 p5, typename PF::PARM_6 p6, typename PF::PARM_7 p7, typename PF::PARM_8 p8, typename PF::PARM_9 p9, typename PF::PARM_10 p10 )
    { return ((*obj_).*memfn_)( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3, typename PF::PARM_4 p4, typename PF::PARM_5 p5, typename PF::PARM_6 p6, typename PF::PARM_7 p7, typename PF::PARM_8 p8, typename PF::PARM_9 p9, typename PF::PARM_10 p10, typename PF::PARM_11 p11 )
    { return ((*obj_).*memfn_)( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3, typename PF::PARM_4 p4, typename PF::PARM_5 p5, typename PF::PARM_6 p6, typename PF::PARM_7 p7, typename PF::PARM_8 p8, typename PF::PARM_9 p9, typename PF::PARM_10 p10, typename PF::PARM_11 p11, typename PF::PARM_12 p12 )
    { return ((*obj_).*memfn_)( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3, typename PF::PARM_4 p4, typename PF::PARM_5 p5, typename PF::PARM_6 p6, typename PF::PARM_7 p7, typename PF::PARM_8 p8, typename PF::PARM_9 p9, typename PF::PARM_10 p10, typename PF::PARM_11 p11, typename PF::PARM_12 p12, typename PF::PARM_13 p13 )
    { return ((*obj_).*memfn_)( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3, typename PF::PARM_4 p4, typename PF::PARM_5 p5, typename PF::PARM_6 p6, typename PF::PARM_7 p7, typename PF::PARM_8 p8, typename PF::PARM_9 p9, typename PF::PARM_10 p10, typename PF::PARM_11 p11, typename PF::PARM_12 p12, typename PF::PARM_13 p13, typename PF::PARM_14 p14 )
    { return ((*obj_).*memfn_)( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3, typename PF::PARM_4 p4, typename PF::PARM_5 p5, typename PF::PARM_6 p6, typename PF::PARM_7 p7, typename PF::PARM_8 p8, typename PF::PARM_9 p9, typename PF::PARM_10 p10, typename PF::PARM_11 p11, typename PF::PARM_12 p12, typename PF::PARM_13 p13, typename PF::PARM_14 p14, typename PF::PARM_15 p15 )
    { return ((*obj_).*memfn_)( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3, typename PF::PARM_4 p4, typename PF::PARM_5 p5, typename PF::PARM_6 p6, typename PF::PARM_7 p7, typename PF::PARM_8 p8, typename PF::PARM_9 p9, typename PF::PARM_10 p10, typename PF::PARM_11 p11, typename PF::PARM_12 p12, typename PF::PARM_13 p13, typename PF::PARM_14 p14, typename PF::PARM_15 p15, typename PF::PARM_16 p16 )
    { return ((*obj_).*memfn_)( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16 ); }

};
COMET_CLOSE_PARTIAL()

COMET_SPECIALISE_PARTIAL3( void, typename,PF, typename,OBJ_PTR, typename,MEMFN_PTR , memfun_handler)
    : public functor_impl<typename  PF::result_type,typename  PF::parm_list >
{
public:
    typedef typename PF::result_type result_type;

    COMET_PARTIAL_NAME(memfun_handler)(const OBJ_PTR& obj, MEMFN_PTR memfn) : obj_(obj), memfn_(memfn) {}

    functor_impl<typename  PF::result_type,typename  PF::parm_list >* clone() const
    { return new COMET_PARTIAL_NAME(memfun_handler)(*this); }

    result_type operator()( )
    { ((*obj_).*memfn_)( ); }

    result_type operator()( typename PF::PARM_1 p1 )
    { ((*obj_).*memfn_)( p1 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2 )
    { ((*obj_).*memfn_)( p1, p2 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3 )
    { ((*obj_).*memfn_)( p1, p2, p3 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3, typename PF::PARM_4 p4 )
    { ((*obj_).*memfn_)( p1, p2, p3, p4 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3, typename PF::PARM_4 p4, typename PF::PARM_5 p5 )
    { ((*obj_).*memfn_)( p1, p2, p3, p4, p5 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3, typename PF::PARM_4 p4, typename PF::PARM_5 p5, typename PF::PARM_6 p6 )
    { ((*obj_).*memfn_)( p1, p2, p3, p4, p5, p6 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3, typename PF::PARM_4 p4, typename PF::PARM_5 p5, typename PF::PARM_6 p6, typename PF::PARM_7 p7 )
    { ((*obj_).*memfn_)( p1, p2, p3, p4, p5, p6, p7 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3, typename PF::PARM_4 p4, typename PF::PARM_5 p5, typename PF::PARM_6 p6, typename PF::PARM_7 p7, typename PF::PARM_8 p8 )
    { ((*obj_).*memfn_)( p1, p2, p3, p4, p5, p6, p7, p8 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3, typename PF::PARM_4 p4, typename PF::PARM_5 p5, typename PF::PARM_6 p6, typename PF::PARM_7 p7, typename PF::PARM_8 p8, typename PF::PARM_9 p9 )
    { ((*obj_).*memfn_)( p1, p2, p3, p4, p5, p6, p7, p8, p9 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3, typename PF::PARM_4 p4, typename PF::PARM_5 p5, typename PF::PARM_6 p6, typename PF::PARM_7 p7, typename PF::PARM_8 p8, typename PF::PARM_9 p9, typename PF::PARM_10 p10 )
    { ((*obj_).*memfn_)( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3, typename PF::PARM_4 p4, typename PF::PARM_5 p5, typename PF::PARM_6 p6, typename PF::PARM_7 p7, typename PF::PARM_8 p8, typename PF::PARM_9 p9, typename PF::PARM_10 p10, typename PF::PARM_11 p11 )
    { ((*obj_).*memfn_)( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3, typename PF::PARM_4 p4, typename PF::PARM_5 p5, typename PF::PARM_6 p6, typename PF::PARM_7 p7, typename PF::PARM_8 p8, typename PF::PARM_9 p9, typename PF::PARM_10 p10, typename PF::PARM_11 p11, typename PF::PARM_12 p12 )
    { ((*obj_).*memfn_)( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3, typename PF::PARM_4 p4, typename PF::PARM_5 p5, typename PF::PARM_6 p6, typename PF::PARM_7 p7, typename PF::PARM_8 p8, typename PF::PARM_9 p9, typename PF::PARM_10 p10, typename PF::PARM_11 p11, typename PF::PARM_12 p12, typename PF::PARM_13 p13 )
    { ((*obj_).*memfn_)( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3, typename PF::PARM_4 p4, typename PF::PARM_5 p5, typename PF::PARM_6 p6, typename PF::PARM_7 p7, typename PF::PARM_8 p8, typename PF::PARM_9 p9, typename PF::PARM_10 p10, typename PF::PARM_11 p11, typename PF::PARM_12 p12, typename PF::PARM_13 p13, typename PF::PARM_14 p14 )
    { ((*obj_).*memfn_)( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3, typename PF::PARM_4 p4, typename PF::PARM_5 p5, typename PF::PARM_6 p6, typename PF::PARM_7 p7, typename PF::PARM_8 p8, typename PF::PARM_9 p9, typename PF::PARM_10 p10, typename PF::PARM_11 p11, typename PF::PARM_12 p12, typename PF::PARM_13 p13, typename PF::PARM_14 p14, typename PF::PARM_15 p15 )
    { ((*obj_).*memfn_)( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15 ); }

    result_type operator()( typename PF::PARM_1 p1, typename PF::PARM_2 p2, typename PF::PARM_3 p3, typename PF::PARM_4 p4, typename PF::PARM_5 p5, typename PF::PARM_6 p6, typename PF::PARM_7 p7, typename PF::PARM_8 p8, typename PF::PARM_9 p9, typename PF::PARM_10 p10, typename PF::PARM_11 p11, typename PF::PARM_12 p12, typename PF::PARM_13 p13, typename PF::PARM_14 p14, typename PF::PARM_15 p15, typename PF::PARM_16 p16 )
    { ((*obj_).*memfn_)( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16 ); }

private:
    OBJ_PTR obj_;
    MEMFN_PTR memfn_;
};
COMET_CLOSE_PARTIAL()

/** \class functor_impl functor.h comet/functor.h
  * Implementation of a functor.
  * \param R  Return type.
  * \param LIST List of argument types.
  */
template<typename R, typename LIST>
class ATL_NO_VTABLE functor_impl : public functor_impl_aux< typelist::length<LIST>::value COMET_PARTIAL_NS_2(R,LIST)
{};

template<typename R, typename LIST> class functor;

/** Provide various operator() versions to call the virtual functor.
  * This is Effectively paritally specialised to return type (for void
  * implementation).
  * \param R Return type.
  * \param LIST argument list.
  */
COMET_DEFINE_PARTIAL( typename,R,typename,LIST, functor_operators ) : public parameter_types<LIST>
{
    typedef functor<R, LIST> BASE;
public:
    R operator()( )
    { return (*(static_cast<BASE*>(this)->impl()))( ); }

    R operator()( PARM_1 p1 )
    { return (*(static_cast<BASE*>(this)->impl()))( p1 ); }

    R operator()( PARM_1 p1, PARM_2 p2 )
    { return (*(static_cast<BASE*>(this)->impl()))( p1, p2 ); }

    R operator()( PARM_1 p1, PARM_2 p2, PARM_3 p3 )
    { return (*(static_cast<BASE*>(this)->impl()))( p1, p2, p3 ); }

    R operator()( PARM_1 p1, PARM_2 p2, PARM_3 p3, PARM_4 p4 )
    { return (*(static_cast<BASE*>(this)->impl()))( p1, p2, p3, p4 ); }

    R operator()( PARM_1 p1, PARM_2 p2, PARM_3 p3, PARM_4 p4, PARM_5 p5 )
    { return (*(static_cast<BASE*>(this)->impl()))( p1, p2, p3, p4, p5 ); }

    R operator()( PARM_1 p1, PARM_2 p2, PARM_3 p3, PARM_4 p4, PARM_5 p5, PARM_6 p6 )
    { return (*(static_cast<BASE*>(this)->impl()))( p1, p2, p3, p4, p5, p6 ); }

    R operator()( PARM_1 p1, PARM_2 p2, PARM_3 p3, PARM_4 p4, PARM_5 p5, PARM_6 p6, PARM_7 p7 )
    { return (*(static_cast<BASE*>(this)->impl()))( p1, p2, p3, p4, p5, p6, p7 ); }

    R operator()( PARM_1 p1, PARM_2 p2, PARM_3 p3, PARM_4 p4, PARM_5 p5, PARM_6 p6, PARM_7 p7, PARM_8 p8 )
    { return (*(static_cast<BASE*>(this)->impl()))( p1, p2, p3, p4, p5, p6, p7, p8 ); }

    R operator()( PARM_1 p1, PARM_2 p2, PARM_3 p3, PARM_4 p4, PARM_5 p5, PARM_6 p6, PARM_7 p7, PARM_8 p8, PARM_9 p9 )
    { return (*(static_cast<BASE*>(this)->impl()))( p1, p2, p3, p4, p5, p6, p7, p8, p9 ); }

    R operator()( PARM_1 p1, PARM_2 p2, PARM_3 p3, PARM_4 p4, PARM_5 p5, PARM_6 p6, PARM_7 p7, PARM_8 p8, PARM_9 p9, PARM_10 p10 )
    { return (*(static_cast<BASE*>(this)->impl()))( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10 ); }

    R operator()( PARM_1 p1, PARM_2 p2, PARM_3 p3, PARM_4 p4, PARM_5 p5, PARM_6 p6, PARM_7 p7, PARM_8 p8, PARM_9 p9, PARM_10 p10, PARM_11 p11 )
    { return (*(static_cast<BASE*>(this)->impl()))( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11 ); }

    R operator()( PARM_1 p1, PARM_2 p2, PARM_3 p3, PARM_4 p4, PARM_5 p5, PARM_6 p6, PARM_7 p7, PARM_8 p8, PARM_9 p9, PARM_10 p10, PARM_11 p11, PARM_12 p12 )
    { return (*(static_cast<BASE*>(this)->impl()))( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12 ); }

    R operator()( PARM_1 p1, PARM_2 p2, PARM_3 p3, PARM_4 p4, PARM_5 p5, PARM_6 p6, PARM_7 p7, PARM_8 p8, PARM_9 p9, PARM_10 p10, PARM_11 p11, PARM_12 p12, PARM_13 p13 )
    { return (*(static_cast<BASE*>(this)->impl()))( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13 ); }

    R operator()( PARM_1 p1, PARM_2 p2, PARM_3 p3, PARM_4 p4, PARM_5 p5, PARM_6 p6, PARM_7 p7, PARM_8 p8, PARM_9 p9, PARM_10 p10, PARM_11 p11, PARM_12 p12, PARM_13 p13, PARM_14 p14 )
    { return (*(static_cast<BASE*>(this)->impl()))( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14 ); }

    R operator()( PARM_1 p1, PARM_2 p2, PARM_3 p3, PARM_4 p4, PARM_5 p5, PARM_6 p6, PARM_7 p7, PARM_8 p8, PARM_9 p9, PARM_10 p10, PARM_11 p11, PARM_12 p12, PARM_13 p13, PARM_14 p14, PARM_15 p15 )
    { return (*(static_cast<BASE*>(this)->impl()))( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15 ); }

    R operator()( PARM_1 p1, PARM_2 p2, PARM_3 p3, PARM_4 p4, PARM_5 p5, PARM_6 p6, PARM_7 p7, PARM_8 p8, PARM_9 p9, PARM_10 p10, PARM_11 p11, PARM_12 p12, PARM_13 p13, PARM_14 p14, PARM_15 p15, PARM_16 p16 )
    { return (*(static_cast<BASE*>(this)->impl()))( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16 ); }

};
COMET_CLOSE_PARTIAL()

COMET_SPECIALISE_PARTIAL( void,typename,LIST , functor_operators)
    : public parameter_types<LIST>
{
    typedef functor<void, LIST> BASE;
public:

    void operator()( )
    { (*(static_cast<BASE*>(this)->impl()))( ); }

    void operator()( PARM_1 p1 )
    { (*(static_cast<BASE*>(this)->impl()))( p1 ); }

    void operator()( PARM_1 p1, PARM_2 p2 )
    { (*(static_cast<BASE*>(this)->impl()))( p1, p2 ); }

    void operator()( PARM_1 p1, PARM_2 p2, PARM_3 p3 )
    { (*(static_cast<BASE*>(this)->impl()))( p1, p2, p3 ); }

    void operator()( PARM_1 p1, PARM_2 p2, PARM_3 p3, PARM_4 p4 )
    { (*(static_cast<BASE*>(this)->impl()))( p1, p2, p3, p4 ); }

    void operator()( PARM_1 p1, PARM_2 p2, PARM_3 p3, PARM_4 p4, PARM_5 p5 )
    { (*(static_cast<BASE*>(this)->impl()))( p1, p2, p3, p4, p5 ); }

    void operator()( PARM_1 p1, PARM_2 p2, PARM_3 p3, PARM_4 p4, PARM_5 p5, PARM_6 p6 )
    { (*(static_cast<BASE*>(this)->impl()))( p1, p2, p3, p4, p5, p6 ); }

    void operator()( PARM_1 p1, PARM_2 p2, PARM_3 p3, PARM_4 p4, PARM_5 p5, PARM_6 p6, PARM_7 p7 )
    { (*(static_cast<BASE*>(this)->impl()))( p1, p2, p3, p4, p5, p6, p7 ); }

    void operator()( PARM_1 p1, PARM_2 p2, PARM_3 p3, PARM_4 p4, PARM_5 p5, PARM_6 p6, PARM_7 p7, PARM_8 p8 )
    { (*(static_cast<BASE*>(this)->impl()))( p1, p2, p3, p4, p5, p6, p7, p8 ); }

    void operator()( PARM_1 p1, PARM_2 p2, PARM_3 p3, PARM_4 p4, PARM_5 p5, PARM_6 p6, PARM_7 p7, PARM_8 p8, PARM_9 p9 )
    { (*(static_cast<BASE*>(this)->impl()))( p1, p2, p3, p4, p5, p6, p7, p8, p9 ); }

    void operator()( PARM_1 p1, PARM_2 p2, PARM_3 p3, PARM_4 p4, PARM_5 p5, PARM_6 p6, PARM_7 p7, PARM_8 p8, PARM_9 p9, PARM_10 p10 )
    { (*(static_cast<BASE*>(this)->impl()))( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10 ); }

    void operator()( PARM_1 p1, PARM_2 p2, PARM_3 p3, PARM_4 p4, PARM_5 p5, PARM_6 p6, PARM_7 p7, PARM_8 p8, PARM_9 p9, PARM_10 p10, PARM_11 p11 )
    { (*(static_cast<BASE*>(this)->impl()))( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11 ); }

    void operator()( PARM_1 p1, PARM_2 p2, PARM_3 p3, PARM_4 p4, PARM_5 p5, PARM_6 p6, PARM_7 p7, PARM_8 p8, PARM_9 p9, PARM_10 p10, PARM_11 p11, PARM_12 p12 )
    { (*(static_cast<BASE*>(this)->impl()))( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12 ); }

    void operator()( PARM_1 p1, PARM_2 p2, PARM_3 p3, PARM_4 p4, PARM_5 p5, PARM_6 p6, PARM_7 p7, PARM_8 p8, PARM_9 p9, PARM_10 p10, PARM_11 p11, PARM_12 p12, PARM_13 p13 )
    { (*(static_cast<BASE*>(this)->impl()))( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13 ); }

    void operator()( PARM_1 p1, PARM_2 p2, PARM_3 p3, PARM_4 p4, PARM_5 p5, PARM_6 p6, PARM_7 p7, PARM_8 p8, PARM_9 p9, PARM_10 p10, PARM_11 p11, PARM_12 p12, PARM_13 p13, PARM_14 p14 )
    { (*(static_cast<BASE*>(this)->impl()))( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14 ); }

    void operator()( PARM_1 p1, PARM_2 p2, PARM_3 p3, PARM_4 p4, PARM_5 p5, PARM_6 p6, PARM_7 p7, PARM_8 p8, PARM_9 p9, PARM_10 p10, PARM_11 p11, PARM_12 p12, PARM_13 p13, PARM_14 p14, PARM_15 p15 )
    { (*(static_cast<BASE*>(this)->impl()))( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15 ); }

    void operator()( PARM_1 p1, PARM_2 p2, PARM_3 p3, PARM_4 p4, PARM_5 p5, PARM_6 p6, PARM_7 p7, PARM_8 p8, PARM_9 p9, PARM_10 p10, PARM_11 p11, PARM_12 p12, PARM_13 p13, PARM_14 p14, PARM_15 p15, PARM_16 p16 )
    { (*(static_cast<BASE*>(this)->impl()))( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16 ); }

};
COMET_CLOSE_PARTIAL()

/** \class functor  functor.h comet/functor.h
  * Functor pointer class and implementation factory.  This provides the interface to the functor library.
  * \param R Return type
  * \param LIST List of types using \link make_list comet::make_list \endlink
  */
template<typename R, typename LIST = nil> class functor : public functor_operators<R COMET_PARTIAL_NS_1(LIST)
{
public:
    typedef functor_impl<R, LIST> IMPL;

    typedef R result_type;
    typedef LIST parm_list;

    functor() {};
    functor(const functor<R, LIST>& f) : impl_((f.impl_.get()== NULL) ? NULL: f.impl_->clone()) {}
    explicit functor(std::auto_ptr< IMPL > impl) : impl_(impl) {}

    template<typename FUN> explicit functor(int, FUN fun) : impl_(new functor_handler<R COMET_PARTIAL_NS_2((functor<R, LIST>), FUN)(fun))
    {}

    template<typename OBJ_PTR, typename MEMFN_PTR> functor(int, const OBJ_PTR& obj, MEMFN_PTR memfn)
    : impl_(new memfun_handler<R>COMET_PARTIAL_NS<functor<R, LIST>, OBJ_PTR, MEMFN_PTR>(obj, memfn))
    {}

    functor& operator=(const functor& rhs)
    {
        std::auto_ptr<IMPL> tmp((rhs.impl_.get() == NULL)? NULL: rhs.impl_->clone());
        impl_ = tmp;
        return *this;
    }

    std::auto_ptr<IMPL> &impl(){return impl_;}

    bool is_null() const { return impl_.get() == NULL; }
protected:
    std::auto_ptr<IMPL> impl_;
};

/** \class chainer  functor.h comet/functor.h
  * Implements a functor that chains one functor to another functor.
  * \param R Return type
  * \param LIST Typelist of arguments
  * \sa chain
  */
COMET_DEFINE_PARTIAL(typename,R, typename,LIST, chainer) : public functor_impl<R, LIST>
{
private:
typedef functor<R, LIST> FUN;

FUN fun1_;
FUN fun2_;


public:
COMET_PARTIAL_NAME(chainer)(const FUN& fun1, const FUN& fun2) : fun1_(fun1), fun2_(fun2)
{}

COMET_PARTIAL_NAME(chainer)(const COMET_PARTIAL_NAME(chainer)& x) : fun1_(x.fun1_), fun2_(x.fun2_)
{}

functor_impl<R, LIST>* clone() const
{ return new COMET_PARTIAL_NAME(chainer)(*this); }

    R operator()( )
    {
        fun1_( );
        return fun2_( );
    }

    R operator()( typename FUN::PARM_1 p1 )
    {
        fun1_( p1 );
        return fun2_( p1 );
    }

    R operator()( typename FUN::PARM_1 p1, typename FUN::PARM_2 p2 )
    {
        fun1_( p1, p2 );
        return fun2_( p1, p2 );
    }

    R operator()( typename FUN::PARM_1 p1, typename FUN::PARM_2 p2, typename FUN::PARM_3 p3 )
    {
        fun1_( p1, p2, p3 );
        return fun2_( p1, p2, p3 );
    }

    R operator()( typename FUN::PARM_1 p1, typename FUN::PARM_2 p2, typename FUN::PARM_3 p3, typename FUN::PARM_4 p4 )
    {
        fun1_( p1, p2, p3, p4 );
        return fun2_( p1, p2, p3, p4 );
    }

    R operator()( typename FUN::PARM_1 p1, typename FUN::PARM_2 p2, typename FUN::PARM_3 p3, typename FUN::PARM_4 p4, typename FUN::PARM_5 p5 )
    {
        fun1_( p1, p2, p3, p4, p5 );
        return fun2_( p1, p2, p3, p4, p5 );
    }

    R operator()( typename FUN::PARM_1 p1, typename FUN::PARM_2 p2, typename FUN::PARM_3 p3, typename FUN::PARM_4 p4, typename FUN::PARM_5 p5, typename FUN::PARM_6 p6 )
    {
        fun1_( p1, p2, p3, p4, p5, p6 );
        return fun2_( p1, p2, p3, p4, p5, p6 );
    }

    R operator()( typename FUN::PARM_1 p1, typename FUN::PARM_2 p2, typename FUN::PARM_3 p3, typename FUN::PARM_4 p4, typename FUN::PARM_5 p5, typename FUN::PARM_6 p6, typename FUN::PARM_7 p7 )
    {
        fun1_( p1, p2, p3, p4, p5, p6, p7 );
        return fun2_( p1, p2, p3, p4, p5, p6, p7 );
    }

    R operator()( typename FUN::PARM_1 p1, typename FUN::PARM_2 p2, typename FUN::PARM_3 p3, typename FUN::PARM_4 p4, typename FUN::PARM_5 p5, typename FUN::PARM_6 p6, typename FUN::PARM_7 p7, typename FUN::PARM_8 p8 )
    {
        fun1_( p1, p2, p3, p4, p5, p6, p7, p8 );
        return fun2_( p1, p2, p3, p4, p5, p6, p7, p8 );
    }

    R operator()( typename FUN::PARM_1 p1, typename FUN::PARM_2 p2, typename FUN::PARM_3 p3, typename FUN::PARM_4 p4, typename FUN::PARM_5 p5, typename FUN::PARM_6 p6, typename FUN::PARM_7 p7, typename FUN::PARM_8 p8, typename FUN::PARM_9 p9 )
    {
        fun1_( p1, p2, p3, p4, p5, p6, p7, p8, p9 );
        return fun2_( p1, p2, p3, p4, p5, p6, p7, p8, p9 );
    }

    R operator()( typename FUN::PARM_1 p1, typename FUN::PARM_2 p2, typename FUN::PARM_3 p3, typename FUN::PARM_4 p4, typename FUN::PARM_5 p5, typename FUN::PARM_6 p6, typename FUN::PARM_7 p7, typename FUN::PARM_8 p8, typename FUN::PARM_9 p9, typename FUN::PARM_10 p10 )
    {
        fun1_( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10 );
        return fun2_( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10 );
    }

    R operator()( typename FUN::PARM_1 p1, typename FUN::PARM_2 p2, typename FUN::PARM_3 p3, typename FUN::PARM_4 p4, typename FUN::PARM_5 p5, typename FUN::PARM_6 p6, typename FUN::PARM_7 p7, typename FUN::PARM_8 p8, typename FUN::PARM_9 p9, typename FUN::PARM_10 p10, typename FUN::PARM_11 p11 )
    {
        fun1_( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11 );
        return fun2_( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11 );
    }

    R operator()( typename FUN::PARM_1 p1, typename FUN::PARM_2 p2, typename FUN::PARM_3 p3, typename FUN::PARM_4 p4, typename FUN::PARM_5 p5, typename FUN::PARM_6 p6, typename FUN::PARM_7 p7, typename FUN::PARM_8 p8, typename FUN::PARM_9 p9, typename FUN::PARM_10 p10, typename FUN::PARM_11 p11, typename FUN::PARM_12 p12 )
    {
        fun1_( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12 );
        return fun2_( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12 );
    }

    R operator()( typename FUN::PARM_1 p1, typename FUN::PARM_2 p2, typename FUN::PARM_3 p3, typename FUN::PARM_4 p4, typename FUN::PARM_5 p5, typename FUN::PARM_6 p6, typename FUN::PARM_7 p7, typename FUN::PARM_8 p8, typename FUN::PARM_9 p9, typename FUN::PARM_10 p10, typename FUN::PARM_11 p11, typename FUN::PARM_12 p12, typename FUN::PARM_13 p13 )
    {
        fun1_( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13 );
        return fun2_( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13 );
    }

    R operator()( typename FUN::PARM_1 p1, typename FUN::PARM_2 p2, typename FUN::PARM_3 p3, typename FUN::PARM_4 p4, typename FUN::PARM_5 p5, typename FUN::PARM_6 p6, typename FUN::PARM_7 p7, typename FUN::PARM_8 p8, typename FUN::PARM_9 p9, typename FUN::PARM_10 p10, typename FUN::PARM_11 p11, typename FUN::PARM_12 p12, typename FUN::PARM_13 p13, typename FUN::PARM_14 p14 )
    {
        fun1_( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14 );
        return fun2_( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14 );
    }

    R operator()( typename FUN::PARM_1 p1, typename FUN::PARM_2 p2, typename FUN::PARM_3 p3, typename FUN::PARM_4 p4, typename FUN::PARM_5 p5, typename FUN::PARM_6 p6, typename FUN::PARM_7 p7, typename FUN::PARM_8 p8, typename FUN::PARM_9 p9, typename FUN::PARM_10 p10, typename FUN::PARM_11 p11, typename FUN::PARM_12 p12, typename FUN::PARM_13 p13, typename FUN::PARM_14 p14, typename FUN::PARM_15 p15 )
    {
        fun1_( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15 );
        return fun2_( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15 );
    }

    R operator()( typename FUN::PARM_1 p1, typename FUN::PARM_2 p2, typename FUN::PARM_3 p3, typename FUN::PARM_4 p4, typename FUN::PARM_5 p5, typename FUN::PARM_6 p6, typename FUN::PARM_7 p7, typename FUN::PARM_8 p8, typename FUN::PARM_9 p9, typename FUN::PARM_10 p10, typename FUN::PARM_11 p11, typename FUN::PARM_12 p12, typename FUN::PARM_13 p13, typename FUN::PARM_14 p14, typename FUN::PARM_15 p15, typename FUN::PARM_16 p16 )
    {
        fun1_( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16 );
        return fun2_( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16 );
    }


};
COMET_CLOSE_PARTIAL()

/** Specialisation for void.
  */
COMET_SPECIALISE_PARTIAL( void, typename,LIST , chainer )
    : public functor_impl<void, LIST>
{
private:
    typedef functor<void, LIST> FUN;

    FUN fun1_;
    FUN fun2_;


public:
    COMET_PARTIAL_NAME(chainer)(const FUN& fun1, const FUN& fun2) : fun1_(fun1), fun2_(fun2)
    {}

    COMET_PARTIAL_NAME(chainer)(const COMET_PARTIAL_NAME(chainer)& x) : fun1_(x.fun1_), fun2_(x.fun2_)
    {}

    functor_impl<void, LIST>* clone() const
    { return new COMET_PARTIAL_NAME(chainer)(*this); }

    void operator()( )
    {
        fun1_( );
        fun2_( );
    }

    void operator()( typename FUN::PARM_1 p1 )
    {
        fun1_( p1 );
        fun2_( p1 );
    }

    void operator()( typename FUN::PARM_1 p1, typename FUN::PARM_2 p2 )
    {
        fun1_( p1, p2 );
        fun2_( p1, p2 );
    }

    void operator()( typename FUN::PARM_1 p1, typename FUN::PARM_2 p2, typename FUN::PARM_3 p3 )
    {
        fun1_( p1, p2, p3 );
        fun2_( p1, p2, p3 );
    }

    void operator()( typename FUN::PARM_1 p1, typename FUN::PARM_2 p2, typename FUN::PARM_3 p3, typename FUN::PARM_4 p4 )
    {
        fun1_( p1, p2, p3, p4 );
        fun2_( p1, p2, p3, p4 );
    }

    void operator()( typename FUN::PARM_1 p1, typename FUN::PARM_2 p2, typename FUN::PARM_3 p3, typename FUN::PARM_4 p4, typename FUN::PARM_5 p5 )
    {
        fun1_( p1, p2, p3, p4, p5 );
        fun2_( p1, p2, p3, p4, p5 );
    }

    void operator()( typename FUN::PARM_1 p1, typename FUN::PARM_2 p2, typename FUN::PARM_3 p3, typename FUN::PARM_4 p4, typename FUN::PARM_5 p5, typename FUN::PARM_6 p6 )
    {
        fun1_( p1, p2, p3, p4, p5, p6 );
        fun2_( p1, p2, p3, p4, p5, p6 );
    }

    void operator()( typename FUN::PARM_1 p1, typename FUN::PARM_2 p2, typename FUN::PARM_3 p3, typename FUN::PARM_4 p4, typename FUN::PARM_5 p5, typename FUN::PARM_6 p6, typename FUN::PARM_7 p7 )
    {
        fun1_( p1, p2, p3, p4, p5, p6, p7 );
        fun2_( p1, p2, p3, p4, p5, p6, p7 );
    }

    void operator()( typename FUN::PARM_1 p1, typename FUN::PARM_2 p2, typename FUN::PARM_3 p3, typename FUN::PARM_4 p4, typename FUN::PARM_5 p5, typename FUN::PARM_6 p6, typename FUN::PARM_7 p7, typename FUN::PARM_8 p8 )
    {
        fun1_( p1, p2, p3, p4, p5, p6, p7, p8 );
        fun2_( p1, p2, p3, p4, p5, p6, p7, p8 );
    }

    void operator()( typename FUN::PARM_1 p1, typename FUN::PARM_2 p2, typename FUN::PARM_3 p3, typename FUN::PARM_4 p4, typename FUN::PARM_5 p5, typename FUN::PARM_6 p6, typename FUN::PARM_7 p7, typename FUN::PARM_8 p8, typename FUN::PARM_9 p9 )
    {
        fun1_( p1, p2, p3, p4, p5, p6, p7, p8, p9 );
        fun2_( p1, p2, p3, p4, p5, p6, p7, p8, p9 );
    }

    void operator()( typename FUN::PARM_1 p1, typename FUN::PARM_2 p2, typename FUN::PARM_3 p3, typename FUN::PARM_4 p4, typename FUN::PARM_5 p5, typename FUN::PARM_6 p6, typename FUN::PARM_7 p7, typename FUN::PARM_8 p8, typename FUN::PARM_9 p9, typename FUN::PARM_10 p10 )
    {
        fun1_( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10 );
        fun2_( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10 );
    }

    void operator()( typename FUN::PARM_1 p1, typename FUN::PARM_2 p2, typename FUN::PARM_3 p3, typename FUN::PARM_4 p4, typename FUN::PARM_5 p5, typename FUN::PARM_6 p6, typename FUN::PARM_7 p7, typename FUN::PARM_8 p8, typename FUN::PARM_9 p9, typename FUN::PARM_10 p10, typename FUN::PARM_11 p11 )
    {
        fun1_( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11 );
        fun2_( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11 );
    }

    void operator()( typename FUN::PARM_1 p1, typename FUN::PARM_2 p2, typename FUN::PARM_3 p3, typename FUN::PARM_4 p4, typename FUN::PARM_5 p5, typename FUN::PARM_6 p6, typename FUN::PARM_7 p7, typename FUN::PARM_8 p8, typename FUN::PARM_9 p9, typename FUN::PARM_10 p10, typename FUN::PARM_11 p11, typename FUN::PARM_12 p12 )
    {
        fun1_( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12 );
        fun2_( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12 );
    }

    void operator()( typename FUN::PARM_1 p1, typename FUN::PARM_2 p2, typename FUN::PARM_3 p3, typename FUN::PARM_4 p4, typename FUN::PARM_5 p5, typename FUN::PARM_6 p6, typename FUN::PARM_7 p7, typename FUN::PARM_8 p8, typename FUN::PARM_9 p9, typename FUN::PARM_10 p10, typename FUN::PARM_11 p11, typename FUN::PARM_12 p12, typename FUN::PARM_13 p13 )
    {
        fun1_( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13 );
        fun2_( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13 );
    }

    void operator()( typename FUN::PARM_1 p1, typename FUN::PARM_2 p2, typename FUN::PARM_3 p3, typename FUN::PARM_4 p4, typename FUN::PARM_5 p5, typename FUN::PARM_6 p6, typename FUN::PARM_7 p7, typename FUN::PARM_8 p8, typename FUN::PARM_9 p9, typename FUN::PARM_10 p10, typename FUN::PARM_11 p11, typename FUN::PARM_12 p12, typename FUN::PARM_13 p13, typename FUN::PARM_14 p14 )
    {
        fun1_( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14 );
        fun2_( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14 );
    }

    void operator()( typename FUN::PARM_1 p1, typename FUN::PARM_2 p2, typename FUN::PARM_3 p3, typename FUN::PARM_4 p4, typename FUN::PARM_5 p5, typename FUN::PARM_6 p6, typename FUN::PARM_7 p7, typename FUN::PARM_8 p8, typename FUN::PARM_9 p9, typename FUN::PARM_10 p10, typename FUN::PARM_11 p11, typename FUN::PARM_12 p12, typename FUN::PARM_13 p13, typename FUN::PARM_14 p14, typename FUN::PARM_15 p15 )
    {
        fun1_( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15 );
        fun2_( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15 );
    }

    void operator()( typename FUN::PARM_1 p1, typename FUN::PARM_2 p2, typename FUN::PARM_3 p3, typename FUN::PARM_4 p4, typename FUN::PARM_5 p5, typename FUN::PARM_6 p6, typename FUN::PARM_7 p7, typename FUN::PARM_8 p8, typename FUN::PARM_9 p9, typename FUN::PARM_10 p10, typename FUN::PARM_11 p11, typename FUN::PARM_12 p12, typename FUN::PARM_13 p13, typename FUN::PARM_14 p14, typename FUN::PARM_15 p15, typename FUN::PARM_16 p16 )
    {
        fun1_( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16 );
        fun2_( p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16 );
    }

};
COMET_CLOSE_PARTIAL()

/** \class binder_first functor.h comet/functor.h
  * Implements a functor that calls the first parameter bound to a specified
  * type. The functor type implemented is effectively functor<R, LIST::tail>.
  * \sa bind_first
  */
COMET_DEFINE_PARTIAL( typename,R, typename,LIST, binder_first)
        : public functor_impl<R, typename LIST::tail>
{
    public:
        typedef functor<R, typename LIST::tail> outgoing_type;
        typedef typename functor<R, LIST>::PARM_1 bound_type;
        typedef R result_type;

    private:
        functor<R, LIST> fun_;
        bound_type bound_;
    public:
        COMET_PARTIAL_NAME(binder_first)(const functor<R, LIST>& fun, bound_type bound) : bound_(bound)
        {
            fun_ = fun;
        }

        COMET_PARTIAL_NAME(binder_first)(const COMET_PARTIAL_NAME(binder_first)& x) : bound_(x.bound_)
        {
            fun_ = x.fun_;
        }

        functor_impl<R, typename LIST::tail>* clone() const
        { return new COMET_PARTIAL_NAME(binder_first)(*this); }

        result_type operator()()
        {
            return fun_(bound_ );
        }

        result_type operator()( typename outgoing_type::PARM_1 p1)
        {
            return fun_(bound_,  p1);
        }

        result_type operator()( typename outgoing_type::PARM_1 p1, typename outgoing_type::PARM_2 p2)
        {
            return fun_(bound_,  p1, p2);
        }

        result_type operator()( typename outgoing_type::PARM_1 p1, typename outgoing_type::PARM_2 p2, typename outgoing_type::PARM_3 p3)
        {
            return fun_(bound_,  p1, p2, p3);
        }

        result_type operator()( typename outgoing_type::PARM_1 p1, typename outgoing_type::PARM_2 p2, typename outgoing_type::PARM_3 p3, typename outgoing_type::PARM_4 p4)
        {
            return fun_(bound_,  p1, p2, p3, p4);
        }

        result_type operator()( typename outgoing_type::PARM_1 p1, typename outgoing_type::PARM_2 p2, typename outgoing_type::PARM_3 p3, typename outgoing_type::PARM_4 p4, typename outgoing_type::PARM_5 p5)
        {
            return fun_(bound_,  p1, p2, p3, p4, p5);
        }

        result_type operator()( typename outgoing_type::PARM_1 p1, typename outgoing_type::PARM_2 p2, typename outgoing_type::PARM_3 p3, typename outgoing_type::PARM_4 p4, typename outgoing_type::PARM_5 p5, typename outgoing_type::PARM_6 p6)
        {
            return fun_(bound_,  p1, p2, p3, p4, p5, p6);
        }

        result_type operator()( typename outgoing_type::PARM_1 p1, typename outgoing_type::PARM_2 p2, typename outgoing_type::PARM_3 p3, typename outgoing_type::PARM_4 p4, typename outgoing_type::PARM_5 p5, typename outgoing_type::PARM_6 p6, typename outgoing_type::PARM_7 p7)
        {
            return fun_(bound_,  p1, p2, p3, p4, p5, p6, p7);
        }

        result_type operator()( typename outgoing_type::PARM_1 p1, typename outgoing_type::PARM_2 p2, typename outgoing_type::PARM_3 p3, typename outgoing_type::PARM_4 p4, typename outgoing_type::PARM_5 p5, typename outgoing_type::PARM_6 p6, typename outgoing_type::PARM_7 p7, typename outgoing_type::PARM_8 p8)
        {
            return fun_(bound_,  p1, p2, p3, p4, p5, p6, p7, p8);
        }

        result_type operator()( typename outgoing_type::PARM_1 p1, typename outgoing_type::PARM_2 p2, typename outgoing_type::PARM_3 p3, typename outgoing_type::PARM_4 p4, typename outgoing_type::PARM_5 p5, typename outgoing_type::PARM_6 p6, typename outgoing_type::PARM_7 p7, typename outgoing_type::PARM_8 p8, typename outgoing_type::PARM_9 p9)
        {
            return fun_(bound_,  p1, p2, p3, p4, p5, p6, p7, p8, p9);
        }

        result_type operator()( typename outgoing_type::PARM_1 p1, typename outgoing_type::PARM_2 p2, typename outgoing_type::PARM_3 p3, typename outgoing_type::PARM_4 p4, typename outgoing_type::PARM_5 p5, typename outgoing_type::PARM_6 p6, typename outgoing_type::PARM_7 p7, typename outgoing_type::PARM_8 p8, typename outgoing_type::PARM_9 p9, typename outgoing_type::PARM_10 p10)
        {
            return fun_(bound_,  p1, p2, p3, p4, p5, p6, p7, p8, p9, p10);
        }

        result_type operator()( typename outgoing_type::PARM_1 p1, typename outgoing_type::PARM_2 p2, typename outgoing_type::PARM_3 p3, typename outgoing_type::PARM_4 p4, typename outgoing_type::PARM_5 p5, typename outgoing_type::PARM_6 p6, typename outgoing_type::PARM_7 p7, typename outgoing_type::PARM_8 p8, typename outgoing_type::PARM_9 p9, typename outgoing_type::PARM_10 p10, typename outgoing_type::PARM_11 p11)
        {
            return fun_(bound_,  p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11);
        }

        result_type operator()( typename outgoing_type::PARM_1 p1, typename outgoing_type::PARM_2 p2, typename outgoing_type::PARM_3 p3, typename outgoing_type::PARM_4 p4, typename outgoing_type::PARM_5 p5, typename outgoing_type::PARM_6 p6, typename outgoing_type::PARM_7 p7, typename outgoing_type::PARM_8 p8, typename outgoing_type::PARM_9 p9, typename outgoing_type::PARM_10 p10, typename outgoing_type::PARM_11 p11, typename outgoing_type::PARM_12 p12)
        {
            return fun_(bound_,  p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12);
        }

        result_type operator()( typename outgoing_type::PARM_1 p1, typename outgoing_type::PARM_2 p2, typename outgoing_type::PARM_3 p3, typename outgoing_type::PARM_4 p4, typename outgoing_type::PARM_5 p5, typename outgoing_type::PARM_6 p6, typename outgoing_type::PARM_7 p7, typename outgoing_type::PARM_8 p8, typename outgoing_type::PARM_9 p9, typename outgoing_type::PARM_10 p10, typename outgoing_type::PARM_11 p11, typename outgoing_type::PARM_12 p12, typename outgoing_type::PARM_13 p13)
        {
            return fun_(bound_,  p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13);
        }

        result_type operator()( typename outgoing_type::PARM_1 p1, typename outgoing_type::PARM_2 p2, typename outgoing_type::PARM_3 p3, typename outgoing_type::PARM_4 p4, typename outgoing_type::PARM_5 p5, typename outgoing_type::PARM_6 p6, typename outgoing_type::PARM_7 p7, typename outgoing_type::PARM_8 p8, typename outgoing_type::PARM_9 p9, typename outgoing_type::PARM_10 p10, typename outgoing_type::PARM_11 p11, typename outgoing_type::PARM_12 p12, typename outgoing_type::PARM_13 p13, typename outgoing_type::PARM_14 p14)
        {
            return fun_(bound_,  p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14);
        }

        result_type operator()( typename outgoing_type::PARM_1 p1, typename outgoing_type::PARM_2 p2, typename outgoing_type::PARM_3 p3, typename outgoing_type::PARM_4 p4, typename outgoing_type::PARM_5 p5, typename outgoing_type::PARM_6 p6, typename outgoing_type::PARM_7 p7, typename outgoing_type::PARM_8 p8, typename outgoing_type::PARM_9 p9, typename outgoing_type::PARM_10 p10, typename outgoing_type::PARM_11 p11, typename outgoing_type::PARM_12 p12, typename outgoing_type::PARM_13 p13, typename outgoing_type::PARM_14 p14, typename outgoing_type::PARM_15 p15)
        {
            return fun_(bound_,  p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15);
        }

        result_type operator()( typename outgoing_type::PARM_1 p1, typename outgoing_type::PARM_2 p2, typename outgoing_type::PARM_3 p3, typename outgoing_type::PARM_4 p4, typename outgoing_type::PARM_5 p5, typename outgoing_type::PARM_6 p6, typename outgoing_type::PARM_7 p7, typename outgoing_type::PARM_8 p8, typename outgoing_type::PARM_9 p9, typename outgoing_type::PARM_10 p10, typename outgoing_type::PARM_11 p11, typename outgoing_type::PARM_12 p12, typename outgoing_type::PARM_13 p13, typename outgoing_type::PARM_14 p14, typename outgoing_type::PARM_15 p15, typename outgoing_type::PARM_16 p16)
        {
            return fun_(bound_,  p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16);
        }


};
COMET_CLOSE_PARTIAL()

COMET_SPECIALISE_PARTIAL( void, typename,LIST , binder_first)
    : public functor_impl<void, typename LIST::tail>
{
public:
typedef functor<void, typename LIST::tail> outgoing_type;
typedef typename functor<void, LIST>::PARM_1 bound_type;
typedef void result_type;

private:
functor<void, LIST> fun_;
bound_type bound_;
public:
COMET_PARTIAL_NAME(binder_first)(const functor<void, LIST>& fun, bound_type bound) : bound_(bound)
{
    fun_ = fun;
}

COMET_PARTIAL_NAME(binder_first)(const COMET_PARTIAL_NAME(binder_first)& x) : bound_(x.bound_)
{
    fun_ = x.fun_;
}

functor_impl<void, typename LIST::tail>* clone() const
{ return new COMET_PARTIAL_NAME(binder_first)(*this); }

result_type operator()()
{
    fun_(bound_ );
}

result_type operator()( typename outgoing_type::PARM_1 p1)
{
    fun_(bound_,  p1);
}

result_type operator()( typename outgoing_type::PARM_1 p1, typename outgoing_type::PARM_2 p2)
{
    fun_(bound_,  p1, p2);
}

result_type operator()( typename outgoing_type::PARM_1 p1, typename outgoing_type::PARM_2 p2, typename outgoing_type::PARM_3 p3)
{
    fun_(bound_,  p1, p2, p3);
}

result_type operator()( typename outgoing_type::PARM_1 p1, typename outgoing_type::PARM_2 p2, typename outgoing_type::PARM_3 p3, typename outgoing_type::PARM_4 p4)
{
    fun_(bound_,  p1, p2, p3, p4);
}

result_type operator()( typename outgoing_type::PARM_1 p1, typename outgoing_type::PARM_2 p2, typename outgoing_type::PARM_3 p3, typename outgoing_type::PARM_4 p4, typename outgoing_type::PARM_5 p5)
{
    fun_(bound_,  p1, p2, p3, p4, p5);
}

result_type operator()( typename outgoing_type::PARM_1 p1, typename outgoing_type::PARM_2 p2, typename outgoing_type::PARM_3 p3, typename outgoing_type::PARM_4 p4, typename outgoing_type::PARM_5 p5, typename outgoing_type::PARM_6 p6)
{
    fun_(bound_,  p1, p2, p3, p4, p5, p6);
}

result_type operator()( typename outgoing_type::PARM_1 p1, typename outgoing_type::PARM_2 p2, typename outgoing_type::PARM_3 p3, typename outgoing_type::PARM_4 p4, typename outgoing_type::PARM_5 p5, typename outgoing_type::PARM_6 p6, typename outgoing_type::PARM_7 p7)
{
    fun_(bound_,  p1, p2, p3, p4, p5, p6, p7);
}

result_type operator()( typename outgoing_type::PARM_1 p1, typename outgoing_type::PARM_2 p2, typename outgoing_type::PARM_3 p3, typename outgoing_type::PARM_4 p4, typename outgoing_type::PARM_5 p5, typename outgoing_type::PARM_6 p6, typename outgoing_type::PARM_7 p7, typename outgoing_type::PARM_8 p8)
{
    fun_(bound_,  p1, p2, p3, p4, p5, p6, p7, p8);
}

result_type operator()( typename outgoing_type::PARM_1 p1, typename outgoing_type::PARM_2 p2, typename outgoing_type::PARM_3 p3, typename outgoing_type::PARM_4 p4, typename outgoing_type::PARM_5 p5, typename outgoing_type::PARM_6 p6, typename outgoing_type::PARM_7 p7, typename outgoing_type::PARM_8 p8, typename outgoing_type::PARM_9 p9)
{
    fun_(bound_,  p1, p2, p3, p4, p5, p6, p7, p8, p9);
}

result_type operator()( typename outgoing_type::PARM_1 p1, typename outgoing_type::PARM_2 p2, typename outgoing_type::PARM_3 p3, typename outgoing_type::PARM_4 p4, typename outgoing_type::PARM_5 p5, typename outgoing_type::PARM_6 p6, typename outgoing_type::PARM_7 p7, typename outgoing_type::PARM_8 p8, typename outgoing_type::PARM_9 p9, typename outgoing_type::PARM_10 p10)
{
    fun_(bound_,  p1, p2, p3, p4, p5, p6, p7, p8, p9, p10);
}

result_type operator()( typename outgoing_type::PARM_1 p1, typename outgoing_type::PARM_2 p2, typename outgoing_type::PARM_3 p3, typename outgoing_type::PARM_4 p4, typename outgoing_type::PARM_5 p5, typename outgoing_type::PARM_6 p6, typename outgoing_type::PARM_7 p7, typename outgoing_type::PARM_8 p8, typename outgoing_type::PARM_9 p9, typename outgoing_type::PARM_10 p10, typename outgoing_type::PARM_11 p11)
{
    fun_(bound_,  p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11);
}

result_type operator()( typename outgoing_type::PARM_1 p1, typename outgoing_type::PARM_2 p2, typename outgoing_type::PARM_3 p3, typename outgoing_type::PARM_4 p4, typename outgoing_type::PARM_5 p5, typename outgoing_type::PARM_6 p6, typename outgoing_type::PARM_7 p7, typename outgoing_type::PARM_8 p8, typename outgoing_type::PARM_9 p9, typename outgoing_type::PARM_10 p10, typename outgoing_type::PARM_11 p11, typename outgoing_type::PARM_12 p12)
{
    fun_(bound_,  p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12);
}

result_type operator()( typename outgoing_type::PARM_1 p1, typename outgoing_type::PARM_2 p2, typename outgoing_type::PARM_3 p3, typename outgoing_type::PARM_4 p4, typename outgoing_type::PARM_5 p5, typename outgoing_type::PARM_6 p6, typename outgoing_type::PARM_7 p7, typename outgoing_type::PARM_8 p8, typename outgoing_type::PARM_9 p9, typename outgoing_type::PARM_10 p10, typename outgoing_type::PARM_11 p11, typename outgoing_type::PARM_12 p12, typename outgoing_type::PARM_13 p13)
{
    fun_(bound_,  p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13);
}

result_type operator()( typename outgoing_type::PARM_1 p1, typename outgoing_type::PARM_2 p2, typename outgoing_type::PARM_3 p3, typename outgoing_type::PARM_4 p4, typename outgoing_type::PARM_5 p5, typename outgoing_type::PARM_6 p6, typename outgoing_type::PARM_7 p7, typename outgoing_type::PARM_8 p8, typename outgoing_type::PARM_9 p9, typename outgoing_type::PARM_10 p10, typename outgoing_type::PARM_11 p11, typename outgoing_type::PARM_12 p12, typename outgoing_type::PARM_13 p13, typename outgoing_type::PARM_14 p14)
{
    fun_(bound_,  p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14);
}

result_type operator()( typename outgoing_type::PARM_1 p1, typename outgoing_type::PARM_2 p2, typename outgoing_type::PARM_3 p3, typename outgoing_type::PARM_4 p4, typename outgoing_type::PARM_5 p5, typename outgoing_type::PARM_6 p6, typename outgoing_type::PARM_7 p7, typename outgoing_type::PARM_8 p8, typename outgoing_type::PARM_9 p9, typename outgoing_type::PARM_10 p10, typename outgoing_type::PARM_11 p11, typename outgoing_type::PARM_12 p12, typename outgoing_type::PARM_13 p13, typename outgoing_type::PARM_14 p14, typename outgoing_type::PARM_15 p15)
{
    fun_(bound_,  p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15);
}

result_type operator()( typename outgoing_type::PARM_1 p1, typename outgoing_type::PARM_2 p2, typename outgoing_type::PARM_3 p3, typename outgoing_type::PARM_4 p4, typename outgoing_type::PARM_5 p5, typename outgoing_type::PARM_6 p6, typename outgoing_type::PARM_7 p7, typename outgoing_type::PARM_8 p8, typename outgoing_type::PARM_9 p9, typename outgoing_type::PARM_10 p10, typename outgoing_type::PARM_11 p11, typename outgoing_type::PARM_12 p12, typename outgoing_type::PARM_13 p13, typename outgoing_type::PARM_14 p14, typename outgoing_type::PARM_15 p15, typename outgoing_type::PARM_16 p16)
{
    fun_(bound_,  p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16);
}

};
COMET_CLOSE_PARTIAL()

/** Constructs a functor that calls a function with an extra parameter (at the
  * beginning) of type \p T.  This gets called with the value \p bound.
  * \relates binder
  */
template<typename R, typename LIST, typename T>
functor<R, typename LIST::tail> bind_first( const functor<R, LIST>& fun, const T& bound)
{
    return functor<R, COMET_STRICT_TYPENAME LIST::tail>(std::auto_ptr< functor<R, COMET_STRICT_TYPENAME LIST::tail>::IMPL >(new binder_first<R>COMET_PARTIAL_NS<LIST>(fun, bound)));
}

/** Chains \p fun1 to \p fun2.
  * \return functor chaining \p fun1 to \p fun2
  * \relates chainer
  */
template<typename R, typename LIST>
functor<R, LIST> chain( const functor<R, LIST>& fun1, const functor<R, LIST>& fun2)
{
    return functor<R,LIST>(std::auto_ptr< functor<R, LIST>::IMPL >(new chainer<R COMET_PARTIAL_NS_1(LIST)(fun1, fun2)));
}
//@}
}

#endif
