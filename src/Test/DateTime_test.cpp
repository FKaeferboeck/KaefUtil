#include "DateTimeTest.h"
#include <Version.h>
#include <iostream>

using namespace PROJECT_NAMESPACE;
using namespace DateTimeTest;
using namespace boost::gregorian;
using namespace std;

namespace {
	constexpr auto boostMin = boost::date_time::special_values::min_date_time;
}


DateTimeTestError::DateTimeTestError(const char* msg, const DateTime& dt, const date& bd) :
	runtime_error(msg),   dt(dt),   bd(bd),   offs(0)
{ }


DateTimeTestError::DateTimeTestError(const char* msg, const DateTime& dt, DateTime::dayOffset_t offs) :
	runtime_error(msg),   dt(dt),   offs(offs)
{ }


void DateTimeTest::DateTimeTestIterate() {
	date b{ boostMin };
	const date_duration oneDay(1);
	DateTime d = fromBoostDate(b);
	if(d != b)     throw DateTimeTestError("Iteration test: setting min date", d, b);
	for(size_t i = 0;     i < 1100*365;     ++i)
		if((b += oneDay) != ++d)   throw DateTimeTestError("Iteration test: ++", d, b);
	for(size_t i = 0;     i < 1100*365;     ++i)
		if((b -= oneDay) != --d)   throw DateTimeTestError("Iteration test: --", d, b);
}


// includes \dayInYear
void DateTimeTest::DateTimeTestDayOffset() {
	DateTime d(-900, 1, 1), d2, dMax(900, 1, 1);
	auto t = d.dayOffset();
	while(++d < dMax) {
		if(d.dayOffset() != ++t)          throw DateTimeTestError("day offset test: get offset", d,  t);
		if(!d2.dayOffset(t) || d2 != d)   throw DateTimeTestError("day offset test: set offset", d2, t);
	}
}
