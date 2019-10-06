#include "DateTime.h"
#include "Version.h"

#include <limits>
#include <cmath>

using namespace PROJECT_NAMESPACE;

using MT = DateTime::month_t;
using DT = DateTime::day_t;
using DY = DateTime::dayInYear_t;
using DO = DateTime::dayOffset_t;

namespace {
	//constexpr DM monthLengths  []{ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31, 0, 0, 0, 0 };
	//constexpr DM monthLengthsLY[]{ 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31, 0, 0, 0, 0 };
	constexpr DY monthBegins   []{ 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };
	constexpr DY monthBeginsLY []{ 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 };
	constexpr DY daysToLYEnd   []{ 366, 335, 306, 275, 245, 214, 184, 153, 122, 92, 61, 31    };
	constexpr DO NODAYOFFSET = std::numeric_limits<DateTime::dayOffset_t>::max();
	constexpr uint64_t INVALID = 0xFFFFFFFFF8000000; // all fields at ~0 except time-of-day, which is 0
	constexpr uint64_t NO_Y = 0xFFFFFFFFF8000000, NO_M = 0x0000000FF8000000, NO_D = 0x00000000F8000000;
	using _W = uint64_t;
	using _CW = const uint64_t;
	constexpr DateTime::timeOfDay_t maxH = DateTime::maxTime / (60 * 60 * 1000);
} /* end of anonymous namespace */



bool DateTime::hasYear     () const { return y != (unsigned int)NOYEAR;  }
bool DateTime::hasMonth    () const { return m != (unsigned int)NOMONTH - 1; }
bool DateTime::hasDay      () const { return d != (unsigned int)NODAY   - 1; }
bool DateTime::hasTime     () const { return t != 0; }
bool DateTime::fullDateTime() const { return d != (unsigned int)NODAY - 1 && t != 0; }
bool DateTime::isValid     () const { return d != (unsigned int)NODAY - 1; }
DateTime::operator bool    () const { return d != (unsigned int)NODAY - 1; }

bool DateTime::isLeapYear() const { return DateTimeBase::isLeapYear_(y); }

DateTime::dayInYear_t DateTime::yearLength() const
	{ return (y != NOYEAR ? (DateTimeBase::isLeapYear_(y) ? 366 : 365) : 0); }

DateTime::day_t DateTime::monthLength() const
	{ return monthLength(y + 1, m + 1); }

DateTime::Weekday DateTime::weekday() const {
	if(d == NODAY - 1)   return Weekday::NODAY;
	constexpr unsigned int offs = ((-minDayOffset) % 7) + 1;
	return static_cast<Weekday>(((DateTimeBase::dayOffset_<0>(y, m, d) + offs) % 7) + 1);
}


DateTime::DateTime(const DateTime& DT) { *(_W*)this = reinterpret_cast<_CW&>(DT); }

DateTime::DateTime(void*, uint64_t val) { *(_W*)this = val; }

DateTime& DateTime::operator=(const DateTime& DT) { *(_W*)this = reinterpret_cast<_CW&>(DT);     return *this; }


double DateTime::partOf24h() const { return (t - 1) / 86400000.; }

