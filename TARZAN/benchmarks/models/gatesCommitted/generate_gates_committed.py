#!/usr/bin/env python3
"""
generate_gates_committed.py

Generate the gatesCommitted family in both formats: Uppaal XTA (for verifyta) and
Liana (for TARZAN), for N = 2, 4, ..., 16 keys.

This is the committed-location counterpart of the gates benchmark: the shared
counter is incremented from a committed location, so the increment is atomic with
respect to the rest of the network.

Rules:
- global int gate;
- key_i() has locations q0 (init), q1, C (committed), done
  * q0 -> q1  with guard x == i; assign x = 0;
  * q1 -> C   with guard x == i; assign x = 0;
  * C  -> done            assign gate = gate + 1;   (committed: atomic)
- unlocker() has u0 (init), goal
  * u0 -> goal with guard y == 2 * N && gate == N;
- query: E<> unlocker.goal
  Reaching goal requires every key to have incremented exactly by the time y == 2N,
  so the N independent punctual clocks must all have fired.

The generator writes both formats itself instead of going through
converters/convert_xta_to_liana.py, because that converter handles urgent
locations only and would silently drop the "commit C;" declarations.

Run it from anywhere: output paths are relative to this file.
"""

from pathlib import Path

SIZES = range(2, 17, 2)


def generate_xta_model(num_keys: int) -> str:
    lines = [f"// Auto-generated XTA model for {num_keys} committed keys\n",
             "int gate;\n"]

    for i in range(1, num_keys + 1):
        lines.append(f"process key{i:02d}() {{")
        lines.append("    clock x;")
        lines.append("    state")
        lines.append("        q0,\n        q1,\n        C,\n        done;")
        lines.append("    commit")
        lines.append("        C;")
        lines.append("    init")
        lines.append("        q0;")
        lines.append("    trans")
        lines.append(f"        q0 -> q1 {{ guard x == {i}; assign x = 0; }},")
        lines.append(f"        q1 -> C {{ guard x == {i}; assign x = 0; }},")
        lines.append("        C -> done { assign gate = gate + 1; };")
        lines.append("}\n")

    lines.append("process unlocker() {")
    lines.append("    clock y;")
    lines.append("    state")
    lines.append("        u0,\n        goal;")
    lines.append("    init")
    lines.append("        u0;")
    lines.append("    trans")
    lines.append(f"        u0 -> goal {{ guard y == {2 * num_keys} && gate == {num_keys}; }};")
    lines.append("}\n")

    instances = ", ".join([f"key{i:02d}" for i in range(1, num_keys + 1)] + ["unlocker"])
    lines.append(f"system {instances};\n")

    return "\n".join(lines)


def generate_liana_key(index: int) -> str:
    return f"""create automaton key{index:02d}
{{
    clocks {{ x; }}
    actions {{ a; }}
    integers {{ gate; }}
    locations {{
        q0   <ini: T>,
        q1   <ini: F>,
        C    <ini: F, com: T>,
        done <ini: F>;
    }}
    transitions {{
        (q0, a, [(x, ==, {index})], [x], q1),
        (q1, a, [(x, ==, {index})], [x], C),
        (C, a, [], [], [gate = gate + 1], done);
    }}
}}
"""


def generate_liana_unlocker(num_keys: int) -> str:
    return f"""create automaton unlocker
{{
    clocks {{ y; }}
    actions {{ a; }}
    integers {{ gate; }}
    locations {{
        u0   <ini: T>,
        goal <ini: F>;
    }}
    transitions {{
        (u0, a, [(y, ==, {2 * num_keys})], gate == {num_keys}, [], goal);
    }}
}}
"""


def main():
    root = Path(__file__).resolve().parent

    for num_keys in SIZES:
        instance = f"gatesCommitted_{num_keys:02d}"

        # Uppaal: one .xta plus the query.
        xta_dir = root / "xta" / instance
        xta_dir.mkdir(parents=True, exist_ok=True)
        (xta_dir / f"{instance}.xta").write_text(generate_xta_model(num_keys), encoding="utf-8")
        (xta_dir / "gatesCommitted.q").write_text("E<> unlocker.goal\n", encoding="utf-8")

        # Liana: one .txt per automaton.
        liana_dir = root / "liana" / instance
        liana_dir.mkdir(parents=True, exist_ok=True)
        for i in range(1, num_keys + 1):
            (liana_dir / f"key{i:02d}.txt").write_text(generate_liana_key(i), encoding="utf-8")
        (liana_dir / "unlocker.txt").write_text(generate_liana_unlocker(num_keys), encoding="utf-8")

        print(f"Generated: {instance} ({num_keys} keys + unlocker)")


if __name__ == "__main__":
    main()
