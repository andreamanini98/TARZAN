#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Flatten Vikings (Soldier + Torch) into a single UPPAAL XML automaton.

This version fixes L = 1 - L handling in mode B: the destination L value is
computed from the raw torch assignment text (not from any normalized label).
"""
import argparse
import re
from itertools import product
from pathlib import Path
from typing import Optional, List, Tuple, Dict

# -------------------------
# Model (source XTA)
# -------------------------
CHANNELS = ["take", "release"]
GLOBAL_INT = "L"

SOLDIER_STATES = [("L0", None), ("safe", None), ("L1", None), ("unsafe", None)]
SOLDIER_INIT = "unsafe"
# Soldier transitions: (src, tgt, guard, sync, assign, reset_y_flag)
SOLDIER_TRANS = [
    ("L1", "unsafe", "y >= 20", "release!", None, False),
    ("safe", "L1", "L == 1", "take!", "y = 0", True),
    ("L0", "safe", "y >= 20", "release!", None, False),
    ("unsafe", "L0", "L == 0", "take!", "y = 0", True),
]

TORCH_STATES = [("one", None), ("L0", None), ("free", None), ("two", None)]
TORCH_INIT = "free"
TORCH_URGENT = {"L0"}
# Torch transitions: (src, tgt, guard, sync, assign)
TORCH_TRANS = [
    ("free", "L0", None, "take?", None),
    ("L0", "one", None, None, None),
    ("L0", "two", None, "take?", None),
    ("one", "free", None, "release?", "L = 1 - L"),
    ("two", "one", None, "release?", None),
]

# -------------------------
# Utilities
# -------------------------
def escape_uppaal(s: Optional[str]) -> Optional[str]:
    if s is None:
        return None
    r = s.replace("<=", "&lt;=").replace(">=", "&gt;=").replace("<", "&lt;").replace(">", "&gt;").strip()
    return r if r != "" else None

def split_conjuncts(guard: Optional[str]) -> List[str]:
    if not guard:
        return []
    g = guard.replace("&&", " and ")
    parts = [p.strip() for p in re.split(r'\band\b', g) if p.strip()]
    return parts

def contains_L(part: str) -> bool:
    return bool(re.search(r'\bL\b', part))

def eval_L_part(part: str, Lval: Optional[int]) -> Optional[bool]:
    if Lval is None:
        return None
    q = part.replace(" ", "")
    m = re.match(r'L==(\d+)', q)
    if m:
        return Lval == int(m.group(1))
    m = re.match(r'L!=?(\d+)', q)
    if m:
        return Lval != int(m.group(1))
    m = re.match(r'L>=(\d+)', q)
    if m:
        return Lval >= int(m.group(1))
    m = re.match(r'L<=(\d+)', q)
    if m:
        return Lval <= int(m.group(1))
    m = re.match(r'L>(\d+)', q)
    if m:
        return Lval > int(m.group(1))
    m = re.match(r'L<(\d+)', q)
    if m:
        return Lval < int(m.group(1))
    return None

def replace_y(expr: Optional[str], idx: Optional[int]) -> Optional[str]:
    if expr is None:
        return None
    s = expr
    if idx is not None:
        s = re.sub(r'\by\b', f"y{idx}", s)
    return escape_uppaal(s)

def normalize_assign(assign: Optional[str], idx: Optional[int], modeB: bool) -> Optional[str]:
    if not assign:
        return None
    s = assign.replace(";", ",")
    parts = [p.strip() for p in s.split(",") if p.strip()]
    out = []
    for p in parts:
        # drop L assignments in modeB (they must not appear)
        if modeB and re.search(r'\bL\b', p):
            continue
        if idx is not None:
            p = re.sub(r'\by\b', f"y{idx}", p)
        out.append(p)
    seen = set(); dedup = []
    for a in out:
        key = a.replace(" ", "")
        if key not in seen:
            seen.add(key); dedup.append(a)
    return ", ".join(dedup) if dedup else None

def parse_sync(sync: Optional[str]) -> Optional[Tuple[str,str]]:
    if not sync:
        return None
    m = re.match(r'\s*([A-Za-z_]\w*)\s*([!?])\s*', sync)
    if not m:
        return None
    return m.group(1), m.group(2)

# robust L computation from raw assign text
def compute_new_L_from_raw_assign(raw_assign: Optional[str], Lval: Optional[int]) -> Optional[int]:
    """
    Robustly compute new L (for modeB) given the raw assignment string from the torch transition.
    Accepts variants with spaces, semicolons, etc.
    Handles:
      - 'L = 1 - L'  (flip)
      - 'L = 0' or 'L = 1' (set)
      - None -> return original Lval
    """
    if raw_assign is None:
        return Lval
    s = raw_assign.replace(" ", "")
    # strip trailing semicolons if present
    s = s.rstrip(";")
    # match L=1-L (flip)
    if re.search(r'^L=1-?L$', s) or 'L=1-L' in s:
        if Lval is None:
            return None
        return 1 - Lval
    m = re.match(r'^L=(0|1)$', s)
    if m:
        return int(m.group(1))
    # more defensive: look for 'L=1-L' anywhere
    if 'L=1-L' in s:
        if Lval is None:
            return None
        return 1 - Lval
    # not recognized -> return original Lval
    return Lval

# -------------------------
# Build product states
# -------------------------
def build_states(N: int, modeB: bool):
    soldier_names = [s for s,_ in SOLDIER_STATES]
    torch_names = [s for s,_ in TORCH_STATES]
    prod_soldiers = list(product(soldier_names, repeat=N))
    states = []
    for t in torch_names:
        for st in prod_soldiers:
            if modeB:
                for Lv in (0,1):
                    states.append((t, st, Lv))
            else:
                states.append((t, st, None))
    index = {states[i]: i for i in range(len(states))}
    return states, index

# -------------------------
# Flattening transitions
# -------------------------
def flatten_model(N: int, modeB: bool):
    states, index = build_states(N, modeB)
    flat_trans = []

    # prepare dicts
    torch_by_src: Dict[str, List[Tuple]] = {}
    for tr in TORCH_TRANS:
        torch_by_src.setdefault(tr[0], []).append(tr)
    sold_by_src: Dict[str, List[Tuple]] = {}
    for tr in SOLDIER_TRANS:
        sold_by_src.setdefault(tr[0], []).append(tr)

    for s_idx, state in enumerate(states):
        torch_loc, sold_tuple, Lval = state

        # torch non-sync transitions
        for t_src, t_tgt, t_guard, t_sync, t_assign in torch_by_src.get(torch_loc, []):
            if t_sync:
                continue
            # compute dst L using raw torch assign
            dst_L = Lval
            if modeB:
                dst_L = compute_new_L_from_raw_assign(t_assign, Lval)
            guard_label = None if modeB else replace_y(t_guard, None)
            assign_label = normalize_assign(t_assign, None, modeB)
            dst = (t_tgt, sold_tuple, dst_L if modeB else None)
            dst_idx = index[dst]
            if dst[0] in TORCH_URGENT:
                assign_label = (assign_label + ", " if assign_label else "") + "z = 0"
            flat_trans.append({"from": s_idx, "to": dst_idx, "guard": guard_label, "assign": assign_label})

        # soldier non-sync transitions
        for i in range(N):
            s_src = sold_tuple[i]
            for s0, s_tgt, s_guard, s_sync, s_assign, s_reset in sold_by_src.get(s_src, []):
                if s_sync:
                    continue
                # handle L parts
                allowed = True
                rem = []
                for part in split_conjuncts(s_guard):
                    if contains_L(part):
                        if modeB:
                            ev = eval_L_part(part, Lval)
                            if ev is False:
                                allowed = False
                                break
                            if ev is None:
                                allowed = False
                                break
                            # if True => drop
                        else:
                            rem.append(part)
                    else:
                        rem.append(part)
                if not allowed:
                    continue
                guard_label = replace_y(" and ".join(rem), i+1) if rem else None
                assign_label = normalize_assign(s_assign, i+1, modeB)
                if s_reset:
                    assign_label = (assign_label + ", " if assign_label else "") + f"y{i+1} = 0"
                new_sold = list(sold_tuple)
                new_sold[i] = s_tgt
                dst = (torch_loc, tuple(new_sold), Lval if modeB else None)
                dst_idx = index[dst]
                flat_trans.append({"from": s_idx, "to": dst_idx, "guard": guard_label, "assign": assign_label})

        # synchronizations: soldier (!) with torch (?)
        for i in range(N):
            s_src = sold_tuple[i]
            for s0, s_tgt, s_guard, s_sync, s_assign, s_reset in sold_by_src.get(s_src, []):
                if not s_sync:
                    continue
                p = parse_sync(s_sync)
                if not p:
                    continue
                sch_name, sch_dir = p
                if sch_dir != "!":
                    continue
                # match with torch recv transitions from current torch_loc
                for t0, t_tgt, t_guard, t_sync, t_assign in torch_by_src.get(torch_loc, []):
                    if not t_sync:
                        continue
                    pt = parse_sync(t_sync)
                    if not pt:
                        continue
                    tch_name, tch_dir = pt
                    if tch_dir != "?" or tch_name != sch_name:
                        continue
                    # process soldier guard and torch guard: remove/evaluate L parts
                    def proc_guard(g_expr, soldier_idx):
                        parts = split_conjuncts(g_expr)
                        remaining = []
                        for part in parts:
                            if contains_L(part):
                                if modeB:
                                    ev = eval_L_part(part, Lval)
                                    if ev is False:
                                        return False, None
                                    if ev is None:
                                        return False, None
                                    # True -> drop
                                else:
                                    remaining.append(part)
                            else:
                                remaining.append(part)
                        if remaining:
                            return True, replace_y(" and ".join(remaining), soldier_idx)
                        return True, None
                    ok_s, g_s = proc_guard(s_guard, i+1)
                    if not ok_s:
                        continue
                    ok_t, g_t = proc_guard(t_guard, None)
                    if not ok_t:
                        continue
                    if g_s and g_t:
                        guard_label = f"{g_s} and {g_t}"
                    else:
                        guard_label = g_s or g_t
                    # assignments: normalized labels omit L in modeB
                    assign_s_label = normalize_assign(s_assign, i+1, modeB)
                    assign_t_label = normalize_assign(t_assign, None, modeB)
                    assign_label = None
                    if assign_s_label or assign_t_label:
                        assign_label = ", ".join([x for x in (assign_s_label, assign_t_label) if x])
                    if s_reset:
                        assign_label = (assign_label + ", " if assign_label else "") + f"y{i+1} = 0"
                    # crucial: compute dst_L from RAW t_assign (not from normalized)
                    dst_L = Lval
                    if modeB:
                        dst_L = compute_new_L_from_raw_assign(t_assign, Lval)
                    new_sold = list(sold_tuple)
                    new_sold[i] = s_tgt
                    dst = (t_tgt, tuple(new_sold), dst_L if modeB else None)
                    dst_idx = index[dst]
                    if dst[0] in TORCH_URGENT:
                        assign_label = (assign_label + ", " if assign_label else "") + "z = 0"
                    flat_trans.append({"from": s_idx, "to": dst_idx, "guard": guard_label, "assign": assign_label})

    return states, flat_trans

# -------------------------
# XML writer
# -------------------------
def write_xml(path: str, N: int, states: List[Tuple], transitions: List[Dict], modeB: bool):
    lines: List[str] = []
    lines.append('<?xml version="1.0" encoding="utf-8"?>')
    lines.append('<!DOCTYPE nta PUBLIC "-//Uppaal Team//DTD Flat System 1.1//EN" '
                 '"http://www.it.uu.se/research/group/darts/uppaal/flat-1_1.dtd">')
    lines.append('<nta>')
    lines.append('  <template>')
    lines.append('    <name>Vikings_flat</name>')
    lines.append('    <declaration>')
    for i in range(N):
        lines.append(f'      clock y{i+1};')
    lines.append('      clock z;')
    if not modeB:
        lines.append(f'      int {GLOBAL_INT} = 0;')
    for ch in CHANNELS:
        lines.append(f'      chan {ch};')
    lines.append('    </declaration>')
    spacing = 220
    cols = int(max(1, round(len(states) ** 0.5)))
    for idx, (tloc, sold_tuple, Lval) in enumerate(states):
        x = spacing * (idx % cols)
        y = spacing * (idx // cols)
        base = f"{tloc}_" + "_".join(sold_tuple)
        name = base + (f"_L{Lval}" if modeB else "")
        lines.append(f'    <location id="S{idx}" x="{x}" y="{y}">')
        lines.append(f'      <name>{name}</name>')
        if tloc in TORCH_URGENT:
            lines.append('      <label kind="invariant">z &lt;= 0</label>')
        lines.append('    </location>')
    init_state = ("free", tuple([SOLDIER_INIT] * N), 0 if modeB else None)
    init_idx = states.index(init_state)
    lines.append(f'    <init ref="S{init_idx}"/>')
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
    lines.append('  <system>')
    lines.append('    system Vikings_flat;')
    lines.append('  </system>')
    # queries: E<> all soldiers safe
    lines.append('  <queries>')
    torch_names = [s for s,_ in TORCH_STATES]
    safe_targets = []
    for t in torch_names:
        if modeB:
            for Lv in (0,1):
                name = f"{t}_" + "_".join(["safe"]*N) + f"_L{Lv}"
                safe_targets.append(f"Vikings_flat.{name}")
        else:
            name = f"{t}_" + "_".join(["safe"]*N)
            safe_targets.append(f"Vikings_flat.{name}")
    if safe_targets:
        formula = " or ".join(safe_targets)
        lines.append('    <query>')
        lines.append(f'      <formula>E&lt;&gt; ({formula})</formula>')
        lines.append('      <comment>Eventually all soldiers reach safe</comment>')
        lines.append('    </query>')
    lines.append('  </queries>')
    lines.append('</nta>')
    Path(path).write_text("\n".join(lines), encoding="utf-8")
    print(f"Wrote flattened Vikings model (N={N}, mode={'B' if modeB else 'A'}) to: {path}")
    print(f"Locations: {len(states)}, Transitions: {len(transitions)}")

# -------------------------
# CLI
# -------------------------
def main():
    ap = argparse.ArgumentParser(description="Flatten Vikings (Soldier + Torch) into a single UPPAAL XML automaton.")
    ap.add_argument("--N", type=int, required=True, help="number of Soldiers")
    ap.add_argument("--mode", choices=["A","B"], default="A", help="A: keep L variable; B: encode L in location names")
    ap.add_argument("--out", required=True, help="output XML filename")
    args = ap.parse_args()
    modeB = (args.mode == "B")
    states, transitions = flatten_model(args.N, modeB)
    write_xml(args.out, args.N, states, transitions, modeB)

if __name__ == "__main__":
    main()
