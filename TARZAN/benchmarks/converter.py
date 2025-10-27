#!/usr/bin/env python3
"""
xta_to_liana.py

Convert .xta-like process definitions into Liana .txt files.
Boolean variables are treated as integers.
"""

import re
from pathlib import Path


def expr_to_tuple(expr, consts=None):
    expr = expr.strip()
    m = re.match(r"(\w+)\s*([<>=!]=?|==)\s*(\w+)", expr)
    if m:
        var, op, val = m.groups()
        if consts and val in consts:
            val = consts[val]
        if val.lower() == "true":
            val = "1"
        elif val.lower() == "false":
            val = "0"
        return f"({var}, {op}, {val})"
    return expr


def substitute_expr(expr, consts=None, bools=None):
    s = expr.strip()
    s = re.sub(r'\btrue\b', '1', s, flags=re.IGNORECASE)
    s = re.sub(r'\bfalse\b', '0', s, flags=re.IGNORECASE)
    if consts:
        for k in sorted(consts.keys(), key=len, reverse=True):
            s = re.sub(rf'\b{k}\b', str(consts[k]), s)
    return s


def extract_global_booleans(xta_text: str):
    bools = {}
    for m in re.finditer(r'^\s*bool\s+(\w+)\s*;', xta_text, re.MULTILINE):
        bools[m.group(1)] = None
    return bools


def extract_global_integers(xta_text: str):
    ints = []
    for m in re.finditer(r'^\s*int\s+([\w\s,]+);', xta_text, re.MULTILINE):
        for var in m.group(1).split(','):
            ints.append(var.strip())
    return ints


def parse_xta_process(process_text, consts):
    name_match = re.search(r'process\s+(\w+)\s*\(', process_text)
    if not name_match:
        return None
    name = name_match.group(1)

    clocks = []
    for m in re.finditer(r'clock\s+([\w\s,]+);', process_text):
        for var in m.group(1).split(','):
            v = var.strip()
            if v:
                clocks.append(v)
    clocks_set = set(clocks)
    clocks_decl = ", ".join(clocks) + ";" if clocks else ""

    ints = [m for m in re.findall(r'int\s+(\w+)\s*(?:=\s*[^;]+)?\s*;', process_text)]

    bools = {}
    for m in re.finditer(r'bool\s+(\w+)(?:\s*=\s*(true|false|0|1))?\s*;', process_text, re.IGNORECASE):
        name_b, val_b = m.groups()
        if val_b is None:
            bools[name_b] = None
        else:
            bools[name_b] = 1 if val_b.lower() in ("true", "1") else 0

    state_block = re.search(r'state\s+(.*?)\s*;', process_text, re.S)
    locations = []
    if state_block:
        for loc in re.finditer(r'(\w+)\s*(?:\{\s*([^}]*)\s*\})?', state_block.group(1)):
            loc_name, inv = loc.groups()
            inv_str = ""
            if inv:
                inv_items = []
                for item in re.split(r'&&|\band\b', inv):
                    item = item.strip()
                    if re.match(r'^\w+\s*[<>=!]=?\s*\w+$', item):
                        inv_items.append(expr_to_tuple(item, consts))
                    else:
                        inv_items.append(substitute_expr(item, consts, bools))
                inv_str = "inv: [" + ", ".join(inv_items) + "]"
            locations.append((loc_name, inv_str))

    init_match = re.search(r'init\s+(\w+)\s*;', process_text)
    init_loc = init_match.group(1) if init_match else None

    urgent_locs = set(re.findall(r'urgent\s+(\w+);', process_text))

    transitions = []
    actions_set = set()
    trans_match = re.search(r'trans(?:itions)?\s+(.*?)\n\}', process_text, re.S)
    if trans_match:
        trans_text = trans_match.group(1).strip()
        parts = re.split(r',\s*(?=\w+\s*->)', trans_text)
        for part in parts:
            m = re.match(r'(\w+)\s*->\s*(\w+)\s*\{([^}]*)\}', part.strip())
            if not m:
                continue
            src, dst, body = m.groups()

            sync_match = re.search(r'sync\s+([\w!?]+);', body)
            action = sync_match.group(1) if sync_match else "a"
            actions_set.add(re.sub(r'[!?]', '', action))

            guard_match = re.search(r'guard\s+([^;]+);?', body)
            clock_guards = []
            bool_expr_items = []
            if guard_match:
                for g in re.split(r'&&|\band\b', guard_match.group(1)):
                    g = g.strip()
                    if re.match(r'^\w+\s*[<>=!]=?\s*\w+$', g) and g.split()[0] in clocks_set:
                        clock_guards.append(expr_to_tuple(g, consts))
                    else:
                        bool_expr_items.append(substitute_expr(g, consts, bools))
            guard_str = "[" + ", ".join(clock_guards) + "]" if clock_guards else "[]"
            bool_expr_str = " && ".join(bool_expr_items) if bool_expr_items else ""

            resets = []
            assigns = []
            assign_match = re.search(r'assign\s+([^;]+);', body)
            if assign_match:
                for a in assign_match.group(1).split(','):
                    a = a.strip()
                    m_assign = re.match(r'^(\w+)\s*[:]?=\s*(.+)$', a)
                    if m_assign:
                        var, val = m_assign.groups()
                        val = substitute_expr(val, consts, bools)
                        if var in clocks_set and val == "0":
                            resets.append(var)
                        else:
                            assigns.append(f"{var} = {val}")
            resets_str = "[" + ", ".join(resets) + "]" if resets else "[]"
            assigns_str = "[" + ", ".join(assigns) + "]" if assigns else ""

            transitions.append({
                "src": src,
                "dst": dst,
                "action": action,
                "guard": guard_str,
                "bool": bool_expr_str,
                "resets": resets_str,
                "assigns": assigns_str
            })

    return {
        "name": name,
        "clocks_decl": clocks_decl,
        "clocks": clocks,
        "ints": ints,
        "bools": bools,
        "locations": locations,
        "init": init_loc,
        "urgent": urgent_locs,
        "transitions": transitions,
        "actions_set": actions_set
    }


