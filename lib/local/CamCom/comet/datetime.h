/** \file
  * Wrappers for DATE.
  */

/* Copyright © 2001 Michael Geddes
 *
 * This class was originally based on ATL/MFC code, however the original
 * implementations have almost entirely been replaced with more efficient code.
 * The core date algorithms are from boost.
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
#ifndef COMET_DATETIME_H
#define COMET_DATETIME_H

#include <comet/error_fwd.h>
#include <comet/bstr.h>
#include <comet/auto_buffer.h>

#include <math.h>
#include <time.h>

// The Platform SDK does not define VAR_FOURDIGITYEARS
#ifndef VAR_FOURDIGITYEARS
#define VAR_FOURDIGITYEARS    ((DWORD)0x00000040)
#endif

namespace comet
{

#define COMET_DIVMOD_( quot,rem, val1, val2) quot = (val1)/(val2); rem = (val1)%(val2);


/*! \addtogroup ErrorHandling
 */
//@{
//! Exception for datetimes.
class datetime_exception : public std::exception
{
public:
    datetime_exception( const char *desc) : desc_(desc)
    {}

    const char* what() const throw()
    {
        return desc_.c_str();
    }

private:
    std::string desc_;
};

//@}

/*! \addtogroup COMType
 */
//@{
/// Initialise date/time value as invalid.
static struct dt_invalid_t {} dt_invalid;
/// Initialise date/time value as null.
static struct dt_null_t {} dt_null;
/// Initialise date/time value as zero.
static struct dt_zero_t {
    operator double() const { return 0.;}
    operator long() const { return 0;}
} dt_zero;

// timeperiod_t
/////////////////

/** \class timeperiod_t datetime.h comet/datetime.h
  * Time-period.  Used with datetime_t math.
  */
class timeperiod_t
{
    enum {
        dt_invalid_ = 2147483647L,
    };
    /// Days.
    double pd_;
public:
    /// Default Constructor.
    timeperiod_t() : pd_(0.){}

    /// Construct invalid
    timeperiod_t( dt_invalid_t )  : pd_(dt_invalid_) {}
    /// Construct zero
    timeperiod_t( dt_zero_t) : pd_(0.) {}

    timeperiod_t( double period) :pd_(period){}
    timeperiod_t( float period) :pd_(period){}
    timeperiod_t( long period) :pd_(period){}
    timeperiod_t( int period) :pd_(period){}
    timeperiod_t( short period) :pd_(period){}
    timeperiod_t( unsigned long period) :pd_(period){}
    timeperiod_t( unsigned int period) :pd_(period){}
    timeperiod_t( unsigned short period) :pd_(period){}

    timeperiod_t( long days, long hours)
        : pd_(days + hours/24){}
    timeperiod_t( long days, long hours, long minutes)
        : pd_(days + (hours*60+minutes)/(24.*60.)){}
    timeperiod_t( long days, long hours, long minutes, long seconds, long milliseconds=0 )
        : pd_(days + ((hours*3600000L) + (minutes*60000L)+ (seconds*1000L)+ milliseconds)/86400000.){}

    /// \name Assignment operators
    //@{
    timeperiod_t &operator =( const double &period){pd_=period; return *this;}
    timeperiod_t &operator =( float period){pd_=period; return *this;}
    timeperiod_t &operator =( long period){pd_=period; return *this;}
    timeperiod_t &operator =( int period){pd_=period; return *this;}
    timeperiod_t &operator =( short period){pd_=period; return *this;}
    //@}

    /// Return time-period as a double (days).
    operator double() const{return pd_;}

    /// \name Comparison operators.
    //@{
    bool operator ==(const timeperiod_t &prd) const { return pd_ == prd.pd_; }
    bool operator !=(const timeperiod_t &prd) const { return pd_ != prd.pd_; }
    bool operator < (const timeperiod_t &prd) const { return pd_ <  prd.pd_; }
    bool operator > (const timeperiod_t &prd) const { return pd_ >  prd.pd_; }
    bool operator <=(const timeperiod_t &prd) const { return pd_ <= prd.pd_; }
    bool operator >=(const timeperiod_t &prd) const { return pd_ >= prd.pd_; }
    bool operator ==(dt_invalid_t) const { return pd_ == dt_invalid_; }
    bool operator !=(dt_invalid_t) const { return pd_ != dt_invalid_; }

    // These shouldn't be needed.
    template<typename T> bool operator < (T prd) const { return pd_ < double(prd); }
    template<typename T> bool operator <= (T prd) const { return pd_ <= double(prd); }
    template<typename T> bool operator > (T prd) const { return pd_ > double(prd); }
    template<typename T> bool operator >= (T prd) const { return pd_ >= double(prd); }
    template<typename T> bool operator == (T prd) const { return pd_ == double(prd); }
    template<typename T> bool operator != (T prd) const { return pd_ != double(prd); }

    //@}
    /// \name Simple math operators.
    //@{
    timeperiod_t operator+(const timeperiod_t &prd) const { return pd_ + prd.pd_; }
    timeperiod_t operator-(const timeperiod_t &prd) const { return pd_ - prd.pd_; }
    timeperiod_t &operator+=(const timeperiod_t &prd) { pd_ += prd.pd_; return *this; }
    timeperiod_t &operator-=(const timeperiod_t &prd) { pd_ -= prd.pd_; return *this; }
    timeperiod_t operator-() const { return -pd_; }
    //@}

    /// \name Conversion functions
    //@{
    double as_days() { return pd_; }
    void as_days(double prd) { pd_=prd; }
    double as_hours() { return pd_*24; }
    void as_hours(double prd) { pd_= prd/24; }
    double as_minutes() { return pd_*24*60; }
    void as_minutes(double prd) { pd_= prd/(24*60); }
    double as_seconds() { return pd_*24*60*60; }
    void as_seconds(double prd) { pd_= prd/(24*60*60); }
    //@}

    /// Split up the time period into days/hours/minutes/seconds.
    /** Backwards compatible.
     * \deprecated
     */
    void split( long& days, long& hours, long& minutes, long& seconds )
    {
        split(&days,&hours,&minutes,&seconds);
    }
    /// Split up the time period into days/hours/minutes/seconds.
    void split( long *days, long *hours, long *minutes, long *seconds, long *milliseconds = 0)
    {
        // Split into days and milliseconds.
        double int_part;
        long mspart = long(modf(pd_, &int_part) * 86400000);
        *days = long(int_part);
        // Optimise for integer.
        if (mspart == 0 )
        {
            *days = *hours = *minutes = *seconds =  0;
            if (milliseconds!=NULL) *milliseconds =0;
            return;
        }
        // Split up parts.
        long ms, quot, quot2;
        COMET_DIVMOD_(quot, ms, mspart, 1000);
        COMET_DIVMOD_(quot2, *seconds, quot, 60);
        COMET_DIVMOD_( *hours, *minutes, quot2, 60);
        if( milliseconds != NULL)
            *milliseconds = ms;
    }

    /// Set as days/hours/minutes/seconds.
    void set_period( long days, long hours, long minutes, long seconds, long milliseconds=0 )
    {
        pd_ = days + ((hours*3600000L) + (minutes*60000L)+ (seconds*1000L)+ milliseconds)/86400000.;
    }

    /// Return true if the period is invalid.
    bool invalid() const
    {
        return pd_ == (double)(dt_invalid_);
    }
    /// Return true if the period is not invalid.
    bool good() const
    {
        return !invalid();
    }
    /** return true if the period is valid.
     * \deprecated
     */
    bool valid() const { return !invalid(); }

    /// Return an invalid period.
    static timeperiod_t invalid_period() { return timeperiod_t( (double)(dt_invalid_)); }

};


/// A wrapper for choosing strftime/wcsftime based on char type.
template< typename CHAR >
inline size_t str_formattime( CHAR *strDest, size_t maxsize, const CHAR *format, const struct tm *timeptr )
{
    return -1;
}
/// @if Internal
template<>
inline size_t str_formattime<char>( char *strDest, size_t maxsize, const char *format, const struct tm *timeptr )
{
    return  strftime( strDest, maxsize, format, timeptr );
}

