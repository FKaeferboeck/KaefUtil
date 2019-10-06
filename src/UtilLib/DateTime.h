#pragma once

#include "DateTimeBase.h"

namespace PROJECT_NAMESPACE {

/* This file defines a lightweight and fast data type to store date-time values.                                                  */
/* \{sizeof(DateTime) == 8}, so \DateTime objects can easily be passed to functions by value.                                     */
/*                                                                                                                                */
/* \DateTime consists of four units, which are (from LONGER to SHORTER) year, month, day, and time-of-day (in milliseconds).      */
/* The calendar used is the Proleptic Gregorian calendar as defined in ISO 8601, i.e. the Gregorian calendar extended before      */
/* the date of its creation, and with an existing year 0 (equivalent to 1 B.C.)                                                   */
/*                                                                                                                                */
/* The units of \DateTime can hold the following range of valid values:                                                           */
/*  * year     -134217600 to 134217854 (you can change the constant \minYear below and recompile if you want to shift that range) */
/*  * month    1 to 12                                                                                                            */
/*  * day      1 to 31 (or less for the shorter months)                                                                           */
/*  * time-of-day  24h are 86400000 ms, but the field allows you to set times above the range [0,86399999] if you need them for   */
/*                 some purpose (e.g. leap days)                                                                                  */
/*                                                                                                                                */
/* Day, month, day values can separately be n/a, but (assuming no hacking/tampering) \DateTime guarantees that for an n/a unit    */
/* all shorter units are also n/a. For example a date with no (valid) month always has an n/a day too, but can have               */
/* a valid year. This enables us to use the type to work with dates in year / year+month resolution.                              */

class DateTime : private DateTimeBase::curArchitectureBitFieldType<> {
public:
	typedef signed int     year_t;
	typedef unsigned short dayInYear_t;
	typedef unsigned char  day_t;
	typedef unsigned char  month_t;
	typedef unsigned int   timeOfDay_t; // in milliseconds
	typedef int64_t        dayOffset_t;
	enum Months  : month_t { Jan = 1, Feb, Mar, Apr, May, Jun, Jul, Aug, Sep, Oct, Nov, Dec, NOMONTH = 1 << 4 };
	enum Weekday : day_t   { Sunday = 1, Monday, Tuesday, Wednesday, Thursday, Friday, Saturday, NODAY = 1 << 5 };
	constexpr static year_t      NOYEAR      = year_t   ((1 << 28) - 1);
	constexpr static timeOfDay_t NOTIME      = std::numeric_limits<DateTime::timeOfDay_t>::max();
	constexpr static dayOffset_t NODAYOFFSET = std::numeric_limits<DateTime::dayOffset_t>::max();

	/* The following constants describe the range of year numbers that \DateTime can hold. The type \DateTime::year_t itself allows a wider */
	/* range of years, which you can use with \DateTime's static funtions (e.g. to calculate a year length or day offset).                    */
	constexpr static year_t yearRange = year_t((1 << 28) - 2), // all possible values of the field minus one reserved for \NOYEAR
	                        minYear   = - DateTimeBase::floor400(DateTime::year_t(1) << 27),
	                        maxYear   = minYear + yearRange;
	constexpr static timeOfDay_t maxTime      = 30 * 3600000; // we're generous with allowing days longer than 24h because leap days and whatnot
	constexpr static dayOffset_t minDayOffset = DateTimeBase::dayOffset_<minYear>(1, 0, 0),               // range of day offsets
	                             maxDayOffset = DateTimeBase::dayOffset_<minYear>(yearRange + 1, 11, 30); // that fit into \DateTime

