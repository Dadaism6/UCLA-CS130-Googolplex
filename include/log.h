// Reference: https://stackoverflow.com/questions/20086754/how-to-use-boost-log-from-multiple-files-with-gloa
#ifndef LOG_H
#define LOG_H

#include <boost/log/expressions.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup.hpp>

// define the logger type used in this project
typedef boost::log::sources::severity_logger_mt<boost::log::trivial::severity_level> logger_t;

// the global logger with a custom initialization (in log.cc)
BOOST_LOG_GLOBAL_LOGGER(global_logger, logger_t)

// define the logging severity io that can be used in other files
#define DEBUG   BOOST_LOG_SEV(global_logger::get(), boost::log::trivial::debug)
#define INFO    BOOST_LOG_SEV(global_logger::get(), boost::log::trivial::info)
#define WARNING BOOST_LOG_SEV(global_logger::get(), boost::log::trivial::warning)
#define ERROR   BOOST_LOG_SEV(global_logger::get(), boost::log::trivial::error)
#define FATAL   BOOST_LOG_SEV(global_logger::get(), boost::log::trivial::fatal)

#endif  // LOG_H