template<>
inline size_t str_formattime<wchar_t>( wchar_t *strDest, size_t maxsize, const wchar_t *format, const struct tm *timeptr )
{
    return  wcsftime( strDest, maxsize, format, timeptr );
}

namespace impl {
// Internally used to group div/mod so optimiser is likely to pick it up.

    const double half_millisecond = 1.0/172800000.0;

    template<typename T>
    struct datetime_base
    {
        T dt_;

        enum convert_mode {
            cmBoth,
            cmOnlyTime,
            cmOnlyDate
        };


        /*! Convert absolute date to date-parts.
        */
        static bool date_from_absdate_( long daysAbsolute, int *tm_year, int *tm_mon, int *tm_mday );

        /*! Convert absolute date to day-of-week.
         */
        static bool dow_from_absdate_( long daysAbsolute, int *tm_wday)
        {
            // Calculate the day of week (sun=0, mon=1...)
            //   -1 because 1/1/0 is Sat.
            *tm_wday = (int)((daysAbsolute + 1) % 7L);
            return true;
        }

        /*! Convert date parts to absolute date.
        */
        static bool absdate_from_date_( long *daysAbsolute, int tm_year, int tm_mon, int tm_mday);

        /*! Convert time in milliseconds to time of day parts.
        */
        static bool time_from_milliseconds_( long milliseconds, int *tm_hour, int *tm_min, int *tm_sec, int *tm_ms);
        /*! Convert  time-of-day parts to milliseconds.
        */
        static bool milliseconds_from_time_( long *milliseconds, unsigned short tm_hour, unsigned short tm_min, unsigned short tm_sec, unsigned short tm_ms);

        /*! Convert ole date to datetime-parts.
        */
        static bool datetime_from_oledate_( DATE date, int *tm_year,  int *tm_mon, int *tm_mday,  int *tm_dow, int *tm_hour, int *tm_min, int *tm_sec, int *tm_ms,  convert_mode mode);

        /*! Get date part of oledate.
        */
        static inline long split_oledate_as_absdate_( DATE date )
        {
            double val = to_double(date)+ (693959 + 1721060) + half_millisecond; // Add days from 1/1/0 to 12/30/1899
            return long(floor(val));
        }
        /*! Split oledate into date/milliseconds.
        */
        static inline long split_oledate_as_absdate_( DATE date, long *ms_part,bool need_ms_part )
        {
            double val = to_double(date)+ (693959 + 1721060) + half_millisecond; // Add days from 1/1/0 to 12/30/1899
            if (!need_ms_part) return long(floor(val));
            *ms_part = long(modf(val, &val) * 86400000);
            return long(val);
        }

        /*! Join oledate.
        */
        static inline DATE join_absdate_as_oledate_( long absDate, long ms_part)
        {
            return to_date( (double(absDate) + (ms_part / 86400000.)) - 693959 - 1721060 );
        }


        /*! Convert datetime-parts to ole date.
        */
        static bool oledate_from_datetime_( DATE *date, unsigned short tm_year, unsigned short tm_mon, unsigned short tm_mday,  unsigned short tm_hour, unsigned short tm_min, unsigned short tm_sec, unsigned short tm_ms, convert_mode mode);

        /*! Convert TM to OLE date.
         * Sets the date to invalid if unsuccessful.
         * \retval true Successful conversion.
         */
        static bool from_tm_( const struct tm &src, DATE *dt, convert_mode mode);

        /*! Convert OLE date to TM.
         * \retval true Successful conversion.
         */
        static bool to_tm_( DATE dt, struct tm *dest, int *ms);

        void set_invalid_() { dt_ = ((T) dt_invalid_); }
        void set_null_()  { dt_ = ((T) dt_null_); }

        /// Convert offset from 0 to DATE type.
        static DATE to_date( double dbl)
        {
            if(dbl>0) return dbl;
            double t=floor(dbl);
            return t+(t-dbl);
        }
        /// Convert DATE type to offset from 0.
        static double to_double( DATE dt)
        {
            if(dt>=0) return dt;
            double t = ceil(dt);
            return t-(dt-t);
        }

        /// Set to value \a dt and check the range.
        bool set_check_range_( T dt)
        {
            bool result = (dt <= dt_max && dt >= dt_min);
            if (result)
                dt_ = dt;
            return result;
        }
        /// Set the value to \a dt and set 'invalid' if out of range.
        void set_invalid_check_range_(T dt)
        {
            if (!set_check_range_(dt) )
                set_invalid_();
        }

        /// Return true if \a year is a leap-year.
        static bool is_leap_year( long year)
        {
            if ((year & 0x3) != 0) return false;
            // && ((year % 100) != 0 || (year % 400) == 0);
            long quot,rem;
            COMET_DIVMOD_(quot,rem, year, 100);
            if (rem != 0) return true;
            return ((quot & 0x3) == 0);
        }

        enum {
            dt_max = 2958465L, // about year 9999
            dt_null_ = 2147483648L,
            dt_invalid_ = 2147483647L,
            dt_min = (-657434L)  // about year 100
        };


        static int days_in_month(int year, int month)
        {
            switch (month)
            {
            case 2: return (is_leap_year(year)?29:28);
            case 4: case 6: case 9: case 11:
                    return 30;
            default:return 31;
            };
        }

    };
    //! @endif


    // Convert TM to OLE date
    template<typename T>
    bool
    datetime_base<T>::from_tm_( const struct tm &src, DATE *dt, convert_mode mode)
    {
        return oledate_from_datetime_( dt, unsigned short(src.tm_year + 1900),unsigned short( src.tm_mon+1),unsigned short( src.tm_mday),unsigned short( src.tm_hour),unsigned short( src.tm_min),unsigned short( src.tm_sec), 0U, mode);
    }

    // Convert OLE date to TM. \retval true Successful conversion.
    template<typename T>
    bool
    datetime_base<T>::to_tm_( DATE dt, struct tm *dest, int *ms)
    {
        int y,m,d;
        if ( !datetime_from_oledate_( dt,  &y,  &m, &d, &dest->tm_wday, &dest->tm_hour, &dest->tm_min, &dest->tm_sec, NULL, cmBoth) )
            return false;
        dest->tm_year = y;
        dest->tm_mon = m;
        dest->tm_mday = d;


        if (dest->tm_year != 0)
        {
            long firstday, thisday;
            absdate_from_date_( &thisday, y,m,d);
            absdate_from_date_(&firstday, y, 1, 1);
            dest->tm_yday =  1+ ( thisday - firstday);
            // Convert afx internal tm to format expected by runtimes (_tcsftime, etc)
            dest->tm_year -= 1900;  // year is based on 1900
            dest->tm_mon -= 1;      // month of year is 0-based
            dest->tm_isdst = -1;    // Don't know DST status.
        }
        else
            dest->tm_yday = 0;
        return true;
    }

    // Convert OLE date to date-parts.
    template<typename T>
    bool
    datetime_base<T>::date_from_absdate_(long daysAbsolute , int *tm_year, int *tm_mon, int *tm_mday)
    {
        // These algorithms are taken from the gregorian_calendar
        // calculations in boost.
        typedef long date_int_type;
        typedef int year_type;
        date_int_type dayNumber = daysAbsolute;
        date_int_type a = dayNumber + 32044 ;
        date_int_type b = (4*a + 3)/146097;
        date_int_type c = a-((146097*b)/4);
        date_int_type d = (4*c + 3)/1461;
        date_int_type e = c - (1461*d)/4;
        date_int_type m = (5*e + 2)/153;
        *tm_mday = static_cast<unsigned short>(e - ((153*m + 2)/5) + 1);
        *tm_mon = static_cast<unsigned short>(m + 3 - 12 * (m/10));
        *tm_year = static_cast<unsigned short>(100*b + d - 4800 + (m/10));
        return true;
    }

