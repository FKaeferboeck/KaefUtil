#pragma once

#include "Version.h"

#include <cstdint>
#include <type_traits>
#include <limits>

/* Some notes on the implementation:                                                                                           */
/* \DateTime uses several serendipitous features of the Gregorian calendar:                                                    */
/* A day has a duration of 86400000 ms, a number which fits into 27 bits, even if you allow anomalous leap days of 25h and     */
/* reserve one value (0) for invalid time-of-days. This means that a time-of day in milliseconds together with a day-of-month  */
/* value (range 0–30, reserving 31 for illegal values —> 5 bits) fits into a four-byte unsigned integer.                       */
/* The higher half of the \DateTime object uses its lowest 4 bits for the month (values 0—11, 12 for invalid month), leaving   */
/* 28 bits for the year. A little-known C feature — bit fields — is used to split the two unsigned ints into those bit chunks. */

namespace PROJECT_NAMESPACE {
namespace DateTimeBase {

	// LE == little-endian == most significant byte (in our case, year) at the highest address
	template<bool is_LE = (const bool&)unsigned short{ 1 } >
	struct curArchitectureBitFieldType {
	protected:
		unsigned int 
			t : 27, d : 5;     unsigned int m : 4, y : 28;
		curArchitectureBitFieldType() = default;
		constexpr curArchitectureBitFieldType(unsigned int y, unsigned int m, unsigned int d) : y(y), m(m), d(d), t(0) { }
	};
	template<>
	struct curArchitectureBitFieldType<false> {
	protected:
		unsigned int y : 28, m : 4;     unsigned int d : 5, t : 27;
		curArchitectureBitFieldType() = default;
		constexpr curArchitectureBitFieldType(unsigned int y, unsigned int m, unsigned int d) : y(y), m(m), d(d), t(0) { }
	};
	static_assert(sizeof(unsigned int) == 4 && sizeof(curArchitectureBitFieldType<>) == 8, "DateTime requires 32 bit ints and reasonable-behaved bit fields");
	// a few static helper functions for \DateTime below
	template<typename T>
	static constexpr T floor400(T t) { return (t >= 0 ? t / 400 : -((T(399) - t) / 400)) * 400; }
	static constexpr bool isLeapYear_(int64_t Y) { return !(Y & 0x03) && ((Y % 100) || !(Y & 0x0F)); } // \Y must be nonnegative
	template<int64_t y0> // number of days from 0001–01–01 to the date (Y+y0)–(M-1)–(D-1);   y0 must be divisible by 400
	constexpr static int64_t dayOffset_(int64_t Y, unsigned char M, unsigned char D) { // \Y >= 0, \M is 0...12, \D is 0...30
		// All this rigmarole with offsets is so that we can use bit shifts instead of division to improve performance;
		constexpr int64_t offs = (y0 / 400 - 1) * 146097;
		// since \{Y - 1} isn't necessarily nonnegative, we instead use \{Y + 399} and accordingly shift \offs by 400 years (== 146097 days)
		// We count ja & feb from the start of the year, the other months back from the next year, that way we bypass checking for leap years
		int64_t e = offs + D + (M <= 1 ? (Y += 399, M * 31) : (Y += 400, 30 * M + ((M + 1 + (M >> 3)) >> 1) - 367));
		return e += Y * 365 + (Y >> 2) - (3 * (Y / 100 + 1) >> 2);
	}
	template<typename T>
	constexpr unsigned char dayInYear_(T& offs, bool isLY) { // returns the month, sets \offs to day-of-month
		unsigned char M = static_cast<unsigned char>(offs >> 5); // division by 32 —> the correct month or the one before it (actually Y holds the next month)
		constexpr unsigned short monthBegins  [] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },
		                         monthBeginsLY[] = { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 };
		const unsigned short* mB = (isLY ? monthBeginsLY : monthBegins);
		if(offs >= mB[12])   { offs = DateTime::NODAY - 1;     return DateTime::NOMONTH - 1; }
		//offs2 = 30 * Y + ((Y + (1 + (Y >> 3))) >> 1) - (isLY ? 1 : 2) * (Y >= 2);
		if(offs >= mB[M + 1])   ++M;
		offs -= mB[M];
		return M;
	}

} /* end of namespace DateTimeBase */
} /* end of project namespace*/
