/** \file
  * SafeArray wrapper implementation.
  */
/*
 * Copyright © 2000, 2001, 2002 Sofus Mortensen, Michael Geddes
 * Copyright © 2012 Alexander Lamaison
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

#ifndef COMET_SAFEARRAY_H
#define COMET_SAFEARRAY_H

#include <comet/config.h>
#include <comet/ptr.h>
#include <comet/bstr.h>
#include <comet/variant.h>
#include <comet/static_assert.h>
//#include <comet/util.h>
#include <comet/type_traits.h>
#include <comet/common.h>
#include <comet/uuid.h>

#include <iterator>
#include <limits>
#include <stdexcept>

#ifndef NDEBUG
#define COMET_ITERATOR_DEBUG
#endif

namespace comet {


    namespace impl {

        template <bool is_class>
        struct access_operator
        {
        // Has operator ->
            template <typename T, typename C>
            struct base
            {
                T *operator->()
                {
                    return &(static_cast<C *>(this)->operator*());
                }
            };
        };
        // Doesn't have operator->
        template<>
        struct access_operator<false>
        {
            template <typename T, typename C>
            struct base
            {
            };
        };

        template<typename T> struct sa_traits; //Moved to common.h

        template<typename T, typename TR> class sa_iterator;

        template<typename T> struct const_traits {
            typedef T value_type;
            typedef typename sa_traits<T>::const_reference reference;
            typedef const T*  pointer;
        };

        template<typename T> struct nonconst_traits {
            typedef T value_type;
            typedef typename sa_traits<T>::reference reference;
            typedef T*  pointer;
        };


        template<> struct sa_traits<long> : public basic_sa_traits<long, VT_I4>    {};
        template<> struct sa_traits<unsigned long> : public basic_sa_traits<unsigned long, VT_UI4>    {};

        template<> struct sa_traits<short> : public basic_sa_traits<short, VT_I2>    {};
        template<> struct sa_traits<unsigned short> : public basic_sa_traits<unsigned short, VT_UI2>    {};

        template<> struct sa_traits<signed char> : public basic_sa_traits<signed char, VT_I1>    {};
        template<> struct sa_traits<unsigned char> : public basic_sa_traits<unsigned char, VT_UI1>    {};
        template<> struct sa_traits<char> : public basic_sa_traits<char, VT_I1>    {};

        template<> struct sa_traits<float> : public basic_sa_traits<float, VT_R4>    {};
        template<> struct sa_traits<double> : public basic_sa_traits<double, VT_R8>    {};

        template<> struct sa_traits<variant_t>
        {
            typedef VARIANT raw;

            enum { vt = VT_VARIANT };
            enum { check_type = impl::stct_features_ok };
            enum { extras_type = stet_null };

            typedef variant_t value_type;
            typedef variant_t& reference;
            typedef const variant_t& const_reference;

            static reference create_reference(raw& x) { return *reinterpret_cast<variant_t*>(&x); }
            static const_reference create_const_reference(raw& x) { return *reinterpret_cast<const variant_t*>(&x); }

            typedef sa_iterator<variant_t, nonconst_traits<variant_t> > iterator;
            typedef sa_iterator<variant_t, const_traits<variant_t> > const_iterator;

            static bool are_features_ok(unsigned short f) { return (f & FADF_VARIANT) != 0; }
            static com_ptr<IRecordInfo> get_record_info() { return 0; }
        };

        template<> struct sa_traits<bstr_t>
        {
            enum { vt = VT_BSTR };
            enum { check_type = impl::stct_vt_ok };
            enum { extras_type = stet_null };

            typedef BSTR raw;
            typedef bstr_t value_type;
            typedef bstr_t& reference;
            typedef const bstr_t& const_reference;

            static reference create_reference(raw& x) { return *reinterpret_cast<bstr_t*>(&x); }
            static const_reference create_const_reference(raw& x) { return *reinterpret_cast<const bstr_t*>(&x); }

            typedef sa_iterator<bstr_t, nonconst_traits<bstr_t> > iterator;
            typedef sa_iterator<bstr_t, const_traits<bstr_t> > const_iterator;

            static bool are_features_ok(unsigned short f) { return (f & FADF_BSTR) != 0; }
            static com_ptr<IRecordInfo> get_record_info() { return 0; }
        };

        template<> struct sa_traits<currency_t>
        {
            enum { vt = VT_CY };
            enum { check_type = impl::stct_vt_ok };
            enum { extras_type = impl::stet_null };

            typedef CY raw;
            typedef currency_t value_type;
            typedef currency_t& reference;
            typedef const currency_t& const_reference;

            static reference create_reference(raw& x) { return *reinterpret_cast<currency_t*>(&x); }
            static const_reference create_const_reference(raw& x) { return *reinterpret_cast<const currency_t*>(&x); }

            typedef sa_iterator<currency_t, nonconst_traits<currency_t> > iterator;
            typedef sa_iterator<currency_t, const_traits<currency_t> > const_iterator;

            static bool are_features_ok(unsigned short) { return true; }
            static com_ptr<IRecordInfo> get_record_info() { return 0; }
        };

        template<> struct sa_traits<datetime_t>
        {
            enum { vt = VT_DATE };
            enum { check_type = impl::stct_vt_ok };
            enum { extras_type = impl::stet_null };

            typedef DATE raw;
            typedef datetime_t value_type;
            typedef datetime_t& reference;
            typedef const datetime_t& const_reference;

            static reference create_reference(raw& x) { return *reinterpret_cast<datetime_t*>(&x); }
            static const_reference create_const_reference(raw& x) { return *reinterpret_cast<const datetime_t*>(&x); }

            typedef sa_iterator<datetime_t, nonconst_traits<datetime_t> > iterator;
            typedef sa_iterator<datetime_t, const_traits<datetime_t> > const_iterator;

            static bool are_features_ok(unsigned short) { return true; }
            static com_ptr<IRecordInfo> get_record_info() { return 0; }
        };

        template<> struct sa_traits<variant_bool_t>
        {
            enum { vt = VT_BOOL };
            enum { check_type = impl::stct_vt_ok };
            enum { extras_type = impl::stet_null };

            typedef VARIANT_BOOL raw;
            typedef variant_bool_t value_type;
            typedef variant_bool_t& reference;
            typedef const variant_bool_t& const_reference;

            static reference create_reference(raw& x) { return *reinterpret_cast<variant_bool_t*>(&x); }
            static const_reference create_const_reference(raw& x) { return *reinterpret_cast<const variant_bool_t*>(&x); }

            typedef sa_iterator<variant_bool_t, nonconst_traits<variant_bool_t> > iterator;
            typedef sa_iterator<variant_bool_t, const_traits<variant_bool_t> > const_iterator;

            static bool are_features_ok(unsigned short) { return true; }
            static com_ptr<IRecordInfo> get_record_info() { return 0; }
        };

        template<> struct sa_traits<bool>: sa_traits<variant_bool_t>
        {
        };

        template<> struct sa_traits< com_ptr< ::IUnknown> >
        {
            enum { vt = VT_UNKNOWN };
            enum { check_type = impl::stct_vt_ok };
            enum { extras_type = impl::stet_iid };

            typedef IUnknown* raw;
            typedef com_ptr< ::IUnknown> value_type;
            typedef com_ptr< ::IUnknown>& reference;
            typedef const com_ptr< ::IUnknown>& const_reference;

            static reference create_reference(raw& x) { return *reinterpret_cast<com_ptr< ::IUnknown>*>(&x); }
            static const_reference create_const_reference(raw& x) { return *reinterpret_cast<const com_ptr< ::IUnknown>*>(&x); }

            typedef sa_iterator<com_ptr< ::IUnknown >, nonconst_traits<com_ptr< ::IUnknown > > > iterator;
            typedef sa_iterator<com_ptr< ::IUnknown >, const_traits<com_ptr< ::IUnknown > > > const_iterator;

            static bool are_features_ok(unsigned short f) { return (f & (FADF_UNKNOWN|FADF_DISPATCH)) != 0; }
            static com_ptr<IRecordInfo> get_record_info() { return 0; }
            static const uuid_t& iid() { return uuid_t::create_const_reference(IID_IUnknown); }
        };

        template<> struct sa_traits< com_ptr< ::IDispatch> >
        {
            enum { vt = VT_DISPATCH };
            enum { check_type = impl::stct_vt_ok };
            enum { extras_type = impl::stet_iid };

            typedef IDispatch* raw;
            typedef com_ptr< ::IDispatch> value_type;
            typedef com_ptr< ::IDispatch>& reference;
            typedef const com_ptr< ::IDispatch>& const_reference;

            static reference create_reference(raw& x) { return *reinterpret_cast<com_ptr< ::IDispatch>*>(&x); }
            static const_reference create_const_reference(raw& x) { return *reinterpret_cast<const com_ptr< ::IDispatch>*>(&x); }

            typedef sa_iterator<com_ptr< ::IDispatch>, nonconst_traits<com_ptr< ::IDispatch> > > iterator;
            typedef sa_iterator<com_ptr< ::IDispatch>, const_traits<com_ptr< ::IDispatch> > > const_iterator;

            static bool are_features_ok(unsigned short f) { return (f & FADF_DISPATCH) != 0; }
            static com_ptr<IRecordInfo> get_record_info() { return 0; }

            static const uuid_t& iid() { return uuid_t::create_const_reference(IID_IDispatch); }
        };

#ifdef COMET_ITERATOR_DEBUG
#define COMET_SAIT_THIS ,this
#define COMET_SAIT_ITER(CONT_, IT_, TRAITS_) impl::sa_debug_iterator<CONT_, TRAITS_ >

        template<typename TRAITS>
        struct sa_debug_traits
        {
            typedef TRAITS traits;
            typedef typename TRAITS::value_type value_type;
            typedef typename TRAITS::raw raw;
            typedef typename TRAITS::reference reference;
            typedef typename TRAITS::iterator nonconst_iterator;
            typedef typename TRAITS::iterator iterator;
            typedef typename TRAITS::const_iterator const_iterator;
        };
        template<typename TRAITS>
        struct sa_const_debug_traits
        {
            typedef TRAITS traits;
            typedef typename TRAITS::value_type value_type;
            typedef typename TRAITS::raw raw;
            typedef typename TRAITS::const_reference reference;
            typedef typename TRAITS::iterator nonconst_iterator;
            typedef typename TRAITS::const_iterator iterator;
            typedef typename TRAITS::const_iterator const_iterator;
        };

        template< typename CONT, typename TRAITS>
        class sa_debug_iterator : public std::iterator<std::random_access_iterator_tag, typename TRAITS::value_type>,
            public access_operator<type_traits::is_class_pointer<typename TRAITS::value_type>::result>::template base<typename TRAITS::raw, sa_debug_iterator<CONT,TRAITS> >
        {
        public:
            const CONT *cont_;
            typename TRAITS::iterator iter_;

            template<typename IT>
            sa_debug_iterator(IT ptr, const CONT *cont) : iter_(ptr), cont_(cont) {}

            sa_debug_iterator( const sa_debug_iterator<CONT, impl::sa_debug_traits<typename TRAITS::traits> > &nc_it ) : iter_(nc_it.iter_), cont_(nc_it.cont_) {}

            sa_debug_iterator(): cont_(NULL) {}

            typename TRAITS::iterator get_raw()const { return iter_; }
            typename TRAITS::const_iterator get_const_raw()const { return iter_; }


            sa_debug_iterator operator++(int) {
                COMET_ASSERT( cont_!=NULL);
                COMET_ASSERT( get_const_raw() < cont_->end().get_raw() );
                sa_debug_iterator t(*this);
                ++iter_;
                return t;
            }

            sa_debug_iterator& operator++() {
                COMET_ASSERT( cont_!=NULL);
                COMET_ASSERT( get_const_raw() < cont_->end().get_raw() );
                ++iter_;
                return *this;
            }

            sa_debug_iterator operator--(int) {
                COMET_ASSERT( cont_!=NULL);
                COMET_ASSERT( get_const_raw() > cont_->begin().get_raw() );
                sa_debug_iterator t(*this);
                --iter_;
                return t;
            }

            sa_debug_iterator& operator--() {
                COMET_ASSERT( cont_!=NULL);
                COMET_ASSERT( get_const_raw() > cont_->begin().get_raw() );
                --iter_;
                return *this;
            }

            typename TRAITS::reference operator[](size_t n) {
                COMET_ASSERT( cont_!=NULL);
                COMET_ASSERT( (get_const_raw()+ n) >= cont_->begin().get_raw());
                COMET_ASSERT( (get_const_raw()+n) < cont_->end().get_raw() );
                return iter_[n];
            }

            sa_debug_iterator& operator+=(size_t n) {
                COMET_ASSERT(cont_!=NULL);
                COMET_ASSERT((get_const_raw()+ n) >= cont_->begin().get_raw());
                COMET_ASSERT((get_const_raw()+n) <= cont_->end().get_raw() );
                iter_ += n;
                return *this;
            }

            sa_debug_iterator& operator-=(size_t n) {
                COMET_ASSERT( cont_!=NULL);
                COMET_ASSERT( (get_const_raw()- n) >= cont_->begin().get_raw());
                COMET_ASSERT( (get_const_raw()- n) <= cont_->end().get_raw() );
                iter_ -= n;
                return *this;
            }

            ptrdiff_t operator-(const sa_debug_iterator& it) const {
                COMET_ASSERT( cont_!=NULL);
                COMET_ASSERT( cont_ == it.cont_);
                return iter_ - it.iter_;
            }

            bool operator<(const sa_debug_iterator& it) const {
                COMET_ASSERT( cont_!=NULL);
                COMET_ASSERT( cont_ == it.cont_);
                return iter_ < it.iter_;
            }

            bool operator>(const sa_debug_iterator& it) const {
                COMET_ASSERT( cont_!=NULL);
                COMET_ASSERT( cont_ == it.cont_);
                return iter_ > it.iter_;
            }

            bool operator<=(const sa_debug_iterator& it) const {
                COMET_ASSERT( cont_!=NULL);
                COMET_ASSERT( cont_ == it.cont_);
                return iter_ <= it.iter_;
            }

            bool operator>=(const sa_debug_iterator& it) const {
                COMET_ASSERT( cont_!=NULL);
                COMET_ASSERT( cont_ == it.cont_);
                return iter_ >= it.iter_;
            }

            bool operator==(const sa_debug_iterator& it) const {
                COMET_ASSERT( cont_!=NULL);
                COMET_ASSERT( cont_ == it.cont_);
                return iter_ == it.iter_;
            }

            bool operator!=(const sa_debug_iterator& it) const {
                COMET_ASSERT( cont_!=NULL);
                COMET_ASSERT( cont_ == it.cont_);
                return iter_ != it.iter_;
            }

            sa_debug_iterator operator+(size_t n) const {
                COMET_ASSERT( cont_!=NULL);
                COMET_ASSERT( (get_const_raw() + n) >= cont_->begin().get_raw());
                COMET_ASSERT( (get_const_raw() + n) <= cont_->end().get_raw() );
                return sa_debug_iterator( iter_+n, cont_);
            }

            sa_debug_iterator operator-(size_t n) const {
                COMET_ASSERT( cont_!=NULL);
                COMET_ASSERT( (get_const_raw() - n) >= cont_->begin().get_raw());
                COMET_ASSERT( (get_const_raw() - n) <= cont_->end().get_raw() );
                return sa_debug_iterator( iter_-n, cont_);
            }

            typename TRAITS::reference operator*() {
                COMET_ASSERT( cont_ != NULL);
                COMET_ASSERT( (get_const_raw()) >= cont_->begin().get_raw());
                COMET_ASSERT( (get_const_raw()) < cont_->end().get_raw() );
                return *iter_;
            }
        };


#else // COMET_ITERATOR_DEBUG
#define COMET_IT_DBG__(x)
#define COMET_SAIT_THIS
#define COMET_SAIT_ITER(CONT_, IT_, TRAITS_) IT_
#endif // COMET_ITERATOR_DEBUG


        /** \internal
         */
        template<typename T, typename TR> class sa_iterator : public std::iterator<std::random_access_iterator_tag, typename TR::value_type>,
            public access_operator<type_traits::is_class_pointer<T>::result>::template base< T, sa_iterator<T,TR> >

        {
            typedef sa_iterator<T, nonconst_traits<T> > nonconst_self;
        public:
            typedef sa_traits<T> traits;
            typename traits::raw* ptr_;

            typedef typename TR::pointer pointer;
            typedef typename TR::reference reference;
            typedef ptrdiff_t difference_type;

            sa_iterator(const nonconst_self& it )
                : ptr_(it.get_raw())
                {}

            explicit sa_iterator(typename traits::raw* ptr) : ptr_(ptr) {}

            sa_iterator() {}

            typename traits::raw* get_raw() const
            {
                return ptr_;
            }

            sa_iterator operator++(int) {
                sa_iterator t(*this);
                ++ptr_;
                return t;
            }

            sa_iterator& operator++() {
                ++ptr_;
                return *this;
            }

            sa_iterator operator--(int) {
                sa_iterator t(*this);
                --ptr_;
                return t;
            }

            sa_iterator& operator--() {
                --ptr_;
                return *this;
            }

            reference operator[](size_t n) {
                return traits::create_reference(ptr_[n]);
            }

            sa_iterator& operator+=(size_t n) {
                ptr_ += n;
                return *this;
            }

            sa_iterator& operator-=(size_t n) {
                ptr_ -= n;
                return *this;
            }

            difference_type operator-(const sa_iterator& it) const {
                return ptr_ - it.ptr_;
            }

            bool operator<(const sa_iterator& it) const {
                return ptr_ < it.ptr_;
            }

            bool operator>(const sa_iterator& it) const {
                return ptr_ > it.ptr_;
            }

            bool operator<=(const sa_iterator& it) const {
                return ptr_ <= it.ptr_;
            }

            bool operator>=(const sa_iterator& it) const {
                return ptr_ >= it.ptr_;
            }

            bool operator==(const sa_iterator& it) const {
                return ptr_ == it.ptr_;
            }

            bool operator!=(const sa_iterator& it) const {
                return ptr_ != it.ptr_;
            }

            sa_iterator operator+(size_t n) const {
                return sa_iterator(ptr_ + n);
            }

            sa_iterator operator-(size_t n) const {
                return sa_iterator(ptr_ - n);
            }

            template<typename T2, typename TR2> friend sa_iterator<T2, TR2> operator+(size_t n, const sa_iterator<T2, TR2>& it);
            // friend sa_iterator operator+(size_t n, const sa_iterator&);

            reference operator*() { return traits::create_reference(*ptr_); }
        };

    }

    namespace impl
    {
        template <typename T>
        class safearray_auto_ref_t;

        template <typename T>
        class safearray_auto_const_ref_t;
    };

    /*! \addtogroup COMType
     */
    //@{
    /**STL container compatible wrapper for a safearray.
      * Provides forwards and reverse iterators.
      */