    // Convert date parts to absolute date.
    template<typename T>
    bool
    datetime_base<T>::absdate_from_date_( long *daysAbsolute, int tm_year, int tm_month, int tm_mday)
    {
        // These algorithms are taken from the gregorian_calendar
        // calculations in boost.
        unsigned short a = static_cast<unsigned short>((14-tm_month)/12);
        unsigned short y = static_cast<unsigned short>(tm_year + 4800 - a);
        unsigned short m = static_cast<unsigned short>(tm_month + 12*a - 3);
        unsigned long  d = tm_mday + ((153*m + 2)/5) + 365*y + (y/4) - (y/100) + (y/400) - 32045;

        *daysAbsolute = d;

        return true;
    }

    // Convert OLE time to time of day parts.
    template<typename T>
    bool
    datetime_base<T>::time_from_milliseconds_( long milliseconds, int *tm_hour, int *tm_min, int *tm_sec, int *tm_ms)
    {
        if (milliseconds == 0 )
        {
            *tm_hour = *tm_min = *tm_sec = 0;
            if (tm_ms!=NULL) *tm_ms =0;
            return true;
        }
        long ms, quot, quot2;
        COMET_DIVMOD_(quot, ms, milliseconds, 1000);
        COMET_DIVMOD_(quot2, *tm_sec, quot, 60);
        COMET_DIVMOD_( *tm_hour, *tm_min, quot2, 60);
        if( tm_ms != NULL)
            *tm_ms =  ms;
        return true;
    }

    // Convert  time-of-day parts to milliseconds.
    template<typename T>
    bool
    datetime_base<T>::milliseconds_from_time_( long *milliseconds, unsigned short tm_hour, unsigned short tm_min, unsigned short tm_sec, unsigned short tm_ms)
    {
        if ( tm_hour > 23 || tm_min > 59 || tm_sec> 59) return false;

        *milliseconds = (tm_hour* 3600000L) + (tm_min*60000L)+ (tm_sec*1000)+ tm_ms;
        return true;
    }

    //
    template<typename T>
    bool
    datetime_base<T>::datetime_from_oledate_( DATE date, int *tm_year, int *tm_mon, int *tm_mday, int *tm_wday, int *tm_hour, int *tm_min, int *tm_sec, int *tm_ms, convert_mode mode)
    {
        long datePart, msPart;
        datePart = split_oledate_as_absdate_(date, &msPart, mode != cmOnlyDate);
        if ( mode != cmOnlyDate && !time_from_milliseconds_( msPart, tm_hour, tm_min, tm_sec, tm_ms))
            return false;
        return (mode == cmOnlyTime) || (date_from_absdate_( datePart, tm_year, tm_mon, tm_mday)) && ( (tm_wday==NULL) || dow_from_absdate_(datePart, tm_wday));
    }

    // Convert datetime-parts to ole date.
    template<typename T>
    bool
    datetime_base<T>::oledate_from_datetime_( DATE *date, unsigned short tm_year, unsigned short tm_mon, unsigned short tm_mday, unsigned short tm_hour, unsigned short tm_min, unsigned short tm_sec, unsigned short tm_ms, convert_mode mode)
    {
        long datePart = 0, timePart = 0;
        if (mode != cmOnlyDate && !milliseconds_from_time_( &timePart, tm_hour, tm_min, tm_sec, tm_ms))
            return false;
        if (mode != cmOnlyTime && !absdate_from_date_( &datePart, tm_year, tm_mon, tm_mday))
            return false;
        *date = join_absdate_as_oledate_(datePart, timePart);
        return true;
    }
}


/** \class datetime_t datetime.h comet\datetime.h
  * Wrapper for DATE.
  * DATE/TIME Represented as days + fraction of days.
  */
class datetime_t : private impl::datetime_base<DATE>
{
public:

    /// UTC/Local conversion mode.
    struct utc_convert_mode
    {
        enum type
        {
            none,         ///< No conversion.
            local_to_utc, ///< Convert from local to utc.
            utc_to_local  ///< Convert from utc to local.
        };
    };

    /// Describe how to get the timezone bias.
    struct timezone_bias_mode
    {
        enum type
        {
            standard,       ///< Standard timezone offset
            daylight_saving ///< Summer timezone offset
        };
    };

    /// Root which a time uses as an offset.
    struct locality
    {
        enum type
        {
            utc,  ///< A local timezone date/time.
            local ///< A UTC date/time.
        };
    };

    /** \name Constructors.
     * Attach to various system date/time types.
     */
    //@{
    /// Constructor
    datetime_t()  { dt_ = 0.;}

    /// Constructor from raw DATE type.
    explicit datetime_t(DATE date)
    {
        dt_ = date;
    }

    //! Construct from date/time components.
    /** If conversion fails, an valid() will return false.
     */
    explicit datetime_t(int year, int month, int day, int hours=-1, int minutes=0, int seconds=0, int milliseconds=0)
    {
        if (!oledate_from_datetime_( &dt_, (unsigned short)year, (unsigned short)month,
                    (unsigned short)day, (unsigned short)hours, (unsigned short)minutes, (unsigned short)seconds, (unsigned short) milliseconds,
                    (hours < 0)?cmOnlyDate:cmBoth ))
            set_invalid_();
    }

    /// Initialise as invalid.
    datetime_t( dt_invalid_t) { dt_ = dt_invalid_; }
    /// Initialise as null.
    datetime_t( dt_null_t) { dt_ = dt_null_; }
    /// Initialise as zero.
    datetime_t( dt_zero_t) { dt_ = 0.; }

    /// Get a 'NULL' datetime.
    static datetime_t get_null() { return datetime_t( DATE(dt_null_) ); }
    /// Get a 'zero' datetime.
    static datetime_t get_zero() { return datetime_t( DATE(0) ); }


    //! Construct from a SYSTEMTIME.
    /** Defaults to no conversion!
     * \sa from_systemtime to_systemtime
      */
    explicit datetime_t(const SYSTEMTIME& systimeSrc)
    {
        if (!from_systemtime(systimeSrc))
            set_invalid_();
    }

    /**
     * Construct from a @e SYSTEMTIME.
     *
     * @param source
     *     @e SYSTEMTIME being converted.
     * @param utc_mode
     *     Timezone conversion mode.
     * @param conversion_time
     *     Optional override date to use for calculating daylight/standard time.
     * @param utc_or_local
     *     Specify whether the override time is a UTC or local date.
     */
    explicit datetime_t(
        const SYSTEMTIME& source, utc_convert_mode::type utc_mode,
        const datetime_t& conversion_time=datetime_t(dt_invalid),
        locality::type utc_or_local=locality::local)
    {
        if (!from_systemtime(source, utc_mode, conversion_time, utc_or_local))
            set_invalid_();
    }

    /**
     * Construct from a @e SYSTEMTIME.
     *
     * @param source
     *     @e SYSTEMTIME being converted.
     * @param utc_mode
     *     Timezone conversion mode.
     * @param bias_mode
     *     Specify whether the local time is daylight/standard time.
     */
    explicit datetime_t(
        const SYSTEMTIME& source, utc_convert_mode::type utc_mode,
        timezone_bias_mode::type bias_mode)
    {
        if (!from_systemtime(source, utc_mode, bias_mode))
            set_invalid_();
    }

    /**
     * Construct from a @e FILETIME.
     *
     * Defaults to no timezone conversion. FILETIME values are a tricky beast.
     * FILETIMEs on FAT are local, as are ZIP files (mostly). On shares and
     * NTFS, they are UTC.
     *
     * @param source
     *     @e FILETIME being converted.
     * @param utc_mode
     *     Timezone conversion mode.
     * @param conversion_time
     *     Optional override date to use for calculating daylight/standard time.
     * @param utc_or_local
     *     Specify whether the override time is a UTC or local date.
     *
     * @sa from_filetime to_filetime
     */
    explicit datetime_t(
        const FILETIME& source,
        utc_convert_mode::type utc_mode=utc_convert_mode::none,
        const datetime_t& conversion_time=datetime_t(dt_invalid),
        locality::type utc_or_local=locality::local)
    {
        if (!from_filetime(source, utc_mode, conversion_time, utc_or_local))
            set_invalid_();
    }

