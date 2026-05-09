@page lianaGrammar Liana Grammar

## Timed Automata and Arenas

The following is the grammar for the Liana DSL used to create Timed Automata and Timed Arenas.
Whether the actions are input (?) or output (!) actions must be specified only in the transitions.
T and F are syntactic sugar for true and false.
Since clock guards cannot directly express disjunctions, OR conditions must be modeled by defining one transition per disjunct.
Timed Automata marked as symmetric must be structurally identical (the integer indicates the symmetry group to which they are assigned).
In Timed Arenas, each location is assigned to a player: controller (`c`) or environment (`e`).

\htmlonly
<pre style="font-family: monospace; font-size: 1.1em;">
<span style="font-weight:bold;">⟨automaton⟩</span> ――> <span style="color:orange;">'create'</span> <span style="color:orange;">'automaton'</span> <span style="font-weight:bold;">⟨literal⟩</span> (ε | <span style="font-weight:bold;">⟨symm_rule⟩</span>)
<span style="color:orange;">                '{'</span>
<span style="color:orange;">                'clocks'</span>        <span style="color:orange;">'{'</span> (ε | <span style="font-weight:bold;">⟨literal⟩</span> (<span style="color:orange;">','</span> <span style="font-weight:bold;">⟨literal⟩</span>)* <span style="color:orange;">';'</span>) <span style="color:orange;">'}'</span>
<span style="color:orange;">                'actions'</span>       <span style="color:orange;">'{'</span> <span style="font-weight:bold;">⟨literal⟩</span> (<span style="color:orange;">','</span> <span style="font-weight:bold;">⟨literal⟩</span>)* <span style="color:orange;">';'</span> <span style="color:orange;">'}'</span>
                (ε | <span style="color:orange;">'integers'</span> <span style="color:orange;">'{'</span> <span style="font-weight:bold;">⟨literal⟩</span> (<span style="color:orange;">','</span> <span style="font-weight:bold;">⟨literal⟩</span>)* <span style="color:orange;">';'</span> <span style="color:orange;">'}'</span>)
<span style="color:orange;">                'locations'</span>     <span style="color:orange;">'{'</span> <span style="font-weight:bold;">⟨locations_rule⟩</span> <span style="color:orange;">'}'</span>
<span style="color:orange;">                'transitions'</span>   <span style="color:orange;">'{'</span> <span style="font-weight:bold;">⟨transition_rule⟩</span> (<span style="color:orange;">','</span> <span style="font-weight:bold;">⟨transition_rule⟩</span>)* <span style="color:orange;">';'</span> <span style="color:orange;">'}'</span>
<span style="color:orange;">                '}'</span>

<span style="font-weight:bold;">⟨arena⟩</span> ――> <span style="color:orange;">'create'</span> <span style="color:orange;">'arena'</span> <span style="font-weight:bold;">⟨literal⟩</span> (ε | <span style="font-weight:bold;">⟨symm_rule⟩</span>)
<span style="color:orange;">             '{'</span>
<span style="color:orange;">             'clocks'</span>        <span style="color:orange;">'{'</span> (ε | <span style="font-weight:bold;">⟨literal⟩</span> (<span style="color:orange;">','</span> <span style="font-weight:bold;">⟨literal⟩</span>)* <span style="color:orange;">';'</span>) <span style="color:orange;">'}'</span>
<span style="color:orange;">             'actions'</span>       <span style="color:orange;">'{'</span> <span style="font-weight:bold;">⟨literal⟩</span> (<span style="color:orange;">','</span> <span style="font-weight:bold;">⟨literal⟩</span>)* <span style="color:orange;">';'</span> <span style="color:orange;">'}'</span>
             (ε | <span style="color:orange;">'integers'</span> <span style="color:orange;">'{'</span> <span style="font-weight:bold;">⟨literal⟩</span> (<span style="color:orange;">','</span> <span style="font-weight:bold;">⟨literal⟩</span>)* <span style="color:orange;">';'</span> <span style="color:orange;">'}'</span>)
