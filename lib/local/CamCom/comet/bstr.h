/** \file
  * BSTR wrapper classes.
  */
/*
 * Copyright © 2000-2004 Sofus Mortensen, Michael Geddes
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

#ifndef COMET_BSTR_H
#define COMET_BSTR_H

#include <comet/config.h>

#ifdef COMET_BROKEN_WTYPES
#include <windows.h>
#endif // COMET_BROKEN_WTYPES
#include <wtypes.h>
#include <malloc.h>
#include <stdexcept>
#ifndef COMET_GCC_HEADERS
#include <oleauto.h>
#endif // COMET_GCC_HEADERS

#include <string>
#include <functional>
#undef max
#include <limits>

#include <comet/assert.h>
#include <comet/common.h>
#include <comet/static_assert.h>
#include <comet/error_fwd.h>
#include <comet/uuid_fwd.h>
#include <comet/currency.h>
#include <comet/type_traits.h>

#pragma warning(push)
#pragma warning(disable : 4522 4521)

#pragma comment( lib, "oleaut32" )

namespace comet {

#ifndef NORM_IGNOREKASHIDA
#define     NORM_IGNOREKASHIDA 0x00040000
#endif // NORM_IGNOREKASHIDA
    /*! \addtogroup COMType
     */
    //@{
    //! Comparsion flags.
    /*! Can be used with \link comet::bstr_t::cmp cmp \endlink or the comparsion functors.
        \sa cmp
            less
            less_equal
            greater
            greater_equal
            equal_to
            not_equal_to
    */

    enum compare_flags_t
    {
        cf_ignore_case = NORM_IGNORECASE,           //!<  Ignore case.
        cf_ingore_nonspace = NORM_IGNORENONSPACE,   //!<  Ignore nonspacing chars.
        cf_ignore_symbols = NORM_IGNORESYMBOLS,     //!<  Ignore symbols.
        cf_ignore_width = NORM_IGNOREWIDTH,         //!<  Ignore string width.
        cf_ignore_kanatype = NORM_IGNOREKANATYPE,   //!<  Ignore Kana type.
        cf_ignore_kashida = NORM_IGNOREKASHIDA      //!<  Ignore Arabic kashida characters.
    };
    //@}

    namespace impl {

        inline const wchar_t* null_to_empty(const wchar_t* s)
        { return s ? s : L""; }

    } // namespace


    /*! \addtogroup COMType
     */
    //@{

    /*! \class bstr_t bstr.h comet/bstr.h
      *  BSTR Wrapper.
      *  \sa bstr_t
      */
    class bstr_t {
    public:
        typedef wchar_t value_type;
#if !(defined(_STLP_DEBUG) || (defined(_HAS_ITERATOR_DEBUGGING)) && _MSC_VER >= 1400)
        typedef std::wstring::iterator iterator;
        typedef std::wstring::const_iterator const_iterator;

        typedef std::wstring::reverse_iterator reverse_iterator;
        typedef std::wstring::const_reverse_iterator const_reverse_iterator;
#else // _STLP_DEBUG
        typedef wchar_t *iterator;
        typedef const wchar_t *const_iterator;
#if defined(COMET_STD_ITERATOR)
        typedef std::reverse_iterator<iterator> reverse_iterator;
        typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
#else
        // workaround for broken reverse_iterator implementations due to no partial specialisation
        typedef std::reverse_iterator<iterator,wchar_t> reverse_iterator;
        typedef std::reverse_iterator<const_iterator,wchar_t> const_reverse_iterator;
#endif
#endif // _STLP_DEBUG

        typedef std::wstring::size_type size_type;
        typedef std::wstring::difference_type difference_type;

        typedef wchar_t& reference;
        typedef const wchar_t const_reference;

    private:
        BSTR str_;

        void construct() { str_ = 0; }
        void construct(BSTR s, bool copy) throw(std::bad_alloc)
        { if (copy) str_ = copy_str(s); else str_ = s; }

        void construct(const wchar_t* s) throw(std::bad_alloc)
        { str_ = copy_str(s); }

        void construct(const wchar_t* s, size_t len) throw(std::bad_alloc)
        { str_ = copy_str(s, len); }

        void construct(const char* s) throw(std::bad_alloc, std::runtime_error) {
            convert_str(s, -1);
        }

        void construct(const char* s, size_t len)
        {
            if (len >= static_cast<size_t>(std::numeric_limits<int>::max()))
                throw std::length_error(
                    "String exceeded maximum length for conversion");

            convert_str(s, static_cast<int>(len+1));
        }

        void construct(const uuid_t& u, bool braces)
        {
            str_ = impl::bad_alloc_check(::SysAllocStringLen(0, braces?38:36));
            u.copy_to_str(str_+(braces?1:0));
            if (braces)
            {
                str_[0]=L'{';
                str_[37]=L'}';
            }
        }

        void construct(const wchar_t* s1, size_t l1, const wchar_t* s2, size_t l2) throw(std::bad_alloc)
        {
            str_ = impl::bad_alloc_check(::SysAllocStringLen(NULL, UINT(l1+l2)));
            if (l1) memcpy(str_, s1, sizeof(wchar_t)*(l1));
            if (l2) memcpy(str_+l1, s2, sizeof(wchar_t)*(1+l2));
        }

        void destroy() throw()
        { if (str_) ::SysFreeString(str_); }

        bool is_regular() const throw()
        { return !str_ || length() == wcslen(str_); }

        static BSTR copy_str(const wchar_t* src) throw(std::bad_alloc)
        { return src ? impl::bad_alloc_check(::SysAllocString(src)) : 0; }

        static BSTR copy_str(const wchar_t* src, size_t len) throw(std::bad_alloc)
        { return src ? impl::bad_alloc_check(::SysAllocStringLen(src, UINT(len))) : 0; }

        static BSTR copy_str(BSTR src) throw(std::bad_alloc)
        { return src ? impl::bad_alloc_check(::SysAllocStringLen(src, ::SysStringLen(src))) : 0; }

        void convert_str(const char* s, int l) throw(std::bad_alloc, std::runtime_error)
        {
            if (s != 0) {
#if defined(_MBCS) || !defined(COMET_NO_MBCS)
                int wl = ::MultiByteToWideChar(CP_ACP, 0, s, l, NULL,0);
#else
                int wl = ((l>=0)?l: (1+strlen(s)));
                COMET_ASSERT( wl == ::MultiByteToWideChar( CP_ACP, 0, s, l, NULL,0));
#endif
                str_ = impl::bad_alloc_check(::SysAllocStringLen(0, wl - 1));
                if (::MultiByteToWideChar(CP_ACP, 0, s, l, str_, wl) == 0)
                {
                    destroy();
                    throw std::runtime_error("MultiByteToWideChar has failed");
                }
            } else str_ = 0;
        }

    public:
        /*! Default constructor
            Constructs a null string.
        */
        bstr_t() throw()
        {
            construct();
        }

        //! Copy constructor
        /*!
            \param s
                String initialise bstr_t from.

            \exception std::bad_alloc
                On memory exhaustion std::bad_alloc is thrown.
        */
        bstr_t(const bstr_t& s) throw(std::bad_alloc)
        {
            construct(s.str_, true);
        }

        //! Construct string from const wchar_t*
        /*!
            \param s
                String to initialise bstr_t from.

            \exception std::bad_alloc
                On memory exhaustion std::bad_alloc is thrown.
        */
        bstr_t(const wchar_t* s) throw(std::bad_alloc)
        {
            construct(s);
        }

        bstr_t(const wchar_t* s, size_t len) throw(std::bad_alloc)
        {
            construct(s, len);
        }

        //! Construct string from const char*
        /*!
            \param s
                String to initialise bstr_t from.

            \exception std::bad_alloc
                On memory exhaustion std::bad_alloc is thrown.
            \exception std::runtime_error
                Should string conversion fail, std::runtime_error will be thrown.
        */
        bstr_t(const char* s) throw(std::runtime_error)
        {
            construct(s);
        }

        bstr_t(const char* s, size_t len) throw(std::bad_alloc)
        {
            construct(s, len);
        }

        //! Construct string from const std::string&
        /*!
            \param s
                String to initialise bstr_t from.

            \exception std::bad_alloc
                On memory exhaustion std::bad_alloc is thrown.
            \exception std::length_error
                If this given string is too long to be converted,
                std::length_error is thrown.
            \exception std::runtime_error
                Should string conversion fail, std::runtime_error is thrown.
        */
        bstr_t(const std::string& s)
        {
            construct(s.c_str(), s.length());
        }

        //! Construct string from BSTR
        /*!
            Takes ownership of specified BSTR. To prevent misuse the BSTR must be wrapped using auto_attach.

            \code
                bstr_t bs( auto_attach( myBSTR ) );
            \endcode

            \param s
                String to initialise bstr_t from.
        */
        bstr_t(const impl::auto_attach_t<BSTR>& s) throw()
        {
            construct(s.get(), false);
        }


        //! Construct string from const std::wstring&
        /*!
            \param s
                String to initialise bstr_t from.

            \exception std::bad_alloc
                On memory exhaustion std::bad_alloc is thrown.
        */
        bstr_t(const std::wstring& s) throw(std::bad_alloc)
        {
            construct(s.c_str(), s.length());
        }

        explicit bstr_t(size_type sz, wchar_t c) throw(std::bad_alloc)
        {
            str_ = impl::bad_alloc_check(::SysAllocStringLen(0, UINT(sz)));
            std::fill(begin(), end(), c);
        }

        explicit bstr_t(size_type sz) throw(std::bad_alloc)
        {
            str_ = impl::bad_alloc_check(::SysAllocStringLen(0, UINT(sz)));
        }

        template<typename IT>
        explicit bstr_t(IT first, IT last)
        {
            str_ = 0;
            assign(first, last);
        }

        explicit bstr_t(const uuid_t& u, bool braces = false)
        {
            construct(u, braces);
        }

    private:
        bstr_t(const wchar_t* s1, size_t l1, const wchar_t* s2, size_t l2) throw(std::bad_alloc)
        {
            construct(s1, l1, s2, l2);
        }

    public:
        //! Destructor
        /*!
            Deletes the wrapped BSTR.
        */
        ~bstr_t() throw()
        {
            destroy();
        }

        //! Swap
        void swap(bstr_t& x) throw()
        {
            std::swap(str_, x.str_);
        }

        //! Explicit conversion to const wchar_t*
        const wchar_t* c_str() const throw()
        { return impl::null_to_empty(str_); }

        //! Explicit conversion to std::wstring
        std::wstring w_str() const throw()
        { return impl::null_to_empty(str_); }

#ifdef _MBCS
#ifndef COMET_NO_MBCS_WARNING
#pragma message( "Warning: _MBCS is defined. bstr_t::s_str may return an std::string containing multibyte characters" )
#endif
#endif

        //! Explicit conversion to std::string
        std::string s_str() const
        {
            if (is_empty()) return std::string();

            if (length() > static_cast<size_t>(std::numeric_limits<int>::max()))
                throw std::length_error("String is too large to be converted");

            int ol = static_cast<int>(length());

#if defined(_MBCS) || !defined(COMET_NO_MBCS)
            // Calculate the required length of the buffer
            int l = WideCharToMultiByte(CP_ACP, 0, str_, ol, NULL, 0, NULL, NULL);
#else // _MBCS
            int l = ol;
            COMET_ASSERT( l == WideCharToMultiByte(CP_ACP, 0, str_, ol, NULL, 0, NULL, NULL));
#endif // _MBCS

            // Create the buffer
            std::string rv(l, std::string::value_type());
            // Do the conversion.
            if (0 == WideCharToMultiByte(
                CP_ACP, 0, str_, ol, &rv[0], l, NULL, NULL))
            {
                DWORD err = GetLastError();
                raise_exception(HRESULT_FROM_WIN32(err));
            }

            return rv;
        }

        //! Explicit conversion to "tstring".
#ifdef _UNICODE
        std::wstring t_str() const
        {
            return w_str();
        }
#else
        std::string t_str() const
        {
            return s_str();
        }
#endif

        //! Implicit conversion to std::wstring
        operator std::wstring() const { return w_str(); }

        //! Implicit conversion to std::string
        operator std::string() const { return s_str(); }

        //! Returns true if and only if wrapped str is null
        bool is_null() const throw()
        { return str_ == 0; }

        /** Returns true if and only if wrapped str has zero length.
         */
        bool is_empty() const throw() { return length() == 0; }

        //! Returns true if and only if wrapped str has zero length.
        bool empty() const throw() { return length() == 0; }

        //! Returns length of wrapped string.
        size_t length() const throw()
        { return is_null() ? 0 : ::SysStringLen(str_); }

        size_t size() const throw()
        { return length(); }

        /*! \internal */
        BSTR get_raw() const
        { return str_; }

        friend
        std::basic_ostream<char> &operator<<(std::basic_ostream<char> &os, const bstr_t &val)
        { os << val.s_str(); return os;    }

        friend
        std::basic_ostream<wchar_t> &operator<<(std::basic_ostream<wchar_t> &os, const bstr_t &val)
        { os << val.w_str(); return os; }

        /// \name Boolean operators
        //@{

        bool operator==(const wchar_t* s) const
        { return 0 == wcscmp(c_str(), impl::null_to_empty(s) ) && is_regular(); }

        bool operator!=(const wchar_t* s) const
        { return !operator==(s); }

        bool operator<(const wchar_t* s) const
        { return wcscmp(c_str(), impl::null_to_empty(s)) < 0 && is_regular(); }

        bool operator>(const wchar_t* s) const
        { return wcscmp(c_str(), impl::null_to_empty(s)) > 0 && !is_regular();    }

        bool operator>=(const wchar_t* s) const
        { return !operator<(s); }

        bool operator<=(const wchar_t* s) const
        { return !operator>(s); }

        bool operator==(const std::wstring& s) const
        {
            size_t l = length();
            if (l != s.length()) return false;
            return 0 == memcmp(str_, s.c_str(), sizeof(wchar_t)*l);
        }

        bool operator!=(const std::wstring& s) const
        { return !operator==(s); }

        bool operator<(const std::wstring& s) const
        { return std::lexicographical_compare(str_, str_+length(), s.begin(), s.end()); }

        bool operator>(const std::wstring& s) const
        { return std::lexicographical_compare(str_, str_+length(), s.begin(), s.end(), std::greater<wchar_t>()); }

        bool operator>=(const std::wstring& s) const
        { return !operator<(s);    }

        bool operator<=(const std::wstring& s) const
        { return !operator>(s); }

        bool operator==(const bstr_t& s) const
        {
            if (str_ == 0 && s.str_ == 0) return true;
            return ::VarBstrCmp(str_, s.str_, ::GetThreadLocale(), 0) == VARCMP_EQ;
        }

        bool operator!=(const bstr_t& s) const
        { return !operator==(s); }

        bool operator<(const bstr_t& s) const
        {
            if (str_ == 0) {
                return s.str_ != 0;
            }

            if (s.str_ == 0) return false;

            return ::VarBstrCmp(str_, s.str_, ::GetThreadLocale(), 0) == VARCMP_LT;
        }

        bool operator>(const bstr_t& s) const
        {
            if (str_ == 0) {
                return s.str_ != 0;
            }

            if (s.str_ == 0) return false;

            return ::VarBstrCmp(str_, s.str_, ::GetThreadLocale(), 0) == VARCMP_GT;
        }

        bool operator>=(const bstr_t& s) const
        { return !operator<(s);    }

        bool operator<=(const bstr_t& s) const
        { return !operator>(s); }
        //@}

        //! String comparsion function.
        /*! \param s String to compare
            \param flags Comparison Flags
            \retval &lt;0 if less
            \retval 0 if Equal
            \retval &gt;0 if greater
        */
        int cmp(const bstr_t& s, compare_flags_t flags = compare_flags_t(0)) const
        {
            HRESULT res = ::VarBstrCmp(str_, s.str_, ::GetThreadLocale(), flags);
            switch(res)
            {
                case VARCMP_EQ: return 0;
                case VARCMP_GT: return 1;
                case VARCMP_LT: return -1;
                case VARCMP_NULL:
                    return ((str_==0)?0:1) - ((s.str_==0)?0:1);
            }
            if (str_==0 || s.str_ ==0)
                return ((str_==0)?0:1) - ((s.str_==0)?0:1);
            raise_exception(res); return 0;
        }

        //!\name Comparison Functors
        //@{
        //! Less Functor.
        /*!  Useful for STL containers.
            \code
                typedef stl::map < comet::bstr_t, long, bstr_t::less<cf_ignore_case> > string_long_map;
            \endcode
             \param CF comparison flags.
             \relates bstr_t
          */
        template<compare_flags_t CF>
        struct less : std::binary_function< bstr_t,bstr_t,bool>{
            /// Functor.
            bool operator()(const bstr_t& l, const bstr_t& r) const
            { return l.cmp(r, CF) <0;    }
        };

        //! less or equal functor.
        /*!  \relates bstr_t */
        template<compare_flags_t CF>
        struct less_equal : std::binary_function< bstr_t,bstr_t,bool> {
            /// Functor.
            bool operator()(const bstr_t& l, const bstr_t& r) const
            { return l.cmp(r, CF) <=0;    }
        };

        //! greater functor.
        /*!  \relates bstr_t */
        template<compare_flags_t CF>
        struct greater : std::binary_function< bstr_t,bstr_t,bool> {
            /// Functor.
            bool operator()(const bstr_t& l, const bstr_t& r) const
            { return l.cmp(r, CF) > 0;    }
        };

        //! greater or equal functor.
        /*!  \relates bstr_t */
        template<compare_flags_t CF>
        struct greater_equal : std::binary_function< bstr_t,bstr_t,bool> {
            /// Functor.
            bool operator()(const bstr_t& l, const bstr_t& r) const
            { return l.cmp(r, CF) >=0;    }
        };

        //! equality functor.
        template<compare_flags_t CF>
        struct equal_to : std::binary_function< bstr_t,bstr_t,bool> {
            bool operator()(const bstr_t& l, const bstr_t& r) const
            { return l.cmp(r, CF) == 0; }
        };

        //! Inequality functor.
        /*!  \relates bstr_t */
        template<compare_flags_t CF>
        struct not_equal_to : std::binary_function< bstr_t,bstr_t,bool>{
            /// Functor.
            bool operator()(const bstr_t& l, const bstr_t& r) const
            { return l.cmp(r, CF) != 0;    }
        };
        //@}

        iterator begin() { return iterator(str_); }
        iterator end() { return iterator(str_ + length()); }
        const_iterator begin() const { return const_iterator(str_); }
        const_iterator end() const { return const_iterator(str_ + length()); }

        reverse_iterator rbegin() { return reverse_iterator(str_); }
        reverse_iterator rend() { return reverse_iterator(str_ + length()); }
        const_reverse_iterator rbegin() const { return const_reverse_iterator(str_); }
        const_reverse_iterator rend() const { return const_reverse_iterator(str_ + length()); }

        reference at(size_type i) { rangecheck(i); return str_[i]; }
        const_reference at(size_type i) const { rangecheck(i); return str_[i]; }

    private:
        // check range (may be private because it is static)
        void rangecheck (size_type i) const {
            if (i >= length()) { throw std::range_error("bstr_t"); }
        }

    public:
        const_reference operator[](size_type idx) const
        { return str_[idx]; }

        reference operator[](size_type idx)
        { return str_[idx]; }

        //! Assign string to be \p sz of character \p c .
        void assign(size_type sz, wchar_t c) throw(std::bad_alloc)
        {
            bstr_t t(sz, c);
            swap(t);
        }

        //! Assign string from two iterators.
        template<typename IT>
        void assign(IT first, IT last)
        {
            bstr_t t( std::distance(first, last) );

#pragma warning(push)
#pragma warning(disable:4996)
            std::copy(first, last, t.begin());
#pragma warning(pop)

            swap(t);
        }


        //! Assignment operator from any (non integer) constructable.
        template<typename T>
        bstr_t& operator=(const T& x)
        {
            COMET_STATIC_ASSERT( type_traits::is_integer<T>::result == false );
            bstr_t t(x);
            swap(t);
            return *this;
        }

        //! Default assignment.
        bstr_t& operator=(const bstr_t& x) throw(std::bad_alloc)
        {
            bstr_t t(x);
            swap(t);
            return *this;
        }

        //! Concat operation
        bstr_t operator+(const bstr_t& s) const throw(std::bad_alloc)
        {
            return bstr_t(str_, length(), s.str_, s.length());
        }

        //! Concat with const wchar_t*
        bstr_t operator+(const wchar_t* s) const throw(std::bad_alloc)
        {
            return bstr_t(str_, length(), s, wcslen(s));
        }

        //! Concat with std::wstring
        bstr_t operator+(const std::wstring& s) const throw(std::bad_alloc)
        {
            return bstr_t(str_, length(), s.c_str(), s.length());
        }

        //! Concat assignment
        bstr_t& operator+=(const bstr_t& s) throw(std::bad_alloc)
        {
            bstr_t t(str_, length(), s.str_, s.length());
            swap(t);
            return *this;
        }

        //! Concat assignment with const wchar_t*
        bstr_t& operator+=(const wchar_t* s) throw(std::bad_alloc)
        {
            bstr_t t(str_, length(), s, wcslen(s));
            swap(t);
            return *this;
        }

        //! Concat assignment with std::wstring
        bstr_t& operator+=(const std::wstring& s) throw(std::bad_alloc)
        {
            bstr_t t(str_, length(), s.c_str(), s.length());
            swap(t);
            return *this;
        }

        // Detach a raw BSTR from it's wrapper - detach function is dangerous.
        BSTR detach()
        {
            BSTR s(str_);
            str_ = 0;
            return s;
        }

    public:

        //!\name Create a reference to a BSTR
        /*!
            Creates a bstr_t that is a reference to the BSTR.
            It will not be reference counted and will not be deleted when the bstr_t goes out of scope.

            This is used by the interface wrappers for [in] BSTR's. Typically clients do not need create_reference.
        */
        //@{
        static const bstr_t& create_const_reference(const BSTR& s) throw()
        { return *reinterpret_cast<const bstr_t*>(&s); }

        static bstr_t& create_reference(BSTR& s) throw()
        { return *reinterpret_cast<bstr_t*>(&s); }
        //@}

        //! Detaches specified bstr
        static BSTR detach(bstr_t& s)
        {
            return s.detach();
        }

        /*! \internal */
        template<typename T>
        static BSTR detach(const T& s)
        {
            return bstr_t(s).detach();
        }

        /*! \internal */
        BSTR* get_ptr_to_raw() const
        {
            return const_cast<BSTR*>(&str_);
        }

        //! [in] adapter.
        /*!
            Used when calling raw interfaces that require an [in] BSTR argument.

            \code
                bstr_t bs;
                HRESULT hr = pRawInterface->raw_Method(bs.in());
            \endcode

            Only use this wrapper when forced to deal with raw interface.
        */
        BSTR in() const throw()
        {
            return str_;
        }

        //! [in] adapter.
        /*!
            Used when calling raw interfaces that require an [in] BSTR* argument.

            \code
                bstr_t bs;
                HRESULT hr = pRawInterface->raw_Method(bs.in_ptr());
            \endcode

            Only use this wrapper when forced to deal with raw interface.
        */
        BSTR* in_ptr() const throw()
        {
            return const_cast<BSTR*>(&str_);
        }

        //! [out] adapter.
        /*!
            Used when calling raw interfaces that require an [out] BSTR * argument.

            \code
                bstr_t bs;
                HRESULT hr = pRawInterface->raw_MethodThatReturnsBSTR(bs.out());
            \endcode

            Only use this wrapper when forced to deal with raw interface.
        */
        BSTR* out() throw()
        {
            destroy();
            return &str_;
        }

        //! [in, out] adapter.
        /*!
            Used when calling raw interfaces that require an [in, out] BSTR * argument.

            \code
                bstr_t bs;
                HRESULT hr = pRawInterface->raw_MethodThatChangesBSTR(bs.inout());
            \endcode

            Only use this wrapper when forced to deal with raw interface.

            \note If the wrapped BSTR is shared. The bstr_t is copied so that only this version is modified.

            \exception std::bad_alloc
                Throws std::bad_alloc if the bstr_t is being copied and memory is exhausted.
        */
        BSTR* inout() throw(std::bad_alloc)
        {
            return &str_;
        }

        friend bstr_t operator+(const std::wstring& s, const bstr_t& t) throw(std::bad_alloc);
        friend bstr_t operator+(const wchar_t* s, const bstr_t& t) throw(std::bad_alloc);
    };

    //! Concat operation
    inline bstr_t operator+(const std::wstring& s, const bstr_t& t) throw(std::bad_alloc)
    {
        return bstr_t(s.c_str(), s.length(), t.str_, t.length());
    }

    //! Concat operation
    inline bstr_t operator+(const wchar_t* s, const bstr_t& t) throw(std::bad_alloc)
    {
        return bstr_t(s, wcslen(s), t.str_, t.length());
    }
    //@}

    /*! \name Boolean Operators on String
     * \relates bstr_t
     */
    //@{
    inline bool operator==(const wchar_t* s1, const bstr_t& s2) throw()
    {
        return s2 == s1;
    }

    inline bool operator!=(const wchar_t* s1, const bstr_t& s2) throw()
    {
        return s2 != s1;
    }

    inline bool operator<(const wchar_t* s1, const bstr_t& s2) throw()
    {
        return s2 > s1;
    }

    inline bool operator>(const wchar_t* s1, const bstr_t& s2) throw()
    {
        return s2 < s1;
    }

    inline bool operator<=(const wchar_t* s1, const bstr_t& s2) throw()
    {
        return s2 >= s1;
    }

    inline bool operator>=(const wchar_t* s1, const bstr_t& s2) throw()
    {
        return s2 <= s1;
    }

    inline bool operator==(const std::wstring& s1, const bstr_t& s2) throw()
    {
        return s2 == s1;
    }

    inline bool operator!=(const std::wstring& s1, const bstr_t& s2) throw()
    {
        return s2 != s1;
    }

    inline bool operator<(const std::wstring& s1, const bstr_t& s2) throw()
    {
        return s2 > s1;
    }

    inline bool operator>(const std::wstring& s1, const bstr_t& s2) throw()
    {
        return s2 < s1;
    }

    inline bool operator<=(const std::wstring& s1, const bstr_t& s2) throw()
    {
        return s2 >= s1;
    }

    inline bool operator>=(const std::wstring& s1, const bstr_t& s2) throw()
    {
        return s2 <= s1;
    }
    //@}


    // Implementation of uuid_t construct from bstr.
    inline uuid_t::uuid_t(const bstr_t& bs)
    {
        if (init_from_str(bs.c_str(), bs.length()) == false) throw std::runtime_error(err_msg());
    }

    inline currency_t& currency_t::parse( const bstr_t &str, LCID locale )
    {
        VarCyFromStr( str.in(), locale, 0, &cy_ ) | raise_exception;
        return *this;
    }

} // namespace

namespace {
    COMET_STATIC_ASSERT( sizeof(comet::bstr_t) == sizeof(BSTR) );
    COMET_STATIC_ASSERT( sizeof(comet::bstr_t) == sizeof(BSTR) );
}

namespace std {
    template<> inline void swap( comet::bstr_t& x, comet::bstr_t& y) COMET_STD_SWAP_NOTHROW { x.swap(y); }
}

#include <comet/uuid.h>

#pragma warning(pop)

#endif /* COMET_BSTR_H */
