#!/usr/bin/env python3
"""
generate_toggle_networks.py
---------------------------

Generate .xta files for a scalable network of timed automata.

Each network_XX.xta (with zero-padded index) contains:

  • Global integers ctr1..ctrN
  • N processes BooleanXX (zero-padded) each with:
        clock x;
        two states s0, s1;
        transitions toggling ctr<i> when x == i
  • Instantiations PXX and
        system P01, P02, ..., PNN;

Creates one file per N in 2..16.
"""

from pathlib import Path

OUT_DIR = Path(".")
MIN_N, MAX_N = 2, 16


def make_globals(n: int) -> str:
    lines = ["// ========================================================",
             "// Global declarations",
             "// ========================================================"]
    for i in range(1, n + 1):
        lines.append(f"int ctr{i};")
    return "\n".join(lines) + "\n"


def make_Boolean(i: int) -> str:
    name = f"Boolean{i:02d}"
    return f"""// ========================================================
// {name}
// ========================================================
process {name}() {{
  clock x;
  state s0, s1;
  init s0;
  trans
    s0 -> s1 {{ guard x == {i}; assign x = 0, ctr{i} = 1 - ctr{i}; }},
    s1 -> s0 {{ guard x == {i}; assign x = 0, ctr{i} = 1 - ctr{i}; }};
}}
"""


def make_system_block(n: int) -> str:
    lines = ["// ========================================================",
             "// Boolean instantiations and system",
             "// ========================================================"]
    for i in range(1, n + 1):
        lines.append(f"P{i:02d} = Boolean{i:02d}();")
    lines.append("system " + ", ".join([f"P{i:02d}" for i in range(1, n + 1)]) + ";")
    return "\n".join(lines) + "\n"


def make_network_file(n: int) -> str:
    parts = [make_globals(n)]
    for i in range(1, n + 1):
        parts.append(make_Boolean(i))
    parts.append(make_system_block(n))
    return "\n".join(parts)


def main():
    OUT_DIR.mkdir(exist_ok=True)
    for n in range(MIN_N, MAX_N + 1):
        fname = OUT_DIR / f"boolean_{n:02d}.xta"
        content = make_network_file(n)
        fname.write_text(content, encoding="utf-8")
        print(f"✓ Wrote {fname}")


if __name__ == "__main__":
    main()