#ifdef COMET_PARTIAL_SPECIALISATION

    template<typename T,enum impl::sa_traits_extras_type STET>
    struct get_extras
    {
            static void *extras(){ return 0; }
    };
    template<typename T>
    struct get_extras<T,impl::stet_record>
    {
        static void *extras(){ return impl::sa_traits<T>::get_record_info().in(); }
    };
    template<typename T>
    struct get_extras<T,impl::stet_iid>
    {
        static void *extras(){ return impl::sa_traits<T>::iid().in_ptr(); }
    };

    template<typename T,enum impl::sa_traits_check_type STCT >
    struct traits_sanity_check
    { static inline void check( const SAFEARRAY *psa) {  } };
    template<typename T>
    struct traits_sanity_check<T,impl::stct_vt_ok>
    {
        static void check(SAFEARRAY *psa) {
            if ((psa->fFeatures & FADF_HAVEVARTYPE)!=0)
            {
                VARTYPE vt;
                ::SafeArrayGetVartype(psa, &vt) | raise_exception ;
                if(vt !=  impl::sa_traits<T>::vt)
                    throw std::runtime_error("safearray_t: VarType mismatch");
            }
        }
    };
    template<typename T>
    struct traits_sanity_check<T,impl::stct_iid_ok> {
        static void check(SAFEARRAY *psa)
        {
            uuid_t iid;
            ::SafeArrayGetIID(psa, &iid) | raise_exception;
            if( iid != impl::sa_traits<T>::iid() )
                throw std::runtime_error("safearray_t: IID mismatch");
        }
    };