<span style="color:orange;">             'locations'</span>     <span style="color:orange;">'{'</span> <span style="font-weight:bold;">⟨arena_locations_rule⟩</span> <span style="color:orange;">'}'</span>
<span style="color:orange;">             'transitions'</span>   <span style="color:orange;">'{'</span> <span style="font-weight:bold;">⟨transition_rule⟩</span> (<span style="color:orange;">','</span> <span style="font-weight:bold;">⟨transition_rule⟩</span>)* <span style="color:orange;">';'</span> <span style="color:orange;">'}'</span>
<span style="color:orange;">             '}'</span>

<span style="font-weight:bold;">⟨symm_rule⟩</span> ――> <span style="color:orange;">'::'</span> <span style="color:orange;">'symm'</span> <span style="color:orange;">'&lt;'</span> <span style="font-weight:bold;">⟨int⟩</span> <span style="color:orange;">'&gt;'</span>

<span style="font-weight:bold;">⟨locations_rule⟩</span> ――> <span style="font-weight:bold;">⟨loc_rule⟩</span> (<span style="color:orange;">','</span> <span style="font-weight:bold;">⟨loc_rule⟩</span>)* <span style="color:orange;">';'</span>

<span style="font-weight:bold;">⟨loc_rule⟩</span> ――> <span style="font-weight:bold;">⟨literal⟩</span> <span style="font-weight:bold;">⟨loc_content_rule⟩</span>

<span style="font-weight:bold;">⟨arena_locations_rule⟩</span> ――> <span style="font-weight:bold;">⟨arena_loc_rule⟩</span> (<span style="color:orange;">','</span> <span style="font-weight:bold;">⟨arena_loc_rule⟩</span>)* <span style="color:orange;">';'</span>

<span style="font-weight:bold;">⟨arena_loc_rule⟩</span> ――> <span style="font-weight:bold;">⟨literal⟩</span> <span style="font-weight:bold;">⟨arena_loc_content_rule⟩</span>

<span style="font-weight:bold;">⟨arena_loc_content_rule⟩</span> ――> <span style="color:orange;">'&lt;'</span> <span style="color:orange;">'player'</span> <span style="color:orange;">':'</span> <span style="font-weight:bold;">⟨player⟩</span> <span style="color:orange;">','</span> <span style="font-weight:bold;">⟨loc_content_rule⟩</span> <span style="color:orange;">'&gt;'</span>

<span style="font-weight:bold;">⟨player⟩</span> ――> <span style="color:orange;">'c'</span> | <span style="color:orange;">'e'</span>

<span style="font-weight:bold;">⟨loc_content_rule⟩</span> ――> <span style="color:orange;">'&lt;'</span>
                       (ε | 
                        <span style="font-weight:bold;">⟨ini⟩</span> | <span style="font-weight:bold;">⟨urg_com⟩</span> | <span style="font-weight:bold;">⟨inv⟩</span> |
                        <span style="font-weight:bold;">⟨ini⟩</span> <span style="color:orange;">','</span> <span style="font-weight:bold;">⟨urg_com⟩</span> | <span style="font-weight:bold;">⟨ini⟩</span> <span style="color:orange;">','</span> <span style="font-weight:bold;">⟨inv⟩</span> | <span style="font-weight:bold;">⟨urg_com⟩</span> <span style="color:orange;">','</span> <span style="font-weight:bold;">⟨inv⟩</span> |
                        <span style="font-weight:bold;">⟨ini⟩</span> <span style="color:orange;">','</span> <span style="font-weight:bold;">⟨urg_com⟩</span> <span style="color:orange;">','</span> <span style="font-weight:bold;">⟨inv⟩</span>)
       <span style="color:orange;">                '&gt;'</span>

<span style="font-weight:bold;">⟨urg_com⟩</span> ――> <span style="font-weight:bold;">⟨urg⟩</span> | <span style="font-weight:bold;">⟨com⟩</span>
  
