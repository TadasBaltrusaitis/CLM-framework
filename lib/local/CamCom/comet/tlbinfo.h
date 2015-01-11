/** \file
  * Wrappers for ITypeLibInfo.
  */
/*
 * Copyright © 2002 Michael Geddes.
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

#ifndef COMET_TLIBINFO_H
#define COMET_TLIBINFO_H

#ifdef _SHOW_INC
#pragma message("   #Include " __FILE__)
#endif

#include <oaidl.h>
#include <comet/ptr.h>
#include <comet/uuid.h>

namespace comet
{
    template<> struct comtype<ITypeLib>
    {
        static const IID& uuid() throw() { return IID_IPictureDisp; }
        typedef ::IUnknown base;
    };
    template<> struct comtype<ITypeInfo>
    {
        static const IID& uuid() throw() { return IID_ITypeInfo; }
        typedef ::IUnknown base;
    };
    template<> struct comtype<ITypeInfo2>
    {
        static const IID& uuid() throw() { return IID_ITypeInfo2; }
        typedef ::ITypeInfo base;
    };
    /*! \addtogroup Interfaces
     */
    //@{
    namespace impl
    {
        /// A class to handle the auto-releaseing of structs returned by by the Typeinfo interfaces.
        template< typename B, typename T>
        struct tlib_info
        {
            typedef   void (__stdcall B::*RelFunc)(T*);
            template< RelFunc RELEASE>
            struct typeinfo_attr_base
            {
                public:
                    typeinfo_attr_base() : info_(NULL), attrib_(NULL), rc_(NULL) {}
                    typeinfo_attr_base( const com_ptr<B> &info, T *attrib ) : info_(info),attrib_(attrib),rc_(NULL)
                    {
                        if (attrib != NULL)
                        {
                            rc_ = new long;
                            (*rc_)=1;
                        }
                    }
                    typeinfo_attr_base( const typeinfo_attr_base &base)
                        : info_(base.info_), attrib_(base.attrib_), rc_(base.rc_)
                        {
                            ++*rc_;
                        }
                    ~typeinfo_attr_base()
                    {
                        release();
                    }
                    typeinfo_attr_base &operator =( const typeinfo_attr_base &base)
                    {
                        release();
                        info_= base.info_;
                        attrib_= base.attrib_;
                        rc_ = base.rc_;
                        ++*rc_;
                        return *this;
                    }
                    void release()
                    {
                        if(attrib_ != NULL && rc_ != NULL )
                        {
                            if (0 == --*rc_)
                            {
                                (info_.raw()->*RELEASE)( attrib_);
                                delete rc_;
                            }
                            rc_ = NULL;
                            attrib_ = NULL;
                            info_ = NULL;
                        }
                    }


                    T *operator->() const
                    {
                        if( attrib_ ==NULL ) throw com_error(E_POINTER);
                            return attrib_;
                    }
                    bool is_null() const { return attrib_ == NULL; }
                protected:
                    long *rc_;
                    T *attrib_;
                    com_ptr<B> info_;
            };
        };
    };

    /// Auto-release wrapper for TYPEATTR.
    typedef impl::tlib_info<ITypeInfo, TYPEATTR>::typeinfo_attr_base< &ITypeInfo::ReleaseTypeAttr > type_attr_t;
    /// Auto-release wrapper for FUNCDESC.
    typedef impl::tlib_info<ITypeInfo, FUNCDESC>::typeinfo_attr_base< &ITypeInfo::ReleaseFuncDesc > func_desc_t;
    /// Auto-release wrapper for VARDESC.
    typedef impl::tlib_info<ITypeInfo, VARDESC>::typeinfo_attr_base< &ITypeInfo::ReleaseVarDesc > var_desc_t;

    /// Auto-release wrapper for TLIBATTR.
    typedef impl::tlib_info<ITypeLib, TLIBATTR>::typeinfo_attr_base< &ITypeLib::ReleaseTLibAttr > tlibattr_t;


    /** Flags for GetImplTypeFlags.
     * \sa IMPLTYPEFLAGS
     */
    enum impl_type_flag
    {
        eft_default=0x1,        ///< The interface or dispinterface represents the default for the source or sink.
        eft_source =0x2,        ///< This member of a coclass is called rather than implemented.
        eft_restricted=0x4,     ///< The member should not be displayed or programmable by users.
        eft_defaultvtable=0x8   ///< Sinks receive events through the VTBL.
    };

    /** Specialisation wrapper for ITypeInfo.
     * \struct wrap_t<ITypeInfo> tlbinfo.h comet/tlbinfo.h
     *  Wrapper for ITypeInfo typelib information.
     *  \sa ITypeInfo
     */
    template<>
    struct wrap_t<ITypeInfo>
    {
        private:
            inline ITypeInfo *raw_(){ return reinterpret_cast<ITypeInfo *>(this); }
        public:

            /** Returns a wrapped Type Attributes struct.
             * \sa TYPEATTR
             */
            type_attr_t GetTypeAttr()
            {
                TYPEATTR *pTypeAttr;
                raw_()->GetTypeAttr( &pTypeAttr) | raise_exception ;
                return type_attr_t( raw_(), pTypeAttr);
            }

            /// Returns an ITypeComp interface.
            com_ptr<ITypeComp> GetTypeComp()
            {
                com_ptr<ITypeComp> ppTComp;
                raw_()->GetTypeComp( ppTComp.out()) | raise_exception;
                return ppTComp;
            }

            /** Get the function description struct.
             * \sa FUNCDESC
             */
            func_desc_t GetFuncDesc( unsigned int index)
            {
                FUNCDESC *pFuncDesc ;
                raw_()->GetFuncDesc(index,  &pFuncDesc) | raise_exception ;
                return func_desc_t( raw_(), pFuncDesc);
            }

            /** Get the Variable/Constant/Datamember description.
             *  \sa VARDESC
             */
            var_desc_t GetVarDesc(  unsigned int  index )
            {
                VARDESC *pVarDesc;
                raw_()->GetVarDesc( index,&pVarDesc) | raise_exception;
                return var_desc_t(raw_(), pVarDesc);
            }


#ifdef NOT_YET
            std::list<std::string> GetNames( MEMBERID memid )
            {
                /* [local] */ HRESULT  GetNames( /* [in] */ MEMBERID memid, /* [length_is][size_is][out] */ BSTR *rgBstrNames,
                        /* [in] */ UINT cMaxNames, /* [out] */ UINT *pcNames) ;
            }
#endif  //

            /// Retrieves the type description of implemented interface types.
            HREFTYPE GetRefTypeOfImplType( unsigned int index)
            {
                HREFTYPE reftype;
                raw_()->GetRefTypeOfImplType(index, &reftype) | raise_exception;
                return reftype;
            }

            /// Retrieves the type description of implemented interface types.
            long GetImplTypeFlags( int index)
            {
                INT implTypeFlags;
                raw_()->GetImplTypeFlags( index, &implTypeFlags) |raise_exception;
                return implTypeFlags;
            }

#ifdef NOT_YET
            std::vector<MEMBERID> GetIDsOfNames( const std::vector<bstr_t> &rgsNames);
            {
                /* [local] */ HRESULT  GetIDsOfNames(
                        /* [size_is][in] */ LPOLESTR *rgszNames,
                        /* [in] */ UINT cNames,
                        /* [size_is][out] */ MEMBERID *pMemId) ;
            }

            /* [local] */ HRESULT  Invoke(
                    /* [in] */ PVOID pvInstance,
                    /* [in] */ MEMBERID memid,
                    /* [in] */ WORD wFlags,
                    /* [out][in] */ DISPPARAMS *pDispParams,
                    /* [out] */ VARIANT *pVarResult,
                    /* [out] */ EXCEPINFO *pExcepInfo,
                    /* [out] */ UINT *puArgErr) ;
#endif // NOT_YET
            /// Raw wrapper to make sure comet still works/compiles.
            HRESULT  GetIDsOfNames( LPOLESTR *rgszNames, UINT cNames,
                    MEMBERID *pMemId)
            {
                return raw_()->GetIDsOfNames( rgszNames, cNames, pMemId);
            }

            /// Raw wrapper to make sure comet still works/compiles.
            HRESULT  Invoke( PVOID pvInstance, MEMBERID memid, WORD wFlags,
                    DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo,
                    UINT *puArgErr)
            {
                return  raw_()->Invoke( pvInstance, memid, wFlags, pDispParams, pVarResult, pExcepInfo,    puArgErr);
            }

            bool GetIDOfName( const wchar_t *name, MEMBERID *id)
            {
                wchar_t *ucname = const_cast<wchar_t *>(name);
                return SUCCEEDED(raw_()->GetIDsOfNames( &ucname, 1, id));
            }


            /// Get the documentation for the specified MEMBERID.
            void GetDocumentation(MEMBERID memid, bstr_t *name, bstr_t *docString, DWORD *helpcontext, bstr_t *helpfile)
            {
                raw_()->GetDocumentation(memid, name?name->out():NULL, docString?docString->out():NULL,
                        helpcontext, helpfile? helpfile->out():NULL) | raise_exception;
            }
            /** Get the member name for the specified MEMBERID.
             * This is a shortcut for GetDocumentation.
             */

            bstr_t GetMemberName(MEMBERID memid)
            {
                bstr_t name;
                raw_()->GetDocumentation(memid, name.out(), NULL, NULL, NULL) | raise_exception;
                return name;
            }
            bstr_t GetName()
            {
                return GetMemberName(-1);
            }

#if NOT_YET
            /* [local] */ HRESULT  GetDllEntry(
                    /* [in] */ MEMBERID memid,
                    /* [in] */ INVOKEKIND invKind,
                    /* [out] */ BSTR *pBstrDllName,
                    /* [out] */ BSTR *pBstrName,
                    /* [out] */ WORD *pwOrdinal) ;
#endif // NOT_YET

            /** Get the referenced Type information.
             */
            com_ptr<ITypeInfo> GetRefTypeInfo( HREFTYPE refType)
            {
                com_ptr<ITypeInfo> refinfo;
                raw_()->GetRefTypeInfo( refType, refinfo.out()) | raise_exception;
                return refinfo;
            }

#if NOT_YET
            /* [local] */ HRESULT  AddressOfMember( /* [in] */ MEMBERID memid, /* [in] */ INVOKEKIND invKind,
                    /* [out] */ PVOID *ppv) ;

            /* [local] */ HRESULT  CreateInstance( /* [in] */ IUnknown *pUnkOuter, /* [in] */ REFIID riid,
                    /* [iid_is][out] */ PVOID *ppvObj) ;

            HRESULT  GetMops( /* [in] */ MEMBERID memid, /* [out] */ BSTR *pBstrMops) ;
#endif // NOT_YET

            /** Get the typelib containing this definition.
             * \return  A pair containing the typelibrary and the index of the type description within the type library.
             */
            std::pair< com_ptr<ITypeLib>, UINT > GetContainingTypeLib()
            {
                std::pair< com_ptr<ITypeLib>, UINT> result;
                raw_()->GetContainingTypeLib( result.first.out(), &(result.second) ) | raise_exception;
                return result;
            }
    };
    //@}

    template<>
    struct wrap_t<ITypeInfo2> : wrap_t<ITypeInfo>
    {
        TYPEKIND GetTypeKind()
        {
            TYPEKIND tkind;
            raw_()->GetTypeKind(&tkind) | raise_exception;
            return tkind;
        }
        private:
        inline ITypeInfo2 *raw_(){ return reinterpret_cast<ITypeInfo2 *>(this); }
    };


    /*! \addtogroup Interfaces
     */
    //@{

    /// Specialisation to handle TypeLibrary API.
    template<>
    struct wrap_t<ITypeLib>
    {
        inline ITypeLib *raw_(){ return reinterpret_cast<ITypeLib *>(this); }
        unsigned int GetTypeInfoCount()
        {
            return raw_()->GetTypeInfoCount();
        }

        /// Get typeinfo at specified index.
        com_ptr<ITypeInfo> GetTypeInfo( UINT index)
        {
            com_ptr<ITypeInfo> tinfo;
            raw_()->GetTypeInfo( index, tinfo.out()) | raise_exception;
            return tinfo;
        }

        /// Get type of information at specified index.
        TYPEKIND GetTypeInfoType( UINT index)
        {
            TYPEKIND retval;
            raw_()->GetTypeInfoType( index, &retval) | raise_exception;
            return retval;
        }

        /// Get typeinfo given GUID.
        com_ptr<ITypeInfo> GetTypeInfoOfGuid( const uuid_t &guid )
        {
            com_ptr<ITypeInfo> tinfo;
            raw_()->GetTypeInfoOfGuid( guid, tinfo.out()) | raise_exception;
            return tinfo;
        }
        /// Get Raw Typeinfo of guid.
        HRESULT GetTypeInfoOfGuid( REFGUID guid, ITypeInfo **ppTinfo)
        {
            return raw_()->GetTypeInfoOfGuid( guid,ppTinfo);
        }

        /// Get attributes of the typelibrary.
        tlibattr_t GetLibAttr()
        {
            TLIBATTR *attr;
            raw_()->GetLibAttr( &attr) | raise_exception;
            return tlibattr_t( raw_(), attr);
        }

        com_ptr<ITypeComp> GetTypeComp()
        {
            com_ptr<ITypeComp> typecomp;
            raw_()->GetTypeComp( typecomp.out() ) | raise_exception;
            return typecomp;
        }

        void GetDocumentation(int index, bstr_t *name, bstr_t *docString, DWORD *helpcontext, bstr_t *helpfile)
        {
            raw_()->GetDocumentation(index, name?name->out():NULL, docString?docString->out():NULL,
                    helpcontext, helpfile? helpfile->out():NULL) | raise_exception;
        }
        bstr_t GetItemName(int index)
        {
            bstr_t name;
            raw_()->GetDocumentation(index, name.out(), NULL, NULL, NULL) | raise_exception;
            return name;
        }
        bstr_t GetName()
        {
            return GetItemName(-1);
        }

        bool IsName( const bstr_t &name, unsigned long hashval )
        {
            BOOL ret;
            raw_()->IsName( name.in(), hashval, &ret) | raise_exception;
            return ret!=0;
        }

        std::pair<com_ptr<ITypeInfo>, MEMBERID> FindName( const bstr_t &name)
        {
            std::pair<com_ptr<ITypeInfo>, MEMBERID> result;
            USHORT tofind = 1;
            raw_()->FindName(name.in(), 0, result.first.out(), &result.second, &tofind) | raise_exception;
            return result;
        }
#ifdef NOT_YET
        std::list<std::pair<com_ptr<ITypeInfo>, MEMBERID> >FindName( const bstr_t &name, int max)
        {}
        [local]
            HRESULT FindName(
                    [in, out] LPOLESTR szNameBuf,
                    [in] ULONG lHashVal,
                    [out,size_is(*pcFound),length_is(*pcFound)] ITypeInfo **ppTInfo,
                    [out,size_is(*pcFound),length_is(*pcFound)] MEMBERID * rgMemId,
                    [in, out] USHORT * pcFound
                    );
#endif // LATER


    };

    /// Load typeinfo from a file.
    namespace typeinfo
    {
        /** Whether to register the typelibrary on load.
         * \relates typeinfo::LoadTypeLib
         */
        enum regkind_t { reg_default = REGKIND_DEFAULT,  reg_register = REGKIND_REGISTER, reg_none = REGKIND_NONE } ;
        /// Load typeinfo from a file.
        static inline com_ptr<ITypeLib> LoadTypeLib( const bstr_t &filename, regkind_t regkind= reg_default)
        {
            com_ptr<ITypeLib> tlib;
            LoadTypeLibEx(filename.in(), (REGKIND)(regkind), tlib.out()) | raise_exception;
            return tlib;
        }
    }
    //@}
} // namespace comet
#endif /* COMET_TLIBINFO_H */
