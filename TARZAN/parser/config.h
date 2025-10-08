#ifndef CONFIG_H
#define CONFIG_H

#include "TARZAN/parser/error_handler.h"

#include <boost/spirit/home/x3.hpp>


namespace parser
{
    namespace x3 = boost::spirit::x3;
    namespace ascii = x3::ascii;

    using iterator_type = std::string::const_iterator;

    using phrase_context_type = x3::phrase_parse_context<ascii::space_type>::type;

    using error_handler_type = error_handler<iterator_type>;

    // Combined Error Handler and Phrase Parse Context
    using context_type = x3::context<error_handler_tag, std::reference_wrapper<error_handler_type>, phrase_context_type>;
}

#endif //CONFIG_H
