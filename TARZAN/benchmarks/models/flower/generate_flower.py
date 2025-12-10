#!/usr/bin/env python3
# Generate XTA timed automata with N clocks (x1..xN) and clock y

def generate_xta(n: int) -> str:
    """Generate an XTA model string with n clocks."""
    clocks = ", ".join([f"x{i}" for i in range(1, n + 1)]) + ", y"

    transitions = []
    for i in range(n, 0, -1):
        transitions.append(f"    Start -> Start {{ guard x{i} == {i}; assign x{i} = 0; }},")

    # Final transition to Goal
    all_zero = " and ".join([f"x{i} == 0" for i in range(1, n + 1)])
    transitions.append(f"    Start -> Goal {{ guard {all_zero} and y >= 1; }};")

    transitions_str = "\n".join(transitions)

    return f"""// Automatically generated XTA flower model for N={n} clocks

process Flower() {{
clock {clocks};
state
    Start,
    Goal;
init
    Start;
trans
{transitions_str}
}}

// Template instantiation
Process = Flower();

// System definition
system Process;
"""


def main():
    for n in range(1, 17):  # Generate models for N = 1..16
        filename = f"flower_{n}.xta"
        with open(filename, "w") as f:
            f.write(generate_xta(n))
        print(f"Generated {filename}")


if __name__ == "__main__":
    main()