#endif
    template<typename T> class safearray_t
    {
    public:
        typedef impl::sa_traits<T> traits;
        typedef size_t size_type;           ///< type for sizes (bounds etc).
        typedef long index_type;            ///< Type for indexing into the array
        typedef ptrdiff_t difference_type;  ///< Type for pointer differences
        typedef typename traits::value_type value_type; ///< The type of the contained value .
        typedef typename traits::reference reference;               ///< Safearray reference type
        typedef typename traits::const_reference const_reference;   ///< Safearray const reference type

        typedef typename COMET_SAIT_ITER( safearray_t, traits::iterator, impl::sa_debug_traits<traits> )
                                        iterator;                   ///< Iterator type
        typedef typename COMET_SAIT_ITER( safearray_t, traits::const_iterator, impl::sa_const_debug_traits<traits>)
                                        const_iterator;             ///< Const iterator type

#if defined(COMET_STD_ITERATOR)
        typedef std::reverse_iterator<iterator> reverse_iterator;
        typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
#else
        // workaround for broken reverse_iterator implementations due to no partial specialisation
        typedef std::reverse_iterator<iterator,T> reverse_iterator;
        typedef std::reverse_iterator<const_iterator,T> const_reverse_iterator;
#endif

        //! \name Iterator functions
        //@{
        iterator begin() {
            return iterator(get_array() COMET_SAIT_THIS );
        }

        iterator end() {
            return iterator(get_array() + size() COMET_SAIT_THIS );
        }

        const_iterator begin() const {
            return const_iterator(get_array() COMET_SAIT_THIS) ;
        }

        const_iterator end() const {
            return const_iterator(get_array() + size() COMET_SAIT_THIS );
        }

        reverse_iterator rbegin() {
            return reverse_iterator(end());
        }

        reverse_iterator rend() {
            return reverse_iterator(begin());
        }

        const_reverse_iterator rbegin() const {
            return const_reverse_iterator(end());
        }

        const_reverse_iterator rend() const {
            return const_reverse_iterator(begin());
        }
        //@}

        /// The number of elements in the array.
        size_type size() const {
            return psa_ ? psa_->rgsabound[0].cElements : 0;
        }

        /// Returns whether the array is empty.
        bool is_empty() const {
            return size() == 0;
        }

        /// Returns element n relative to lower_bound()
        reference operator[](index_type n) {
            COMET_ASSERT( (size_type)(n - lower_bound()) < size() );
            return traits::create_reference(get_element(n));
        }

        /// Returns const element n relative to lower_bound()
        const_reference operator[](index_type n) const {
            COMET_ASSERT( (size_type)(n - lower_bound()) < size() );
            return traits::create_reference(get_element(n));
        }

        //! Returns element n relative to lower_bound().
        /*! \throw out_of_range The index is out of range.
         */
        reference at(index_type n) {
            range_check(n);
            return traits::create_reference(get_element(n));
        }

        //! Returns const element n relative to lower_bound().
        /*! \throw out_of_range The index is out of range.
         */
        const_reference at(index_type n) const {
            range_check(n);
            return traits::create_reference(get_element(n));
        }

        //! The front element.
        reference front() { return *begin(); }
        //! The front element - const.
        const_reference front() const { return *begin(); }
        //! The back element.
        reference back() { return *(end() - 1); }
        //! The back element - const.
        const_reference back() const { return *(end() - 1); }

    private:
#ifndef COMET_PARTIAL_SPECIALISATION
        template<enum impl::sa_traits_extras_type STET>
        struct get_extras
        {
             static void *extras(){ return 0; }
        };
        template<>
        struct get_extras<impl::stet_record>
        {
            static void *extras(){ return impl::sa_traits<T>::get_record_info().in(); }
        };
        template<>
        struct get_extras<impl::stet_iid>
        {
            static void *extras(){ return impl::sa_traits<T>::iid().in_ptr(); }
        };
#endif
    public:

        /// \name Constructors
        //@{
        /// Construct a null array.
        safearray_t() : psa_(0)
        {}

        /*! Attach to (and take ownership of) an existing array.
          \code
          SAFEARRAY *psa = SafeArrayCreateVectorEx(VT_BSTR, 0, 5, NULL);
          safearray_t<bstr_t> sa(auto_attach(psa));
          \endcode
          */
        safearray_t(const impl::auto_attach_t<SAFEARRAY*>& psa) : psa_(psa.get())
        {
            sanity_check(psa_);

            if (psa_) try {
                ::SafeArrayLock(psa_) | raise_exception;
            } catch (...)
            {
                ::SafeArrayDestroy(psa_);
                throw;
            }
        }

        /// Copy from a variant, making converting if necessary.
        safearray_t(const variant_t& var)
        {
            if(var.get_vt() == (VT_ARRAY|traits::vt))
            {
                SafeArrayCopy(var.in().parray, &psa_) | raise_exception;
            } else {
                variant_t v2(var, VT_ARRAY|traits::vt);
                SafeArrayCopy(v2.in().parray, &psa_) | raise_exception;
            }

            if (psa_) try {
                ::SafeArrayLock(psa_) | raise_exception;
            } catch (...)
            {
                ::SafeArrayDestroy(psa_);
                throw;
            }

        }

        /// Copy construction
        safearray_t(const safearray_t& sa)
        {
            ::SafeArrayCopy(sa.psa_, &psa_) | raise_exception;

            if (psa_) try {
                ::SafeArrayLock(psa_) | raise_exception;
            } catch (...)
            {
                ::SafeArrayDestroy(psa_);
                throw;
            }
        }

        /// Construct a new safearray vector.
        /*! \param sz Size of the vector.
         *  \param lb Lower bound for the vector.
         */
        explicit safearray_t(size_type sz, index_type lb)
        {
            if (sz > (std::numeric_limits<ULONG>::max)() ||
                sz < (std::numeric_limits<ULONG>::min)())
                throw std::overflow_error(
                    "Cannot create array of requested size");

#ifndef COMET_PARTIAL_SPECIALISATION
            psa_ = ::SafeArrayCreateVectorEx(
                traits::vt, lb, static_cast<ULONG>(sz),
                get_extras<impl::sa_traits_extras_type(traits::extras_type)>::extras());
#else
            psa_ = ::SafeArrayCreateVectorEx(
                traits::vt, lb, static_cast<ULONG>(sz),
                get_extras<T,impl::sa_traits_extras_type(traits::extras_type)>::extras());
#endif
            if (psa_ == 0) throw std::bad_alloc();

            try {
                ::SafeArrayLock(psa_) | raise_exception;
            } catch (...)
            {
                ::SafeArrayDestroy(psa_);
                throw;
            }
        }

        /// Construct a new safearray vector.
        /*! \param sz Size of the vector.
         *  \param lb Lower bound for the vector.
         *  \param val Initial value for the elements.
         */
        safearray_t(size_type sz, index_type lb, const T& val)
        {
#ifndef COMET_PARTIAL_SPECIALISATION
            psa_ = ::SafeArrayCreateVectorEx(traits::vt, lb, sz, get_extras<impl::sa_traits_extras_type(traits::extras_type)>::extras());
#else
            psa_ = ::SafeArrayCreateVectorEx(traits::vt, lb, sz, get_extras<T,impl::sa_traits_extras_type(traits::extras_type)>::extras());
#endif // COMET_PARTIAL_SPECIALISATION
            if (psa_ == 0) throw std::bad_alloc();

            try {
                ::SafeArrayLock(psa_) | raise_exception;

                for (iterator it = begin(); it != end(); ++it) *it = val;
            } catch (...) {
                ::SafeArrayUnlock(psa_);
                ::SafeArrayDestroy(psa_);
                throw;
            }
        }

        /// Construct a safearray from an iterator, specifying the lower bound.
        /** \param first First element of the container.
         *  \param last  Beyond the last element of the container.
         *  \param lb    Lower bound of the new safearray_t.
         */
        template<typename InputIterator> safearray_t(InputIterator first, InputIterator last, index_type lb)
        {
            initialise_aux(first, last, lb, type_traits::int_holder< type_traits::is_integer<InputIterator>::result >());
        }
        //@}

private:
        template<typename InputIterator> void initialise_aux(InputIterator first, InputIterator last, index_type lb, type_traits::int_holder<false>)
        {
            size_type sz = std::distance(first, last);
#ifndef COMET_PARTIAL_SPECIALISATION
            psa_ = ::SafeArrayCreateVectorEx(traits::vt, lb, sz,get_extras<impl::sa_traits_extras_type(traits::extras_type)>::extras());
#else
            psa_ = ::SafeArrayCreateVectorEx(traits::vt, lb, sz,get_extras<T,impl::sa_traits_extras_type(traits::extras_type)>::extras());
#endif //COMET_PARTIAL_SPECIALISATION

            if (psa_ == 0) throw std::bad_alloc();

            try {
                ::SafeArrayLock(psa_) | raise_exception;

                for (iterator it = begin(); first != last; ++first, ++it) {
                    *it = *first;
                }
            } catch (...) {
                ::SafeArrayUnlock(psa_);
                ::SafeArrayDestroy(psa_);
                throw;
            }
        }

        template<typename Integer> void initialise_aux(Integer sz, Integer lb, index_type dummy, type_traits::int_holder<true>)
        {
#ifndef COMET_PARTIAL_SPECIALISATION
            psa_ = ::SafeArrayCreateVectorEx(traits::vt, lb, sz,get_extras<impl::sa_traits_extras_type(traits::extras_type)>::extras());
#else
            psa_ = ::SafeArrayCreateVectorEx(traits::vt, lb, sz,get_extras<T,impl::sa_traits_extras_type(traits::extras_type)>::extras());
#endif // COMET_PARTIAL_SPECIALISATION

            if (psa_ == 0) throw std::bad_alloc();

            ::SafeArrayLock(psa_) | raise_exception;
        }

public:
        /// Resize the array, preserving lower_bound
        /** If the array is null, uses 0.
         */
        void resize(size_type n)
        {
            resize_bound(n, (psa_==NULL)?0:lower_bound());
        }
        /// Resize the array, specifying the lower_bound
        // This has been renamed to prevent ambiguous functions when T = size_type
        void resize_bound(size_type n, size_type lb)
        {
            safearray_t t(n, lb);

            iterator i1 = begin();
            iterator i2 = t.begin();

            for (;i1 != end() && i2 != t.end(); ++i1, ++i2) {
                std::swap(*i1, *i2);
            }

            swap(t);
        }

        /** Resize the array, preserving lower_bound and specifying an initial
         * value for uninitialised values.
         */
        void resize( size_type n, const T& val) {
            resize_bound(n,(psa_==NULL)?0:lower_bound(),val);
        }
        /** Resize the array, specifying lower_bound and specifying an initial
         * value for uninitialised values.
         */
        void resize_bound(size_type n, size_type lb, const T& val) {
            safearray_t t(n, lb);

            iterator i1 = begin();
            iterator i2 = t.begin();

            for (;i1 != end() && i2 != t.end(); ++i1, ++i2) {
                std::swap(*i1, *i2);
            }

            for (;i2 != t.end(); ++i2) *i2 = val;

            swap(t);
        }

        /// Assign the safearray to be \p v elements of \p val.
        void assign(size_type n, const T& val) {
            safearray_t t(n, lower_bound(), val);
            swap(t);
        }

        /// Assign the safearray from a f \p first and \p last iterators.
        template<typename InputIterator> void assign(InputIterator first, InputIterator last) {
            assign_aux(first, last, type_traits::int_holder< type_traits::is_integer<InputIterator>::result >());
        }

        /** Return the IRecordInfo struct for the array for VT_RECORD.
         */
        com_ptr<IRecordInfo> get_record_info()
        {
            com_ptr<IRecordInfo> rec_info;
            ::SafeArrayGetRecordInfo(psa_, rec_info.out()) | raise_exception;
            return rec_info;
        }
        /** Get the Variant Type of the members of the array.
         */
        VARTYPE get_vt()
        {
            // Something appears broken in SafeArrayGetVartype - returning VT_UNKNOWN when it has FADF_DISPATCH set.
            if (psa_->fFeatures & FADF_DISPATCH) return VT_DISPATCH;
            if (psa_->fFeatures & FADF_UNKNOWN) return VT_UNKNOWN;
            VARTYPE retval;
            ::SafeArrayGetVartype(psa_, &retval) | raise_exception;
            return retval;
        }

        /** Return interface-id of the members of the array.
         */
        uuid_t get_iid()
        {
            uuid_t iid;
            ::SafeArrayGetIID(psa_, &iid) | raise_exception;
            return iid;
        }

private:
        template<typename InputIterator> void assign_aux(InputIterator first, InputIterator last, type_traits::int_holder<false>)
        {
            safearray_t t( first, last, lower_bound() );
            swap(t);
        }

        template<typename Integer> void assign_aux(Integer sz, const T& val, type_traits::int_holder<true>)
        {
            safearray_t t(sz, lower_bound(), val);
            swap(t);
        }
public:

        /// Insert \p n elements of \p val at position \p pos.
        void insert(iterator pos, size_type n, const T& val) {
            safearray_t t(n+size(), lower_bound());
            iterator i1 = t.begin();
            iterator i2 = begin();

            for (;i2 != pos; ++i1, ++i2) *i1 = *i2;
            for (;n>0;--n, ++i1) *i1 = val;
            for (;i2 != end(); ++i1, ++i2) *i1 = *i2;

            swap(t);
        }


        /// Insert elements coppied from iterator \p first to \p last at position pos.
        template<typename InputIterator> void insert(iterator pos, InputIterator first, InputIterator last) {
            insert_aux(pos, first, last, type_traits::int_holder<type_traits::is_integer<InputIterator>::result >());
        }

        /// Push an element to the back of the list (ensure lower-bound);
        void push_back( const T& val, index_type lb )
        {
            safearray_t t(size()+1, lb);

            iterator i1 = begin(), i2 = t.begin();

            for (;i1 != end() ; ++i1, ++i2)
                std::swap(*i1, *i2);

            *i2 = val;

            swap(t);
        }

        /// Push an element to the back of the list.
        inline void push_back( const T& val)
        {
            push_back(val, lower_bound());
        }

        /// Pop an element from the back of the list.
        inline void pop_back()
        {
            size_type lastone = size();
            if (lastone > 0)
                erase(begin()+(lastone-1));
        }

        /// Push an element to the front of the list (ensure lower-bound).
        void push_front( const T &val, index_type lb)
        {
            safearray_t t(size()+1, lb);

            iterator i1 = begin(), i2 = t.begin();

            *i2 = val;

            for (++i2; i1 != end(); ++i1, ++i2)
                std::swap(*i1, *i2);

            swap(t);
        }
        /// Push an element to the front of the list.
        inline void push_front( const T &val)
        {
            push_front(val, lower_bound());
        }
        /// Pop an element from the front of the list.
        inline void pop_front()
        {
            erase(begin());
        }

        /// Erase a specified item.
        /** \param it Item to erase
         *  \return Item beyond erased item.
         */
        iterator erase(iterator it)
        {
            if (it == end())
                return end();
            size_type where= it-begin();

            safearray_t t(size()-1, lower_bound());
            iterator ret = t.end();
            iterator i1 = begin(), i2 = t.begin();
            // Copy up to iterator
            for (; i1 != end() && i1 != it; ++i1, ++i2)
                std::swap(*i1,*i2);
            ++i1;// Skip this one
            for (; i1 != end(); ++i1, ++i2)
                std::swap(*i1,*i2);

            swap(t);
            return begin()+where;
        }

        /// Erase a range of items.
        /** \param first Item to erase from
         *  \param second Item after range to be erased.
         *  \return Item beyond erased range.
         */
        iterator erase(iterator first, iterator second)
        {
            safearray_t t(size()-(second-first), lower_bound());
            size_type where= first-begin();
            iterator i1 = begin(), i2 = t.begin();
            // Copy up to first.
            for (; i1 != end() && i1 != first; ++i1, ++i2)
                std::swap(*i1,*i2);
            // Skip up to second
            for( ;  i1 != second; ++i1)
                ;
            // skip to end.
            for (; i1 != end(); ++i1, ++i2)
                std::swap(*i1,*i2);

            swap(t);
            return begin()+where;
        }


    private:
        template<typename InputIterator> void insert_aux(iterator pos, InputIterator first, InputIterator last, type_traits::int_holder<false>) {
            size_type n = std::distance(first, last);

            safearray_t t(n+size(), lower_bound());
            iterator i1 = t.begin();
            iterator i2 = begin();

            for (;i2 != pos; ++i1, ++i2) *i1 = *i2;
            for (;first != last; ++i1, ++first) *i1 = *first;
            for (;i2 != end(); ++i1, ++i2) *i1 = *i2;

            swap(t);
        }

        template<typename Integer> void insert_aux(iterator pos, Integer n, const T& val, type_traits::int_holder<true>) {
            safearray_t t(n+size(), lower_bound());
            iterator i1 = t.begin();
            iterator i2 = begin();

            for (;i2 != pos; ++i1, ++i2) *i1 = *i2;
            for (;n>0;--n, ++i1) *i1 = val;
            for (;i2 != end(); ++i1, ++i2) *i1 = *i2;

            swap(t);
        }
    public:

        //! \name Assignment Operators
        //@{
        safearray_t& operator=(const safearray_t& sa)
        {
            safearray_t t(sa);
            swap(t);
            return *this;
        }

        safearray_t& operator=(const variant_t& v)
        {
            safearray_t t(v);
            swap(t);
            return *this;
        }

        safearray_t& operator=(const impl::auto_attach_t<SAFEARRAY*>& sa)
        {
            safearray_t t(sa);
            swap(t);
            return *this;
        }
        //@}

    private:
        void destroy() {
            if (psa_ != 0) {
                COMET_ASSERT(psa_->cLocks == 1);
                ::SafeArrayUnlock(psa_);
                ::SafeArrayDestroy(psa_);
                psa_ = 0;
            }
        }

    public:

        ~safearray_t() {
            destroy();
        }

        /// Unlock and detach a raw SAFEARRAY.
        SAFEARRAY* detach() {
            if (psa_) {
                ::SafeArrayUnlock(psa_);
            }
            SAFEARRAY* rv = psa_;
            psa_ = 0;
            return rv;
        }

        /*! Detach the safearray to the variant \p var.
          The safearray becomes invalid after this point.
          \code
              safe_array_t<int> ints(2,0);
              variant_t var;
              ints.detach_to(var);
           \endcode
          */
        void detach_to( variant_t &var)
        {
            COMET_ASSERT(psa_->cLocks == 1);
            if (psa_) ::SafeArrayUnlock(psa_) | raise_exception;
            var = auto_attach( psa_ );
            psa_= 0;
        }

        /*! Detach a safearray from the variant \p var.
          An attempt is made to cast the type of the variant before attaching.
          */
        void detach_from( variant_t &var)
        {
            if(var.get_vt()!=(VT_ARRAY|traits::vt))
            {
                var.change_type(VT_ARRAY|traits::vt);
            }
            safearray_t t(auto_attach(var.detach().parray));
            swap(t);
        }

        /// The lower bound of the array.
        /** \sa get_at
         */
        index_type lower_bound() const {
            return psa_ ? psa_->rgsabound[0].lLbound : 0;
        }

        /// Change the lower_bound of the array.
        void lower_bound(index_type lb) {
            psa_->rgsabound[0].lLbound = lb;
        }

    private:
        class sa_auto_lock_t
        {
            SAFEARRAY** ppsa_;

            // These are not available
            sa_auto_lock_t();
            sa_auto_lock_t(const sa_auto_lock_t&);
            sa_auto_lock_t& operator=(const sa_auto_lock_t&);
        public:
            operator SAFEARRAY**() throw() { return ppsa_; }

            sa_auto_lock_t(SAFEARRAY** ppsa) : ppsa_(ppsa) {}

            ~sa_auto_lock_t()
            {
                if (*ppsa_) {
                    HRESULT hr = ::SafeArrayLock(*ppsa_);
                    COMET_ASSERT( SUCCEEDED(hr) );
                    hr;
                }
            }
        };

    public:
        //! \name Access converters
        //@{
        SAFEARRAY* in() const throw() {
            return const_cast<SAFEARRAY*>(psa_);
        }
        SAFEARRAY** in_ptr() const throw() {
            return const_cast<SAFEARRAY**>(&psa_);
        }
        sa_auto_lock_t inout() throw() {
            if (psa_) {
                ::SafeArrayUnlock(psa_);
            }
            return &psa_;
        }
        sa_auto_lock_t out() throw() {
            destroy();
            return &psa_;
        }
        //@}
        /*! Detach a raw SAFEARRAY pointer from a safearray_t.
         */
        static SAFEARRAY* detach(safearray_t& sa)
        {
            return sa.detach();
        }

        /** Create a reference to a safearray from a raw SAFEARRAY pointer.
          * Mainly used by the implementation wrappers.
          */
        static const impl::safearray_auto_const_ref_t<T> create_const_reference(SAFEARRAY* const & sa);
        static impl::safearray_auto_ref_t<T> create_reference(SAFEARRAY* & sa);

        /** Create a reference to a safearray from a variant.
          \code
             function( const variant_t &var)
             {
                const safe_array<bstr_t> &stringarray = safe_array<bstr_t>::create_reference( var );
             }
          \endcode
          */
        static const impl::safearray_auto_const_ref_t<T> create_const_reference(const variant_t &var);
        /** Create c const reference to a safearray from a variant.
         */
        static impl::safearray_auto_ref_t<T> create_reference(variant_t &var);

        void swap(safearray_t& sa) throw()
        {
            std::swap(psa_, sa.psa_);
        }

    private:

        void range_check(index_type n) const {
            size_type m = (size_type)(n - lower_bound());
            if (/*m < 0 || */ m >= size()) throw std::out_of_range("safearray_t");
        }

        typename traits::raw* get_array() const {
            if (psa_) {
                COMET_ASSERT(psa_->cLocks != 0);
                return static_cast<typename traits::raw*>(psa_->pvData);
            }
            return NULL;
        }

        typename traits::raw& get_element(size_type n) const {
            return get_array()[n - lower_bound()];
        }

    protected:
        SAFEARRAY* psa_;

#ifndef COMET_PARTIAL_SPECIALISATION

        template< enum impl::sa_traits_check_type STCT >
        struct traits_sanity_check
        { static inline void check( const SAFEARRAY *psa) {  } };
        template<>
        struct traits_sanity_check<impl::stct_vt_ok>
        {
            static void check(SAFEARRAY *psa) {
                if ((psa->fFeatures & FADF_HAVEVARTYPE)!=0)
                {
                    VARTYPE vt;
                    ::SafeArrayGetVartype(psa, &vt) | raise_exception ;
                    if(vt !=  impl::sa_traits<T>::vt)
                        throw std::runtime_error("safearray_t: VarType mismatch");
                }
            }
        };
        template<>
        struct traits_sanity_check<impl::stct_iid_ok> {
            static void check(SAFEARRAY *psa)
            {
                uuid_t iid;
                ::SafeArrayGetIID(psa, &iid) | raise_exception;
                if( iid != impl::sa_traits<T>::iid() )
                    throw std::runtime_error("safearray_t: IID mismatch");
            }
        };
#endif
        /// Make sure the passed in safearray agrees with the type of the safearray_t
        static void sanity_check(SAFEARRAY* psa) {
            if (psa == 0) return;
            if (psa->cDims != 1) throw std::runtime_error("safearray_t: Invalid dimension");
            if (!traits::are_features_ok( psa->fFeatures )) throw std::runtime_error("safearray_t: fFeatures is invalid");
#ifndef COMET_PARTIAL_SPECIALISATION
            traits_sanity_check< impl::sa_traits_check_type(traits::check_type)>::check(psa);
#else
            traits_sanity_check<T,impl::sa_traits_check_type(traits::check_type)>::check(psa);
#endif
            if (sizeof(T) != psa->cbElements) throw std::runtime_error("safearray_t: cbElements mismatch");
        }

    };
    //@}

    namespace impl {

        template< typename T>
        class safearray_auto_ref_t : public safearray_t<T>
        {
            // Don't allow any of these.
            safearray_auto_ref_t();
            safearray_auto_ref_t &operator=(const safearray_auto_ref_t &);
            safearray_auto_ref_t &operator=(const safearray_t<T> &);
            safearray_t<T>& operator=(const impl::auto_attach_t<SAFEARRAY*> &);
            void swap(safearray_t<T>& sa);

            // Remember where we got the original for a non-const reference
            SAFEARRAY *& psa_;

        public:
            safearray_auto_ref_t(const safearray_auto_ref_t &sa)
                : safearray_t<T>(auto_attach(sa.psa_)), psa_(sa.psa_)
            {
                COMET_STATIC_ASSERT(false);
            }

            explicit safearray_auto_ref_t(SAFEARRAY *& psa)
                : safearray_t<T>(auto_attach(psa)), psa_(psa)
            {
            }

            ~safearray_auto_ref_t()
            {
                psa_ = this->detach();
            }
        };

        template< typename T>
        class safearray_auto_const_ref_t : public safearray_t<T>
        {
            // Don't allow any of these.
            safearray_auto_const_ref_t();
            safearray_auto_const_ref_t &operator=(const safearray_auto_const_ref_t &);
            safearray_auto_const_ref_t &operator=(const safearray_t<T> &);
            safearray_t<T>& operator=(const impl::auto_attach_t<SAFEARRAY*> &);
            void swap(safearray_t<T>& sa);

        public:
            safearray_auto_const_ref_t(const safearray_auto_const_ref_t &sa)
                : safearray_t<T>(auto_attach(sa.psa_))
            {
                COMET_STATIC_ASSERT(false);
            }

            explicit safearray_auto_const_ref_t(SAFEARRAY *psa)
                : safearray_t<T>(auto_attach(psa))
            {
            }

            ~safearray_auto_const_ref_t()
            {
                this->detach();
            }
        };

    }

    template<typename T>
    const impl::safearray_auto_const_ref_t<T> safearray_t<T>::create_const_reference(SAFEARRAY* const & sa)
    {
        return impl::safearray_auto_const_ref_t<T>(sa);
    }
    template<typename T>
    impl::safearray_auto_ref_t<T> safearray_t<T>::create_reference(SAFEARRAY* & sa)
    {
        return impl::safearray_auto_ref_t<T>(sa);
    }
    template<typename T>
    const impl::safearray_auto_const_ref_t<T> safearray_t<T>::create_const_reference(const variant_t &var)
    {
        if(var.get_vt()!=(VT_ARRAY|traits::vt))
            throw std::exception("unexepected array type");

        SAFEARRAY *sa = var.get().parray;
        return impl::safearray_auto_const_ref_t<T>(sa);
    }
    template<typename T>
    impl::safearray_auto_ref_t<T> safearray_t<T>::create_reference(variant_t &var)
    {
        if(var.get_vt()!=(VT_ARRAY|traits::vt))
            throw std::exception("unexepected array type");

        SAFEARRAY *sa = var.get().parray;
        return impl::safearray_auto_ref_t<T>(sa);
    }



    template<typename T, typename TR> inline comet::impl::sa_iterator<T, TR> operator+(size_t n, const comet::impl::sa_iterator<T, TR>& it) {
        return it + n;
    }

} // namespace comet