/*
double DateTime::years(FloatingPointConversionMode mode, bool includeTimeOfDay) const {
	unsigned int x, x2;
	if((x = m) == NOMONTH)   return ((x2 = y) == NOYEAR ? 0 : double(x2) + minYear);
	double e = 0;
	if(d != NODAY) {
		if(includeTimeOfDay && t)   e += (t - 1) / 86400000.;
		e += d;
	}
	x2 = y;
	if(mode == EQUALDAYS) {
		if(isLeapYear_(x2))
					 (e += monthBeginsLY[x]) /= 366;
		else   (e += monthBegins  [x]) /= 365;
	} else {
		e /= (isLeapYear_(x2) ? monthLengthsLY : monthLengths)[x]; // \e is now fraction of month
		(e += x) /= 12;
	}
	return e += double(x2) + minYear;
}


#include <iostream>
bool DateTime::years(double t, FloatingPointConversionMode mode, bool includeTimeOfDay) {
	using std::floor;
	if(t < double(minYear) - 2 || t > double(maxYear) + 2)   return false; // certainly outside range; otherwise the year can at least be stored by \year_t
	year_t Y;     DY x, M;     DM z;
	const DY* begins = nullptr;
	if(!includeTimeOfDay)   switch(mode) {
		case EQUALMONTHS:
			Y = static_cast<year_t>(floor(t + .5 / 372)); // the last day of the year is always an 31th of a 12th (== 1/372) —> year unambiguously determined
			if(Y < minYear || Y > maxYear)   return false;
			y = Y - minYear;
			(t -= Y) *= 12;
			M = static_cast<DY>(floor(t + .5 / 31)); // try with the longest month; we may mistakenly obtain the previous month
			z = (isLeapYear_(Y) ? monthLengthsLY : monthLengths)[M];
			if((x = static_cast<DY>(floor(((t -= M) *= z) + .5))) == z)   { m = ++M;     d = 0; }
			else                                                          { m =   M;     d = x; }
			break;
		case EQUALDAYS:
			t -= (Y = static_cast<year_t>(floor(t + .5 / 366))); // try with leap year first — they have shorter days, so perhaps we get the previous year instead
			if(isLeapYear_(Y)) { M = 366;     begins = monthBeginsLY; }
			else               { M = 365;     begins = monthBegins;   }
			if((x = static_cast<DY>(floor((t *= M) + .5))) == M) { // Correction: it isn't the last day of a leap year, it's the first day of a year after a non-leap year.
				if(++Y < minYear || Y > maxYear)   return false;
				y = Y - minYear;     m = 0;     d = 0;     t = 0;
				return true;
			}
			if(Y < minYear || Y > maxYear)   return false;
			y = Y - minYear;
			M = x >> 5;
			if(x >= begins[M + 1])   ++M; // cf. \dayInYear(dayInYear_t)
			d = x - begins[M];
			m = M;
		}
	else { // this should be a lot simpler: we put the date where it falls, damn the rounding errors
		Y = static_cast<year_t>(floor(t));
		if(Y < minYear || Y > maxYear)   return false;
		y = Y - minYear;
		switch(mode) {
		case EQUALMONTHS:
			m = z = static_cast<DM>(floor((t -= Y) *= 12));
			d = z = static_cast<DM>(floor((t -= z) *= monthLength(Y, z)));
			partOf24h(t -= z);     break;
		case EQUALDAYS:
			dayInYear(M = static_cast<DY>(floor((t -= Y) *= yearLength(Y))));
			partOf24h(t -= M);
		}
	}
	return true;
}*/



bool DateTime::time(unsigned short* H, unsigned short* M, unsigned short* S, unsigned short* L) const {
	unsigned int T = t;
	if(T == 0)   return false;     else --T;
	if(H)   T -= 3600000 * (*H = T / 3600000);     else T %= 3600000;
	if(M)   T -=   60000 * (*M = T /   60000);     else T %=   60000;
	if(S)   T -=    1000 * (*S = T /    1000);     else T %=    1000;
	if(L)   *L = T;
	return true;
}



bool DateTime::set(year_t Y) {
	if(Y < minYear || Y > maxYear)   { *(_W*)this |= NO_Y;     return false; } // leaves time-of-day unchanged
	y = Y - minYear;
	*(_W*)this &= 0xFFFFFFF007FFFFFF; // remove month and day (i.e. set them to 1-1)
	return true;
}
bool DateTime::set(year_t Y, month_t M) {
	if(Y < minYear || Y > maxYear)   { *(_W*)this |= NO_Y;     return false; } // leaves time-of-day unchanged
	y = Y - minYear;
	if(--M >= 12)                    { *(_W*)this |= NO_M;     return false; }
	m = M;
	*(_W*)this &= 0xFFFFFFFF07FFFFFF; // set day to 1
	return true;
}
bool DateTime::set(year_t Y, month_t M, day_t D) {
	if(Y < minYear || Y > maxYear)   { *(_W*)this |= NO_Y;     return false; } // leaves time-of-day unchanged
	y = Y - minYear;
	if(--M >= 12)                    { *(_W*)this |= NO_M;     return false; }
	m = M;
	if(--D >= monthLength())         { *(_W*)this |= NO_D;     return false; }
	d = D;
	return true;
}



