/** \file
  * uuid wrapper class.
  */
/*
 * Copyright © 2001, 2002 Sofus Mortensen
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

#ifndef COMET_UUID_FWD_H
#define COMET_UUID_FWD_H

#include <comet/config.h>
#include <comet/tstring.h>

#include <rpc.h>
#include <string>
#include <algorithm>
#include <iostream>
#include <ostream>

namespace comet{


/*! \addtogroup COMType
 */
//@{

/// UUID wrapper.
class uuid_t : public COMET_GUID_BASE
{
    struct unitialized_t {};
    uuid_t(unitialized_t) {}

    friend class bstr_t;

public:
    /// Default constructor - constructs a nil uuid.
    uuid_t()
    {
        UuidCreateNil(this);
    }

    /// Copy constructor
    uuid_t(const COMET_GUID_BASE& u)
    {
        memcpy(this, &u, sizeof(COMET_GUID_BASE));
    }

    /// Assignment operator
    uuid_t& operator=(const COMET_GUID_BASE& u)
    {
        memcpy(this, &u, sizeof(COMET_GUID_BASE));
        return *this;
    }

    /// \name Construct uuid from string
    //@{
    explicit uuid_t(const std::string& s)
    {
        if (!init_from_str(s.c_str(), s.length()))
            throw std::runtime_error(err_msg());
    }

    explicit uuid_t(const char* s)
    {
        if (!init_from_str(s, strlen(s)))
            throw std::runtime_error(err_msg());
    }

    explicit uuid_t(const std::wstring& s)
    {
        if (!init_from_str(s.c_str(), s.length()))
            throw std::runtime_error(err_msg());
    }

    explicit uuid_t(const wchar_t* s)
    {
        if (!init_from_str(s, wcslen(s)))
            throw std::runtime_error(err_msg());
    }

    explicit uuid_t(const bstr_t& bs);
    //@}

    //! Generate new guid.
    static uuid_t create()
    {
        unitialized_t x;
        uuid_t g(x);
        UuidCreate(&g);
        return g;
    }

    //! Set to nil uuid.
    void clear()
    {
        UuidCreateNil(this);
    }

    //! Returns hash of uuid
    unsigned short hash() const
    {
        RPC_STATUS status;
        unsigned short r = UuidHash(const_cast<uuid_t*>(this), &status);
        if (status != RPC_S_OK)
            throw std::runtime_error("uuid_t::hash - UuidHash failed.");
        return r;
    }

    //! Convert uuid to string
    //@{
    /// Return a std::string version of the uuid.
    std::string str() const
    {
        std::string r;
        r.resize(36);
        copy_to_str(&r[0]);
        return r;
    }
    /// Return a std::string version of the uuid.
    inline std::string s_str() const
    {
        return str();
    }

    /// Return a std::wstring version of the uuid.
    std::wstring w_str() const
    {
        std::wstring r;
        r.resize(36);
        copy_to_str(&r[0]);
        return r;
    }

    /// Return a tstring (templated to TCHAR) version of the uuid.
    tstring t_str() const
    {
        tstring r;
        r.resize(36);
        copy_to_str(&r[0]);
        return r;
    }
    //@}

    /// Returns true if and only if uuid is nil.
    bool is_null() const throw()
    {
        return ((PLONG) this)[0] == 0 &&
               ((PLONG) this)[1] == 0 &&
               ((PLONG) this)[2] == 0 &&
               ((PLONG) this)[3] == 0;
    }

    //! Returns true if and only if uuid is nil
    bool operator!() const throw()
    {
        return is_null();
    }

    /// Output to an ostream.
    template<class E, class TR>
    friend inline std::basic_ostream<E, TR>& operator<<(std::basic_ostream<E, TR>& os, const uuid_t& u);

    /// Input from an ostream
    template<class E, class TR>
    friend inline std::basic_istream<E, TR>& operator>>(std::basic_istream<E, TR>& is, uuid_t& u);

    /** \internal
     */
    class bool_tester
    { void operator delete(void*); };

