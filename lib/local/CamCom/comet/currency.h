/** \file
  * Currency wrapper.
  */
/* Copyright © 2001 Michael Geddes, Sofus Mortensen
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

#ifndef COMET_CURRENCY_H
#define COMET_CURRENCY_H

#include <comet/config.h>

#include <comet/error_fwd.h>
#include <comet/assert.h>

#include <wtypes.h>

#include <iostream>
#include <algorithm>

namespace comet
{

    class bstr_t;

    // currency_t
    ///////////////

    /*! \addtogroup COMType
     */
    //@{

    /** Wrapper for CURRENCY type.
      * CURRENCY is a fixed point (to 4 decimal places) 64 bit value.
      */

    class currency_t
    {
        public:
            /// Default Constructor
            currency_t() throw() { cy_.int64 = 0; }

            /// CY constructor.
            currency_t(const CY &cy): cy_(cy) { }

            /// Double Conversion constructor.
            explicit currency_t(double val)
            {
                VarCyFromR8(val,&cy_) | raise_exception;
            }

#if 0
//            //! Construct currency from CY/CURRENCY.
//            /*!
//                Takes ownership of specified CY value. To prevent misuse the CY must be wrapped using auto_attach.
//
//                \code
//                    currency_t cy( auto_attach( myCY ) );
//                \endcode
//
//                \param cyVal
//                    Value to initialise currency_t from.
//            */
//            currency_t( const impl::auto_attach_t<CY> &cyVal) throw() : cy_(cyVal.get())
//            {}
#endif  //0

            /// Long Conversion constructor.
            currency_t( long val )
            {
                VarCyFromI4(val,&cy_) | raise_exception;
            }

            currency_t( int val )
            {
                VarCyFromI4(val,&cy_) | raise_exception;
            }

            void swap(currency_t& c) throw()
            {
                std::swap(cy_, c.cy_);
            }

            static const currency_t& create_const_reference(const CY& s) throw()
            { return *reinterpret_cast<const currency_t*>(&s); }

            static currency_t& create_reference(CY& s) throw()
            { return *reinterpret_cast<currency_t*>(&s); }

            //! \name Assignment Operators
            //@{
            currency_t &operator=(double newVal)
            {
                currency_t c( newVal );
                swap(c);
                return *this;
            }
            currency_t &operator=(long newVal)
            {
                currency_t c( newVal );
                swap(c);
                return *this;
            }
            currency_t &operator=(int newVal)
            {
                currency_t c( newVal );
                swap(c);
                return *this;
            }
            currency_t &operator=(const tagCY &newVal) throw()
            {
                cy_ = newVal;
                return *this;
            }
            //@}

            /// \name Mathematical Operators
            //@{
            currency_t &operator+=(const currency_t &cy)
            {
                currency_t c( *this + cy );
                swap(c);
                return *this;
            }
            currency_t &operator-=(const currency_t &cy)
            {
                currency_t c( *this - cy );
                swap(c);
                return *this;
            }
            currency_t &operator*=(const currency_t &cy)
            {
                currency_t c( *this * cy );
                swap(c);
                return *this;
            }
            currency_t &operator*=(long cy)
            {
                currency_t c( *this * cy );
                swap(c);
                return *this;
            }
            currency_t &operator*=(int val)
            {
                return operator*=((long)val);
            }
            currency_t &operator*=(double val)
            {
                cy_.int64 = (LONGLONG)(cy_.int64 * val);
                return *this;
            }

            currency_t &operator/=(int val) const
            {
                return operator/=((long)val);
            }
            currency_t &operator/=(long val)
            {

                if(!valid())
                {
                    // Check for invalid number
                    return *this;
                }

                // Check for divide by 0
                if (val == 0)
                {
                    // Set to maximum negative value
                    cy_.Hi = 0x80000000;
                    cy_.Lo = 0x00000000;

                    return *this;
                }
                cy_.int64/=val;

                return *this;
            }

            currency_t operator+(const currency_t &cy)const
            {
                currency_t rv;
                VarCyAdd(cy_,cy.cy_,&rv.cy_) | raise_exception ;
                return rv;
            }
            currency_t operator-(const currency_t &cy)const
            {
                currency_t rv;
                VarCySub(cy_,cy.cy_,&rv.cy_) | raise_exception ;
                return rv;
            }
            currency_t operator*(const currency_t &cy)const
            {
                currency_t rv;
                VarCyMul(cy_,cy.cy_,&rv.cy_) | raise_exception ;
                return rv;
            }
            currency_t operator*(long cy)const
            {
                currency_t rv;
                VarCyMulI4(cy_,cy,&rv.cy_) | raise_exception;
                return rv;
            }
            currency_t operator*(int cy)const
            {
                return operator *((long)cy);
            }

            currency_t operator*(double cy) const
            {
                currency_t val(*this);
                val *=cy;
                return val;
            }


            /// Calculate approximate ratio.
            double operator/(const currency_t &cy)const
            {
                return ((double)cy_.int64 /(double)cy.cy_.int64);
            }

            ///  Divide by int.
            currency_t operator/(int val) const
            {
                return operator/((long)val);
            }
            ///  Divide by long.
            currency_t operator/(long val) const
            {
                currency_t tmp(cy_);
                tmp/=val;
                return tmp;
            }

            ///  Divide by double
            double operator/(double val) const
            {
                if(!valid())
                {
                    // Check for invalid number
                    throw std::invalid_argument("Invalid divide");
                }

                // Check for divide by 0
                if (val == 0)
                {
                    throw std::overflow_error("Divide by 0");
                }
                return cy_.int64/(val*10000);
            }

            /// Unary negate.
            currency_t operator-()const
            {
                currency_t cy;
                VarCyNeg(cy_,&(cy.cy_)) | raise_exception;
                return cy;
            }

            //@}

            /** Rounds the value to specified number of decimal places.
              * \param decimals  Number of places to round to.
            */
            currency_t &round_to(int decimals)
            {
                VarCyRound(cy_,decimals,&cy_) | raise_exception;
                return *this;
            }

            /// \name Logical Operators
            //@{

            bool operator!=(const currency_t &cy) const { return cmp(cy)!=0; }
            bool operator!=(double val) const{ return cmp(val)!=0; }
            bool operator==(const currency_t &cy) const { return cmp(cy)==0; }
            bool operator==(double val) const{ return cmp(val)==0; }
            bool operator<=(const currency_t &cy) const{ return cmp(cy)<=0; }
            bool operator<=(double val) const{ return cmp(val)<=0; }
            bool operator>=(const currency_t &cy) const{ return cmp(cy)>=0; }
            bool operator>=(double val) const{ return cmp(val)>=0; }
            bool operator<(const currency_t &cy) const{ return cmp(cy)<0; }
            bool operator<(double val) const{ return cmp(val)<0; }
            bool operator>(const currency_t &cy) const{ return cmp(cy)>0; }
            bool operator>(double val) const{ return cmp(val)>0; }

            //@}

            /** Compares the value like strcmp.
              * \param cy Number to be compared.
            */
            int cmp(const currency_t &cy) const
            {
                return _cmpResult(VarCyCmp(cy_,cy.cy_));

            }

            /** Compares the value like strcmp.
              * \param cy Number to be compared.
            */
            int cmp(double cy) const
            {
                return _cmpResult(VarCyCmpR8(cy_,cy));
            }


            //! \name Access converters
            //@{
            tagCY get() const { return cy_;}
            tagCY in() const { return cy_;}
            tagCY *in_ptr() const { return const_cast<CY*>(&cy_);}
            tagCY *out() { return &cy_;}
            tagCY *inout() { return &cy_;}
            //@}


#if 0
            friend std::ostream &operator <<(std::ostream &str, const currency_t &val)
            {
                std::string strval=val.format( 1, str.precision(), str.width() );
                return str<< strval.c_str();
            }
#endif



            friend
            std::basic_ostream<char> &operator<<(std::basic_ostream<char> &str, const currency_t &val)
            {
                std::basic_string<char> strval;
                val.do_format(strval, 1, str.precision(), str.width() );
                return str << strval.c_str();
            }

            friend
            std::basic_ostream<wchar_t> &operator<<(std::basic_ostream<wchar_t> &str, const currency_t &val)
            {
                std::basic_string<wchar_t> strval;
                val.do_format(strval, 1, str.precision(), str.width() );
                return str << strval.c_str();
            }

            //! Format the string with the given digits, precision and width.
            std::basic_string<TCHAR> format(
                std::streamsize mindigits=0, std::streamsize minprecision=0,
                std::streamsize width=0) const
            {
                std::basic_string<TCHAR> strval;
                do_format(strval, mindigits, minprecision, width);
                return strval;
            }

            //! Parse the string to a currency.
            currency_t &parse( const bstr_t &str, LCID locale =::GetThreadLocale() );
/*            {
                VarCyFromStr( str.in(), locale, 0, &cy_ ) | raise_exception;
                return *this;
            }*/

        protected:
            /** Return a string representation of the value.
              * \param val output string (return values can't automatically detect template arguments)
              * \param mindigits Minimum number before decimal point.
              * \param minprecision Minimum number after decimal point.
              * \todo Obey ostream formats for: fillchar(), ios_base::left, ios_base::internal, ios_base::showpos
              */
            template <typename CH>
            void do_format(
                std::basic_string<CH>& val, std::streamsize mindigits,
                std::streamsize minprecision, std::streamsize /*width*/) const
            {
                COMET_ASSERT(mindigits>=0 && minprecision >=0 );
                if(minprecision> 4)  minprecision =4;

                // Add in the 4 fixed decimal points
                std::streamsize pr =
                    ((0 <= minprecision && minprecision <=4) ?
                        (4-minprecision) : 0);
                mindigits+=4;

                val.erase();
                val.reserve(22);
                LONGLONG value=cy_.int64;
                bool neg=value<0;
                if(neg)
                {
                    value=-value;
                }
                // Put in the digits backwards
                std::streamsize digit=0;
                bool output=false;
                while(value !=0 || digit < mindigits)
                {
                    CH dig=CH(value%10);
                    if(output || true==(output= (dig >0 || digit>=pr))) // Once 'output' is set - output everything.
                    {
                        val+=(CH('0'+dig));
                    }
                    if(++digit == 4)
                    {
                        val+=CH('.');
                        output=true;
                    }
                    value/=10;
                }
                if(neg)
                {
                    val+=CH('-');
                }

                // Now reverse the digits
                std::reverse(val.begin(), val.end());
            }
        public:

            /// Returns true if this is a valid number
            bool valid() const throw()
            { return !(cy_.Hi==(long)0x80000000 && cy_.Lo==0);}

            /// Detaches the CY value.  Provided for consistancy.
            CY detach() throw()
            {
                CY val = cy_;
                cy_.int64 = 0;
                return val;
            }

            /// Detaches the CY value.  Provided for consistancy.
            static CY detach(currency_t& cy) throw()
            {
                return cy.detach();
            }

        private:
            CY cy_;

            static int _cmpResult(HRESULT hr)
            {
                if(SUCCEEDED(hr))
                {
                    switch(hr)
                    {
                        case VARCMP_LT:
                            return -1;
                        case VARCMP_EQ :
                            return 0;
                        case VARCMP_GT:
                            return 1;
                        case VARCMP_NULL:
                            COMET_ASSERT(!"What do we do with this?");
                    }
                }
                else
                {
                    hr | raise_exception;
                }
                return 0; // shut the compiler up
            }
    };
    //@}
}

#endif // COMET_CURRENCY_H