def process_to_liana_text(process):
    lines = [f"create automaton {process['name']}", "{"]
    lines.append(f"    clocks {{ {process['clocks_decl']} }}")
    actions = process['actions_set']
    lines.append(f"    actions {{ {', '.join(sorted(actions)) + ';' if actions else 'a;'} }}")
    all_ints = sorted(set(process['ints']).union(process['bools'].keys()))
    if all_ints:
        lines.append(f"    integers {{ {', '.join(all_ints)}; }}")

    # locations
    lines.append("    locations {")
    max_loc_len = max((len(loc) for loc, _ in process["locations"]), default=0)
    max_ini_len = 0
    for loc, inv in process["locations"]:
        ini = "ini: T" if loc == process["init"] else "ini: F"
        if loc in process["urgent"]:
            ini += ", urg: T"
        if inv:
            ini += f", {inv}"
        max_ini_len = max(max_ini_len, len(ini))
    num_locs = len(process["locations"])
    for i, (loc, inv) in enumerate(process["locations"]):
        ini = "ini: T" if loc == process["init"] else "ini: F"
        if loc in process["urgent"]:
            ini += ", urg: T"
        if inv:
            ini += f", {inv}"
        loc_fmt = loc.ljust(max_loc_len)
        ini_fmt = ini.ljust(max_ini_len)
        sep = "," if i < num_locs - 1 else ";"
        lines.append(f"        {loc_fmt} <{ini_fmt}>{sep}")
    lines.append("    }")

    # transitions
    lines.append("    transitions {")
    for i, t in enumerate(process["transitions"]):
        parts = [t["src"], t["action"]]
        parts.append(t["guard"] if t["guard"] else "[]")
        if t["bool"].strip():
            parts.append(t["bool"])
        parts.append(t["resets"] if t["resets"] else "[]")
        if t["assigns"].strip():
            parts.append(t["assigns"])
        parts.append(t["dst"])
        tup_str = f"({', '.join(parts)})"
        sep = "," if i < len(process["transitions"]) - 1 else ";"
        lines.append(f"        {tup_str}{sep}")
    lines.append("    }")
    lines.append("}")
    return "\n".join(lines)


def convert_xta_file(input_path, output_root="converted_xta"):
    text = Path(input_path).read_text(encoding="utf-8", errors="ignore")
    consts = dict(re.findall(r'const\s+int\s+(\w+)\s*=\s*(\d+)\s*;', text))
    global_bools = extract_global_booleans(text)
    global_ints = extract_global_integers(text)

    text_clean = re.sub(r"//.*", "", text)
    text_clean = re.sub(r"(?s)const\s+int\s+.*?;", "", text_clean)
    text_clean = re.sub(r"system\s+.*?;", "", text_clean)

    processes = re.findall(r'process\s+\w+\s*\(.*?\)\s*\{.*?\n\}', text_clean, re.S)
    out_dir = Path(output_root) / Path(input_path).stem
    out_dir.mkdir(parents=True, exist_ok=True)

    for proc_text in processes:
        parsed = parse_xta_process(proc_text, consts)
        parsed['bools'].update(global_bools)
        parsed['ints'].extend([i for i in global_ints if i not in parsed['ints']])
        liana_txt = process_to_liana_text(parsed)
        out_path = out_dir / f"{parsed['name']}.txt"
        out_path.write_text(liana_txt, encoding="utf-8")
        print(f"✓ Converted automaton '{parsed['name']}' → {out_path}")


if __name__ == "__main__":
    import sys
    if len(sys.argv) < 2:
        print("Usage: python xta_to_liana.py <xta_folder_or_file>")
        sys.exit(1)
    folder = Path(sys.argv[1])
    if folder.is_file():
        convert_xta_file(folder)
    else:
        for file in folder.glob("*.xta"):
            convert_xta_file(file)