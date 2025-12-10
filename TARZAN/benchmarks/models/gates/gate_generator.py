#!/usr/bin/env python3
"""
Generate Uppaal XTA models gates_1.xta ... gates_16.xta

Rules:
- global int gate;
- key_i() has i+1 locations L0..Li
  * init L0
  * transitions L0->L1, ..., L{i-2}->L{i-1} with guard x == i; assign x = 0;
  * last transition L{i-1}->L{i} with guard x == i; assign gate = gate + 1;
- unlocker() has G0..G{N}, goal (total N+2 locs)
  * init G0
  * transitions G0->G1, ..., G{N-1}->G{N}, G{N}->G0 with guard x == N; assign x = 0;
  * plus G{N}->goal with guard x == N && gate == N; assign x = 0;
- system instantiates all keys, unlocker
"""

from pathlib import Path

def generate_xta_model(num_keys: int) -> str:
    lines = []
    lines.append(f"// Auto-generated XTA model for {num_keys} keys\n")
    lines.append("int gate;\n\n")

    # --- key_i processes ---
    for i in range(1, num_keys + 1):
        lines.append(f"process key{i:02d}() {{")
        lines.append("    clock x;")
        locs = ",\n    ".join(f"L{j}" for j in range(i + 1))
        lines.append("    state")
        lines.append("        " + locs + ";")
        # init L0
        lines.append("    init\n        L0;")
        lines.append("    trans")
        trans = []
        # first i-1 transitions with reset
        for j in range(i - 1):
            trans.append(f"        L{j} -> L{j+1} {{ guard x == {i}; assign x = 0; }}")
        # last transition increments gate
        trans.append(f"        L{i-1} -> L{i} {{ guard x == {i}; assign gate = gate + 1; }}")
        lines.append(",\n".join(trans) + ";")
        lines.append("}\n")

    # --- unlocker() process ---
    lines.append("process unlocker() {")
    lines.append("    clock x;")
    G_indices = [f"G{j}" for j in range(num_keys + 1)]
    locations = G_indices + ["goal"]
    lines.append("    state")
    lines.append("        " + ",\n    ".join(locations) + ";")
    lines.append("    init\n        G0;")
    lines.append("    trans")
    trans = []
    # G0->G1..G{N-1}->G{N}
    for j in range(num_keys):
        trans.append(f"        G{j} -> G{j+1} {{ guard x == {num_keys}; assign x = 0; }}")
    # G{N}->goal guarded by gate == N
    trans.append(f"        G{num_keys} -> goal {{ guard x == {num_keys} && gate == {num_keys}; assign x = 0; }}")
    lines.append(",\n".join(trans) + ";")
    lines.append("}\n")

    # --- system instantiation ---
    instances = ", ".join([f"key{i:02d}" for i in range(1, num_keys + 1)] + ["unlocker"])
    lines.append(f"system {instances};\n")

    return "\n".join(lines)


def main():
    out_dir = Path(".")
    for N in range(1, 17):
        text = generate_xta_model(N)
        filename = out_dir / f"gates_{N:02d}.xta"
        filename.write_text(text, encoding="utf-8")
        print(f"Generated: {filename}")

if __name__ == "__main__":
    main()