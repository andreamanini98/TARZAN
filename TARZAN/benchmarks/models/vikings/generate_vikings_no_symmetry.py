#!/usr/bin/env python3
from pathlib import Path

OUT_DIR = Path(".")
MIN_VIKINGS, MAX_VIKINGS = 2, 10
DELAY = 20  # fixed delay for all Vikings


def make_viking_process(i: int) -> str:
    """Return an explicit Viking process named VikingXX with delay"""
    return f"""process Viking{i:02d}() {{
  clock y;
  state L0, safe, L1, unsafe;
  init unsafe;
  trans
    L1 -> unsafe {{ guard y >= {DELAY}; sync release!; }},
    safe -> L1 {{ guard L == 1; sync take!; assign y = 0; }},
    L0 -> safe {{ guard y >= {DELAY}; sync release!; }},
    unsafe -> L0 {{ guard L == 0; sync take!; assign y = 0; }};
}}
"""


def make_torch_process() -> str:
    """Return Torch process exactly as requested"""
    return """process Torch() {

state
    one,
    L0,
    free,
    two;
urgent
    L0;
init
    free;
trans
    free -> L0 { sync take?; },
    L0 -> one { },
    L0 -> two { sync take?; },
    one -> free { sync release?; assign L = 1 - L; },
    two -> one { sync release?; };
}
"""


def make_model(n: int) -> str:
    """Build the full model with n Vikings (each explicit) and a Torch"""
    parts = ["chan take, release;", "int L;\n"]
    # Add explicit Viking processes
    for i in range(n):
        parts.append(make_viking_process(i))
    # Torch process
    parts.append(make_torch_process())
    # System declaration
    system_decl = "system " + ", ".join([f"Viking{i:02d}" for i in range(n)] + ["Torch"]) + ";"
    parts.append(system_decl)
    return "\n\n".join(parts)


def main():
    OUT_DIR.mkdir(exist_ok=True)
    for n in range(MIN_VIKINGS, MAX_VIKINGS + 1):
        fname = OUT_DIR / f"vikings_{n:02d}.xta"
        content = make_model(n)
        fname.write_text(content, encoding="utf-8")
        print(f"✓ Wrote {fname.name}")


if __name__ == "__main__":
    main()