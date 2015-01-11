/** \file
  * Common utility classes wrappers.
  */
/*
 * Copyright © 2000, 2001 Sofus Mortensen
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

#ifndef COMET_COMMON_H
#define COMET_COMMON_H

#include <comet/config.h>
#include <comet/interface.h>
#include <wtypes.h>

namespace comet {

    class uuid_t;
    template<typename Itf>class com_ptr;
    namespace impl {
        template<typename T> T* bad_alloc_check(T* x)
        {
            if (!x) throw std::bad_alloc();
            return x;
        }

        /** Implementation struct for auto_attach.
         * \internal
         */
        template<typename T> class auto_attach_t
        {
        public:
            explicit auto_attach_t(const T& v) : val_(v) {};
            const T& get() const { return val_; }
        private:
            const T& val_;
            auto_attach_t& operator=(const auto_attach_t&);
        };

        // Forward declare all these - only used if we actually WANT a
        // safearray.
        /** Safearray traits for the given type.
         * \internal
         */
        template<typename T> struct sa_traits;
        /** Safearray iterator for the given type.
         * \internal
         */
        template<typename S,typename T> class sa_iterator;
        /** Safearray const traits for the given type.
         * \internal
         */
        template<typename T> struct const_traits;
        /** Safearray non-const traits for the given type.
         * \internal
         */
        template<typename T> struct nonconst_traits;

        enum sa_traits_check_type { stct_features_ok, stct_vt_ok, stct_iid_ok };
        enum sa_traits_extras_type { stet_null, stet_record, stet_iid };

        // Interface traits are needed by all interfaces so that we can create
        // safearrays of the types.
        template<typename INTERFACE, VARTYPE VT, long FEATURE_FLAG>
        struct interface_sa_traits
        {
            enum { vt = VT };
            enum { check_type = stct_iid_ok };
            enum { extras_type = stet_iid };

            typedef INTERFACE* raw;
            typedef com_ptr<INTERFACE>  value_type;
            typedef com_ptr<INTERFACE> & reference;
            typedef const com_ptr<INTERFACE> & const_reference;

            static reference create_reference(raw& x) { return *reinterpret_cast<com_ptr< INTERFACE>*>(&x); }
            static const_reference create_const_reference(raw& x) { return *reinterpret_cast<const com_ptr< INTERFACE >*>(&x); }
            typedef nonconst_traits<com_ptr<INTERFACE> > nct;
            typedef sa_iterator<com_ptr<INTERFACE>, nct > iterator;
            typedef sa_iterator<com_ptr<INTERFACE>, const_traits<com_ptr<INTERFACE> > > const_iterator;

            static bool are_features_ok(unsigned short f) { return (f & FEATURE_FLAG) != 0 && (f & FADF_HAVEIID) != 0; }
            static const uuid_t& iid() { return uuidof<INTERFACE>(); }
        };

        /** Basic safearray traits  - used by enums.
         * \internal
         */
        template<typename T, VARTYPE VT> struct basic_sa_traits
        {
            enum { vt = VT };
            enum { check_type = stct_vt_ok };
            enum { extras_type = stet_null };

            typedef T raw;
            typedef T value_type;
            typedef T& reference;
            typedef const T& const_reference;

            static reference create_reference(T& x) { return x; }
            static const_reference create_const_reference(T& x) { return x; }

            typedef T* iterator;
            typedef const T* const_iterator;

            static bool are_features_ok(unsigned short) { return true; }
        };
    }

    /*! \addtogroup COMType
     */
    //@{
    /// Used to attach a raw parameter to a wrapper.
    template<typename T> impl::auto_attach_t<T> auto_attach(const T& t) { return impl::auto_attach_t<T>(t); }