bool DateTime::year(year_t Y) {
	if(Y < minYear || Y > maxYear)             { *(_W*)this |= NO_Y;     return false; }
	*(_W*)this &= 0xFFFFFFF007FFFFFF; // remove month and day (i.e. set them to 1-1)
	y = Y - minYear;     return true;
}
bool DateTime::month(month_t M) {
	if(y == NOYEAR || --M >= 12)               { *(_W*)this |= NO_M;     return false; }
	*(_W*)this &= 0xFFFFFFFF07FFFFFF; // set day to 1
	m = M;     return true;
}
bool DateTime::day(day_t D) {
	if(m == NOMONTH || --D >= monthLength())   { *(_W*)this |= NO_D;     return false; } // no year implies no month
	d = D;     return true;
}


bool DateTime::time(timeOfDay_t T) {
	if(T < maxTime)   { t = ++T;     return true;  }
	else              { t = 0;       return false; }
}
bool DateTime::time(unsigned short H, unsigned short M, unsigned short S, unsigned short MS) {
	if(MS >= 1000 || S >= 60 || M >= 60 || H >= maxH)   { t = 0;                                           return false; }
	else                                                { t = 1 + MS + 1000 * (S + 60 * (M + 60 * H));     return true;  }
}
bool DateTime::time(unsigned short H, unsigned short M, double S) {
	if(S < 0 || S >= 60 || M >= 60 || H >= maxH)   { t = 0;                                                           return false; }
	else                                           { t = (timeOfDay_t(2000 * S + 3) >> 1) + 60000 * (M + 60 * H);     return true;  }
}
bool DateTime::partOf24h(double T) {
	if(T < 0 || T >= maxH / 24.) { t = 0;                                       return false; }
	else                         { t = timeOfDay_t(T * 172800000 + 3) >> 1;     return true;  } // round to the nearest millisecond
}
void DateTime::unsetTime() { t = 0; }


bool DateTime::isMonthFirst() const { return m == 0; }

// since invalid days are 0x1F==31 and \monthLength() of invalud months is always 0 the function returns false if any fiels is nAn
bool DateTime::isMonthLast()  const { return d + 1 == monthLength(); }


DateTime DateTime::monthFirst() const { return DateTime( *(_CW*)this & (m != NOMONTH - 1 ? 0xFFFFFFFF00000000 : INVALID)); }
DateTime DateTime::monthLast () const { return DateTime((*(_CW*)this & (m != NOMONTH - 1 ? 0xFFFFFFFF00000000 : INVALID)) | (_W(monthLength() - 1) << 27)); }
DateTime DateTime::yearFirst () const { return DateTime( *(_CW*)this & (y != NOYEAR      ? 0xFFFFFFF000000000 : INVALID)); }
DateTime DateTime::yearLast  () const { return DateTime((*(_CW*)this & (y != NOYEAR      ? 0xFFFFFFF000000000 : INVALID)) | 0x0000000BF0000000); }


bool DateTime::dayInYear(dayInYear_t dY) {
	if(y == NOYEAR)   return false;
	m = DateTimeBase::dayInYear_(dY, DateTimeBase::isLeapYear_(y));
	return (d = dY) != NODAY - 1;
}


bool DateTime::dayOffset(dayOffset_t dt) {
	operator=(DateTime{ dt });
	return (d != NODAY - 1);
}


