#include "DateTime_boost.h"
#include "Version.h"

using namespace PROJECT_NAMESPACE;
using namespace boost::gregorian;


date PROJECT_NAMESPACE::toBoostDate(const DateTime& D) {
	if(!D.isValid())   return date(special_values::not_a_date_time);
	// Todo!! Ranges!
	return date(D.year(), D.month(), D.day());
}


DateTime& PROJECT_NAMESPACE::setToBoostDate(DateTime& DT, const date& BD) {
	DT.set(BD.year(), static_cast<DateTime::month_t>(BD.month()), static_cast<DateTime::day_t>(BD.day()));
	return DT;
}


DateTime PROJECT_NAMESPACE::fromBoostDate(const date& BD)
	{ return DateTime{ BD.year(), static_cast<DateTime::month_t>(BD.month()), static_cast<DateTime::day_t>(BD.day()) }; }


bool operator==(const DateTime& DT, const date& BD)
	{ return DT.year() == BD.year() && DT.month() == BD.month() && DT.day() == BD.day(); }
bool operator==(const date& BD, const DateTime& DT)
	{ return DT.year() == BD.year() && DT.month() == BD.month() && DT.day() == BD.day(); }


bool operator!=(const DateTime& DT, const date& BD)
	{ return DT.year() != BD.year() || DT.month() != BD.month() || DT.day() != BD.day(); }
bool operator!=(const date& BD, const DateTime& DT)
	{ return DT.year() != BD.year() || DT.month() != BD.month() || DT.day() != BD.day(); }