<span style="font-weight:bold;">⟨ini⟩</span> ――> <span style="color:orange;">'ini'</span> <span style="color:orange;">':'</span> <span style="font-weight:bold;">⟨bool⟩</span>

<span style="font-weight:bold;">⟨urg⟩</span> ――> <span style="color:orange;">'urg'</span> <span style="color:orange;">':'</span> <span style="font-weight:bold;">⟨bool⟩</span>

<span style="font-weight:bold;">⟨com⟩</span> ――> <span style="color:orange;">'com'</span> <span style="color:orange;">':'</span> <span style="font-weight:bold;">⟨bool⟩</span>
  
<span style="font-weight:bold;">⟨inv⟩</span> ――> <span style="color:orange;">'inv'</span> <span style="color:orange;">':'</span> <span style="font-weight:bold;">⟨guard_rule⟩</span>

<span style="font-weight:bold;">⟨bool⟩</span> ――> <span style="color:orange;">'T'</span> | <span style="color:orange;">'F'</span> | <span style="color:orange;">'true'</span> | <span style="color:orange;">'false'</span>

<span style="font-weight:bold;">⟨transition_rule⟩</span> ――> <span style="color:orange;">'('</span>
<span style="font-weight:bold;">                      ⟨literal⟩</span> <span style="color:orange;">','</span>
<span style="font-weight:bold;">                      ⟨actions_rule⟩</span> <span style="color:orange;">','</span>
<span style="font-weight:bold;">                      ⟨guard_rule⟩</span> <span style="color:orange;">','</span>
                      (ε | <span style="font-weight:bold;">⟨boolean_expr⟩</span> <span style="color:orange;">','</span>)
<span style="color:orange;">                      '['</span> (ε | <span style="font-weight:bold;">⟨literal⟩</span> (<span style="color:orange;">','</span> <span style="font-weight:bold;">⟨literal⟩</span>)*) <span style="color:orange;">']'</span> <span style="color:orange;">','</span>
                      (ε | <span style="color:orange;">'['</span> <span style="font-weight:bold;">⟨assignment_expr⟩</span> (<span style="color:orange;">','</span> <span style="font-weight:bold;">⟨assignment_expr⟩</span>)* <span style="color:orange;">']'</span> <span style="color:orange;">','</span>)
<span style="font-weight:bold;">                      ⟨literal⟩</span>
<span style="color:orange;">                      ')'</span>

<span style="font-weight:bold;">⟨guard_rule⟩</span> ――> <span style="color:orange;">'['</span> (ε | <span style="font-weight:bold;">⟨clock_constraint_rule⟩</span> (<span style="color:orange;">','</span> <span style="font-weight:bold;">⟨clock_constraint_rule⟩</span>)*) <span style="color:orange;">']'</span>

<span style="font-weight:bold;">⟨actions_rule⟩</span> ――> <span style="font-weight:bold;">⟨literal⟩</span> (ε | <span style="font-weight:bold;">⟨input_output_action⟩</span>)

<span style="font-weight:bold;">⟨input_output_action⟩</span> ――> <span style="color:orange;">'!'</span> | <span style="color:orange;">'?'</span> | <span style="color:orange;">'!!'</span> | <span style="color:orange;">'??'</span>

<span style="font-weight:bold;">⟨clock_constraint_rule⟩</span> ――> <span style="color:orange;">'('</span> <span style="font-weight:bold;">⟨literal⟩</span> <span style="color:orange;">','</span> <span style="font-weight:bold;">⟨comparison_operator⟩</span> <span style="color:orange;">','</span> <span style="font-weight:bold;">⟨int⟩</span> <span style="color:orange;">')'</span>

<span style="font-weight:bold;">⟨comparison_operator⟩</span> ――> <span style="color:orange;">'&lt;'</span> | <span style="color:orange;">'&lt;='</span> | <span style="color:orange;">'=='</span> | <span style="color:orange;">'&gt;='</span> | <span style="color:orange;">'&gt;'</span>

