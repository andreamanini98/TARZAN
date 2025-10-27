#!/usr/bin/env python3

HEADER = """const int N = {n};

const int a = 2;
const int b = 5;
const int c = 1;
const int d = 2;
const int e = 1;

int cnt;

chan texit;
chan lower;
chan raise;
chan approach;
"""

TRAIN_TEMPLATE = """process Train{idx:02d}() {{
    clock x;
state
    train0,
    train1 {{ x <= b }},
    train2 {{ x <= b }},
    train3 {{ x <= b }};
init
    train0;
trans
    train2 -> train3 {{ }},
    train3 -> train0 {{ guard x <= b; sync texit?; }},
    train1 -> train2 {{ guard x > a; }},
    train0 -> train1 {{ sync approach?; assign x = 0; }};
}}
"""

GATE_PROCESS = """process gate() {
    clock y;
state
    gate0,
    gate1 { y <= c },
    gate2,
    gate3 { y <= d };
init
    gate0;
trans
    gate1 -> gate2 { guard y <= c; },
    gate2 -> gate3 { sync raise?; assign y = 0; },
    gate0 -> gate1 { sync lower?; assign y = 0; },
    gate3 -> gate0 { guard y >= c && y <= d; };
}
"""

CONTROLLER_PROCESS = """process controller() {
    clock z;
state
    controller0,
    controller1 { z <= e },
    controller2,
    controller3 { z <= e };
init
    controller0;
trans
    controller0 -> controller1 { sync approach!; assign z = 0, cnt = 1; },
    controller1 -> controller1 { sync approach!; assign cnt = cnt + 1; },
    controller1 -> controller1 { sync texit!; assign cnt = cnt - 1; },
    controller2 -> controller2 { sync approach!; assign cnt = cnt + 1; },
    controller2 -> controller2 { guard cnt > 1; sync texit!; assign cnt = cnt - 1; },
    controller2 -> controller3 { guard cnt == 1; sync texit!; assign z = 0, cnt = 0; },
    controller3 -> controller0 { guard z <= e; sync raise!; },
    controller3 -> controller2 { guard z <= e; sync approach!; assign cnt = cnt + 1; },
    controller1 -> controller2 { guard z == e; sync lower!; };
}
"""

def generate_xta(n: int) -> str:
    """Generate the full XTA text for n trains."""
    parts = [HEADER.format(n=n)]
    # Add all trains
    for i in range(1, n + 1):
        parts.append(TRAIN_TEMPLATE.format(idx=i))
    # Add gate and controller
    parts.append(GATE_PROCESS)
    parts.append(CONTROLLER_PROCESS)
    # System declaration
    train_names = ", ".join(f"Train{i:02d}" for i in range(1, n + 1))
    parts.append(f"system {train_names}, gate, controller;\n")
    return "\n".join(parts)

def main():
    for n in range(2, 10):
        xta_content = generate_xta(n)
        filename = f"trainAHV93_{n}.xta"
        with open(filename, "w") as f:
            f.write(xta_content)
        print(f"✅ Generated {filename}")

if __name__ == "__main__":
    main()