bool DateTime::operator==(const DateTime& d) const { return *(_CW*)this == reinterpret_cast<_CW&>(d); }
bool DateTime::operator!=(const DateTime& d) const { return *(_CW*)this != reinterpret_cast<_CW&>(d); }
bool DateTime::operator< (const DateTime& d) const { return *(_CW*)this <  reinterpret_cast<_CW&>(d); }
bool DateTime::operator> (const DateTime& d) const { return *(_CW*)this >  reinterpret_cast<_CW&>(d); }
bool DateTime::operator<=(const DateTime& d) const { return *(_CW*)this <= reinterpret_cast<_CW&>(d); }
bool DateTime::operator>=(const DateTime& d) const { return *(_CW*)this >= reinterpret_cast<_CW&>(d); }



DateTime& DateTime::operator+=(dayOffset_t dt) {
	if(dt < 0)   return operator-=(-dt);
	if(d == NODAY)   return *this;
	// simplified calculation for offsets that stay in the same year
	if(dt < 366) { // first a rough check to ensure no overflow
		dayOffset_t dt2 = dt + d;
		if(DateTimeBase::isLeapYear_(y)) {
			if((dt2 += monthBeginsLY[m]) < 366) {
				month_t mo = static_cast<month_t>(dt2 >> 5);
				if(dt2 >= monthBeginsLY[mo + 1])   ++mo;
				d = dt2 - monthBeginsLY[mo];     m = mo;
				return *this;
			}
		} else if((dt2 += monthBegins[m]) < 365) {
			month_t mo = static_cast<month_t>(dt2 >> 5);
			if(dt2 >= monthBegins[mo + 1])   ++mo;
			d = dt2 - monthBegins[mo];     m = mo;
			return *this;
		}
	}
	if(dt > maxDayOffset - minDayOffset || (dt += dayOffset()) > maxDayOffset) // offset too large —> make date invalid
		{ *(_W*)this = NO_Y;     return *this; }
	dayOffset(dt);
	return *this;
}

DateTime& DateTime::operator-=(dayOffset_t) { return *this; }


DateTime  DateTime::operator+ (dayOffset_t doff) const
	{ return DateTime{ *this } += doff; }
DateTime  DateTime::operator- (dayOffset_t doff) const
	{ return DateTime{ *this } -= doff; }


DateTime& DateTime::operator++() {
	unsigned int D = d;
	if(D == NODAY)   return *this;
	if(D < 27 || D < unsigned int(monthLength() - 1))   { d = ++D;   return *this; }
	d = 0;
	if(++m < 12)   return *this;
	if(++y == NOYEAR)   *(_W*)this |= INVALID;
	m = 0;
	return *this;
}
DateTime& DateTime::operator--() {
	unsigned int D = d;
	if(D == NODAY)   return *this;
	if(D)   { d = --D;     return *this; }
	if(m)   { --m;     d = static_cast<day_t>(monthLength() - 1);     return *this; }
	if(y-- == 0)   { *(_W*)this |= INVALID;     return *this; }
	m = 11;     d = 30;
	return *this;
}

DateTime DateTime::operator++(int) const
	{ return ++DateTime{ *this }; }
DateTime DateTime::operator--(int) const
	{ return --DateTime{ *this }; }



// number of days from one date to another (disregarding time-of-day)
DateTime::dayOffset_t DateTime::operator-(const DateTime& DT) const
	{ return dayOffset() - DT.dayOffset(); }



/*int DateTime::parse(const char* s) {
	using DOT = DateTime::dayOffset_t;
	constexpr DOT maxDayOffs = static_cast<DOT>(DateTime::maxYear) * 10000 + 1231;
	struct { DOT val;     int nDigits; } data[4];

	int i = 0;
	char c;
	while(c = *s) {
		if(c >= '0' && c <= '9') {
			auto& d = data[i++];
			for(d.val = d.nDigits = 0, --s;     (c = *++s) >= '0' && c <= '9'; ) {
				if(((d.val *= 10) += c - '0') > maxDayOffs)   return false; // overflow
				++d.nDigits;
			}
			if(c != ':' && i < 3)   continue;
			if(c == ':')   --i;
			// so 
				if(i > 1) // 

			}
		}
	}
}
*/