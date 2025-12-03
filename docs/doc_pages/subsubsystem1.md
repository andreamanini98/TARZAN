@page subsubsystem1 subsubtitle
@tableofcontents

This is the content for subsystem 1.

Some text here describing this subsystem.

Here's the formatted grammar with the same style as your document:

<pre style="font-family: monospace; font-size: 1.1em;">
<span style="font-weight:bold;">⟨automaton⟩</span> ――> <span style="color:orange;">'create'</span> <span style="color:orange;">'automaton'</span> <span style="font-weight:bold;">⟨literal⟩</span>
<span style="color:orange;">                '{'</span>
<span style="color:orange;">                'clocks'</span>      <span style="color:orange;">'{'</span> <span style="font-weight:bold;">⟨clocks_rule⟩</span> <span style="color:orange;">'}'</span>
<span style="color:orange;">                'actions'</span>     <span style="color:orange;">'{'</span> <span style="font-weight:bold;">⟨literal⟩</span> (<span style="color:orange;">','</span> <span style="font-weight:bold;">⟨literal⟩</span>)* <span style="color:orange;">'}'</span>
<span style="color:orange;">                'locations'</span>   <span style="color:orange;">'{'</span> <span style="font-weight:bold;">⟨locations_rule⟩</span> <span style="color:orange;">'}'</span>
<span style="color:orange;">                'transitions'</span> <span style="color:orange;">'{'</span> <span style="font-weight:bold;">⟨transition_rule⟩</span> (<span style="color:orange;">','</span> <span style="font-weight:bold;">⟨transition_rule⟩</span>)* <span style="color:orange;">';'</span> <span style="color:orange;">'}'</span>
<span style="color:orange;">                '}'</span>

<span style="font-weight:bold;">⟨clocks_rule⟩</span> ――> <span style="font-weight:bold;">⟨literal⟩</span> (<span style="color:orange;">','</span> <span style="font-weight:bold;">⟨literal⟩</span>)* <span style="color:orange;">';'</span>

<span style="font-weight:bold;">⟨locations_rule⟩</span> ――> <span style="font-weight:bold;">⟨loc_rule⟩</span> (<span style="color:orange;">','</span> <span style="font-weight:bold;">⟨loc_rule⟩</span>)* <span style="color:orange;">';'</span>

<span style="font-weight:bold;">⟨loc_rule⟩</span> ――> <span style="font-weight:bold;">⟨literal⟩</span> <span style="font-weight:bold;">⟨loc_content_rule⟩</span>

<span style="font-weight:bold;">⟨loc_content_rule⟩</span> ――> <span style="color:orange;">'&lt;'</span>
(ε | <span style="color:orange;">'ini'</span> <span style="color:orange;">':'</span> <span style="font-weight:bold;">⟨bool⟩</span> (ε | <span style="color:orange;">','</span> <span style="color:orange;">'inv'</span> <span style="color:orange;">':'</span> <span style="font-weight:bold;">⟨guard_rule⟩</span>) | <span style="color:orange;">'inv'</span> <span style="color:orange;">':'</span> <span style="font-weight:bold;">⟨guard_rule⟩</span>)
<span style="color:orange;">'&gt;'</span>

<span style="font-weight:bold;">⟨bool⟩</span> ――> <span style="color:orange;">'T'</span> | <span style="color:orange;">'F'</span> | <span style="color:orange;">'true'</span> | <span style="color:orange;">'false'</span>

<span style="font-weight:bold;">⟨transition_rule⟩</span> ――> <span style="color:orange;">'('</span>
<span style="font-weight:bold;">⟨literal⟩</span> <span style="color:orange;">','</span>
<span style="font-weight:bold;">⟨actions_rule⟩</span> <span style="color:orange;">','</span>
<span style="font-weight:bold;">⟨guard_rule⟩</span> <span style="color:orange;">','</span>
<span style="color:orange;">'['</span> (ε | <span style="font-weight:bold;">⟨literal⟩</span> (<span style="color:orange;">','</span> <span style="font-weight:bold;">⟨literal⟩</span>)*) <span style="color:orange;">']'</span> <span style="color:orange;">','</span>
<span style="font-weight:bold;">⟨literal⟩</span>
<span style="color:orange;">')'</span>

<span style="font-weight:bold;">⟨guard_rule⟩</span> ――> <span style="color:orange;">'['</span> (ε | <span style="font-weight:bold;">⟨clock_constraint_rule⟩</span> (<span style="color:orange;">','</span> <span style="font-weight:bold;">⟨clock_constraint_rule⟩</span>)*) <span style="color:orange;">']'</span>

<span style="font-weight:bold;">⟨actions_rule⟩</span> ――> <span style="font-weight:bold;">⟨literal⟩</span> (ε | <span style="font-weight:bold;">⟨input_output_action⟩</span>)

