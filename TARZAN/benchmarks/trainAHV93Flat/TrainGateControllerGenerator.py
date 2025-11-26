#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Flatten Train-Gate-Controller UPPAAL model into a single UPPAAL XML automaton.

Modes:
  - A (default): keep 'cnt' as a template-local integer variable and keep cnt guards/assignments
  - B         : encode 'cnt' into the location names (0..N) and remove cnt from all labels

Usage:
  python3 flatten_traingate_xml_modeAB_final.py --N 4 --mode B --out flattened.xml
"""
import argparse
import re
from itertools import product
from pathlib import Path
from typing import Optional, Tuple, List, Dict

# -------------------------
# Model constants
# -------------------------
CONST = {"a": 2, "b": 5, "c": 1, "d": 2, "e": 1}
CHANNELS = ["texit", "lower", "raise", "approach"]
GLOBAL_INT_NAME = "cnt"

# -------------------------
# Templates (process + transitions)
# Each transition tuple: (src, tgt, guard, sync, assign, resets_train_x_flag)
# -------------------------
TRAIN_STATES = [("train0", None),
                ("train1", f"x <= {CONST['b']}"),
                ("train2", f"x <= {CONST['b']}"),
                ("train3", f"x <= {CONST['b']}")]
TRAIN_TRANS = [
    ("train2", "train3", None, None, None, False),
    ("train3", "train0", f"x <= {CONST['b']}", "texit?", None, False),
    ("train1", "train2", f"x > {CONST['a']}", None, None, False),
    ("train0", "train1", None, "approach?", "x = 0", True),
]

GATE_STATES = [("gate0", None),
               ("gate1", f"y <= {CONST['c']}"),
               ("gate2", None),
               ("gate3", f"y <= {CONST['d']}")]
GATE_TRANS = [
    ("gate1", "gate2", f"y <= {CONST['c']}", None, None, False),
    ("gate2", "gate3", None, "raise?", "y = 0", False),
    ("gate0", "gate1", None, "lower?", "y = 0", False),
    ("gate3", "gate0", f"y >= {CONST['c']} and y <= {CONST['d']}", None, None, False),
]

CTRL_STATES = [("controller0", None),
               ("controller1", f"z <= {CONST['e']}"),
               ("controller2", None),
               ("controller3", f"z <= {CONST['e']}")]
CTRL_TRANS = [
    ("controller0", "controller1", None, "approach!", "z = 0, cnt = 1", False),
    ("controller1", "controller1", None, "approach!", "cnt = cnt + 1", False),
    ("controller1", "controller1", None, "texit!", "cnt = cnt - 1", False),
    ("controller2", "controller2", None, "approach!", "cnt = cnt + 1", False),
    ("controller2", "controller2", "cnt > 1", "texit!", "cnt = cnt - 1", False),
    ("controller2", "controller3", "cnt == 1", "texit!", "z = 0, cnt = 0", False),
    ("controller3", "controller0", f"z <= {CONST['e']}", "raise!", None, False),
    ("controller3", "controller2", f"z <= {CONST['e']}", "approach!", "cnt = cnt + 1", False),
    ("controller1", "controller2", f"z == {CONST['e']}", "lower!", None, False),
]

# -------------------------
# Utilities
# -------------------------
def escape_uppaal(s: Optional[str]) -> Optional[str]:
    if s is None:
        return None
    s2 = s.replace("<=", "&lt;=").replace(">=", "&gt;=").replace("<", "&lt;").replace(">", "&gt;")
    return s2.strip() if s2.strip() else None

def split_conjuncts(guard: Optional[str]) -> List[str]:
    if not guard:
        return []
    g = guard.replace("&&", " and ")
    parts = [p.strip() for p in re.split(r'\band\b', g) if p.strip()]
    return parts

def is_cnt_conjunct(s: str) -> bool:
    return "cnt" in s

def evaluate_cnt_conjunct(s: str, cntv: Optional[int]) -> Optional[bool]:
    """
    Evaluate a simple cnt conjunct like 'cnt == 1' or 'cnt > 1' using numeric cntv.
    Returns True/False if can evaluate; None if unrecognized or cntv is None.
    """
    if cntv is None:
        return None
    q = s.replace(" ", "")
    # equality
    m = re.match(r'cnt==(\d+)', q)
    if m:
        return cntv == int(m.group(1))
    m = re.match(r'cnt>(\d+)', q)
    if m:
        return cntv > int(m.group(1))
    m = re.match(r'cnt<(\d+)', q)
    if m:
        return cntv < int(m.group(1))
    m = re.match(r'cnt>=(\d+)', q)
    if m:
        return cntv >= int(m.group(1))
    m = re.match(r'cnt<=(\d+)', q)
    if m:
        return cntv <= int(m.group(1))
    m = re.match(r'cnt!=(\d+)', q)
    if m:
        return cntv != int(m.group(1))
    return None

def sanitize_guard_for_modeB(guard: Optional[str], cntv: Optional[int], train_index: Optional[int]) -> Tuple[bool, Optional[str]]:
    """
    For modeB: evaluate cnt conjuncts (if any) against cntv.
      - If any cnt-conjunct evaluates to False => transition not allowed (return (False, None))
      - Otherwise remove cnt-conjuncts from guard and return the escaped guard without them.
    For modeA: simply return (True, escaped guard)
    train_index: if provided, will be used to replace 'x' with 'x{train_index}' in the remaining guard
    """
    if guard is None or guard.strip() == "":
        return True, None
    parts = split_conjuncts(guard)
    remaining = []
    for p in parts:
        if is_cnt_conjunct(p):
            val = evaluate_cnt_conjunct(p, cntv)
            if val is False:
                return False, None
            # if val is True or None (cannot evaluate but contains cnt), we treat:
            # - if val is True: drop the cnt conjunct
            # - if val is None: **conservative**: treat as not allowed (since cnt unknown) -> but we want to allow if modeA only
            # Here in flattening we will assume cntv is always provided when modeB; if it's None we disallow
            if val is None:
                return False, None
            # else val==True -> skip adding to remaining (we remove cnt conjunct)
            continue
        else:
            remaining.append(p)
    # now remaining contains non-cnt conjuncts; replace x if needed and escape
    replaced = []
    for r in remaining:
        rr = r
        if train_index is not None:
            rr = re.sub(r"\bx\b", f"x{train_index}", rr)
        rr = escape_uppaal(rr)
        if rr:
            replaced.append(rr)
    if not replaced:
        return True, None
    return True, " and ".join(replaced)

def replace_train_x(expr: Optional[str], train_idx: Optional[int]) -> Optional[str]:
    if expr is None:
        return None
    s = expr
    if train_idx is not None:
        s = re.sub(r"\bx\b", f"x{train_idx}", s)
    return escape_uppaal(s)

def normalize_assign(assign: Optional[str], train_idx: Optional[int], modeB: bool) -> Optional[str]:
    """
    Normalize assignment text:
    - remove cnt assignments entirely when modeB==True
    - replace x -> x{train_idx} when train_idx provided
    - split separators ';' or ',' and rejoin with ', '
    """
    if not assign:
        return None
    parts = [p.strip() for p in assign.replace(";", ",").split(",") if p.strip()]
    out = []
    for p in parts:
        # strip cnt updates in modeB
        if modeB and re.search(r'\bcnt\b', p):
            continue
        if train_idx is not None:
            p = re.sub(r"\bx\b", f"x{train_idx}", p)
        out.append(p)
    # deduplicate preserving order
    seen = set()
    dedup = []
    for a in out:
        key = a.replace(" ", "")
        if key not in seen:
            seen.add(key)
            dedup.append(a)
    return ", ".join(dedup) if dedup else None

def join_assigns(*assigns: Optional[str]) -> Optional[str]:
    parts = []
    for a in assigns:
        if a:
            parts.append(a)
    if not parts:
        return None
    out = []
    seen = set()
    for p in parts:
        for q in [x.strip() for x in p.split(",") if x.strip()]:
            key = q.replace(" ", "")
            if key not in seen:
                seen.add(key)
                out.append(q)
    return ", ".join(out) if out else None

def parse_sync(sync: Optional[str]) -> Optional[Tuple[str, str]]:
    if not sync:
        return None
    m = re.match(r'\s*([A-Za-z_]\w*)\s*([!?])\s*', sync)
    if not m:
        return None
    return m.group(1), m.group(2)

def interpret_cnt_assignment(assign: Optional[str]) -> Optional[Tuple[str, int]]:
    """
    Interpret cnt-related assignment in controller transitions.
    Returns:
      - ("abs", k)  if 'cnt = k'
      - ("delta", d) if 'cnt = cnt + 1' or 'cnt = cnt - 1'
      - None if no cnt assignment recognized.
    """
    if not assign:
        return None
    s = assign.replace(" ", "")
    m_abs = re.search(r'cnt=(\d+)', s)
    if m_abs:
        return ("abs", int(m_abs.group(1)))
    if 'cnt=cnt+1' in s:
        return ("delta", 1)
    if 'cnt=cnt-1' in s:
        return ("delta", -1)
    if 'cnt=0' in s:
        return ("abs", 0)
    return None

# -------------------------
# Build product states
# -------------------------
def build_states(N: int, modeB: bool):
    gate_names = [s for s, _ in GATE_STATES]
    ctrl_names = [s for s, _ in CTRL_STATES]
    train_names = [s for s, _ in TRAIN_STATES]
    product_trains = list(product(train_names, repeat=N))
    states = []
    for g in gate_names:
        for c in ctrl_names:
            for ttuple in product_trains:
                if modeB:
                    for cnt in range(0, N + 1):
                        states.append((g, c, ttuple, cnt))
                else:
                    states.append((g, c, ttuple, None))
    index = {states[i]: i for i in range(len(states))}
    return states, index

# -------------------------
# Flatten transitions (including binary sync), respecting modeB
# -------------------------
def flatten_model(N: int, modeB: bool):
    states, index = build_states(N, modeB)
    flat_trans = []

    gate_by_src: Dict[str, List[Tuple]] = {}
    for tr in GATE_TRANS:
        gate_by_src.setdefault(tr[0], []).append(tr)
    ctrl_by_src: Dict[str, List[Tuple]] = {}
    for tr in CTRL_TRANS:
        ctrl_by_src.setdefault(tr[0], []).append(tr)
    train_by_src: Dict[str, List[Tuple]] = {}
    for tr in TRAIN_TRANS:
        train_by_src.setdefault(tr[0], []).append(tr)

    for s_idx, state in enumerate(states):
        gstate, cstate, ttuple, cntv = state

        # helper to create non-sync transitions
        def add_non_sync(src_val, dict_, train_index: Optional[int] = None):
            for (src, tgt, guard, sync, assign, resetx) in dict_.get(src_val, []):
                if sync:
                    continue
                # For modeB: evaluate cnt-conjuncts and remove them from label
                if modeB:
                    allowed, guard_label = sanitize_guard_for_modeB(guard, cntv, train_index)
                    if not allowed:
                        continue
                else:
                    guard_label = replace_train_x(guard, train_index)
                # Assignments: in modeB remove cnt updates using normalize_assign
                assign_label = normalize_assign(assign, train_index, modeB)
                # reset train clock if requested
                if resetx and train_index is not None:
                    assign_label = join_assigns(assign_label, f"x{train_index} = 0")
                # compute destination cnt when controller transition
                dst_cnt = cntv
                if dict_ is ctrl_by_src:
                    if modeB and assign:
                        cnt_effect = interpret_cnt_assignment(assign)
                        if cnt_effect:
                            typ, val = cnt_effect
                            if typ == "abs":
                                dst_cnt = val
                            else:
                                dst_cnt = (cntv if cntv is not None else 0) + val
                                if dst_cnt < 0:
                                    dst_cnt = 0
                                if dst_cnt > N:
                                    dst_cnt = N
                # build destination
                if dict_ is gate_by_src:
                    dst = (tgt, cstate, ttuple, dst_cnt if modeB else None)
                elif dict_ is ctrl_by_src:
                    dst = (gstate, tgt, ttuple, dst_cnt if modeB else None)
                else:
                    new_ttuple = list(ttuple)
                    new_ttuple[train_index - 1] = tgt
                    dst = (gstate, cstate, tuple(new_ttuple), dst_cnt if modeB else None)
                dst_idx = index[dst]
                flat_trans.append({"from": s_idx, "to": dst_idx, "guard": guard_label, "assign": assign_label})

        # add non-syncs
        add_non_sync(gstate, gate_by_src, None)
        add_non_sync(cstate, ctrl_by_src, None)
        for t_i in range(1, N + 1):
            add_non_sync(ttuple[t_i - 1], train_by_src, t_i)

        # collect sync transitions (gate, ctrl, trains)
        sync_pool = []
        for tr in gate_by_src.get(gstate, []):
            if tr[3]:
                sync_pool.append(("gate", None, tr))
        for tr in ctrl_by_src.get(cstate, []):
            if tr[3]:
                sync_pool.append(("ctrl", None, tr))
        for tid in range(N):
            local = ttuple[tid]
            for tr in train_by_src.get(local, []):
                if tr[3]:
                    sync_pool.append(("train", tid, tr))

        # pairwise synchronization
        for i in range(len(sync_pool)):
            for j in range(i + 1, len(sync_pool)):
                comp1, idx1, tr1 = sync_pool[i]
                comp2, idx2, tr2 = sync_pool[j]
                (s1, t1, g1, syn1, a1, r1) = tr1
                (s2, t2, g2, syn2, a2, r2) = tr2
                p1 = parse_sync(syn1)
                p2 = parse_sync(syn2)
                if p1 is None or p2 is None:
                    continue
                chan1, dir1 = p1
                chan2, dir2 = p2
                if chan1 != chan2:
                    continue
                if dir1 == dir2:
                    continue
                # determine new component states
                new_g, new_c, new_ttuple = gstate, cstate, list(ttuple)
                if comp1 == "gate":
                    new_g = t1
                elif comp1 == "ctrl":
                    new_c = t1
                else:
                    new_ttuple[idx1] = t1
                if comp2 == "gate":
                    new_g = t2
                elif comp2 == "ctrl":
                    new_c = t2
                else:
                    new_ttuple[idx2] = t2
                # compute dst_cnt (modeB) from controller assignments in either participant
                dst_cnt = cntv
                if modeB:
                    for assign_text in (a1, a2):
                        cnt_eff = interpret_cnt_assignment(assign_text)
                        if cnt_eff:
                            typ, val = cnt_eff
                            if typ == "abs":
                                dst_cnt = val
                            else:
                                dst_cnt = (dst_cnt if dst_cnt is not None else 0) + val
                                if dst_cnt < 0:
                                    dst_cnt = 0
                                if dst_cnt > N:
                                    dst_cnt = N
                # evaluate cnt-related conjuncts in guards (modeB) and build combined guard without cnt parts
                if modeB:
                    allowed1, g1_label = sanitize_guard_for_modeB(g1, cntv, idx1 + 1 if comp1 == "train" else None)
                    allowed2, g2_label = sanitize_guard_for_modeB(g2, cntv, idx2 + 1 if comp2 == "train" else None)
                    if not (allowed1 and allowed2):
                        continue
                    guard_comb = None
                    if g1_label and g2_label:
                        guard_comb = f"{g1_label} and {g2_label}"
                    elif g1_label:
                        guard_comb = g1_label
                    elif g2_label:
                        guard_comb = g2_label
                else:
                    # modeA: simply combine guards with train x replaced
                    g1r = replace_train_x(g1, idx1 + 1 if comp1 == "train" else None)
                    g2r = replace_train_x(g2, idx2 + 1 if comp2 == "train" else None)
                    parts = [p for p in (g1r, g2r) if p]
                    guard_comb = " and ".join(parts) if parts else None
                # combine assignments: in modeB we remove cnt updates (normalize_assign does it)
                if modeB:
                    assign_comb = join_assigns(normalize_assign(a1, idx1 + 1 if comp1 == "train" else None, modeB),
                                                normalize_assign(a2, idx2 + 1 if comp2 == "train" else None, modeB))
                else:
                    assign_comb = join_assigns(normalize_assign(a1, idx1 + 1 if comp1 == "train" else None, modeB),
                                                normalize_assign(a2, idx2 + 1 if comp2 == "train" else None, modeB))
                # include train resets
                if comp1 == "train" and r1:
                    assign_comb = join_assigns(assign_comb, f"x{idx1+1} = 0")
                if comp2 == "train" and r2:
                    assign_comb = join_assigns(assign_comb, f"x{idx2+1} = 0")
                # build destination
                dst = (new_g, new_c, tuple(new_ttuple), dst_cnt if modeB else None)
                dst_idx = index[dst]
                flat_trans.append({"from": s_idx, "to": dst_idx, "guard": guard_comb, "assign": assign_comb})

    return states, flat_trans

# -------------------------
# Produce UPPAAL XML
# -------------------------
def write_xml(path: str, N: int, states: List[Tuple], transitions: List[Dict], modeB: bool):
    lines: List[str] = []
    lines.append('<?xml version="1.0" encoding="utf-8"?>')
    lines.append('<!DOCTYPE nta PUBLIC "-//Uppaal Team//DTD Flat System 1.1//EN" '
                 '"http://www.it.uu.se/research/group/darts/uppaal/flat-1_1.dtd">')
    lines.append('<nta>')
    # Global declaration left minimal
    lines.append('  <declaration>// All declarations are local to the template</declaration>')
    # Template
    lines.append('  <template>')
    lines.append('    <name>Flatten</name>')
    lines.append('    <declaration>')
    # clocks: x1..xN, y, z
    for i in range(N):
        lines.append(f'      clock x{i+1};')
    lines.append('      clock y;')
    lines.append('      clock z;')
    # constants
    for k, v in CONST.items():
        lines.append(f'      const int {k} = {v};')
    # cnt only in mode A (kept as template-level int)
    if not modeB:
        lines.append(f'      int {GLOBAL_INT_NAME};')
    # channels
    for ch in CHANNELS:
        lines.append(f'      chan {ch};')
    lines.append('    </declaration>')

    # locations
    spacing = 220  # adjustable grid spacing
    cols = int(max(1, round(len(states) ** 0.5)))
    for idx, (gstate, cstate, ttuple, cntv) in enumerate(states):
        x = spacing * (idx % cols)
        y = spacing * (idx // cols)
        base = f"{gstate}_{cstate}_" + "_".join(ttuple)
        locname = f"{base}" + (f"_cnt{cntv}" if modeB else "")
        # invariants
        inv_parts = []
        if gstate == "gate1":
            inv_parts.append(f"y &lt;= {CONST['c']}")
        if gstate == "gate3":
            inv_parts.append(f"y &lt;= {CONST['d']}")
        if cstate in ("controller1", "controller3"):
            inv_parts.append(f"z &lt;= {CONST['e']}")
        for i, t in enumerate(ttuple):
            if t in ("train1", "train2", "train3"):
                inv_parts.append(f"x{i+1} &lt;= {CONST['b']}")
        lines.append(f'    <location id="S{idx}" x="{x}" y="{y}">')
        lines.append(f'      <name>{locname}</name>')
        if inv_parts:
            lines.append(f'      <label kind="invariant">{" and ".join(inv_parts)}</label>')
        lines.append('    </location>')

    # initial state
    init_state = ("gate0", "controller0", tuple(["train0"] * N), 0 if modeB else None)
    init_idx = states.index(init_state)
    lines.append(f'    <init ref="S{init_idx}"/>')

    # transitions
    for tr in transitions:
        lines.append('    <transition>')
        lines.append(f'      <source ref="S{tr["from"]}"/>')
        lines.append(f'      <target ref="S{tr["to"]}"/>')
        if tr.get("guard"):
            lines.append(f'      <label kind="guard">{tr["guard"]}</label>')
        if tr.get("assign"):
            lines.append(f'      <label kind="assignment">{tr["assign"]}</label>')
        lines.append('    </transition>')

    lines.append('  </template>')

    # system
    lines.append('  <system>')
    lines.append('    system Flatten;')
    lines.append('  </system>')

    # queries: 1) no two trains in train3, 2) gate3 and some train3
    lines.append('  <queries>')
    bad_states_1 = []
    for (g, c, ttuple, cntv) in states:
        cnt3 = sum(1 for t in ttuple if t == "train3")
        if cnt3 >= 2:
            base = f"{g}_{c}_" + "_".join(ttuple)
            if modeB:
                base += f"_cnt{cntv}"
            bad_states_1.append(f"Flatten.{base}")
    if bad_states_1:
        formula = " or ".join(bad_states_1)
        lines.append('    <query>')
        lines.append(f'      <formula>A[] !({formula})</formula>')
        lines.append('      <comment>No two trains may be in train3 simultaneously</comment>')
        lines.append('    </query>')

    bad_states_2 = []
    for (g, c, ttuple, cntv) in states:
        if g == "gate3" and any(t == "train3" for t in ttuple):
            base = f"{g}_{c}_" + "_".join(ttuple)
            if modeB:
                base += f"_cnt{cntv}"
            bad_states_2.append(f"Flatten.{base}")
    if bad_states_2:
        formula = " or ".join(bad_states_2)
        lines.append('    <query>')
        lines.append(f'      <formula>A[] !({formula})</formula>')
        lines.append('      <comment>Gate cannot be up (gate3) while any train is in train3</comment>')
        lines.append('    </query>')
    lines.append('  </queries>')

    lines.append('</nta>')

    Path(path).write_text("\n".join(lines), encoding="utf-8")
    print(f"Wrote flattened XML to: {path}")
    print(f"States: {len(states)}, Transitions: {len(transitions)}")

# -------------------------
# CLI
# -------------------------
def main():
    ap = argparse.ArgumentParser(description="Flatten Train-Gate-Controller UPPAAL model into a single UPPAAL XML automaton.")
    ap.add_argument("--N", type=int, required=True, help="number of trains")
    ap.add_argument("--mode", choices=["A", "B"], default="A", help="A: keep cnt var; B: encode cnt in location names (bounded 0..N)")
    ap.add_argument("--out", type=str, required=True, help="output XML filename")
    args = ap.parse_args()

    modeB = (args.mode == "B")
    states, transitions = flatten_model(args.N, modeB)
    write_xml(args.out, args.N, states, transitions, modeB)

if __name__ == "__main__":
    main()