namespace {
    COMET_STATIC_ASSERT( sizeof(SAFEARRAY*) == sizeof(comet::safearray_t<long>) );
}

namespace std {
    template<> inline void swap( comet::safearray_t<long>& x, comet::safearray_t<long>& y) COMET_STD_SWAP_NOTHROW;
    template<> inline void swap( comet::safearray_t<unsigned long>& x, comet::safearray_t<unsigned long>& y) COMET_STD_SWAP_NOTHROW;
    template<> inline void swap( comet::safearray_t<short>& x, comet::safearray_t<short>& y) COMET_STD_SWAP_NOTHROW;
    template<> inline void swap( comet::safearray_t<unsigned short>& x, comet::safearray_t<unsigned short>& y) COMET_STD_SWAP_NOTHROW;
    template<> inline void swap( comet::safearray_t<float>& x, comet::safearray_t<float>& y) COMET_STD_SWAP_NOTHROW;
    template<> inline void swap( comet::safearray_t<double>& x, comet::safearray_t<double>& y) COMET_STD_SWAP_NOTHROW;
    template<> inline void swap( comet::safearray_t<comet::bstr_t>& x, comet::safearray_t<comet::bstr_t>& y) COMET_STD_SWAP_NOTHROW;
    template<> inline void swap( comet::safearray_t<comet::variant_t>& x, comet::safearray_t<comet::variant_t>& y) COMET_STD_SWAP_NOTHROW;
    template<> inline void swap( comet::safearray_t<comet::com_ptr< ::IUnknown> >& x, comet::safearray_t<comet::com_ptr< ::IUnknown> >& y) COMET_STD_SWAP_NOTHROW;
    template<> inline void swap( comet::safearray_t<comet::com_ptr< ::IDispatch> >& x, comet::safearray_t<comet::com_ptr< ::IDispatch> >& y) COMET_STD_SWAP_NOTHROW;
}

