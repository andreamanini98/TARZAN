#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include <boost/spirit/home/x3/support/utility/error_reporting.hpp>

namespace timed_automaton::parser {
    namespace x3 = boost::spirit::x3;

    // X3 Error Handler Utility
    template<typename Iterator>
    using error_handler = x3::error_handler<Iterator>;

    // tag used to get our error handler from the context
    using error_handler_tag = x3::error_handler_tag;

    struct error_handler_base {
        error_handler_base();

        template<typename Iterator, typename Exception, typename Context>
        x3::error_handler_result on_error(Iterator &first,
                                          Iterator const &last,
                                          Exception const &x,
                                          Context const &context);
    };

    inline error_handler_base::error_handler_base() = default;


    template<typename Iterator, typename Exception, typename Context>
    x3::error_handler_result
    error_handler_base::on_error(Iterator &, Iterator const &, Exception const &x, Context const &context)
    {
        const std::string which = x.which();

        std::string message = "[PARSER] Error! Expecting: " + which + " here:";
        auto &error_handler = x3::get<error_handler_tag>(context).get();
        error_handler(x.where(), message);

        return x3::error_handler_result::fail;
    }
}

#endif //ERROR_HANDLER_H