<span style="font-weight:bold;">⟨int⟩</span> ――> <span style="color:orange;">'1..9'</span> (<span style="color:orange;">'0..9'</span>)*

<span style="font-weight:bold;">⟨literal⟩</span> ――> (<span style="color:orange;">'a..z'</span> | <span style="color:orange;">'A..Z'</span> | <span style="color:orange;">'0..9'</span> | <span style="color:orange;">'_'</span>)+
</pre>
\endhtmlonly

---

## Arithmetic Expressions

The following is the grammar for the Liana DSL used to declare arithmetic expressions.
Integer variables in Timed Automata are automatically initialized to zero. 
To overcome this, a transition can be added to initialize them in the Timed Automaton itself.

\htmlonly
<pre style="font-family: monospace; font-size: 1.1em;">
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
\endhtmlonly

---

## CLTLoc Formulae

The following is the grammar for CLTLoc (Constraint Linear Temporal Logic over clocks) formulae.

\htmlonly
<pre style="font-family: monospace; font-size: 1.1em;">
<span style="font-weight:bold;">⟨general_cltloc_formula⟩</span> ――> <span style="color:orange;">'('</span> (<span style="font-weight:bold;">⟨unary_cltloc_formula⟩</span> | <span style="font-weight:bold;">⟨binary_cltloc_formula⟩</span> | <span style="font-weight:bold;">⟨pure_cltloc_formula⟩</span>) <span style="color:orange;">')'</span>

<span style="font-weight:bold;">⟨unary_cltloc_formula⟩</span> ――> <span style="font-weight:bold;">⟨unary_cltloc_op⟩</span> (ε | <span style="color:orange;">'^'</span> <span style="font-weight:bold;">⟨int⟩</span>) <span style="font-weight:bold;">⟨general_cltloc_formula⟩</span>

<span style="font-weight:bold;">⟨binary_cltloc_formula⟩</span> ――> <span style="font-weight:bold;">⟨general_cltloc_formula⟩</span> <span style="font-weight:bold;">⟨binary_cltloc_op⟩</span> <span style="font-weight:bold;">⟨general_cltloc_formula⟩</span>

<span style="font-weight:bold;">⟨pure_cltloc_formula⟩</span> ――> <span style="font-weight:bold;">⟨pure_disjunct⟩</span> (<span style="color:orange;">'||'</span> <span style="font-weight:bold;">⟨pure_disjunct⟩</span>)*

<span style="font-weight:bold;">⟨pure_disjunct⟩</span> ――> <span style="color:orange;">'['</span> (ε | <span style="font-weight:bold;">⟨literal⟩</span> (<span style="color:orange;">','</span> <span style="font-weight:bold;">⟨literal⟩</span>)*) <span style="color:orange;">']'</span> <span style="color:orange;">','</span>
                    <span style="color:orange;">'['</span> (ε | <span style="font-weight:bold;">⟨clock_constraint_rule⟩</span> (<span style="color:orange;">','</span> <span style="font-weight:bold;">⟨clock_constraint_rule⟩</span>)*) <span style="color:orange;">']'</span>

<span style="font-weight:bold;">⟨unary_cltloc_op⟩</span> ――> <span style="color:orange;">'BOX'</span> | <span style="color:orange;">'DIAMOND'</span>

<span style="font-weight:bold;">⟨binary_cltloc_op⟩</span> ――> <span style="color:orange;">'UNTIL'</span>

<span style="font-weight:bold;">⟨conjunction_of_formulae⟩</span> ――> <span style="font-weight:bold;">⟨conjunction_type⟩</span> <span style="font-weight:bold;">⟨general_cltloc_formula⟩</span> (<span style="font-weight:bold;">⟨and_op⟩</span> <span style="font-weight:bold;">⟨general_cltloc_formula⟩</span>)*

<span style="font-weight:bold;">⟨conjunction_type⟩</span> ――> <span style="color:orange;">'AND_GENERAL'</span> | <span style="color:orange;">'AND_NEXT'</span>
</pre>
\endhtmlonly

[← Back to Main Page](@ref index)
