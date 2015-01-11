/** \file
  * Windows Registry iteration and manipulation functions.
  */
/*
 * Copyright © 2000, 2001 Paul Hollingsworth
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

#ifndef COMET_REGISTRY_H
#define COMET_REGISTRY_H

#include <windows.h>

#include <comet/reference_count.h>
#include <comet/assert.h>
#include <comet/tstring.h>

#ifdef __BORLANDC__
 #define    COMET_ITERATOR_VOID(tag) std::iterator<tag, void, void, void *, void >
#else
 #ifndef __SGI_STL
  #ifdef __MINGW32__
   #define    COMET_ITERATOR_VOID(tag) std::forward_iterator<tag, void>
  #else
   #ifdef _CPPLIB_VER
    #define COMET_ITERATOR_VOID(tag) std::iterator<tag, void, void, void, void>
   #else
    #define COMET_ITERATOR_VOID(tag) std::iterator<tag, void, void>
   #endif
  #endif
 #else
  #define COMET_ITERATOR_VOID(tag) std::iterator<tag, void, void, void, void>
 #endif
#endif

namespace comet {
    /** Contains implementation of registry classes.
      */
    namespace registry {
        /// Unicode compatible string class used in registry operations.
//        typedef std::basic_string<TCHAR> tstring;

        namespace impl {
            // Only when an iterator is dereferenced
            // do we want to go about reading the value
            // from the registry. This requires us to create
            // a temporary object. But in order to overload
            // operator->, our temporary object must overload
            // operator-> - and often it doesn't.
            // Proxy is here to effectively overload operator-> for the temporary
            // object.
            template<typename T>
                class proxy
            {
                T t_;
            public:
                proxy(const T &t) : t_(t) {}
                T *operator->()
                {
                    return &t_;
                }

                const T *operator->() const
                {
                    return &t_;
                }
            };

            class key_base
            {
                HKEY key_;
                mutable reference_count rc_;

                static bool valid_key_(HKEY key) {
                    return key != 0;
                }

                void close_() {
                    if(valid_key_(key_) && (--rc_ == 0)) {
                        LONG errcode = ::RegCloseKey(key_);
                        COMET_ASSERT(ERROR_SUCCESS == errcode);
                        /* C4189 */ errcode;
                    }
                    key_ = 0; // invalidate key handle
                }
            public:
                key_base(const key_base &rhs)
                    : key_(rhs.key_) {
                    if(valid_key_(key_))
                        ++rhs.rc_;
                    rc_ = rhs.rc_;
                }

                key_base(HKEY key = 0) : key_(key) {
                }

                ~key_base() throw() {
                    close_();
                }

                key_base &operator=(const key_base &rhs)
                {
                    key_base tmp(rhs);
                    swap(tmp);
                    return *this;
                }

                void swap(key_base &rhs) throw() {
                    std::swap(key_, rhs.key_);
                    std::swap(rc_, rhs.rc_);
                }

                LONG open(const tstring &subkey,
                    REGSAM sam_desired,
                    HKEY *childkey) const {
                    *childkey = 0;
                    return ::RegOpenKeyEx(key_,
                        subkey.c_str(),
                        0,
                        sam_desired,
                        childkey);
                }

                HKEY open_nothrow(const tstring &subkey,
                    REGSAM sam_desired,
                    LONG *errcode) const {
                    HKEY childkey_ = 0;
                    LONG errcode_ = open(subkey,
                        sam_desired,
                        &childkey_);
                    if(errcode) *errcode = errcode_;
                    return childkey_;
                }

                LONG create(const tstring &subkey,
                    DWORD options,
                    REGSAM sam_desired,
                    LPSECURITY_ATTRIBUTES security_attributes,
                    LPDWORD disposition,
                    HKEY *childkey) const {
                    return ::RegCreateKeyEx(key_,
                        subkey.c_str(),
                        0,
                        REG_NONE,
                        options,
                        sam_desired,
                        security_attributes,
                        childkey,
                        disposition);
                }

                HKEY create_nothrow(const tstring &subkey,
                    DWORD options,
                    REGSAM sam_desired,
                    LPSECURITY_ATTRIBUTES security_attributes,
                    LPDWORD disposition,
                    LONG *errcode) const {
                    HKEY childkey = 0;
                    LONG errcode_ = create(subkey,
                        options,
                        sam_desired,
                        security_attributes,
                        disposition,
                        &childkey);
                    if(errcode) *errcode = errcode_;
                    return childkey;
                }

                LONG flush_nothrow() const {
                    return ::RegFlushKey(key_);
                }

                LONG delete_subkey_nothrow(const tstring &subkey) const {
                    return ::RegDeleteKey(key_, subkey.c_str());
                }

                LONG delete_value_nothrow(const tstring &value_name) const {
                    return ::RegDeleteValue(key_, value_name.c_str());
                }

                void close() {
                    close_();
                }

                HKEY get() const {
                    return key_;
                }

                bool is_valid() const throw()
                {
                    return key_ ? true : false;
                }

                operator const void *() const throw()
                {
                    return is_valid() ? this : 0;
                }
            }; // class key_base

            // Enumerating through keys or values is
            // very similar. These two class encapsulate
            // all that is different between the two algorithms.
            struct next_value
            {
                static LONG perform(HKEY key,
                    DWORD dwIndex,
                    LPTSTR lpName,
                    LPDWORD lpcName)
                {
                    return ::RegEnumValue(key, dwIndex, lpName, lpcName, 0, 0, 0, 0);
                }
            };

            struct next_key
            {
                static LONG perform(HKEY key,
                    DWORD dwIndex,
                    LPTSTR lpName,
                    LPDWORD lpcName)
                {
                    return ::RegEnumKeyEx(key, dwIndex, lpName, lpcName, 0, 0, 0, 0);
                }
            };
        } // namespace impl


        /** \struct name_iterator  registry.h comet/registry.h
          *  Iterates through a list of names. The names might be either key
          *  names or value names, depending on get_next.
          */
        template<class error_policy, class get_next>
            struct name_iterator : public COMET_ITERATOR_VOID(std::forward_iterator_tag)
        {
            // key_ is the only assignment that can fail. So
            // we make it the first member, so that
            // default assignment is exception safe.
            impl::key_base key_;
            DWORD index_;
            DWORD buf_size_;
            DWORD num_values_;

            static void check_exception_(LONG errcode)
            {
                if(ERROR_SUCCESS != errcode)
                    error_policy::on_error(errcode);
            }

            // Is this iterator an end iterator?
            bool is_end() const
            {
                return key_ ? false : true;
            }

            // Make this iterator an end
            // iterator
            void make_end()
            {
                key_.close();
            }

        public:
            typedef tstring value_type;
            name_iterator(const impl::key_base &key,
                DWORD num_values,
                DWORD buf_size)
                : key_(key),
                num_values_(num_values),
                buf_size_(buf_size),
                index_(0)
            {
                if(0 == num_values)
                    make_end();
            }

            void swap(name_iterator &rhs)
            {
                key_.swap(rhs.key_);
                std::swap(index_, rhs.index_);
                std::swap(buf_size_, rhs.buf_size_);
                std::swap(num_values_, rhs.num_values_);
            }

            name_iterator &operator=(const name_iterator &rhs)
            {
                key_ = rhs.key_;
                index_ = rhs.index_;
                buf_size_ = rhs.buf_size_;
                num_values_ = rhs.num_values_;
                return *this;
            }

            impl::key_base key() const
            {
                return key_;
            }

            name_iterator()
            {
                make_end();
            }

            value_type operator*() const
            {
                tstring::value_type *buf = static_cast<tstring::value_type *>(_alloca(buf_size_));
                DWORD dummy = buf_size_;
                check_exception_(get_next::perform(key_.get(),
                    index_,
                    buf,
                    &dummy));
                return buf;
            }

            impl::proxy<value_type> operator->() const
            {
                return **this;
            }

            name_iterator &operator++()
            {
                ++index_;
                if(index_ == num_values_)
                    make_end();
                return *this;
            }

            name_iterator operator++(int) const
            {
                name_iterator retval = *this;
                ++(*this);
                return retval;
            }

            bool operator==(const name_iterator &rhs) const
            {
                if(is_end()) return rhs.is_end();
                if(rhs.is_end()) return is_end();
                return index_ == rhs.index_;
            }

            bool operator!=(const name_iterator &rhs) const
            {
                return !(*this == rhs);
            }
        };

        /** \class value registry.h comet/registry.h
          * A pseudo-reference to a value in the registry.
          * Assign to instances of this object to make changes to the corresponding registry value
          * Read from this object to read values from the registry.
          */
        template<class error_policy>
        class value
        {
            impl::key_base key_;
            tstring value_name_;
            static void check_exception_(LONG errcode)
            {
                if(ERROR_SUCCESS != errcode)
                    error_policy::on_error(errcode);
            }
            static void type_mismatch()
            {
                error_policy::on_typemismatch();
            }

            LONG get_value_(DWORD *type,
                BYTE *buffer,
                DWORD *number_bytes) const
            {
                return ::RegQueryValueEx(key_.get(),
                    value_name_.c_str(),
                    0,
                    type,
                    buffer,
                    number_bytes);
            }

            void get_value(DWORD *type,
                BYTE *buffer,
                DWORD *number_bytes) const {
                check_exception_(
                    get_value_(type,
                    buffer,
                    number_bytes)
                    );
            }

            LONG set_value_(DWORD type,
                const BYTE *data,
                DWORD number_bytes)
            {
                return ::RegSetValueEx(key_.get(),
                    value_name_.c_str(),
                    0,
                    type,
                    data,
                    number_bytes);
            }

            void set_value(DWORD type,
                const BYTE *data,
                DWORD number_bytes)
            {
                check_exception_(
                    set_value_(type,
                    data,
                    number_bytes));
            }

            tstring name() const
            {
                return value_name_;
            }
        public:

            //! This can be used to query if a value exists.
            /*!
            For example:
            \code
            string get_thread_model(const regkey &clsid_key)
            {
                regkey::value_type t = clsid_key.open("InprocServer32", KEY_READ)["ThreadingModel"];
                if(t.exists())
                    return t.str();
                else
                    return "Single";
            }
            \endcode
            */
            bool exists() const
            {
                return ERROR_SUCCESS == get_value_(0, 0, 0);
            }

            value(const impl::key_base &key,
                const tstring &value_name)
                : key_(key),
                value_name_(value_name)
            {
            }

            //! Non throwing swap
            /*! This is for efficiency only.
            operator= is overloaded to have a different
            meaning (copying one part of the
            registry to another part of the registry).
            */
            void swap(value &rhs)
            {
                key_.swap(rhs.key_);
                value_name_.swap(rhs.value_name_);
            }


            //! Get a value of any type. The arguments are passed directly to RegQueryValueEx
            /*!
            \param type Pointer to the type - can be 0
            \param buffer Pointer to a buffer - can be 0
            \param number_bytes Indicates size of the buffer - can be 0 if buffer is 0
            */
            void get(DWORD *type,
                BYTE *buffer,
                DWORD *number_bytes) const
            {
                get_value(type,
                    buffer,
                    number_bytes);
            }

            //! Get a value - return errcode
            /*!
            \param type Pointer to the type - can be 0
            \param buffer Pointer to a buffer - can be 0
            \param number_bytes Indicates size of the buffer - can be 0 if buffer is 0
            */
            LONG get_nothrow(DWORD *type,
                BYTE *buffer,
                DWORD *number_bytes) const
            {
                return get_value_(type, buffer, number_bytes);
            }

            //! Set a value arbitrarily. The arguments are passed directly to RegSetValueEx
            /*!
            \param type Type to set it to (e.g. REG_SZ)
            \param buffer Pointer to a buffer
            \param number_bytes Indicates size of the buffer
            */
            void set(DWORD type,
                const BYTE *buffer,
                DWORD number_bytes)
            {
                set_value(type,
                    buffer,
                    number_bytes);
            }

            //! Set a value - return errcode
            /*!
            \param type Type to set it to (e.g. REG_SZ)
            \param buffer Pointer to a buffer
            \param number_bytes Indicates size of the buffer
            */
            LONG set_nothrow(DWORD type,
                const BYTE *buffer,
                DWORD number_bytes)
            {
                return set_value_(type, buffer, number_bytes);
            }

            //! Interpret value as a string
            /*!
                \exception com_error If the type is not REG_SZ or REG_EXPAND_SZ (using standard error_policy)
            */
            tstring str() const
            {
                DWORD number_bytes = 0;
                DWORD type;
                get_value(&type,
                    0,
                    &number_bytes);
                if( (REG_SZ != type) && (REG_EXPAND_SZ != type))
                    type_mismatch();
                BYTE *buffer = static_cast<BYTE *>(_alloca(number_bytes));
                get_value(0,
                    buffer,
                    &number_bytes);
                return tstring(reinterpret_cast<const tstring::value_type *>(buffer),
                    (number_bytes/sizeof(tstring::value_type)) - 1);
            }

            tstring str(const tstring& default_val) const
            {
                DWORD number_bytes = 0;
                DWORD type;
                if (get_value_(&type,
                    0,
                    &number_bytes) != ERROR_SUCCESS) return default_val;
                if( (REG_SZ != type) && (REG_EXPAND_SZ != type))
                    type_mismatch();
                BYTE *buffer = static_cast<BYTE *>(_alloca(number_bytes));
                get_value(0,
                    buffer,
                    &number_bytes);
                return tstring(reinterpret_cast<const tstring::value_type *>(buffer),
                    (number_bytes/sizeof(tstring::value_type)) - 1);
            }

            //! Implicit conversion to string.
            operator tstring() const
            {
                return str();
            }

            //! Implicit conversion to unsigned int
            operator DWORD() const
            {
                return dword();
            }

            //! Interpret value as a DWORD
            /*!
                \exception com_error If the type is not REG_DWORD or REG_DWORD_LITTLE_ENDIAN (using standard error_policy)
            */
            DWORD dword() const
            {
                DWORD number_bytes = sizeof( DWORD );
                DWORD type;
                DWORD retval;
                get_value(&type,
                    reinterpret_cast<BYTE *>(&retval),
                    &number_bytes);
                if( (REG_DWORD != type) && (REG_DWORD_LITTLE_ENDIAN != type) )
                    type_mismatch();
                return retval;
            }

            DWORD dword(DWORD default_val) const
            {
                DWORD number_bytes = sizeof( DWORD );
                DWORD type;
                DWORD retval;
                if (get_value_(&type,
                    reinterpret_cast<BYTE *>(&retval),
                    &number_bytes) != ERROR_SUCCESS) return default_val;
                if( (REG_DWORD != type) && (REG_DWORD_LITTLE_ENDIAN != type) )
                    type_mismatch();
                return retval;
            }

            // These two operators are currently useless
            // because VC++ 6.0 appears to have a bug -
            // it claims that no conversion to pair<string,string>
            // exists even though it clearly does here.
            // However, it's possible these would be
            // useful on other compilers (or future compilers).
            operator std::pair<tstring, tstring>() const
            {
                return std::make_pair(name(), str());
            }

            operator std::pair<tstring, DWORD>() const
            {
                return std::make_pair(name(), dword());
            }

            //! Assign a string value and set the type to REG_SZ
            value &operator=(const tstring &rhs)
            {
                set_value(REG_SZ,
                    reinterpret_cast<const BYTE *>(rhs.c_str()),
                    (rhs.length() + 1) * sizeof ( tstring::value_type));
                return *this;
            }

            //! Assign a DWORD value and set the type to REG_DWORD
            value &operator=(const DWORD &rhs)
            {
                set_value(REG_DWORD,
                    reinterpret_cast<const BYTE *>(&rhs),
                    sizeof (DWORD) );
                return *this;
            }

            //! Assign an integer value - sets type to REG_DWORD
            value &operator=(int rhs)
            {
                return *this = DWORD(rhs);
            }

            //! Assign value from another registry value
            /*!
            Because value objects always refer to a part of the registry,
            this effectively copies a registry value
            from somewhere else in the registry.
            */
            value &operator=(const value &rhs)
            {
                DWORD type;
                DWORD size;
                rhs.get_value(&type,
                    0,
                    &size);
                BYTE *buffer = static_cast<BYTE *>(_alloca(size));
                rhs.get_value_(0,
                    buffer,
                    &size);
                set_value(type,
                    buffer,
                    size);
                return *this;
            }
        }; // class value

        /** \class const_value_iterator  registry.h comet/registry.h
          * Forward const iterator over registry values.
          */
        template<typename error_policy>
            class const_value_iterator : public COMET_ITERATOR_VOID(std::forward_iterator_tag)
        {
            typedef const value<error_policy> second_type;
            typedef std::pair<const tstring, second_type > value_type_;

            value_type_ get_value() const
            {
                tstring name = *index_;
                return std::make_pair(name, second_type(index_.key(), name) );
            }
            protected:
            name_iterator<error_policy, impl::next_value> index_;
            public:
            typedef value_type_ value_type;

            const_value_iterator(const impl::key_base &key,
                DWORD num_values,
                DWORD buf_size)
                : index_(key, num_values, buf_size)
            {
            }

            const_value_iterator() {}

            void swap(const_value_iterator &rhs)
            {
                index_.swap(rhs.index_);
            }

            value_type operator*() const
            {
                return get_value();
            }

            const_value_iterator &operator++()
            {
                ++index_;
                return *this;
            }

            const_value_iterator operator++(int)
            {
                const_value_iterator retval(*this);
                ++(*this);
                return retval;
            }

            impl::proxy<value_type> operator->()
            {
                return **this;
            }

            bool operator==(const const_value_iterator &rhs) const
            {
                return index_ == rhs.index_;
            }

            bool operator!=(const const_value_iterator &rhs) const
            {
                return index_ != rhs.index_;
            }
        };

        /** \class value_iterator  registry.h comet/registry.h
          * Forward iterator over registry values.
          */
        template<typename error_policy>
            class value_iterator : public const_value_iterator<error_policy>
        {
            typedef value<error_policy> second_type;
            typedef std::pair<const tstring, second_type> value_type_;
            typedef const_value_iterator<error_policy> base;
            value_type_ get_value() const
            {
                tstring name = *this->index_;
                return std::make_pair(name, second_type(this->index_.key(), name) );
            }

            public:
            typedef value_type_ value_type;
            value_iterator(const impl::key_base &key,
                DWORD num_values,
                DWORD buf_size)
                : base(key, num_values, buf_size) {}

            value_iterator() {}

            value_type operator*() const
            {
                return get_value();
            }

            value_iterator &operator++()
            {
                base::operator++();
                return *this;
            }

            value_iterator operator++(int)
            {
                value_iterator retval(*this);
                ++(*this);
                return retval;
            }

            impl::proxy<value_type> operator->()
            {
                return get_value();
            }

            bool operator==(const value_iterator &rhs) const
            {
                return this->index_ == rhs.index_;
            }

            bool operator!=(const value_iterator &rhs) const
            {
                return this->index_ != rhs.index_;
            }
        };

        /** \struct collection registry.h comet/registry.h
          * STL style container class for various types of registry based
          * aggregations.
          */
        template<typename error_policy,
            typename iterator_,
            typename const_iterator_ = iterator_>
        struct collection
        {
            impl::key_base key_;
            DWORD num_values_;
            DWORD buf_size_;

            public:
            collection(const impl::key_base &key,
                DWORD num_values,
                DWORD buf_size)
                : key_(key),
                num_values_(num_values),
                buf_size_(buf_size)
            {
            }

            typedef iterator_ iterator;
            typedef const_iterator_ const_iterator;
            typedef typename iterator_::value_type value_type;
            typedef size_t size_type;

            //! Number of elements in the collection
            size_type size() const { return num_values_; }

            //! Exception safe swap
            void swap(collection &rhs)
            {
                key_.swap(rhs.key_);
                std::swap(num_values_, rhs.num_values_);
                std::swap(buf_size_, rhs.buf_size_);
            }

            //! Get the first iterator
            iterator begin()
            {
                return iterator(key_, num_values_, buf_size_);
            }

            //! Signature iterator marking the end of the sequence
            iterator end()
            {
                return iterator();
            }

            const_iterator begin() const
            {
                return const_iterator(key_, num_values_, buf_size_);
            }

            const_iterator end() const
            {
                return const_iterator();
            }
        };

        /** \class info registry.h comet/registry.h
          * Structure returned by regkey.enumerate()
          */
        template<typename error_policy>
        class info
        {
            // Make key_ the first member - this
            // ensures exception safe assignment.
            impl::key_base key_;

            // Number of values
            DWORD num_values_;

            // Maximum length of a value name
            DWORD value_name_tchars_;

            // Number of sub keys
            DWORD num_subkeys_;

            // Maximum length of a sub key name
            DWORD subkey_name_tchars_;

            static void check_exception_(LONG errcode)
            {
                if(ERROR_SUCCESS != errcode)
                    error_policy::on_error(errcode);
            }

            static DWORD bytes(DWORD tchars)
            {
                return (tchars + 1) * sizeof( tstring::value_type);
            }
        public:
            info(const impl::key_base &key)
                : key_(key)
            {
                check_exception_(::RegQueryInfoKey(key_.get(),
                    0, // lpClass - reserved
                    0, // lpcClass - reserved
                    0, // lpReserved
                    &num_subkeys_,
                    &subkey_name_tchars_,
                    0, // lpcMaxClassLen - I think this is also reserved
                    &num_values_,
                    &value_name_tchars_,
                    0, // lpcMaxValueLen - not necessary for us
                    0, // lpcbSecurityDescriptor
                    0)); // lpftLastWriteTime
            }

            //! Exception safe swap
            void swap(info &rhs)
            {
                key_.swap(rhs.key_);
                std::swap(num_subkeys_, rhs.num_subkeys_);
                std::swap(subkey_name_tchars_, rhs.subkey_name_tchars_);
                std::swap(value_name_tchars_, rhs.value_name_tchars_);
                std::swap(num_values_, rhs.num_values_);
            }

            //! Type returned by values()
            typedef collection<error_policy, value_iterator<error_policy>, const_value_iterator<error_policy> > values_type;

            //! Type returned by value_names()
            typedef collection<error_policy, name_iterator<error_policy, impl::next_value> > value_names_type;

            //! Type returned by subkeys()
            typedef collection<error_policy, name_iterator<error_policy, impl::next_key> > subkeys_type;

            //! Number of values under this key (excluding the default value)
            size_t num_values() const
            {
                return num_values_;
            }

            //! Number of subkeys under this key
            size_t num_subkeys() const
            {
                return num_subkeys_;
            }

            //! Length of the longest value_name under this key (in TCHARs)
            size_t max_value_name() const
            {
                return value_name_tchars_;
            }

            //! Length of the longest subkey name under this key (in TCHARs)
            size_t max_subkey_name() const
            {
                return subkey_name_tchars_;
            }

            //! Return the collection of values
            /*!
            The value_type of the collection is std::pair<const std::basic_string<TCHAR>, regkey::mapped_type>
            regkey::mapped_type has implicit conversions to unsigned int and std::basic_string however.
            Example - copy all value-name/value pairs into a map
            \code
            typedef std::basic_string<TCHAR> tstring;
            regkey::info_type info(regkey(HKEY_LOCAL_MACHINE).enumerate());
            map<tstring, regkey::mapped_type> values_map;
            copy(info.values().begin(), info.values().end(), inserter(values_map, values_map.end()));
            \endcode
            Example - copy all value-name/value pairs into a string map - exception will
            be thrown if a non string value is encountered.
            \code
            typedef std::basic_string<TCHAR> tstring;
            regkey::info_type info(regkey(HKEY_LOCAL_MACHINE).enumerate());
            map<bstr_t, bstr_t> values_map;
            copy(info.values().begin(), info.values().end(), inserter(values_map, values_map.end()));
            \endcode
            Example - copy all value-name/value pairs into a string/int map - exception will
            be thrown if a non string value is encountered.
            \code
            typedef std::basic_string<TCHAR> tstring;
            regkey::info_type info(regkey(HKEY_LOCAL_MACHINE).enumerate());
            map<bstr_t, int> values_map;
            copy(info.values().begin(), info.values().end(), inserter(values_map, values_map.end()));
            \endcode
            */
            values_type values() const
            {
                return values_type(key_, num_values_, bytes(value_name_tchars_));
            }

            //! Returns the collection of value names
            /*! The value_type of the collection is std::basic_string<TCHAR>.
            Example - copy all value names of HKEY_LOCAL_MACHINE into a list
            \code
            regkey::info_type info(regkey(HKEY_LOCAL_MACHINE).enumerate());
            vector<string> value_names;
            copy(info.value_names().begin(), info.value_names().end(), back_inserter(value_names));
            \endcode
            */
            value_names_type value_names() const
            {
                return value_names_type(key_, num_values_, bytes(value_name_tchars_));
            }

            //! Returns the collection of subkey names
            /*! The value_type of the collection is std::basic_string<TCHAR>.
            Example - copy all subkeys of HKEY_LOCAL_MACHINE into a list
            \code
            regkey::info_type info(regkey(HKEY_LOCAL_MACHINE).enumerate());
            vector<string> subkey_names;
            copy(info.subkeys().begin(), info.subkeys().end(), back_inserter(subkey_names));
            \endcode
            */
            subkeys_type subkeys() const
            {
                return subkeys_type(key_, num_subkeys_, bytes(subkey_name_tchars_));
            }
        };

#ifdef __BORLANDC__
        using impl::key_base;
#endif

        /** \class key  registry.h comet/registry.h
          * Registry key wrapper.
           Because an HKEY cannot be duplicated in a platform independent way,
           reference counting is used to enable copying of key objects.

           Methods with the _nothrow suffix do not throw exceptions other than
           std::bad_alloc.

           Key instances can be used as output iterators for std::pair
           assignments. The second argument of each pair must be assignable to
           an instance of regkey::mapped_type. Currently, this means that the
           second member of each pair must be convertable to either an int,
           unsigned int, or std::basic_string<TCHAR>.
        Example:
           \code
           regkey the_key = regkey(HKEY_LOCAL_MACHINE).open("Software\\Comet");
           map<string,string> names;
           names["one"] = "one";
           names["two"] = "two";
           copy(names.begin(), names.end(), the_key);
           map<string,int> values;
           values["three"] = 3;
           values["four"] = 4;
           copy(values.begin(), values.end(), the_key);
           \endcode
        */
        template<class error_policy>
            class key : private impl::key_base {
        private:
            static void check_exception_(LONG errcode)
            {
                if(ERROR_SUCCESS != errcode)
                    error_policy::on_error(errcode);
            }
        public:
            key(HKEY key_handle = 0) : key_base(key_handle) {}

            //! Copy a key.
            /*! In reality this
            only increments a reference count.
            We have to use reference counting
            because ::DuplicateHandle does not
            work for registry keys on windows 95/98.
            */

            key(const key &rhs)
                : key_base(rhs) {
            }
            //! Copy a key.
            /*! This is useful for attaching to keys from a name iterator.
             */
            key( const impl::key_base &rhs)
                : key_base(rhs) {
            }

            void swap(key &rhs)
            {
                key_base::swap(rhs);
            }

            //! Operator overload to allow you to put key instances in a conditional.
            /*! This operator allows you to write code like this:
            \code
            if(subkey = key(HKEY_LOCAL_MACHINE).open_nothrow(_T("Software")))
            {
               ...
            };
            \endcode
            const void * is used instead of the more obvious "bool" to disable
            implicit conversions to int and the like.
            Example:
            \code
            int success = key(HKEY_LOCAL_MACHINE).open_nothrow(_T("Software")); // Won't compile fortunately!!
            \endcode
            */
            operator const void *() const throw()
            {
                return is_valid() ? this : 0;
            }


            //! Open a subkey
            key open(const tstring &subkey,
                REGSAM sam_desired = KEY_ALL_ACCESS) const {
                HKEY childkey_ = 0;
                check_exception_(
                    key_base::open(subkey,
                    sam_desired,
                    &childkey_)
                    );
                return childkey_;
            }

            //! Open a subkey, no exceptions
            key open_nothrow(const tstring &subkey,
                REGSAM sam_desired = KEY_ALL_ACCESS,
                LONG *errcode = 0) const {
                return key_base::open_nothrow(subkey,
                    sam_desired, errcode);
            }

            //! Create a subkey
            key create(const tstring &subkey,
                DWORD options = REG_OPTION_NON_VOLATILE,
                REGSAM sam_desired = KEY_ALL_ACCESS,
                LPSECURITY_ATTRIBUTES security_attributes = 0,
                LPDWORD disposition = 0) const {
                HKEY childkey_ = 0;
                check_exception_(
                    key_base::create(subkey,
                    options,
                    sam_desired,
                    security_attributes,
                    disposition,
                    &childkey_)
                    );
                return childkey_;
            }

            //! Create a subkey - no exceptions
            key create_nothrow(const tstring &subkey,
                DWORD options = REG_OPTION_NON_VOLATILE,
                REGSAM sam_desired = KEY_ALL_ACCESS,
                LPSECURITY_ATTRIBUTES security_attributes = 0,
                LPDWORD disposition = 0,
                LONG *errcode = 0) const {
                return key_base::create_nothrow(subkey,
                    options,
                    sam_desired,
                    security_attributes,
                    disposition,
                    errcode);
            }

            //! Call RegFlushKey with the contained key
            void flush() const {
                check_exception_(
                    flush_nothrow()
                    );
            }

            //! Call RegFlushKey with the contained key - no exceptions
            LONG flush_nothrow() const {
                return key_base::flush_nothrow();
            }

            //! Delete the subkey
            /*! Warning - the behaviour is different
            between WinNT and Win95/98 if
            sub keys are present.
            See documentation for ::RegDeleteKey for
            more information.
            */
            void delete_subkey(const tstring &subkey) const {
                check_exception_(
                    delete_subkey_nothrow(subkey.c_str())
                    );
            }

            //! Delete the subkey
            /*! Warning - the behaviour is different
            between WinNT and Win95/98 if
            sub keys are present.
            See documentation for ::RegDeleteKey for
            more information.
            */
            LONG delete_subkey_nothrow(const tstring &subkey) const {
                return key_base::delete_subkey_nothrow(subkey);
            }

            //! delete a value
            void delete_value(const tstring &value_name) const {
                check_exception_(
                    delete_value_nothrow(value_name)
                    );
            }

            //! delete a value - no exceptions
            LONG delete_value_nothrow(const tstring &value_name) const {
                return key_base::delete_value_nothrow(value_name);
            }

            //! Release reference to the key.
            /*! Note that this will only call ::RegCloseKey
            if this was the last reference to the outstanding key.
            This method is implicitly called by the destructor.
            */
            void close() {
                key_base::close();
            }

            //! Get access to the raw key without releasing ownership
            HKEY get() const {
                return key_base::get();
            }

            typedef value<error_policy> mapped_type;

            // All of these methods are const because I figured
            // it was more useful. It allows you to create
            // temporary regkey objects for the purposes of
            // updating.

            //! Get a reference to a value in the registry.
            /*! The returned value can be used on both sides of an assignment. Example:
            \code
            key.get_value("Name") = "Paul";
            string name = key.get_value("Name");
            cout << key.get_value("Name").str() << endl;
            \endcode
            */
            mapped_type get_value(const tstring &value_name = _T("")) const
            {
                return mapped_type(*this, value_name);
            }

            //! Subscript operator overload - syntactic sugar for get_value().
            /*! Example:
                \code
                key["Name"] = "Paul";
                string name = key["Name"];
                cout << key["Name"].str() << endl;
                \endcode
            */
            mapped_type operator[](const tstring &value_name) const
            {
                return get_value(value_name);
            }

            //! Type returned by enumerate()
            typedef info<error_policy> info_type;
            //! Type returned by enumerate().subkeys()
            typedef typename info_type::subkeys_type subkeys_type;
            //! Type returned by enumerate().values()
            typedef typename info_type::values_type values_type;
            //! Type returned by enumerate().value_names()
            typedef typename info_type::value_names_type value_names_type;

            //! Enumerate the subkeys, values or value_names, or obtain other information about the key. See also info.
            /*!
            The enumerate() method effectively calls RegQueryInfoKey, so you should
            minimize the number of calls to enumerate() if efficiency is
            a concern. e.g.
            The following
            \code
            regkey::info_type info = key.enumerate();
            copy(info.values().begin(), info.values().end(), inserter(value_map, value_map.end()));
            \endcode
            is more efficient than
            \code
            copy(key.enumerate().values().begin(), key.enumerate().values().end(), inserter(value_map, value_map.end()));
            \endcode
            The second version will end up calling RegQueryInfoKey twice.
            */
            info_type enumerate() const
            {
                return info_type(*this);
            }

            //! Part of making key into an output iterator
            key &operator*()
            {
                return *this;
            }

            //! Assignment. This is designed to work with 'std::pair's - the value type of map classes
            template<typename T>
                key &operator=(const T &val)
            {
                get_value(val.first) = val.second;
                return *this;
            }

            //! Exception safe assignment operator.
            //! Can still throw std::bad_alloc due
            //! to reference counting.
            //template<>
            key &operator=(const key &rhs)
            {
                key_base::operator=(rhs);
                return *this;
            }

            //! Noop increment
            key &operator++() { return *this; }
            //! Noop decrement
            key &operator++(int) { return *this; }
        }; // class key
    } // namespace registry
} // namespace comet

#endif // COMET_REGISTRY_H