    /// Copy the uuid_t to a string/container.
    template<typename C> void copy_to_str(C s[]) const throw();
    /// Initialise the uuid_t from a string/container.
    template<typename C> bool init_from_str(const C s[], size_t len) throw();

    static const int* uuid_table()
    {
        static const int table[] = { 3, 2, 1, 0, -1, 5, 4, -1, 7, 6, -1, 8, 9, -1, 10, 11, 12, 13, 14, 15 };
        return table;
    }

    static const char* hex_table()
    {
        static const char hex[] = "0123456789abcdef";
        return hex;
    }

    static const char* err_msg()
    {
        static const char msg[] = "uuid_t: invalid format";
        return msg;
    }

    static int parse_nibble(int c)
    {
        return c >= 'A' ? (c|0x20) - 'a' + 10 : c - '0';
    }

    /// Compare two guids.
    inline long cmp(const comet::uuid_t &rhs) const
    {
        long i; // The code below may be a bit dense, but it is compact and works.
        if((i = (Data1 - rhs.Data1)) == 0 && (i = (Data2 - rhs.Data2)) == 0 && (i = (Data3 - rhs.Data3)) == 0)
            (i = ::memcmp(Data4, rhs.Data4, 8));
        return i;
    }

    /// Compare two guids for equality.
    inline bool is_equal( const comet::uuid_t &rhs) const
    {
        // From ATL
        return ((PLONG) this)[0] == ((PLONG) &rhs)[0] &&
            ((PLONG) this)[1] == ((PLONG) &rhs)[1] &&
            ((PLONG) this)[2] == ((PLONG) &rhs)[2] &&
            ((PLONG) this)[3] == ((PLONG) &rhs)[3];
    }

public:
    /**\internal
     */
    operator bool_tester*() const throw()
    { if (is_null()) return 0; static bool_tester test;    return &test; }

    /// Construct const reference to uuid from a raw GUID
    static const uuid_t& create_const_reference(const COMET_GUID_BASE& x)
    {
        return *reinterpret_cast<const uuid_t*>(&x);
    }

    /// Construct reference to uuid from a raw GUID
    static uuid_t& create_reference(COMET_GUID_BASE& x)
    {
        return *reinterpret_cast<uuid_t*>(&x);
    }

    //! \name Methods for converting uuid to raw GUID when calling raw COM functions or legacy code.
    //@{
    COMET_GUID_BASE* out() { return this; }
    COMET_GUID_BASE* inout() { return this; }
    COMET_GUID_BASE in() const { return *this; }
    COMET_GUID_BASE* in_ptr() const { return const_cast<COMET_GUID_BASE*>(static_cast<const COMET_GUID_BASE*>(this)); }
    //@}
};
//@}

/// \name comparison operators
//@{
/** Equals operator.
 * \relates uuid_t
 */
inline bool operator==(const comet::uuid_t& lhs, const comet::uuid_t& rhs)
{
    return lhs.is_equal(rhs);
}

/** Inequality operator.
 * \relates uuid_t
 */
inline bool operator!=(const comet::uuid_t& lhs, const comet::uuid_t& rhs)
{
    return !lhs.is_equal(rhs);
}

/** Equals operator.
 * \relates uuid_t
 */
inline bool operator==(const comet::uuid_t& lhs, const COMET_GUID_BASE& rhs)
{
    return lhs.is_equal( comet::uuid_t::create_const_reference(rhs) );
}

/** Inequality operator.
 * \relates uuid_t
 */
inline bool operator!=(const comet::uuid_t& lhs, const COMET_GUID_BASE& rhs)
{
    return !lhs.is_equal( comet::uuid_t::create_const_reference(rhs) );
}

/** Equals operator.
 * \relates uuid_t
 */
inline bool operator==(const COMET_GUID_BASE& lhs, const comet::uuid_t& rhs)
{
    return comet::uuid_t::create_const_reference(lhs).is_equal(rhs);
}

/** Inequality operator.
 * \relates uuid_t
 */
