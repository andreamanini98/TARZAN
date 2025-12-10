#!/usr/bin/env python3
"""
Generate XTA models with BFrame processes in steps of 3: 3,6,...,30.

IandPframes layout:
 - extras = extra1 .. extra(2*N)
 - all extras declared urgent
 - bstart chain: L1 -> extra1 (bstart001), extra1 -> extra2 (bstart002), ..., extra{N} -> L0 (bstartNNN)
 - ibx chain: L7 -> extra{N+1} (ibx001), extra{N+1} -> extra{N+2} (ibx002), ..., extra{2N} -> L6 (ibxNNN)
 - spaces around <=, ==, >=, etc.
 - all transitions comma separated except the final one ends with a semicolon
"""

from pathlib import Path

DEADLINE = 6673

BFRAME_TEMPLATE = """process BFrame{n:03d}() {{
    clock x;
    state Bout, L0, L1, L2, L3, L4, L5, L6;
    init L6;
    trans
        L6 -> L6 {{ sync ibx{n:03d}?; assign ib{n:03d} = 1; }},
        L0 -> Bout {{ guard x <= 0; }},
        L1 -> L0 {{ guard x >= 189 && x <= 454 && mvb{n:03d} == 1; assign x = 0, mvb{n:03d} = 0; }},
        L2 -> L1 {{ guard x == 521; assign x = 0; }},
        L3 -> L2 {{ guard x <= 0 && berror{n:03d} == 1; assign x = 0, berror{n:03d} = 0; }},
        L4 -> L3 {{ guard x == 48; assign x = 0; }},
        L5 -> L4 {{ guard x >= 2399 && x <= 2422 && ib{n:03d} == 1 && B{n:03d} == 1;
                    assign mvb{n:03d} = 1, ib{n:03d} = 0, B{n:03d} = 0, x = 0; }},
        L6 -> L5 {{ sync bstart{n:03d}?; assign x = 0, B{n:03d} = 1, berror{n:03d} = 1; }};
}}
"""

def generate_iandpframes(num_frames: int) -> str:
    """
    Generate IandPframes process for num_frames BFrames.
    - bstart extras: extra1 .. extra{N-1}
    - ibx extras:    extra{N} .. extra{2N-2}
    - all extras urgent
    - transitions comma-separated, last ends with semicolon
    """
    assert num_frames >= 1

    # Each chain has (num_frames - 1) extras
    num_extras = num_frames - 1
    extras = [f"extra{i}" for i in range(1, 2 * num_extras + 1)]
    b_extras = extras[:num_extras]  # extra1 .. extra{N-1}
    i_extras = extras[num_extras:]  # extra{N} .. extra{2N-2}

    # Locations: L0, all extras, L1..L8, I
    locations = ", ".join(["L0"] + extras + ["L1", "L2", "L3", "L4", "L5", "L6", "L7", "L8", "I"])

    transitions = []

    # --- bstart chain: L1 -> extra1 -> ... -> extra{N-1} -> L0 ---
    transitions.append(f"L1 -> {b_extras[0]} {{ guard x <= 0; sync bstart001!; }}")
    for k in range(1, num_extras):
        transitions.append(f"{b_extras[k-1]} -> {b_extras[k]} {{ guard x <= 0; sync bstart{k+1:03d}!; }}")
    transitions.append(f"{b_extras[-1]} -> L0 {{ guard x <= 0; sync bstart{num_frames:03d}!; }}")

    # --- fixed core transitions ---
    transitions += [
        "L2 -> L1 { guard x == 515; assign x = 0; }",
        "L3 -> L2 { guard x == 521; sync dctq!; assign x = 0; }",
        "L4 -> L3 { guard perror == 1 && x <= 0; assign x = 0, perror = 0; }",
        "L5 -> L4 { guard x == 32; assign x = 0; }",
        "L6 -> L5 { guard x >= 1180 && x <= 1181 && P == 1; assign mvp = 1, P = 0, x = 0; }",
    ]

    # --- ibx chain: L7 -> extraN -> ... -> extra{2N-2} -> L6 ---
    transitions.append(f"L7 -> {i_extras[0]} {{ guard x <= 0; sync ibx001!; assign perror = 1, P = 1, x = 0; }}")
    for k in range(1, num_extras):
        transitions.append(f"{i_extras[k-1]} -> {i_extras[k]} {{ guard x <= 0; sync ibx{k+1:03d}!; assign perror = 1, P = 1, x = 0; }}")
    transitions.append(f"{i_extras[-1]} -> L6 {{ guard x <= 0; sync ibx{num_frames:03d}!; assign perror = 1, P = 1, x = 0; }}")

    # --- tail transitions ---
    transitions += [
        "L8 -> L7 { guard x == 673; assign x = 0; }",
        "I -> L8 { guard x == 595; sync dctq1!; assign x = 0; }",
    ]

    # Format transitions: comma after every transition except last; indent 4 spaces
    formatted_lines = []
    for i, t in enumerate(transitions):
        end = ";" if i == len(transitions) - 1 else ","
        formatted_lines.append(f"    {t}{end}")

    urgent_decl = ", ".join(extras)

    return f"""process IandPframes() {{
    clock x;
    int perror = 0;
    int P = 0;
    state {locations};
    urgent {urgent_decl};
    init I;
    trans
{chr(10).join(formatted_lines)}
}}
"""