    /**
     * Construct from a @e FILETIME.
     *
     * Defaults to no timezone conversion. FILETIME values are a tricky beast.
     * FILETIMEs on FAT are local, as are ZIP files (mostly). On shares and
     * NTFS, they are UTC.
     *
     * @param source
     *     @e FILETIME being converted.
     * @param utc_mode
     *     Timezone conversion mode.
     * @param bias_mode
     *     Specify whether the local time is daylight/standard time.
     *
     * @sa from_filetime to_filetime
     */
    explicit datetime_t(
        const FILETIME& source, utc_convert_mode::type utc_mode,
        timezone_bias_mode::type bias_mode)
    {
        if (!from_filetime(source, utc_mode, bias_mode))
            set_invalid_();
    }

    /**
     * Construct from a Unix time.
     *
     * Defaults to conversion from utc to local time as time_t is in UTC.
     *
     * @param source
     *    Unix time being converted.
     * @param utc_mode
     *     Timezone conversion mode.
     * @param conversion_time
     *     Optional override date to use for calculating daylight/standard time.
     * @param utc_or_local
     *     Specify whether the override time is a UTC or local date.
     *
     * @sa from_unixtime to_unixtime
     */
    explicit datetime_t(
        time_t source,
        utc_convert_mode::type utc_mode=utc_convert_mode::utc_to_local,
        const datetime_t& conversion_time=datetime_t(dt_invalid),
        locality::type utc_or_local=locality::local)
    {
        if (!from_unixtime(source, utc_mode, conversion_time, utc_or_local))
            set_invalid_();
    }

    /**
     * Construct from a Unix time.
     *
     * Defaults to conversion from utc to local time as time_t is in UTC.
     *
     * @param source
     *     Unix time being converted.
     * @param utc_mode
     *     Timezone conversion mode.
     * @param bias_mode
     *     Specify whether the local time is daylight/standard time.
     *
     * @sa from_unixtime to_unixtime
     */
    explicit datetime_t(
        time_t source, utc_convert_mode::type utc_mode,
        timezone_bias_mode::type bias_mode)
    {
        if (!from_unixtime(source, utc_mode, bias_mode))
            set_invalid_();
    }

    //! Copy constructor.
    datetime_t(const datetime_t& date)
    {
        dt_ = date.dt_;
    }

    //@}

    /// Create a const reference.
    static const datetime_t& create_const_reference(const DATE& s) throw()
    { return *reinterpret_cast<const datetime_t*>(&s); }
    /// Create a non-const reference.
    static datetime_t& create_reference(DATE& s) throw()
    { return *reinterpret_cast<datetime_t*>(&s); }

    /// Day-of-week enumeration.
    enum day_of_week {
        dow_sun=0, dow_mon, dow_tue, dow_wed, dow_thu, dow_fri, dow_sat
    };

    /// Return the current time.
    static datetime_t now()
    {
        SYSTEMTIME lt;
        ::GetLocalTime(&lt);
        return datetime_t(lt);
    }
    /// Return the current utc time.
    static datetime_t now_utc()
    {
        SYSTEMTIME lt;
        ::GetSystemTime(&lt);
        return datetime_t(lt);
    }

    /** Add specified number of months.
     *  If the day is not valid (ie 1 month from 31 December)
     *  an exception will be thrown.
     *  \todo Add an enum to be more smart about this.
     */
    datetime_t &add_months(int inc_months)
    {
        int year,month,day;

        split_date(&year,&month,&day);
        long months = (month-1)+(year*12)+inc_months;

        long quot,rem;
        COMET_DIVMOD_(quot, rem, months, 12);
        if(!set_date( quot, rem+1, day))
            throw datetime_exception("Invalid Date");

        return *this;
    }

    /// Add specified number of years.
    datetime_t &add_years(int inc_years)
    {
        int year,month,day;
        split_date(&year,&month,&day);
        if(!set_date( year+inc_years, month, day))
            throw datetime_exception("Invalid Date");
        return *this;
    }

    /// Return year/month/day values.
    void split_date(int *year, int *month, int *day) const
    {
        if (good())
        {
            long datePart = split_oledate_as_absdate_(dt_);
            if (date_from_absdate_( datePart, year, month, day) )
                return;
        }
        throw datetime_exception("Invalid Date");
    }

    /// Return hours/minutes/second values.
    void split_time( int *hours, int *minutes, int *seconds, int *milliseconds=NULL) const
    {
        if(!good() || !datetime_from_oledate_(dt_, NULL,  NULL, NULL, NULL, hours, minutes, seconds, milliseconds, cmOnlyTime))
            throw datetime_exception("Invalid DateTime");
    }
    /// Return date/time split up.
    void split(int *year, int *month, int *day, int *hours, int *minutes, int *seconds, int *milliseconds=NULL)
    {
        if(!good() || !datetime_from_oledate_(dt_, year, month, day, NULL, hours, minutes, seconds, milliseconds, cmBoth))
            throw datetime_exception("Invalid DateTime");
    }


    /// \name Access date/time parts.
    //@{
    /// Year.
    int year() const
    {
        int year,month,day;
        split_date(&year,&month,&day);
        return year;
    }
    /// Month of year (1-based)
    int month() const
    {
        int year,month,day;
        split_date(&year,&month,&day);
        return month;
    }
    /// Day of month (1-based)
    int day() const
    {
        int year,month,day;
        split_date(&year,&month,&day);
        return day;
    }
    /// Hour part of time (0-based) ???
    int hour() const
    {
        int hours,minutes,seconds;
        split_time(&hours,&minutes,&seconds);
        return hours;
    }
    /// Minute part of time (0-based)
    int minute() const
    {
        int hours,minutes,seconds;
        split_time(&hours,&minutes,&seconds);
        return minutes;
    }
    /// Second
    int second() const
    {
        int hours,minutes,seconds;
        split_time(&hours,&minutes,&seconds);
        return seconds;
    }
    /// Milliseconds
    int millisecond() const
    {
        int hours,minutes,seconds,ms;
        split_time(&hours,&minutes,&seconds,&ms);
        return ms;
    }

    /// The day of week.
    day_of_week dow() const
    {
        long datePart;
        datePart = split_oledate_as_absdate_(dt_);
        int wday;
        if(!good() || !dow_from_absdate_(datePart, &wday))
            throw datetime_exception("Invalid Date");
        return day_of_week(wday);
    }
    /// Day of the year (0 -based)
    int year_day() const
    {
        if (good())
        {
            long datepart = split_oledate_as_absdate_(dt_);
            int y,m,d;
            date_from_absdate_(datepart, &y,&m,&d);
            long firstday;
            if ( absdate_from_date_(&firstday, y, 1, 1))
                return 1+ ( datepart - firstday);
        }
        throw datetime_exception("Invalid Date");
    }
    /// Days in the month;
    int days_in_month()
    {
        int year,month,day;
        split_date(&year,&month,&day);
        return impl::datetime_base<DATE>::days_in_month(year,month);
    }
    //@}
    static inline int days_in_month(int year, int month)
    {
        return impl::datetime_base<DATE>::days_in_month(year,month);
    }

    /// \name Assignment operators
    //@{

    datetime_t &operator=( const datetime_t& date)
    {
        dt_ = date.dt_;
        return *this;
    }

    datetime_t &operator=( DATE date )
    {
        set_invalid_check_range_(date);
        return *this;
    }
    //@}