<span style="font-weight:bold;">⟨input_output_action⟩</span> ――> <span style="color:orange;">'!'</span> | <span style="color:orange;">'?'</span>

<span style="font-weight:bold;">⟨clock_constraint_rule⟩</span> ――> <span style="color:orange;">'('</span> <span style="font-weight:bold;">⟨literal⟩</span> <span style="color:orange;">','</span> <span style="font-weight:bold;">⟨comparison_operator⟩</span> <span style="color:orange;">','</span> <span style="font-weight:bold;">⟨int⟩</span> <span style="color:orange;">')'</span>

<span style="font-weight:bold;">⟨comparison_operator⟩</span> ――> <span style="color:orange;">'&lt;'</span> | <span style="color:orange;">'&lt;='</span> | <span style="color:orange;">'=='</span> | <span style="color:orange;">'&gt;='</span> | <span style="color:orange;">'&gt;'</span>

<span style="font-weight:bold;">⟨int⟩</span> ――> <span style="color:orange;">'1..9'</span> (<span style="color:orange;">'0..9'</span>)*

<span style="font-weight:bold;">⟨literal⟩</span> ――> (<span style="color:orange;">'a..z'</span> | <span style="color:orange;">'A..Z'</span> | <span style="color:orange;">'0..9'</span> | <span style="color:orange;">'_'</span>)+

<span style="font-weight:bold;">⟨assignment_expr⟩</span> ――> <span style="font-weight:bold;">⟨variable⟩</span> <span style="color:orange;">'='</span> <span style="font-weight:bold;">⟨arithmetic_expr⟩</span>

<span style="font-weight:bold;">⟨arithmetic_expr⟩</span> ――> <span style="font-weight:bold;">⟨additive_expr⟩</span>

<span style="font-weight:bold;">⟨additive_expr⟩</span> ――> <span style="font-weight:bold;">⟨multiplicative_expr⟩</span> (<span style="font-weight:bold;">⟨add_op⟩</span> <span style="font-weight:bold;">⟨multiplicative_expr⟩</span>)*

<span style="font-weight:bold;">⟨multiplicative_expr⟩</span> ――> <span style="font-weight:bold;">⟨primary_expr⟩</span> (<span style="font-weight:bold;">⟨mult_op⟩</span> <span style="font-weight:bold;">⟨primary_expr⟩</span>)*

<span style="font-weight:bold;">⟨primary_expr⟩</span> ――> <span style="font-weight:bold;">⟨int⟩</span> | <span style="font-weight:bold;">⟨variable⟩</span> | <span style="color:orange;">'('</span> <span style="font-weight:bold;">⟨additive_expr⟩</span> <span style="color:orange;">')'</span>

<span style="font-weight:bold;">⟨variable⟩</span> ――> <span style="font-weight:bold;">⟨literal⟩</span>

<span style="font-weight:bold;">⟨add_op⟩</span> ――> <span style="color:orange;">'+'</span> | <span style="color:orange;">'-'</span>

<span style="font-weight:bold;">⟨mult_op⟩</span> ――> <span style="color:orange;">'*'</span> | <span style="color:orange;">'/'</span>

<span style="font-weight:bold;">⟨boolean_expr⟩</span> ――> <span style="font-weight:bold;">⟨boolean_or_expr⟩</span>

<span style="font-weight:bold;">⟨boolean_or_expr⟩</span> ――> <span style="font-weight:bold;">⟨boolean_and_expr⟩</span> (<span style="font-weight:bold;">⟨or_op⟩</span> <span style="font-weight:bold;">⟨boolean_and_expr⟩</span>)*

<span style="font-weight:bold;">⟨boolean_and_expr⟩</span> ――> <span style="font-weight:bold;">⟨boolean_term⟩</span> (<span style="font-weight:bold;">⟨and_op⟩</span> <span style="font-weight:bold;">⟨boolean_term⟩</span>)*

<span style="font-weight:bold;">⟨boolean_term⟩</span> ――> <span style="color:orange;">'('</span> <span style="font-weight:bold;">⟨boolean_or_expr⟩</span> <span style="color:orange;">')'</span> | <span style="font-weight:bold;">⟨comparison_expr⟩</span> | <span style="font-weight:bold;">⟨bool⟩</span>

<span style="font-weight:bold;">⟨comparison_expr⟩</span> ――> <span style="font-weight:bold;">⟨arithmetic_expr⟩</span> <span style="font-weight:bold;">⟨comparison_operator⟩</span> <span style="font-weight:bold;">⟨arithmetic_expr⟩</span>

<span style="font-weight:bold;">⟨or_op⟩</span> ――> <span style="color:orange;">'||'</span>

<span style="font-weight:bold;">⟨and_op⟩</span> ――> <span style="color:orange;">'&&'</span>
</pre>