//    template<typename T, typename U> inline T up_cast(const U& u, T* = 0) { return u; }


    /*! VARIANT_BOOL to bool [in] converter.
      *  This is used by the generated wrappers.
      */
    inline VARIANT_BOOL bool_in(bool x) { return x ? COMET_VARIANT_TRUE : COMET_VARIANT_FALSE; }

    /** \class bool_out  common.h comet/common.h
      *  VARIANT_BOOL to bool [out] converter.
      *  This is used by the generated wrappers.
      */
    class bool_out {
    public:
        operator VARIANT_BOOL*() { return &vb_; }
        bool_out(bool& b) : b_(b) {}
        ~bool_out() { b_ = vb_ ? true : false; }
    private:
        bool_out &operator=( const bool_out &);
        VARIANT_BOOL vb_;
        bool& b_;
    };

    namespace impl {
        class bool_adapter_t {
        public:
            bool_adapter_t(VARIANT_BOOL* p) : pb_(p) { b_ = *pb_ ? true : false; }
            ~bool_adapter_t() { *pb_ = b_ ? COMET_VARIANT_TRUE : COMET_VARIANT_FALSE; }

            bool& ref() { return b_; }
        private:
            bool_adapter_t(const bool_adapter_t&);
            bool_adapter_t& operator=(const bool_adapter_t&);

            VARIANT_BOOL* pb_;
            bool b_;
        };
    }

    /** \class bool_inout  common.h comet/common.h
      *  VARIANT_BOOL to bool [in,out] converter.
      *  This is used by the generated wrappers.
      */
    class bool_inout {
    public:
        operator VARIANT_BOOL*() { return &vb_; }
        bool_inout(bool& b) : b_(b), vb_(b ? COMET_VARIANT_TRUE : COMET_VARIANT_FALSE) {}
        ~bool_inout() { b_ = vb_ ? true : false; }
    private:
        bool_inout &operator=(const bool_inout &);
        VARIANT_BOOL vb_;
        bool& b_;
    };

    /** \class variant_bool_t common.h comet/common.h
      * VARIANT_BOOL wrapper for structs.
      * Stands in place of a VARIANT_BOOL in a struct, and behaves like a bool.
      * This is imporant as sizeof(VARIANT_BOOL) != sizeof(bool).
      */
    class variant_bool_t
    {
        VARIANT_BOOL vb_;
    public:
        /// \name Constructors.
        //@{
        variant_bool_t(): vb_(COMET_VARIANT_FALSE) {}
        variant_bool_t(const impl::auto_attach_t<VARIANT_BOOL> &b) : vb_(b.get()) {}
        variant_bool_t(bool b) : vb_(b?COMET_VARIANT_TRUE:COMET_VARIANT_FALSE) {}
        //@}

        /// \name Assignment operators.
        //@{
        variant_bool_t &operator=( bool b) { vb_ = b?COMET_VARIANT_TRUE:COMET_VARIANT_FALSE;  return *this;}
        variant_bool_t &operator=( const impl::auto_attach_t<VARIANT_BOOL> &b) { vb_ = b.get(); return *this; }
        //@}

        /// \name Boolean operators.
        //@{
        operator bool() const{ return vb_!= COMET_VARIANT_FALSE; }
        bool operator !() const { return vb_== COMET_VARIANT_FALSE; }
        bool operator==( variant_bool_t vb) const { return vb.vb_ == vb_; }
        bool operator!=( variant_bool_t vb) const { return vb.vb_ != vb_; }
        //@}

        /// \name Bitwise operators
        //@{
        variant_bool_t operator~() const { variant_bool_t temp(*this); temp.vb_ = (VARIANT_BOOL)~(temp.vb_); return temp; }
        variant_bool_t &operator&=( const variant_bool_t &b) { vb_ &= b.vb_; return *this; }
        variant_bool_t &operator|=( const variant_bool_t &b) { vb_ |= b.vb_; return *this; }
        variant_bool_t &operator^=( const variant_bool_t &b) { vb_ ^= b.vb_; return *this; }
        variant_bool_t operator&( const variant_bool_t &b)const { variant_bool_t temp(*this); temp.vb_ &= b.vb_; return temp; }
        variant_bool_t operator|( const variant_bool_t &b)const { variant_bool_t temp(*this); temp.vb_ |= b.vb_; return temp; }
        variant_bool_t operator^( const variant_bool_t &b)const { variant_bool_t temp(*this); temp.vb_ ^= b.vb_; return temp; }
        //@}
        /// \name bool operators
        //@{
        bool operator&( bool b)const { return b & operator bool(); }
        bool operator|( bool b)const { return b | operator bool(); }
        bool operator^( bool b)const { return b ^ operator bool(); }
        //@}

        static const variant_bool_t &create_const_reference(const VARIANT_BOOL &vb) { return reinterpret_cast<const variant_bool_t &>(vb); }
        static variant_bool_t &create_reference(VARIANT_BOOL &vb) { return reinterpret_cast<variant_bool_t &>(vb); }

        ///\name Raw accessors
        //@{
        VARIANT_BOOL in() { return vb_; }
        VARIANT_BOOL *out() { return &vb_; }
        VARIANT_BOOL *inout() { return &vb_; }
        //@}


        /** Allow treating of class as a bool *.
         * \sa bool_ptr()
          */
        class bool_pointer_t
        {
            friend class variant_bool_t;
            protected:
                bool_pointer_t( VARIANT_BOOL &vb) :  vb_(vb), b_( vb != COMET_VARIANT_FALSE) {}
            public:
                ~bool_pointer_t() { vb_ = b_ ? COMET_VARIANT_TRUE: COMET_VARIANT_FALSE; }
                operator bool*(){ return &b_; }
                operator const bool*()const{ return &b_; }
            private:
                bool_pointer_t &operator=(const bool_pointer_t &);
                bool b_;
                VARIANT_BOOL &vb_;
        };
        /** Return a class that stands in for a bool *.
           Should be used in place of operator & for passing in to a bool * function.
          \code
              variant_bool_t vb;
            SomeFunc(vb.bool_ptr());
          \endcode
         */
        bool_pointer_t bool_ptr()
        {
            return bool_pointer_t(vb_);
        }
        const bool_pointer_t bool_ptr() const
        {
            return bool_pointer_t(const_cast<VARIANT_BOOL &>(vb_));
        }

        friend class bool_reference_t;
        /** Allow efficient choosing between a bool& and a variant_bool&.
         */
        class bool_reference_chooser_t
        {
            friend class variant_bool_t;
            variant_bool_t &vbt_;
        protected:
            bool_reference_chooser_t(variant_bool_t &vbt):vbt_(vbt) {}
        private:
            bool_reference_chooser_t &operator=(const bool_reference_chooser_t &);
        public:
            inline operator variant_bool_t&() { return vbt_;}
            inline operator const variant_bool_t&()const { return vbt_;}
        };
        /** Allow treating of a class as a bool &.
         * \sa bool_ref()
         */
        class bool_reference_t : protected bool_pointer_t
        {
            public:
                bool_reference_t( bool_reference_chooser_t &brc )
                    : bool_pointer_t(*static_cast<variant_bool_t&>(brc).inout())
                    {}
                operator bool &(){ return tmp; /*return * (bool_pointer_t::operator bool*());*/ }
                operator const bool &()const { return *(bool_pointer_t::operator const bool*()); }
                bool tmp;
        };

        /** Return a class that stands in for a bool & or a variant_bool_t &.
         */
        bool_reference_chooser_t bool_ref()
        {
            return bool_reference_chooser_t(*this);
        }
        const bool_reference_chooser_t bool_ref() const
        {
            return bool_reference_chooser_t(const_cast<variant_bool_t &>(*this));
        }
    };
    //@}

} // namespace

#endif