    ///\name Comparison operators
    //@{
    bool operator==(const datetime_t& date) const{ return date.dt_==dt_; }
    bool operator!=(const datetime_t& date) const{ return date.dt_!=dt_; }
    bool operator<(const datetime_t& date) const { return to_double(dt_)<to_double(date.dt_); }
    bool operator>(const datetime_t& date) const{ return to_double(dt_)>to_double(date.dt_); }
    bool operator<=(const datetime_t& date) const{ return to_double(dt_)<=to_double(date.dt_); }
    bool operator>=(const datetime_t& date) const{ return to_double(dt_)>=to_double(date.dt_); }
    bool operator==(dt_invalid_t) const { return invalid(); }
    bool operator!=(dt_invalid_t) const { return !invalid(); }
    bool operator==(dt_zero_t) const { return dt_==0.; }
    bool operator!=(dt_zero_t) const { return dt_!=0.; }
    bool operator==(dt_null_t) const { return null(); }
    bool operator!=(dt_null_t) const { return !null(); }
    //@}

    ///\name Arithmetic operators
    //@{
    datetime_t operator+(const timeperiod_t& dateSpan) const
    {
        datetime_t dt(*this);
        dt+=dateSpan;
        return dt;
    }
    datetime_t operator-(const timeperiod_t& dateSpan) const
    {
        datetime_t dt(*this);
        dt-=dateSpan;
        return dt;
    }
    datetime_t& operator+=(const timeperiod_t &dateSpan)
    {
        COMET_ASSERT( good() );
        if(!good())
            set_invalid_();
        else
            set_invalid_check_range_(to_date( to_double(dt_) + (double)dateSpan ));

        return *this;
    }
    datetime_t& operator-=(const timeperiod_t &dateSpan)
    {
        COMET_ASSERT( good() );
        if(!good())
            set_invalid_();
        else
            set_invalid_check_range_(to_date( to_double(dt_) - (double)dateSpan ));
        return *this;
    }
    timeperiod_t operator-(const datetime_t& date) const
    {
        COMET_ASSERT( good() && date.good() );
        if( !good() || ! date.good())
            return timeperiod_t::invalid_period();
        return to_double(dt_) - to_double(date.dt_);
    }
    datetime_t &operator++()
    {
        (*this)+=1;
        return *this;
    }
    datetime_t operator++(int)
    {
        datetime_t t(*this); (*this)+=1; return t;
    }
    datetime_t &operator--()
    {
        (*this)-=1; return *this;
    }

    datetime_t operator--(int)
    {
        datetime_t t(*this); (*this)-=1; return t;
    }
    //@}

    /// return true if the date is marked 'invalid'.
    inline bool invalid() const { return dt_ == ((double) dt_invalid_); }
    /// return true if the date is marked 'null'
    inline bool null()  const { return dt_ == ((double) dt_null_); }

    /// return true if date is zero
    inline bool zero() const { return dt_ == 0; }

    /** return true if the date is not marked 'invalid'.
     * \deprecated
     */
    inline bool valid() const { return !invalid(); }

    /// Return true if the date is usable.
    inline bool good() const
    {
        switch ((long)dt_)
        {
            case dt_invalid_: case dt_null_: return false;
            default: return true;
        }
    }

    ///\name Accessor methods
    //@{
    DATE get() const { if(invalid()) throw("Invalid Date"); return null()?0:dt_;}
    DATE in() const { return get(); }
    DATE *in_ptr() const { return const_cast<DATE *>(&dt_);}
    DATE *out() { return &dt_;}
    DATE *inout() { return &dt_;}
    //@}
    /** Set date part as year/month/day.
     * \param year Year (from year 0 - as in 2000).
     * \param month Month of year (1-based).
     * \param day Day of month (1-based).
     * \retval true Successfully set date.
     * \retval false Conversion unsuccessful - date not set.
     */
    bool set_date( int year, int month, int day)
    {
        long datePart, timePart;
        datePart = split_oledate_as_absdate_(dt_, &timePart, true);
        if (!absdate_from_date_(&datePart, year,month, day))
            return false;
        dt_ = join_absdate_as_oledate_( datePart, timePart);
        return true;
    }

    /** Set time part as hours/minutes/seconds.
     * \param hours As in a 24-hour clock.
     * \param minutes Minutes past the hour.
     * \param seconds Seconds past the minute.
     * \param milliseconds Milliseconds past the second.
     * \retval true Successfully set time.
     * \retval false Conversion unsuccessful - time not set.
     */
    bool set_time( int hours, int minutes, int seconds, int milliseconds =0)
    {
        long datePart, timePart;
        datePart = split_oledate_as_absdate_(dt_, &timePart, true);
        if (!milliseconds_from_time_(&timePart, (unsigned short)hours, (unsigned short)minutes, (unsigned short)seconds, (unsigned short)milliseconds))
            return false;
        dt_ = join_absdate_as_oledate_( datePart, timePart);
        return true;
    }
    /** Set both date and time.
     * \param year Year (from year 0 - as in 2000).
     * \param month Month of year (1-based).
     * \param day Day of month (1-based).
     * \param hours As in a 24-hour clock.
     * \param minutes Minutes past the hour.
     * \param seconds Seconds past the minute.
     * \param milliseconds Milliseconds past the second.
     * \retval true Successfully set date/time.
     * \retval false Conversion unsuccessful - date/time not set.
     */
    bool set_date_time(int year, int month, int day, int hours, int minutes, int seconds, int milliseconds = 0 )
    {
        return oledate_from_datetime_(&dt_, (unsigned short)year, (unsigned short)month, (unsigned short)day, (unsigned short)hours, (unsigned short)minutes, (unsigned short)seconds, (unsigned short)milliseconds, cmBoth);
    }

    /// Flags for formatting.
    enum format_flags{
        ff_default = 0,                                 ///< Default formatting.
        ff_system_locale = LOCALE_NOUSEROVERRIDE,       ///< Use system locale
        ff_hijri = VAR_CALENDAR_HIJRI,                  ///< Use HIJRI calendar.
        ff_thai =  0x10, /* VAR_CALENDAR_THAI, */       ///< Use thai calendar.
        ff_gregorian = 0x20, /*VAR_CALENDAR_GREGORIAN*/ ///< Use gregorian calendar.
        ff_four_digits = VAR_FOURDIGITYEARS,            ///< Four digits for years
        ff_time_only = VAR_TIMEVALUEONLY,               ///< Only output time.
        ff_date_only = VAR_DATEVALUEONLY                ///< Only output date.
    };
    /** Parse bstring to a datetime_t.
     * \param val String to parse.
     * \param flags valid format_flags are: ff_default, ff_system_locale, ff_hijri, ff_time_only, ff_date_only
     * \param locale Locale to use.  Default \p locale is the user default.
     */
    datetime_t &parse( const bstr_t &val, format_flags flags = ff_default, LCID locale = LOCALE_USER_DEFAULT)
    {
        VarDateFromStr( val.in(), locale, flags, &dt_) | raise_exception;
        return *this;
    }

    /** Return a double that is sortable / can be subtracted.
     * Dates before 12/30/1899 will not sort propperly.
     */
    double as_sortable_double() const { COMET_ASSERT( good() ); return to_double(dt_); }

public:

    /**
     * Convert a local time to UTC.
     * 
     * Takes a local time (like that inside a ZIP file, or on a FAT file
     * system) and converts it to UTC, using the timezone rules in effect as
     * of the date specified.  Typically the "as of" date is specified as the
     * modification or creation date of the ZIP file, or left missing to
     * default to the given local date. It is also possible to specify if the
     * "as of" date is in UTC or not.  If missing, it defaults to false.
     */

    /**
     * Create UTC version of this local time.
     *
     * Assuming this datetime is a local time (like that inside a ZIP file or
     * on a FAT file system) this creates a new version of it as a UTC datetime.
     * By default, the adjustment is made based on the timezone rules that 
     * would have been in effect at the UTC date this object represents.  
     * However, the time can also be converted as though it were on another 
     * date by passing another date as an argument.
     * 
     * Typically the "as of" date is specified as the current time or possibly
     * the modification or creation date of an enclosing ZIP file.
     *
     * @param as_of_date
     *     Optional alternative date on which to base the timezone conversion.
     * @param utc_or_local
     *     Whether `as_of_date` is a local or UTC date.  Defaults to UTC.
     */
    datetime_t local_to_utc(
        datetime_t as_of_date=datetime_t(dt_invalid),
        locality::type utc_or_local=locality::local) const
    {
        // if they didn't specify an AS OF date, use the current date which
        // will be local
        if (as_of_date.invalid())
        {
            utc_or_local = locality::local; // no break
            as_of_date = *this;
        }

        double timezone_bias =
            local_timezone_bias(as_of_date, utc_or_local) / (24.*60.);
        double local_date_continuous = to_double(dt_);
        DATE utc_date = to_date(local_date_continuous + timezone_bias);

        return datetime_t(utc_date);
    }

