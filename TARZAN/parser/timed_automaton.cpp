#include "TARZAN/parser/timed_automaton_def.h"
#include "TARZAN/parser/config.h"


namespace parser
{
    using parser::iterator_type;
    using parser::context_type;

    BOOST_SPIRIT_INSTANTIATE(variable_type, iterator_type, context_type);

    BOOST_SPIRIT_INSTANTIATE(assignmentExpr_type, iterator_type, context_type);

    BOOST_SPIRIT_INSTANTIATE(clockConstraint_type, iterator_type, context_type);

    BOOST_SPIRIT_INSTANTIATE(locationContent_type, iterator_type, context_type);

    BOOST_SPIRIT_INSTANTIATE(transition_type, iterator_type, context_type);

    BOOST_SPIRIT_INSTANTIATE(timedAutomaton_type, iterator_type, context_type);

    BOOST_SPIRIT_INSTANTIATE(timedArena_type, iterator_type, context_type);
}
