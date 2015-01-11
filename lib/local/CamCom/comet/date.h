/** \file
  * Date wrapper (part of datetime).
  */
/*
* Copyright © 2004  Michael Geddes
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

#ifndef INCLUDE_COMET_DATE_H
#define INCLUDE_COMET_DATE_H

#ifdef _SHOW_INC
#pragma message("   #Include " __FILE__)
#endif
#include <comet/datetime.h>

namespace comet
{
    /*! \addtogroup Misc
     */
    //@{

    //! Wrapper for a date only class.
    /**This is based on an integer version of DATE, except that it is an
     * absolute offset from 1/1/0.
     */
    struct dateonly_t : protected impl::datetime_base<long>
    {

        /// constructor
        dateonly_t() { dt_ = 0; }

        /// Construct from a datetime_t.
        explicit dateonly_t( const datetime_t &dt)
        {
            if (dt == dt_invalid )
                dt_ = dt_invalid_;
            else if (dt == dt_null)
                dt_ = dt_null_;
            else
                dt_ = split_oledate_as_absdate_( dt.get());
        }

        /// Attach from a long.
        dateonly_t( const impl::auto_attach_t<long> &dateval)
        {
            dt_ = dateval.get();
        }

        /// Get the raw 'long' value.
        long get() const { return dt_; }
        dateonly_t &operator=(const impl::auto_attach_t<long> & dateval)
        {
            dt_ = dateval.get();
            return *this;
        }

        /// Initialise as invalid.
        dateonly_t( dt_invalid_t ) { dt_ = dt_invalid_; }
        /// Initialise as null.
        dateonly_t( dt_null_t ) { dt_ = dt_null_; }
        /// Initialise as zero.
        dateonly_t( dt_zero_t) { dt_ = 0; }


        /// Construct from year/month/day.
        dateonly_t( int year, int month, int day )
        {
            if (!absdate_from_date_( &dt_, year, month, day))
                set_invalid_();
        }

        /// Get a 'NULL' datetime.
        static dateonly_t get_null() { return dateonly_t( dt_null_ ); }
        /// Get a 'zero' datetime.
        static dateonly_t get_zero() { return dateonly_t( 0 ); }

        /// Return today.
        static dateonly_t today()
        {
            return dateonly_t(datetime_t::now());
        }
        /// Return today (UTC).
        static dateonly_t today_utc()
        {
            return dateonly_t(datetime_t::now_utc());
        }

        /// Return year/month/day values.
        void split(int *year, int *month, int *day) const
        {
            if(!good() || ! date_from_absdate_( dt_, year, month, day))
                throw datetime_exception("Invalid Date");
        }

        /** Set date part as year/month/day.
         * \param year Year (from year 0 - as in 2000).
         * \param month Month of year (1-based).
         * \param day Day of month (1-based).
         * \retval true Successfully set date.
         * \retval false Conversion unsuccessful - date not set.
         */
        bool set_date( int year, int month, int day)
        {
            return absdate_from_date_( &dt_, year, month, day);
        }

        /// Convert to datetime.
        operator datetime_t() const
        {
            if (!good()) return datetime_t((DATE)dt_);
            return datetime_t(join_absdate_as_oledate_( dt_, 0));
        }

        /// \name Access date parts.
        //@{
        /// Year.
        int year() const
        {
            int year;
            split(&year,NULL,NULL);
            return year;
        }
        /// Month of year (1-based)
        int month() const
        {
            int year,month,day;
            split(&year,&month,&day);
            return month;
        }
        /// Day of month (1-based)
        int day() const
        {
            int year,month,day;
            split(&year,&month,&day);
            return day;
        }
        /// The day of week.
        datetime_t::day_of_week dow() const
        {
            int wday;
            if(!good() || ! dow_from_absdate_( dt_, &wday))
                throw datetime_exception("Invalid Date");
            return datetime_t::day_of_week(wday);
        }
        /// Day of the year (0 -based)
        int year_day() const
        {
            if (good())
            {
                int y,m,d;
                date_from_absdate_(dt_, &y,&m,&d);
                long firstday;
                if ( absdate_from_date_(&firstday, y, 1, 1))
                    return 1 + ( dt_ - firstday);
            }
            throw datetime_exception("Invalid Date");
        }
        /// Days in the month.
        int days_in_month() const
        {
            int year,month,day;
            split(&year,&month,&day);
            return days_in_month(year,month);
        }

        //@}

        /** Add specified number of months.
         *  If the day is not valid, force to the last day in the month.
         */
        dateonly_t &add_months(int inc_months)
        {
            int year,month,day;
            split(&year,&month,&day);
            long months = (month-1)+(year*12)+inc_months;

            long quot,rem;
            quot = months/12;
            rem = months%12;
            set_date( quot, rem+1, day);
            return *this;
        }

        /// Add specified number of years.
        dateonly_t &add_years(int inc_years)
        {
            int year,month,day;
            split(&year,&month,&day);
            set_date( year+inc_years, month, day);
            return *this;
        }

        ///\name Comparison operators
        //@{
        bool operator==(const dateonly_t& date) const { return date.dt_ == dt_; }
        bool operator!=(const dateonly_t& date) const{ return date.dt_ != dt_; }
        bool operator<(const dateonly_t& date) const
        {
            COMET_ASSERT( good() );
            COMET_ASSERT( date.good() );
            return dt_ < date.dt_;
        }
        bool operator>(const dateonly_t& date) const
        {
            COMET_ASSERT( good() );
            COMET_ASSERT( date.good() );
            return dt_ > date.dt_;
        }
        bool operator<=(const dateonly_t& date) const
        {
            COMET_ASSERT( good() );
            COMET_ASSERT( date.good() );
            return dt_ <= date.dt_;
        }
        bool operator>=(const dateonly_t& date) const
        {
            COMET_ASSERT( good() );
            COMET_ASSERT( date.good() );
            return dt_ >= date.dt_;
        }
        bool operator==(dt_invalid_t) const { return invalid(); }
        bool operator!=(dt_invalid_t) const { return !invalid(); }
        bool operator==(dt_zero_t) const { return dt_==0; }
        bool operator!=(dt_zero_t) const { return dt_!=0; }
        bool operator==(dt_null_t) const { return null(); }
        bool operator!=(dt_null_t) const { return !null(); }
        //@}

        ///\name Arithmetic operators
        //@{
        dateonly_t operator+(long dateSpan) const
        {
            dateonly_t dt(*this);
            dt+=dateSpan;
            return dt;
        }
        dateonly_t operator-(long dateSpan) const
        {
            dateonly_t dt(*this);
            dt-=dateSpan;
            return dt;
        }
        dateonly_t& operator+=(long dateSpan)
        {
            COMET_ASSERT( good() );
            dt_ += dateSpan;
            return *this;
        }
        dateonly_t& operator-=(long dateSpan)
        {
            COMET_ASSERT( good() );
            dt_ -= dateSpan;
            return *this;
        }
        long operator-(const dateonly_t& date) const
        {
            COMET_ASSERT( good() );
            COMET_ASSERT( date.good() );
            return dt_ - date.dt_;
        }
        dateonly_t &operator++()
        {
            COMET_ASSERT( good() );
            ++dt_;
            return *this;
        }
        dateonly_t operator++(int)
        {
            COMET_ASSERT( good() );
            dateonly_t t(*this);
            ++dt_;
            return t;
        }
        dateonly_t &operator--()
        {
            COMET_ASSERT( good() );
            --dt_;
            return *this;
        }
        dateonly_t operator--(int)
        {
            COMET_ASSERT( good() );
            dateonly_t t(*this);
            --dt_;
            return t;
        }
        //@}

        /// return true if the date is marked 'invalid'.
        inline bool invalid() const { return dt_ == ( dt_invalid_); }
        /// Return true if the date is marked 'null'
        inline bool null()  const { return dt_ == ( dt_null_); }

        /// return true if date is zero
        inline bool zero() const { return dt_ == 0; }

        /// Return true if the date is usable.
        inline bool good() const
        {
            switch (dt_)
            {
                case 0: case dt_invalid_: case dt_null_: return false;
                default: return true;
            }
        }

        /** Format as bstr.
         * \param flags Format (can be or-ed). All format_flags are valid.
         * \param locale Locale ID (default to User Default.
         */
        bstr_t format( datetime_t::format_flags flags = datetime_t::ff_default , LCID locale = LOCALE_USER_DEFAULT) const
        {
            flags = datetime_t::format_flags((flags | datetime_t::ff_date_only) & ~datetime_t::ff_time_only);
            bstr_t strDate;
            if (null() || invalid())
                return strDate;

            DATE dt = join_absdate_as_oledate_( dt_, 0);
            VarBstrFromDate(dt, locale, flags, strDate.out()) | raise_exception;
            return strDate;
        }

        /** Parse bstring to a datetime_t.
         * \param val String to parse.
         * \param flags valid format_flags are: ff_default, ff_system_locale, ff_hijri, ff_time_only, ff_date_only
         * \param locale Locale to use.  Default \p locale is the user default.
         */
        dateonly_t &parse( const bstr_t &val, datetime_t::format_flags flags = datetime_t::ff_default, LCID locale = LOCALE_USER_DEFAULT)
        {
            flags = datetime_t::format_flags((flags | datetime_t::ff_date_only) & ~datetime_t::ff_time_only);
            DATE dt;
            VarDateFromStr( val.in(), locale, flags, &dt) | raise_exception;
            dt_ = split_oledate_as_absdate_( dt);

            return *this;
        }

        /** Format as basic_string.
         * \param fmt See system documentation for strftime for details.
         */
        template<typename CHAR>
            std::basic_string<CHAR> format( const std::basic_string<CHAR> &fmt ) const
            {
                return format(fmt.c_str());
            }

        /** Format as basic_string.
         * \param fmt See system documentation for strftime for details.
         */
        template<typename CHAR>
            std::basic_string<CHAR> format( const CHAR *fmt ) const
            {
                // Just use datetime_t to handle this.
                datetime_t dt(*this);
                return dt.format(fmt);
            }

        /// Stream operator.
        friend
            std::basic_ostream<char> &operator<<(std::basic_ostream<char> &os, const dateonly_t &val)
            {
                os << val.format();
                return os;
            }

        /// Stream operator.
        friend
            std::basic_ostream<wchar_t> &operator<<(std::basic_ostream<wchar_t> &os, const dateonly_t &val)
            {
                os << val.format();
                return os;
            }

        static inline int days_in_month(int year, int month)
        {
            return impl::datetime_base<long>::days_in_month(year,month);
        }

        protected:
            explicit dateonly_t(long l) { dt_ = l; }
    };
    //@}
};

#endif /* INCLUDE_COMET_DATE_H */