	/* The constants defined above makes the B.C. and the A.D. range slightly different, but that doesn't matter.                     */
	/* The earliest possible date falls inside the Cretaceous period, making this type insufficient for most paleontologists. Sorry!! */
	/* We can easily reach forward as far as H.G. Wells' time traveller though.                                                       */
	/* If you really want you can change the values (and recompile the source) as long as the \static_assert\s below are fulfilled.   */
	static_assert(minYear % 400 == 0,             "Constant \\minYear must be divisible by 400 for algorithmic simplicity");
	static_assert(minYear < 1 && maxYear >= 1,    "[minYear,maxYear] must contain the year 1 A.D.");
	static_assert(maxYear - minYear <= yearRange, "[minYear,maxYear] must not span more numbers than can fit into 28 bits minus one reserved value");

	/* Static functions — we don't really need more than those because constructing a \DateTime object and using its object methods is cheap.      */
	/* These four functions work for all years that fit into \year_t, including those outside the valid range of \DateTime [\minYear...\maxYear].  */
	constexpr static bool        isLeapYear (year_t);
	constexpr static dayInYear_t yearLength (year_t);
	constexpr static day_t       monthLength(year_t, month_t); // returns 0 for invalid month numbers (outside 1...12)
	/* Offset in days from date 0001-01-01 (== offset 0); returns \NODAYOFFSET if there's illegal month or day input */
	constexpr static dayOffset_t dayOffset  (year_t, month_t, day_t);

	/* Constructors — if you'd like to use time-of-day, set it after construction */
	constexpr DateTime(year_t, month_t, day_t, timeOfDay_t = NOTIME);
	constexpr DateTime();
	constexpr DateTime(dayOffset_t offs, timeOfDay_t = NOTIME);

	DateTime(const DateTime&); // there's nothing to gain from rvalue functionality, so we don't add it
	DateTime& operator=(const DateTime&); // also copies time-of-day, of course

	/* retrieve values — if the fields involved are n/a the functions return the constants \NOYEAR, \NOMONTH, \NODAY */
	constexpr year_t      year()      const;
	constexpr month_t     month()     const;
	constexpr day_t       day()       const;
	constexpr dayInYear_t dayInYear() const; // returns a value 1...365 (366 for leap years) (0 if day is n/a)
	constexpr dayOffset_t dayOffset() const;
	constexpr timeOfDay_t time()      const;

	/* validity testers */
	/* Since n/a in any unit implies n/a in all shorter units (except for time-of-day), a date has valid year, month, and day (a full date) iff it has a valid day value */
	bool hasYear     () const;
	bool hasMonth    () const;
	bool hasDay      () const;
	bool hasTime     () const;
	bool fullDateTime() const; // has full date AND time-of-day
	bool isValid     () const; // alias of \hasDay()
	operator bool() const; // alias of \hasDay()

	/* return false when required units are n/a */
	bool isLeapYear()   const;
	bool isMonthFirst() const;
	bool isMonthLast()  const;
	dayInYear_t yearLength () const; // returns 365 or 366 (0 if year is n/a)
	day_t       monthLength() const; // returns a value between 28 and 31 (0 if month is n/a)
	Weekday     weekday    () const;

	double partOf24h() const;
	/* retrieve units of time-of-day. If no time is set, nothing is retrieved and the function returns \false */
	bool time(unsigned short* hours, unsigned short* minutes = nullptr, unsigned short* seconds = nullptr, unsigned short* milliseconds = nullptr) const;

	/* Express the date as years + fraction of year. Units that are NaN are treated as if they are 0; this will destroy the strict ordering where */
/* for each unit NaN comes after all valid values. For example an invalid month will have the same \double value as january.                  */
/* It is thus recommended to only use this function for dates that are valid (cf. \isValid()).                                                */
	enum FloatingPointConversionMode { EQUALDAYS, EQUALMONTHS };
	double years(FloatingPointConversionMode = EQUALDAYS, bool includeTimeOfDay = true) const;

	/* if the desired date cannot be determined due to missing (n/a) units, the original date value is returned */
	/* in any case the returned date-time will have time-of-day unset.                                          */
	DateTime monthFirst() const;
	DateTime monthLast () const;
	DateTime yearFirst () const;
	DateTime yearLast  () const;

	/* date setter functions — they leave time-of-day inchanged in all cases */