def generate_model(num_frames: int) -> str:
    chans = [f"ibx{i:03d}, bstart{i:03d}" for i in range(1, num_frames + 1)]
    chans_str = ", ".join(["dctq1", "dctq"] + chans)

    ints = [f"{v}{i:03d}" for i in range(1, num_frames + 1)
            for v in ("ib", "mvb", "B", "berror")]
    all_ints = ["mvp"] + ints

    decls = f"""const int N = {num_frames};
const int DEADLINE = {DEADLINE};

chan {chans_str};

int {", ".join(all_ints)};
"""

    frames = "\n".join(BFRAME_TEMPLATE.format(n=i) for i in range(1, num_frames + 1))

    return f"""{decls}

process Time() {{
    clock x;
    state L0 {{ x <= DEADLINE }};
    init L0;
    trans
        L0 -> L0 {{}};
}}

{frames}

process Pfinish() {{
    clock x;
    state Pout, L0, L1, L2;
    init L2;
    trans
        L0 -> Pout {{ guard x <= 0; }},
        L1 -> L0 {{ guard x >= 378 && x <= 389 && mvp == 1; assign x = 0, mvp = 0; }},
        L2 -> L1 {{ sync dctq?; assign x = 0; }};
}}

process Ifinish() {{
    clock x;
    state Iout, L0, L1, L2;
    init L2;
    trans
        L0 -> Iout {{ guard x <= 0; }},
        L1 -> L0 {{ guard x >= 298 && x <= 358; assign x = 0; }},
        L2 -> L1 {{ sync dctq1?; assign x = 0; }};
}}

{generate_iandpframes(num_frames)}

system IandPframes, Ifinish, Pfinish, {", ".join(f"BFrame{i:03d}" for i in range(1, num_frames+1))}, Time;
"""

def generate_query(num_frames: int) -> str:
    """Generate UPPAAL query for the given number of frames."""
    conjuncts = " and ".join(f"BFrame{i:03d}.Bout" for i in range(1, num_frames + 1))
    return f"E<> {conjuncts}\n"

def main():
    outdir = Path("xta_models")
    outdir.mkdir(exist_ok=True)

    for num_frames in range(4, 21, 4):
        base = f"mpeg2_{num_frames:02d}"
        xta_path = outdir / f"{base}.xta"
        q_path = outdir / f"{base}.q"

        # Generate and write the .xta model
        xta_path.write_text(generate_model(num_frames))
        print(f"Generated {xta_path}")

        # --- NEW: generate .q file ---
        q_path.write_text(generate_query(num_frames))
        print(f"Generated {q_path}")

if __name__ == "__main__":
    main()