template<> inline void std::swap( comet::safearray_t<long>& x, comet::safearray_t<long>& y) COMET_STD_SWAP_NOTHROW { x.swap(y); }
template<> inline void std::swap( comet::safearray_t<unsigned long>& x, comet::safearray_t<unsigned long>& y) COMET_STD_SWAP_NOTHROW { x.swap(y); }
template<> inline void std::swap( comet::safearray_t<short>& x, comet::safearray_t<short>& y) COMET_STD_SWAP_NOTHROW { x.swap(y); }
template<> inline void std::swap( comet::safearray_t<unsigned short>& x, comet::safearray_t<unsigned short>& y) COMET_STD_SWAP_NOTHROW { x.swap(y); }
template<> inline void std::swap( comet::safearray_t<float>& x, comet::safearray_t<float>& y) COMET_STD_SWAP_NOTHROW { x.swap(y); }
template<> inline void std::swap( comet::safearray_t<double>& x, comet::safearray_t<double>& y) COMET_STD_SWAP_NOTHROW { x.swap(y); }
template<> inline void std::swap( comet::safearray_t<comet::bstr_t>& x, comet::safearray_t<comet::bstr_t>& y) COMET_STD_SWAP_NOTHROW { x.swap(y); }
template<> inline void std::swap( comet::safearray_t<comet::variant_t>& x, comet::safearray_t<comet::variant_t>& y) COMET_STD_SWAP_NOTHROW { x.swap(y); }
template<> inline void std::swap( comet::safearray_t<comet::com_ptr< ::IUnknown> >& x, comet::safearray_t<comet::com_ptr< ::IUnknown> >& y) COMET_STD_SWAP_NOTHROW { x.swap(y); }
template<> inline void std::swap( comet::safearray_t<comet::com_ptr< ::IDispatch> >& x, comet::safearray_t<comet::com_ptr< ::IDispatch> >& y) COMET_STD_SWAP_NOTHROW { x.swap(y); }

#endif