inline bool operator!=(const COMET_GUID_BASE& lhs, const comet::uuid_t& rhs)
{
    return !comet::uuid_t::create_const_reference(lhs).is_equal(rhs);
}

/** Less-than operator.
 * \relates uuid_t
 */
inline bool operator<(const comet::uuid_t& lhs, const comet::uuid_t& rhs)
{
    return lhs.cmp(rhs)<0;
}

/** Less-than operator.
 * \relates uuid_t
 */
inline bool operator<(const comet::uuid_t& lhs, const COMET_GUID_BASE& rhs)
{
    return lhs.cmp(comet::uuid_t::create_const_reference(rhs))<0;
}

/** Less-than operator.
 * \relates uuid_t
 */
inline bool operator<(const COMET_GUID_BASE& lhs, const comet::uuid_t& rhs)
{
    return comet::uuid_t::create_const_reference(lhs).cmp(rhs)<0;
}

/** Greater-than-equals operator.
 * \relates uuid_t
 */
inline bool operator>=(const comet::uuid_t& lhs, const comet::uuid_t& rhs)
{
    return lhs.cmp(rhs)>=0;
}

/** Greater-than-equals operator.
 * \relates uuid_t
 */
inline bool operator>=(const comet::uuid_t& lhs, const COMET_GUID_BASE& rhs)
{
    return lhs.cmp(comet::uuid_t::create_const_reference(rhs))>=0;
}

/** Greater-than-equals operator.
 * \relates uuid_t
 */
inline bool operator>=(const COMET_GUID_BASE& lhs, const comet::uuid_t& rhs)
{
    return comet::uuid_t::create_const_reference(lhs).cmp(rhs)>=0;
}

/** Greater-than operator.
 * \relates uuid_t
 */
inline bool operator>(const comet::uuid_t& lhs, const comet::uuid_t& rhs)
{
    return lhs.cmp(rhs)>0;
}

/** Greater-than operator.
 * \relates uuid_t
 */
inline bool operator>(const comet::uuid_t& lhs, const COMET_GUID_BASE& rhs)
{
    return lhs.cmp(comet::uuid_t::create_const_reference(rhs))>0;
}

/** Greater-than operator.
 * \relates uuid_t
 */
inline bool operator>(const COMET_GUID_BASE& lhs, const comet::uuid_t& rhs)
{
    return comet::uuid_t::create_const_reference(lhs).cmp(rhs)>0;
}

/** Less-than-equals operator.
 * \relates uuid_t
 */
inline bool operator<=(const comet::uuid_t& lhs, const comet::uuid_t& rhs)
{
    return lhs.cmp(rhs)<=0;
}

/** Less-than-equals operator.
 * \relates uuid_t
 */
inline bool operator<=(const comet::uuid_t& lhs, const COMET_GUID_BASE& rhs)
{
    return lhs.cmp(comet::uuid_t::create_const_reference(rhs))<=0;
}

/** Less-than-equals operator.
 * \relates uuid_t
 */
inline bool operator<=(const COMET_GUID_BASE& lhs, const comet::uuid_t& rhs)
{
    return comet::uuid_t::create_const_reference(lhs).cmp(rhs)<=0;
}
//@}

/// \name overloads for output/input to/from streams
/// @{

/** Output to an ostream.
 * \relates uuid_t
 */
template<class E, class TR>
inline std::basic_ostream<E, TR>& operator<<(std::basic_ostream<E, TR>& os, const comet::uuid_t& u)
{
    E buf[36];
    u.copy_to_str(buf);
    os.write(buf, 36);
    return os;
}

/** Input from an istream.
 * \relates uuid_t
 */
template<class E, class TR>
inline std::basic_istream<E, TR>& operator>>(std::basic_istream<E, TR>& is, comet::uuid_t& u)
{
    typename std::basic_istream<E,TR>::sentry se(is);
    if (se)
    {
        E buf[36];
        is.read(buf, 36);
        if (!u.init_from_str(buf, is.gcount()))
            is.setstate(std::ios::badbit);
    }
    return is;
}
//@}

} // namespace comet

#endif
