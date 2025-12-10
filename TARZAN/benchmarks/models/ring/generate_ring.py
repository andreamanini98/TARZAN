#!/usr/bin/env python3
"""
generate_networks.py

Generate .xta files containing networks of timed automata.
For each N in 2..10, produce a file network_N.xta containing N processes
Ring0..Ring{N-1} and a system instantiating P0..P{N-1}.

Each RingK is the same structural automaton as in the example:
 - clock x;
 - states: L0, L1, L2, L3, L4, goal
 - transitions are a 6-step ring (goal->L0, L4->goal, L3->L4, ... L0->L1)
 - each transition guarded with: x == (K+1) and assigns x = 0
"""

from pathlib import Path

# Configurable parameters
OUT_DIR = Path(".")
MIN_N = 2
MAX_N = 10
NUM_LOCATIONS = 5  # L0 .. L4

Ring_PROC = """process {tname}() {{
clock x;
state
    L0,
    L1,
    L2,
    L3,
    L4,
    goal;
init
    L0;
trans
{transitions}
}}
"""

TRANSITION_PATTERN = """    goal -> L0 {{ guard x == {k}; assign x = 0; }},
    L4 -> goal {{ guard x == {k}; assign x = 0; }},
    L3 -> L4 {{ guard x == {k}; assign x = 0; }},
    L2 -> L3 {{ guard x == {k}; assign x = 0; }},
    L1 -> L2 {{ guard x == {k}; assign x = 0; }},
    L0 -> L1 {{ guard x == {k}; assign x = 0; }};"""

SYSTEM_HEADER = "// Place Ring instantiations here.\n"

def generate_network(N: int) -> str:
    """
    Return the .xta content for a network with N automata.
    Each automaton Ring{i} uses guard x == (i+1).
    """
    procs_text = []
    for i in range(N):
        tname = f"Ring{i}"
        k = i + 1
        transitions = TRANSITION_PATTERN.format(k=k)
        proc = Ring_PROC.format(tname=tname, transitions=transitions)
        procs_text.append(proc)

    # Instantiate process variables P0..P{N-1}
    inst_lines = []
    for i in range(N):
        # instantiate P{i} = Ring{i}();
        inst_lines.append(f"P{i} = Ring{i}();")
    inst_block = "\n".join(inst_lines)

    # system line listing all processes
    system_line = "system " + ", ".join([f"P{i}" for i in range(N)]) + ";"

    # assemble whole file
    file_text = "// Place global declarations here.\n\n"
    file_text += "\n\n".join(procs_text)
    file_text += "\n\n" + SYSTEM_HEADER + inst_block + "\n" + system_line + "\n"
    return file_text

def main():
    for n in range(MIN_N, MAX_N + 1):
        fname = OUT_DIR / f"ring_{n}.xta"
        content = generate_network(n)
        fname.write_text(content, encoding="utf-8")
        print(f"✓ Wrote {fname}")

if __name__ == "__main__":
    main()