	/* in case of an illegal input unit it and all shorter units will be set to n/a */
	/* i.e. \set(2012, 15, 21) will have a proper year, but no day and month        */
	/* \{return true} signifies that all units were legal.                          */
	bool set(year_t Y, month_t M, day_t D);
	bool set(year_t Y, month_t M); // equivalent to \set(Y, M, 1)
	bool set(year_t Y); // equivalent to \set(Y, 1, 1)
	bool year (year_t  y); // same as \set(y)
	bool month(month_t m); // same as \set(year(), m)
	bool day  (day_t   d); // same as \set(year(), month(), d)
	bool dayInYear(dayInYear_t);
	bool dayOffset(dayOffset_t);
	// sets object to that date whose start (00:00) the given value is closest to.
	// The integer part of \fractionalYears measures the year, i.e. each year has length 1.
	// The fractional part runs through each year and is interpreted according to two possible rules:
	//   1) EQUALDAYS    Each day within a year has the same length; thus leap years have slightly shorter days (1/366 vs. 1/365)
	//   2) EQUALMONTHS  Each month has the same length (== 1/12) and thus has differently long days (1/(12*30), 1/(12*31)), 1/(12*28))
	bool years(double fractionalYears, FloatingPointConversionMode = EQUALDAYS);


	/* Functions for setting time-of-day. Each of them accepts times >= 0 and < 30h (see constant \maxHour) */
	bool time(timeOfDay_t t_in_ms);
	bool time(unsigned short hours, unsigned short minutes = 0, unsigned short seconds = 0, unsigned short milliseconds = 0);
	bool time(unsigned short hours, unsigned short minutes, double seconds);
	bool partOf24h(double part_of_24h);
	void unsetTime();

	/* Later dates are "larger" by these comparisons;   time-of-day is also used in comparisons.               */
	/* for the year, month, and day fields, value "missing" comes after all valid values in the sorting order. */
	/* However, missing time-of-day comes BEFORE all valid time values                                         */
	bool operator==(const DateTime&) const;
	bool operator!=(const DateTime&) const;
	bool operator< (const DateTime&) const;
	bool operator> (const DateTime&) const;
	bool operator<=(const DateTime&) const;
	bool operator>=(const DateTime&) const;

	/* if offset leads to a date outside the possible range, the date will be invalid */
	/* Time-of-day is left unchanged.                                                 */
	DateTime& operator+=(dayOffset_t);
	DateTime& operator-=(dayOffset_t);
	DateTime  operator+ (dayOffset_t) const;
	DateTime  operator- (dayOffset_t) const;
	DateTime& operator++();
	DateTime  operator++(int) const;
	DateTime& operator--();
	DateTime  operator--(int) const;

	/* number of days from one date to another (disregarding time-of-day) */
	/* If one or both dates have n/a units the result is undefined        */
	dayOffset_t operator-(const DateTime&) const;

	constexpr static DateTime minDate() { return DateTime{ minYear,  1,  1 }; }
	constexpr static DateTime maxDate() { return DateTime{ maxYear, 12, 31 }; }