    /**
     * Create a UTC version of this local time, explicitly using standard
     * time or daylight savings.
     *
     * Assuming this datetime is a local time (like that inside a ZIP file or
     * on a FAT file system) this creates a new version of it as a UTC datetime.
     * Depending on the argument passed, the adjustment is made as though 
     * daylight savings were in operation in the local timezone or not.
     *
     * @param bias_mode
     *     Whether to assume daylight savings is in effect.
     *
     *     - standard: create local time as though it were not
     *       daylight savings time
     *     - daylight_savings: create local time as though it were daylight
     *       savings time
     */
    datetime_t local_to_utc(timezone_bias_mode::type bias_mode) const
    {
        double timezone_bias = local_timezone_bias(bias_mode) / (24.*60.);
        double local_date_continuous = to_double(dt_);
        DATE utc_date = to_date(local_date_continuous + timezone_bias);

        return datetime_t(utc_date);
    }

    /**
     * Create local time version of this UTC time.
     *
     * Assuming this datetime is a UTC time (like that on an NTFS file system)
     * this creates a new version of it in the local timezone.  By default,
     * the adjustment is made based on the timezone rules that would have been
     * in effect at the UTC date this object represents.  However, the time can
     * also be converted as though it were on another date by passing another
     * date as an argument.
     * 
     * Typically the "as of" date is specified as the current time or possibly
     * the modification or creation date of an enclosing ZIP file.
     *
     * @param as_of_date
     *     Optional alternative date on which to base the timezone conversion.
     * @param utc_or_local
     *     Whether `as_of_date` is a local or UTC date.  Defaults to UTC.
     */
    datetime_t utc_to_local(
        datetime_t as_of_date=datetime_t(dt_invalid),
        locality::type utc_or_local=locality::utc) const
    {
        // if they didn't specify an AS OF date, use the current date which
        // will be UTC
        if (as_of_date.invalid())
        {
            as_of_date = *this;
            utc_or_local = locality::utc;
        }

        long timezone_bias = local_timezone_bias(as_of_date, utc_or_local);
        double timezone_bias_days = timezone_bias / (24.*60.);
        double utc_date_continuous = to_double(dt_);
        DATE local_date = to_date(utc_date_continuous - timezone_bias_days);

        return datetime_t(local_date);
    }

    /**
     * Create local time version of this UTC time, explicitly using standard
     * time or daylight savings.
     *
     * Assuming this datetime is a UTC time (like that on an NTFS file system)
     * this creates a new version of it in the local timezone.  Depending on
     * the argument passed, the adjustment is made as though daylight savings
     * were in operation in the timezone or not.
     *
     * @param bias_mode
     *     Whether to assume daylight savings.
     *
     *     - standard: create local time as though it were not
     *       daylight savings time
     *     - daylight_savings: create local time as though it were daylight
     *       savings time
     */
    datetime_t utc_to_local(timezone_bias_mode::type bias_mode) const
    {
        long timezone_bias = local_timezone_bias(bias_mode);
        double timezone_bias_days = timezone_bias / (24.*60.);
        double utc_date_continuous = to_double(dt_);
        DATE local_date = to_date(utc_date_continuous - timezone_bias_days);

        return datetime_t(local_date);
    }

    /** Convert to SYSTEMTIME struct.
     */
    bool to_systemtime( SYSTEMTIME *sysTime) const
    {
        int year,month,day,dow,hour,minute,second,ms;
        if (!datetime_from_oledate_( dt_, &year,  &month, &day, &dow, &hour, &minute, &second, &ms, cmBoth))
            return false;
        sysTime->wYear = (short)year;
        sysTime->wMonth = (short)month;
        sysTime->wDay = (short)day;
        sysTime->wDayOfWeek = (short)dow; // Sunday=0
        sysTime->wHour = (short)hour;
        sysTime->wMinute = (short)minute;
        sysTime->wSecond = (short)second;
        sysTime->wMilliseconds = (short)ms;
        return true;
    }

    /** Convert from a \e SYSTEMTIME struct.
     */
    bool from_systemtime(const SYSTEMTIME& src)
    {
        return oledate_from_datetime_( &dt_, src.wYear, src.wMonth, src.wDay, src.wHour, src.wMinute, src.wSecond, src.wMilliseconds, cmBoth);
    }

    /**
     * Convert from a @e SYSTEMTIME struct.
     *
     * @param source
     *     @e SYSTEMTIME being converted.
     * @param utc_mode
     *     Timezone conversion mode.
     * @param conversion_time
     *     Optional override date to use for calculating daylight/standard time.
     * @param utc_or_local
     *     Specify whether the override time is a UTC or local date.
     */
    bool from_systemtime(
        const SYSTEMTIME& source, utc_convert_mode::type utc_mode,
        const datetime_t& conversion_time=datetime_t(dt_invalid),
        locality::type utc_or_local=locality::local)
    {
        if (!from_systemtime(source))
            return false;

        switch(utc_mode)
        {
        case utc_convert_mode::none:
            break;
        case utc_convert_mode::local_to_utc:
            *this = local_to_utc(conversion_time, utc_or_local);
            break;
        case utc_convert_mode::utc_to_local:
            *this = utc_to_local(conversion_time, utc_or_local);
            break;
        }
        return true;
    }

    /**
     * Convert from a @e SYSTEMTIME struct.
     *
     * @param source
     *     @e SYSTEMTIME being converted.
     * @param utc_mode
     *     Timezone conversion mode.
     * @param bias_mode
     *     Specify whether the local time is daylight/standard time.
     */
    bool from_systemtime(
        const SYSTEMTIME& source, utc_convert_mode::type utc_mode,
        timezone_bias_mode::type bias_mode)
    {
        if (!from_systemtime(source))
            return false;

        switch(utc_mode)
        {
        case utc_convert_mode::none:
            break;
        case utc_convert_mode::local_to_utc:
            *this = local_to_utc(bias_mode);
            break;
        case utc_convert_mode::utc_to_local:
            *this = utc_to_local(bias_mode);
            break;
        }
        return true;
    }

    /** Convert from a \e FILETIME struct.
     */
    bool from_filetime(const FILETIME& src)
    {
        double ftd = (((__int64(src.dwHighDateTime) << 32 | src.dwLowDateTime)/(36000000000.)) - 2620920.)/24;
        return  set_check_range_( to_date(ftd));
    }

    /**
     * Convert from a @e FILETIME struct.
     *
     * @param source
     *     @e FILETIME being converted.
     * @param utc_mode
     *     Timezone conversion mode.
     * @param conversion_time
     *     Optional override date to use for calculating daylight/standard time.
     * @param utc_or_local
     *     Specify whether the override time is a UTC or local date.
     */
    bool from_filetime(
        const FILETIME& source, utc_convert_mode::type utc_mode,
        const datetime_t& conversion_time=datetime_t(dt_invalid),
        locality::type utc_or_local=locality::local)
    {
        if (!from_filetime(source))
            return false;

        switch(utc_mode)
        {
        case utc_convert_mode::none:
            break;
        case utc_convert_mode::local_to_utc:
            *this = local_to_utc(conversion_time, utc_or_local);
            break;
        case utc_convert_mode::utc_to_local:
            *this = utc_to_local(conversion_time, utc_or_local);
            break;
        }
        return true;
    }

