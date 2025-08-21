#include "TARZAN/parser/timed_automaton_def.h"
#include "TARZAN/parser/config.h"

namespace timed_automaton::parser
{
    using parser::iterator_type;
    using parser::context_type;

    BOOST_SPIRIT_INSTANTIATE(clockConstraint_type, iterator_type, context_type);

    BOOST_SPIRIT_INSTANTIATE(transition_type, iterator_type, context_type);

    BOOST_SPIRIT_INSTANTIATE(timedAutomaton_type, iterator_type, context_type);

    BOOST_SPIRIT_INSTANTIATE(timedArena_type, iterator_type, context_type);
}
