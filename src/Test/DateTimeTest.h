#pragma once

#include <DateTime_boost.h>
#include <Version.h>

namespace DateTimeTest {


struct DateTimeTestError : std::runtime_error {
	PROJECT_NAMESPACE::DateTime dt;
	boost::gregorian::date      bd;
	PROJECT_NAMESPACE::DateTime::dayOffset_t offs;

	DateTimeTestError(const char* msg, const PROJECT_NAMESPACE::DateTime&, const boost::gregorian::date&);
	DateTimeTestError(const char* msg, const PROJECT_NAMESPACE::DateTime&, PROJECT_NAMESPACE::DateTime::dayOffset_t);
};


/* Note: each test presumes that all the others above it have been usccessfully performed */

void DateTimeTestIterate();

void DateTimeTestDayOffset();



} /* end of namespace DateTimeTest*/

#undef PROJECT_NAMESPACE