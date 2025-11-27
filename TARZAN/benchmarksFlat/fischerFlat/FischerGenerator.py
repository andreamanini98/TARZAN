#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Flatten Fischer (Option A) — id encoded in location names as suffix _idV.
id is NOT a variable anymore: removed from declarations, guards, assignments.

Usage:
    python3 fischer_flat_id_in_name_final.py --N 3 --out fischer_flat_N3.xml
"""

import itertools
import argparse
from pathlib import Path

# Single-process template (fixed)
LOCATIONS = ["A", "req", "wait", "cs"]
K = 2

# transitions: (source, target, guard_text, assign_text, resets_x_flag)
# guard_text may include 'id' checks like 'id == 0' or 'id == pid' — used only to decide applicability
# assign_text may include 'id = pid' or 'id = 0' — used only to determine destination suffix
SINGLE_TRANSITIONS = [
    ("A",   "req",  "id == 0",             "x = 0",           True),
    ("req", "wait", "x <= k",              "x = 0, id = pid", True),
    ("wait","req",  "id == 0",             "x = 0",           True),
    ("wait","cs",   "x > k and id == pid", "",                False),
    ("cs",  "A",    "",                    "id = 0",          False)
]

# ---------------------
# Utilities
# ---------------------
def escape_uppaal(s: str) -> str:
    if not s:
        return s
    # order matters: handle <= >= before < >
    s = s.replace(">=", "&gt;=")
    s = s.replace("<=", "&lt;=")
    s = s.replace(">", "&gt;")
    s = s.replace("<", "&lt;")
    return s

def split_conjuncts(guard: str):
    """Split guard into conjuncts using 'and' and '&&' heuristically."""
    if not guard:
        return []
    g = guard.replace("&&", " and ")
    parts = [p.strip() for p in g.split(" and ") if p.strip()]
    return parts

def id_condition_holds(cond: str, proc_index: int, current_id: int) -> bool:
    """
    Evaluate id-related condition cond at runtime:
      - cond examples: 'id == 0', 'id == pid', 'id != 0', 'id != pid'
    Returns True if condition holds for (proc_index (0-based), current_id).
    If condition is not recognized, conservatively return False.
    """
    if cond is None or cond.strip() == "":
        return True
    c = cond.replace(" ", "")
    if "id==" in c:
        rhs = c.split("id==",1)[1]
        if rhs == "pid":
            return current_id == (proc_index + 1)
        try:
            return current_id == int(rhs)
        except ValueError:
            return False
    if "id!=" in c:
        rhs = c.split("id!=",1)[1]
        if rhs == "pid":
            return current_id != (proc_index + 1)
        try:
            return current_id != int(rhs)
        except ValueError:
            return True
    # unknown id expression -> conservative False
    return False

def remove_id_conjuncts(parts, proc_index, current_id):
    """
    Given conjunct parts (list of strings), evaluate id-related conjuncts against current_id.
    Return (allowed: bool, remaining_parts: list[str]) where remaining_parts excludes id conjuncts.
    """
    remaining = []
    for p in parts:
        if "id" in p:
            if not id_condition_holds(p, proc_index, current_id):
                return False, []
            # if holds, we drop the id conjunct from remaining (id encoded in location name)
        else:
            remaining.append(p)
    return True, remaining

def normalize_assignments(assign_text: str, proc_index: int):
    """
    Normalize assignment text:
    - remove id assignments entirely (they only affect destination suffix)
    - replace 'pid' with proc_index+1
    - replace 'x' with x{proc_index+1}
    - split separators ';' or ',' and rejoin with ', ' for UPPAAL
    Returns list of assignment strings (may be empty).
    """
    if not assign_text:
        return []
    parts = [p.strip() for p in assign_text.replace(";", ",").split(",") if p.strip()]
    out = []
    for a in parts:
        a_nospace = a.replace(" ", "")
        if a_nospace.startswith("id="):
            # drop id assignments entirely
            continue
        # replace pid and x occurrences (simple replace)
        a2 = a.replace("pid", str(proc_index+1)).replace("x", f"x{proc_index+1}")
        out.append(a2)
    # remove duplicates while preserving order
    seen = set()
    dedup = []
    for a in out:
        key = a.replace(" ", "")
        if key not in seen:
            seen.add(key)
            dedup.append(a)
    return dedup

# ---------------------
# Build full state space (Option A)
# ---------------------
def build_states(N):
    product_locs = list(itertools.product(LOCATIONS, repeat=N))  # tuples of location names
    states = []
    for loc_tuple in product_locs:
        for idv in range(0, N+1):
            states.append((loc_tuple, idv))
    return product_locs, states

# ---------------------
# Generate transitions under new semantics
# ---------------------
def generate_transitions(N, product_locs, states):
    state_index = {s: i for i, s in enumerate(states)}
    transitions = []

    for s_idx, (loc_tuple, idv) in enumerate(states):
        for p in range(N):  # process p moves
            local = loc_tuple[p]
            for (src, tgt, guard_text, assign_text, resets_x) in SINGLE_TRANSITIONS:
                if src != local:
                    continue

                # split conjuncts and evaluate id-related conjuncts against idv
                conjuncts = split_conjuncts(guard_text)
                ok, remaining = remove_id_conjuncts(conjuncts, p, idv)
                if not ok:
                    continue  # id-related guard fails => transition not allowed from this state

                # build final guard from remaining conjuncts:
                final_guard_parts = []
                for r in remaining:
                    # replace pid and x with per-process ones; escape
                    r2 = r.replace("pid", str(p+1)).replace("x", f"x{p+1}")
                    r2 = escape_uppaal(r2)
                    if r2:
                        final_guard_parts.append(r2)
                final_guard = " and ".join(final_guard_parts) if final_guard_parts else None

                # determine destination id suffix based on assignments in assign_text
                dest_id = idv
                if assign_text:
                    # check id assignments in original assign_text
                    parts = [a.strip() for a in assign_text.replace(";", ",").split(",") if a.strip()]
                    for a in parts:
                        a_ns = a.replace(" ", "")
                        if a_ns.startswith("id="):
                            rhs = a_ns.split("=",1)[1]
                            if rhs == "pid":
                                dest_id = p+1
                            else:
                                try:
                                    dest_id = int(rhs)
                                except ValueError:
                                    pass

                # Build destination location tuple
                dst_tuple = list(loc_tuple)
                dst_tuple[p] = tgt
                dst_state = (tuple(dst_tuple), dest_id)
                if dst_state not in state_index:
                    # shouldn't happen (we built full state space)
                    continue
                dst_idx = state_index[dst_state]

                # Build final assignments excluding id=... (they are encoded in dest_state)
                assigns = normalize_assignments(assign_text, p)
                # include clock reset if needed (ensure no duplicate)
                clock_assign = f"x{p+1} = 0"
                if resets_x and not any(a.replace(" ", "") == clock_assign.replace(" ", "") for a in assigns):
                    assigns.append(clock_assign)
                final_assign = ", ".join(assigns) if assigns else None

                transitions.append({
                    "from": s_idx,
                    "to": dst_idx,
                    "guard": final_guard,
                    "assign": final_assign,
                    "comment": f"proc{p+1}:{src}->{tgt} (id {idv} -> {dest_id})"
                })
    return transitions

# ---------------------
# Build mutual-exclusion query (using Fischer_flat.<locname>)
# ---------------------
def build_mutual_query(product_locs, N):
    bad = []
    for loc_tuple in product_locs:
        cs_count = sum(1 for l in loc_tuple if l == "cs")
        if cs_count >= 2:
            base = "_".join(f"{l}{i+1}" for i, l in enumerate(loc_tuple))
            for idv in range(0, N+1):
                name = f"Fischer_flat.{base}_id{idv}"
                bad.append(name)
    if not bad:
        return None
    formula = " or ".join(bad)
    # E<> (bad) would be reachability of bad — you asked earlier for a query that says we cannot reach any location with 2 processes in cs.
    # We emit a property expressing the negation as A[] !(bad), but keep an explicit reachability query as well.
    lines = []
    # Reachability query (E<> bad): if reachable, mutual exclusion violated
    lines.append("  <query>")
    lines.append(f"    <formula>E&lt;&gt; ({formula})</formula>")
    lines.append("    <comment>Mutual exclusion violation: at least 2 processes in cs</comment>")
    lines.append("  </query>")
    # Also add the invariance form A[] !(bad) as a separate query
    lines.append("  <query>")
    lines.append(f"    <formula>A[] !({formula})</formula>")
    lines.append("    <comment>Mutual exclusion holds: never two processes in cs</comment>")
    lines.append("  </query>")
    return "\n".join(lines)

# ---------------------
# Produce complete UPPAAL XML
# ---------------------
def generate_xml(N, out_path):
    product_locs, states = build_states(N)
    transitions = generate_transitions(N, product_locs, states)

    # header
    lines = []
    lines.append('<?xml version="1.0" encoding="utf-8"?>')
    lines.append('<!DOCTYPE nta PUBLIC "-//Uppaal Team//DTD Flat System 1.1//EN" '
                 '"http://www.it.uu.se/research/group/darts/uppaal/flat-1_1.dtd">')
    lines.append('<nta>')

    # global declaration: clocks only (no id variable)
    lines.append('  <declaration>')
    for i in range(N):
        lines.append(f'    clock x{i+1};')
    lines.append(f'    const int k = {K};')
    lines.append('  </declaration>')

    # template
    lines.append('  <template>')
    lines.append('    <name>Fischer_flat</name>')
    lines.append('    <declaration>// flattened Fischer (id encoded in location names)</declaration>')

    # locations: one per (loc_tuple, idv)
    spacing = 200
    cols = int(max(1, round((len(states))**0.5)))
    for idx, (loc_tuple, idv) in enumerate(states):
        x = spacing * (idx % cols)
        y = spacing * (idx // cols)
        base = "_".join(f"{l}{i+1}" for i, l in enumerate(loc_tuple))
        locname = f"{base}_id{idv}"
        lines.append(f'    <location id="S{idx}" x="{x}" y="{y}">')
        lines.append(f'      <name>{locname}</name>')
        # combine invariants (req -> x{i} <= k)
        inv_parts = []
        for i, l in enumerate(loc_tuple):
            if l == "req":
                inv_parts.append(f"x{i+1} &lt;= {K}")
        if inv_parts:
            lines.append(f'      <label kind="invariant">{" and ".join(inv_parts)}</label>')
        lines.append(f'      <label kind="comment">{base}_id{idv}</label>')
        lines.append('    </location>')

    # initial location: all A and id0
    init_state = (tuple(["A"]*N), 0)
    init_idx = states.index(init_state)
    lines.append(f'    <init ref="S{init_idx}"/>')

    # transitions
    for tr in transitions:
        lines.append('    <transition>')
        lines.append(f'      <source ref="S{tr["from"]}"/>')
        lines.append(f'      <target ref="S{tr["to"]}"/>')
        if tr["guard"]:
            lines.append(f'      <label kind="guard">{tr["guard"]}</label>')
        if tr["assign"]:
            lines.append(f'      <label kind="assignment">{tr["assign"]}</label>')
        lines.append(f'      <label kind="comment">{tr["comment"]}</label>')
        lines.append('    </transition>')

    lines.append('  </template>')

    # System declaration (UPPAAL instance section)
    lines.append('  <system>')
    lines.append('    system Fischer_flat;')
    lines.append('  </system>')

    # Build queries block
    qtext = build_mutual_query(product_locs, N)
    if qtext:
        lines.append('  <queries>')
        lines.append(qtext)
        lines.append('  </queries>')

    lines.append('</nta>')

    Path(out_path).write_text("\n".join(lines), encoding="utf-8")
    print(f"Wrote flattened model (Option A) for N={N} to: {out_path}")
    print(f"Locations (states): {len(states)}, Transitions: {len(transitions)}")

# ---------------------
# CLI
# ---------------------
def main():
    parser = argparse.ArgumentParser(description="Flatten Fischer protocol (id encoded in location names).")
    parser.add_argument("--N", type=int, required=True, help="number of processes")
    parser.add_argument("--out", type=str, required=True, help="output filename (.xml)")
    args = parser.parse_args()
    generate_xml(args.N, args.out)

if __name__ == "__main__":
    main()


