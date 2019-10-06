#pragma once

#include "DateTime.h"
#include <boost/date_time/gregorian/gregorian.hpp>
#include "Version.h"


namespace PROJECT_NAMESPACE {

boost::gregorian::date toBoostDate(const DateTime&);

DateTime& setToBoostDate(DateTime&, const boost::gregorian::date&);

DateTime fromBoostDate(const boost::gregorian::date&);


} /* end of namespace */


bool operator==(const PROJECT_NAMESPACE::DateTime&, const boost::gregorian::date&);
bool operator==(const boost::gregorian::date&, const PROJECT_NAMESPACE::DateTime&);

bool operator!=(const PROJECT_NAMESPACE::DateTime&, const boost::gregorian::date&);
bool operator!=(const boost::gregorian::date&, const PROJECT_NAMESPACE::DateTime&);


#undef PROJECT_NAMESPACE
