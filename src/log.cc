#include "log.h"

namespace logging = boost::log;

//Defines a global logger initialization routine
BOOST_LOG_GLOBAL_LOGGER_INIT(global_logger, logger_t)
{
    logger_t lg;
    std::string format = "[%TimeStamp%] [%ThreadID%] <%Severity%> %Message%";

    logging::register_simple_formatter_factory<logging::trivial::severity_level, char>("Severity");
    logging::add_common_attributes();

    logging::add_file_log
    (
        logging::keywords::file_name = "../log/SERVER_LOG_%N.log",                                        /*< file name pattern >*/
        logging::keywords::rotation_size = 10 * 1024 * 1024,                                   /*< rotate files every 10 MiB... >*/
        logging::keywords::time_based_rotation = logging::sinks::file::rotation_at_time_point(0, 0, 0), /*< ...or at midnight >*/
		logging::keywords::auto_flush = true,
		logging::keywords::format = format
    );

    logging::add_console_log(
            std::cout,
            logging::keywords::format = format
    );

    logging::core::get()->set_filter
    (
        logging::trivial::severity >= logging::trivial::info
    );

    return lg;
}