    /**
     * Convert from a @e FILETIME struct.
     *
     * @param source
     *     @e FILETIME being converted.
     * @param utc_mode
     *     Timezone conversion mode.
     * @param bias_mode
     *     Specify whether the local time is daylight/standard time.
     */
    bool from_filetime(
        const FILETIME& source, utc_convert_mode::type utc_mode,
        timezone_bias_mode::type bias_mode)
    {
        if (!from_filetime(source))
            return false;

        switch(utc_mode)
        {
        case utc_convert_mode::none:
            break;
        case utc_convert_mode::local_to_utc:
            *this = local_to_utc(bias_mode);
            break;
        case utc_convert_mode::utc_to_local:
            *this = utc_to_local(bias_mode);
            break;
        }
        return true;
    }
    
    /** Convert to a \e FILETIME struct.
     */
    bool to_filetime( FILETIME *filetime) const
    {
        double val = ((to_double(dt_)  * 24.) + 2620920.)*(36000000000.) ;

        __int64 llval = __int64(val);
        filetime->dwHighDateTime = long (llval >> 32);
        filetime->dwLowDateTime = long (llval);
        return val > 0;
    }

    /** Convert from a \e tm struct.
     */
    bool from_tm(const struct tm &tm_time)
    {
        return from_tm_( tm_time, &dt_, cmBoth);
    }

    /**
     * Convert from a @e tm struct.
     * @param tm_time
     *     @e tm struct being converted.
     * @param utc_mode
     *     Timezone conversion mode.
     * @param conversion_time
     *     Optional override date to use for calculating daylight/standard time.
     *     Only used if the information cannot be derived from the @e tm struct.
     * @param utc_or_local
     *     Whether the optional conversion date is UTC or local.
     */
    bool from_tm(
        const struct tm &tm_time, utc_convert_mode::type utc_mode,
        datetime_t conversion_time=datetime_t(dt_invalid),
        locality::type utc_or_local=locality::local)
    {
        if(!from_tm(tm_time))
            return false;

        switch(utc_mode)
        {
        case utc_convert_mode::none:
            break;
        case utc_convert_mode::local_to_utc:
            // Take advantage of tm_isdst to work out dst mode!
            //
            // XXX: This doesn't use the conversion_time at all. No, I don't
            // know why we have this behaviour
            if (tm_time.tm_isdst == 0)
            {
                *this = local_to_utc(timezone_bias_mode::standard);
            }
            else if (tm_time.tm_isdst > 0)
            {
                *this = local_to_utc(timezone_bias_mode::daylight_saving);
            }
            else
            {
                *this = local_to_utc(conversion_time, utc_or_local);
            }
            break;
        case utc_convert_mode::utc_to_local:
            *this = utc_to_local(conversion_time, utc_or_local);
            break;
        }

        return true;
    }

    /**
     * Convert from a @e tm struct.
     *
     * @param tm_time
     *     @e 'tm' struct being converted.
     * @param utc_mode
     *     Timezone conversion mode.
     * @param daylight_hint
     *     Strategy to use for daylight savings time if it cannot be derived
     *     from the @e tm struct.
     */
    bool from_tm(
        const struct tm &tm_time, utc_convert_mode::type utc_mode,
        timezone_bias_mode::type daylight_hint)
    {
        if(!from_tm(tm_time))
            return false;

        switch(utc_mode)
        {
        case utc_convert_mode::none:
            break;
        case utc_convert_mode::local_to_utc:
            // Take advantage of tm_isdst to work out dst mode!
            //
            // XXX: This overrides the specified conversion. No, I don't
            // know why we have this behaviour
            if (tm_time.tm_isdst == 0)
            {
                *this = local_to_utc(timezone_bias_mode::standard);
            }
            else if (tm_time.tm_isdst > 0)
            {
                *this = local_to_utc(timezone_bias_mode::daylight_saving);
            }
            else
            {
                *this = local_to_utc(daylight_hint);
            }
            break;
        case utc_convert_mode::utc_to_local:
            *this = utc_to_local(daylight_hint);
            break;
        }

        return true;
    }


    /**
     * Convert from a @e time_t value.
     *
     * @param source
     *     Unix time to convert.
     * @param utc_mode
     *     Timezone conversion mode.  By default converts a UTC time_t into
     *     a local datetime.
     * @param conversion_time
     *     Optional override date to use for calculating daylight/standard time.
     * @param utc_or_local
     *     Specify whether the override time is a UTC or local date.
     */
    bool from_unixtime(
        time_t source,
        utc_convert_mode::type utc_mode=utc_convert_mode::utc_to_local,
        const datetime_t& conversion_time=datetime_t(dt_invalid),
        locality::type utc_or_local=locality::local)
    {
        FILETIME ft;
        __int64 ll = (__int64(source) * 10000000L) + 116444736000000000L;
        ft.dwLowDateTime = (DWORD) ll;
        ft.dwHighDateTime = (DWORD)(ll >>32);
        return from_filetime(ft, utc_mode, conversion_time, utc_or_local);
    }

    /**
     * Convert from a @e time_t value.
     *
     * @param source
     *     Unix time being converted.
     * @param utc_mode
     *     Timezone conversion mode.
     * @param bias_mode
     *     Specify whether the local time is daylight/standard time.
     */
    bool from_unixtime(
        time_t source, utc_convert_mode::type utc_mode,
        timezone_bias_mode::type bias_mode)
    {
        FILETIME ft;
        __int64 ll = (__int64(source) * 10000000L) + 116444736000000000L;
        ft.dwLowDateTime = (DWORD) ll;
        ft.dwHighDateTime = (DWORD)(ll >>32);
        return from_filetime(ft, utc_mode, bias_mode);
    }

    /**
     * Convert to a @e time_t value.
     *
     * @param unix_time_out
     *     Destination of conversion result.
     * @param utc_mode
     *     Timezone conversion mode.
     * @param conversion_time
     *     Optional override date to use for calculating daylight/standard time.
     * @param utc_or_local
     *     Specify whether the override time is a UTC or local date.
     */
    bool to_unixtime(
        time_t* unix_time_out,
        utc_convert_mode::type utc_mode=utc_convert_mode::local_to_utc,
        const datetime_t& conversion_time=datetime_t(dt_invalid),
        locality::type utc_or_local=locality::local) const
    {
        datetime_t dtval;

        switch(utc_mode)
        {
        case utc_convert_mode::none:
            dtval = *this;
            break;
        case utc_convert_mode::local_to_utc:
            dtval = local_to_utc(conversion_time, utc_or_local);
            break;
        case utc_convert_mode::utc_to_local:
            dtval = utc_to_local(conversion_time, utc_or_local);
            break;
        }

        FILETIME ft;
        if (!dtval.to_filetime(&ft))
            return false;

        *unix_time_out =
            time_t(((__int64(ft.dwHighDateTime) << 32 | ft.dwLowDateTime) -
            116444736000000000L)/10000000L);
        return true;
    }

    /**
     * Convert to a @e time_t value.
     *
     * @param unix_time_out
     *     Destination of conversion result.
     * @param utc_mode
     *     Timezone conversion mode.
     * @param conversion_time
     *     Optional override date to use for calculating daylight/standard time.
     * @param utc_or_local
     *     Specify whether the override time is a UTC or local date.
     */
    bool to_unixtime(
        time_t* unix_time_out, utc_convert_mode::type utc_mode,
        timezone_bias_mode::type bias_mode) const
    {
        datetime_t dtval;

        switch(utc_mode)
        {
        case utc_convert_mode::none:
            dtval = *this;
            break;
        case utc_convert_mode::local_to_utc:
            dtval = local_to_utc(bias_mode);
            break;
        case utc_convert_mode::utc_to_local:
            dtval = utc_to_local(bias_mode);
            break;
        }

        FILETIME ft;
        if (!dtval.to_filetime(&ft))
            return false;

        *unix_time_out =
            time_t(((__int64(ft.dwHighDateTime) << 32 | ft.dwLowDateTime) -
            116444736000000000L)/10000000L);
        return true;
    }