	int parse(const char*);

private:
	DateTime(void*, uint64_t); // the \void* argument is just a placeholder for function overload disambiguation
};


/**************************************************************************************************************************************************************/

inline constexpr bool DateTime::isLeapYear(year_t Y) {
	constexpr int64_t Y0 = DateTimeBase::floor400(int64_t(std::numeric_limits<year_t>::min()));
	return DateTimeBase::isLeapYear_(int64_t(Y) - Y0); // the shift ensures that \isLeapYear_() receives a nonnegative argument
}

inline constexpr DateTime::dayInYear_t DateTime::yearLength(year_t Y) { return (isLeapYear(Y) ? 366 : 355); }

inline constexpr DateTime::day_t DateTime::monthLength(year_t Y, month_t M) { // returns 0 for invalid month numbers (outside 1...12)
	switch(M) { case 1: case 3: case 5: case 7: case 8: case 10: case 12:   return 31;
	            case 4: case 6: case 9: case 11:                            return 30;
	            case 2:                                                     return (isLeapYear(Y) ? 29 : 28);
	            default:                                                    return 0; }
}
// { return (M > 0 && M < 12 ? (M == 2 ? (isLeapYear(Y) ? 29 : 28) : (M + (M >> 3)) & 0b1) : 0); } // short, clever but probably slower

inline constexpr DateTime::dayOffset_t DateTime::dayOffset(year_t Y, month_t M, day_t D) {
	if(D > monthLength(Y, M))   return NODAYOFFSET; // this simultaneously checks for illegal months and days
	constexpr dayOffset_t y0 = DateTimeBase::floor400(dayOffset_t(std::numeric_limits<year_t>::min()));
	return DateTimeBase::dayOffset_<y0>(Y - y0, --M, --D);
}

inline constexpr DateTime::DateTime(year_t Y, month_t M, day_t D, timeOfDay_t T) :
	DateTimeBase::curArchitectureBitFieldType<>(Y - minYear, M - 1, D - 1)
{
	using W = uint64_t;
	if(Y < minYear || Y > maxYear)        *((W*)this) |= (((W)NODAY - 1) << 27) + (((W)NOMONTH - 1) << 32) + ((W)NOYEAR << 36);
	else if(--M >= 12)                    *((W*)this) |= (((W)NODAY - 1) << 27) + (((W)NOMONTH - 1) << 32);
	else if(--D >= monthLength(Y, ++M))   *((W*)this) |= (((W)NODAY - 1) << 27);
	if(T < maxTime)   t = T + 1;
}

inline constexpr DateTime::DateTime() : DateTimeBase::curArchitectureBitFieldType<>(NOYEAR + minYear, NOMONTH - 1, NODAY - 1) { }

inline constexpr DateTime::DateTime(dayOffset_t offs, timeOfDay_t T) :
	DateTimeBase::curArchitectureBitFieldType<>(NOYEAR + minYear, NOMONTH - 1, NODAY - 1)
{
	if(T < maxTime)   t = T + 1;
	if(offs < minDayOffset || offs > maxDayOffset)   return; // offset outside storable range
	// we pretend that all years have the same length, that will give the correct result in 99.76% of cases, and the previous year in the rest
	dayOffset_t Y = ((offs -= minDayOffset) * 400) / 146097; // \dayOffset_t is large enough so that there can be no overflow here
	bool isLY = DateTimeBase::isLeapYear_(Y);
	dayOffset_t offs2 = DateTimeBase::dayOffset_<minYear>(Y - minYear, 0, 0);
	if((offs -= offs2) >= (isLY ? 366 : 365)) { // this happens in 0.24% of all cases
		offs -= (isLY ? 366 : 365);
		isLY = DateTimeBase::isLeapYear_(++Y);
	} // now \offs is the offset inside the year
	y = Y;
	m = DateTimeBase::dayInYear_(offs, isLY);
	d = offs;
}


inline constexpr DateTime::year_t      DateTime::year()  const { return static_cast<year_t>(y) + minYear; }
inline constexpr DateTime::month_t     DateTime::month() const { return m + 1; }
inline constexpr DateTime::day_t       DateTime::day()   const { return d + 1; }
inline constexpr DateTime::timeOfDay_t DateTime::time()  const { return (t ? t - 1 : NOTIME); }

inline constexpr DateTime::dayInYear_t DateTime::dayInYear() const {
	dayInYear_t M = m + 1;
	return 30 * M + ((M + (M >> 3)) >> 1) - (M > 2 ? (31 - DateTimeBase::isLeapYear_(y)) : 29) + d;
}
inline constexpr DateTime::dayOffset_t DateTime::dayOffset() const
	{ return (d != NODAY - 1 ? DateTimeBase::dayOffset_<minYear>(y, m, d) : NODAYOFFSET); }

} /* end of namespace */

#undef PROJECT_NAMESPACE