    /**
     * Calculate the local timezone's offset from UTC on the given date, in
     * minutes.
     *
     * This offset is called the bias and is the number of minutes to subtract
     * from a UTC time to make a local one.
     *
     * @param dt
     *     The date and time for which to calculate the bias.
     * @param is_utc
     *     Whether to interpret the date as UTC or local.
     *
     * @todo A better way might be to make UTCness a fundamental property of
     *       times at construction so they know whether they are URL or local
     *       themselves.
     */
    static long local_timezone_bias(datetime_t dt, locality::type utc_or_local)
    {
        TIME_ZONE_INFORMATION tzi;
        ::GetTimeZoneInformation(&tzi);

        long baseBias = tzi.Bias;

        // if we've even got both time zones set, we have to choose which is
        // active...
        if ((tzi.DaylightDate.wMonth != 0) && (tzi.StandardDate.wMonth != 0))
        {
            // all local standard time/daylight savings time rules are based on
            // local-time, so add the base bias FIRST
            if (utc_or_local == locality::utc)
                dt -= (baseBias / (24.*60.));

            SYSTEMTIME sysTime;
            if (!dt.to_systemtime(&sysTime))
                throw datetime_exception("Invalid Date");

            bool DSTbeforeLST =
                tzi.DaylightDate.wMonth < tzi.StandardDate.wMonth;

            bool afterDaylightStarts =
                tz_on_or_after_in_year(sysTime, tzi.DaylightDate);
            bool afterStandardStarts =
                tz_on_or_after_in_year(sysTime, tzi.StandardDate);

            if (((afterDaylightStarts== afterStandardStarts)!= DSTbeforeLST))
                return baseBias + tzi.DaylightBias;
        }

        return baseBias + tzi.StandardBias;
    }

    /**
     * Calculate the local timezone's offset from UTC in minutes.
     *
     * The value depends on the argument to the function which specifies
     * whether to assume daylight savings is in operation in the local timezone.     * occuring outside of daylight savings time.
     */
    static long local_timezone_bias(timezone_bias_mode::type dst_state)
    {
        TIME_ZONE_INFORMATION tzi;
        ::GetTimeZoneInformation(&tzi);

        switch (dst_state)
        {
        case timezone_bias_mode::standard:
            return tzi.Bias + tzi.StandardBias;
        case timezone_bias_mode::daylight_saving:
            return tzi.Bias + tzi.DaylightBias;
        default:
            COMET_ASSERT(!"Invalid timezone daylight savings state");
        }
    }

protected:

    /** Compares two SYSTEMTIME values to decide if the second is after (or on) the
     * first.
     * If the year is supplied, the two dates are assumed static, otherwise it
     * computes the proper day-of-week instance (like last Sunday in October) for
     * the specified test year.  See the encoding rules documented with
     * TIME_ZONE_INFORMATION
     */
    static bool tz_on_or_after_in_year(SYSTEMTIME testST, SYSTEMTIME tziST)
    {
        // assume month check first...
        long cmp = testST.wMonth - tziST.wMonth;
        if (cmp!=0)
            return cmp > 0;

        SYSTEMTIME absST;

        // if year is given, then the specified date is already exact...
        if (tziST.wYear != 0)
        {
            // first test the year...
            cmp = testST.wYear - tziST.wYear;
            if (cmp !=0)
                return cmp > 0;
            // carry on with the exact day known
            absST = tziST;
        }
        else
        {
            // compute the appropriate day from the specified instance of the set day-of-week
            // use the testST's year for the year in the calculation
            tz_convert_relative_dow_to_absolute(testST, tziST, &absST);
        }

        // month same... check day/hour/minute/second/millisecond
        if ((cmp = testST.wDay  - absST.wDay)==0)
            if ((cmp = testST.wHour - absST.wHour)==0)
                if ((cmp = testST.wMinute - absST.wMinute)==0)
                    if ((cmp = testST.wSecond - absST.wSecond)==0)
                        cmp = testST.wMilliseconds - absST.wMilliseconds;
        return cmp >= 0;
    }

    // Computes the proper day-of-week instance (like last Sunday in October) for the
    // specified test year.   See the encoding rules documented with TIME_ZONE_INFORMATION.
    // This ASSUMES that testST.wMonth == tziST.wMonth
    static void tz_convert_relative_dow_to_absolute(const SYSTEMTIME &testST , const SYSTEMTIME &tziST, SYSTEMTIME *absST)
    {
        COMET_ASSERT(testST.wMonth == tziST.wMonth);

        // Set up the absolute date except for wDay, which we must find
        absST->wYear = testST.wYear;          // year is only valid in the testST
        int month = absST->wMonth = tziST.wMonth;
        absST->wDayOfWeek = tziST.wDayOfWeek;

        absST->wHour = tziST.wHour;
        absST->wMinute = tziST.wMinute;
        absST->wSecond = tziST.wSecond;
        absST->wMilliseconds = tziST.wMilliseconds;

        // Find a day of the month that falls on the same day of the week as
        // the transition.

        // If test day is the 29th of the month (testST.wDay = 29) and today
        // is a Tuesday (testST.wDayOfWeek = 2) and the transition occurs on
        // Sunday (testST.wDayOfWeek = 0) we compute absDay = 29 + 0 + 7 -
        // 2, giving the 34th

        // then adjust that to a day of month adjustment
        long absDay = ((testST.wDay + tziST.wDayOfWeek + (7-1) - testST.wDayOfWeek) % 7) +1;

        // now multiply this time the "which DOW" setting from the TZI
        // (1 = first, 5 = last)
        // add the requisite number of weeks to the base point
        absDay += (7 * (tziST.wDay - 1));

        // and if we exceeded the number of days in the month, back up by a
        // week (this handles the 5=last situation)

        int daysInMonth = days_in_month( absST->wYear,  month);

        if (absDay > daysInMonth)
            absDay -= 7;

        absST->wDay = (unsigned short)absDay;
    }

public:

    /** Format datetime_t as bstr.
     * \param flags Format (can be or-ed). All format_flags are valid.
     * \param locale Locale ID (default to User Default.
     */
    bstr_t format( format_flags flags = ff_default , LCID locale = LOCALE_USER_DEFAULT) const
    {
        bstr_t strDate;
        if (!good())
        {
            return strDate;
        }
        VarBstrFromDate(dt_, locale, flags, strDate.out()) | raise_exception;
        return strDate;
    }

    /** Format datetime_t as basic_string.
     * \param fmt See system documentation for strftime for details.
     */
    template<typename CHAR>
    std::basic_string<CHAR> format( const std::basic_string<CHAR> &fmt ) const
    {
        return format(fmt.c_str());
    }

    /** Format datetime_t as basic_string.
     * \param fmt See system documentation for strftime for details.
     */
    template<typename CHAR>
    std::basic_string<CHAR> format( const CHAR *fmt ) const
    {
        if (!good())
        {
            return std::basic_string<CHAR>();
        }
        struct tm src;
        if(!to_tm_( dt_, &src, NULL))
            throw datetime_exception("Invalid Date");

        typename auto_buffer_t<CHAR>::size_type capacity = 50;
        auto_buffer_t<CHAR> buf(capacity);
        size_t ret;
        while( (ret = str_formattime( buf.get() , capacity, fmt, &src ))==0 && capacity < 1024)
        {
            capacity += 50;
            buf.resize(capacity);
        }
        if(ret == 0)
            buf.at(0)='\0';

        return std::basic_string<CHAR>(buf.get(), ret);
    }

    /// Detach the raw date from the class.
    DATE detach()
    {
        DATE val = dt_;
        dt_ = 0.;
        return val;
    }

    /// Detach the raw date from the class.
    static DATE detach( datetime_t &dt)
    {
        return dt.detach();
    }

    /// Stream operator.
    friend
    std::basic_ostream<char> &operator<<(std::basic_ostream<char> &os, const datetime_t &val)
    {
        os << val.format();
        return os;
    }

    /// Stream operator.
    friend
    std::basic_ostream<wchar_t> &operator<<(std::basic_ostream<wchar_t> &os, const datetime_t &val)
    {
        os << val.format();
        return os;
    }

private:
};
#undef COMET_DIVMOD_
//@}

}
#endif
