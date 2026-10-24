#!/usr/bin/env python3
"""
generate_mpeg2_broadcast.py

Generate the broadcast mpeg2 family in both formats, Uppaal XTA (for verifyta) and
Liana (for TARZAN), for N = 4, 8, ..., 20 B-frames: the same sizes as the mpeg2 family.

This is the original model of the TAPAAL experiments (uppaal-original_sym/mpeg2_bN.xml),
where IandPframes wakes every B-frame with the two broadcast channels bstart and ibx.
The mpeg2 family of this repository is instead a point-to-point rewrite: one binary
channel per B-frame, fired in a zero-time chain, plus an always-enabled self-loop on Time.
Keeping both families lets the two synchronisation styles be compared on the same sizes.

Differences from the original Uppaal file, all applied to both formats:
  - typedef scalar[N] is expanded into explicit BFrame001 ... BFrameNNN processes, so
    Uppaal's symmetry reduction plays no part;
  - the bool variables become ints (Liana has no bools);
  - DEADLINE = 6673 instead of 6672: the reachable variant (with 6672 the query is
    unreachable and neither tool terminates on the larger instances).

The `system` line lists the processes in alphabetical order, which is the order in which
TARZAN's parseTimedAutomataFromFolder reads the Liana files: depth-first search then
expands the automata in the same order in both tools.

Query: E<> BFrame001.Bout and ... and BFrameNNN.Bout

Run it from anywhere: output paths are relative to this file.
"""

from pathlib import Path

SIZES = range(4, 21, 4)
DEADLINE = 6673


# ----------------------------------------------------------------------------- XTA

def xta_bframe(i: int) -> str:
    n = f"{i:03d}"
    return f"""process BFrame{n}() {{
    clock x;
    state Bout, id2, id3, id4, id5, id6, id7, id8;
    init id8;
    trans
        id8 -> id8 {{ sync ibx?; assign ib{n} = 1; }},
        id2 -> Bout {{ guard x <= 0; }},
        id3 -> id2 {{ guard x >= 189 && x <= 454 && mvb{n} == 1; assign x = 0, mvb{n} = 0; }},
        id4 -> id3 {{ guard x == 521; assign x = 0; }},
        id5 -> id4 {{ guard x <= 0 && berror{n} == 1; assign x = 0, berror{n} = 0; }},
        id6 -> id5 {{ guard x == 48; assign x = 0; }},
        id7 -> id6 {{ guard x >= 2399 && x <= 2422 && ib{n} == 1 && B{n} == 1; assign mvb{n} = 1, ib{n} = 0, B{n} = 0, x = 0; }},
        id8 -> id7 {{ sync bstart?; assign x = 0, B{n} = 1, berror{n} = 1; }};
}}
"""

XTA_FIXED = {
    "IandPframes": """process IandPframes() {
    clock x;
    state id17, id18, id19, id20, id21, id22, id23, id24, id25, I;
    init I;
    trans
        id18 -> id17 { guard x <= 0; sync bstart!; },
        id19 -> id18 { guard x == 515; assign x = 0; },
        id20 -> id19 { guard x == 521; sync dctq!; assign x = 0; },
        id21 -> id20 { guard x <= 0 && perror == 1; assign x = 0, perror = 0; },
        id22 -> id21 { guard x == 32; assign x = 0; },
        id23 -> id22 { guard x >= 1180 && x <= 1181 && P == 1; assign mvp = 1, P = 0, x = 0; },
        id24 -> id23 { guard x <= 0; sync ibx!; assign perror = 1, P = 1, x = 0; },
        id25 -> id24 { guard x == 673; assign x = 0; },
        I -> id25 { guard x == 595; sync dctq1!; assign x = 0; };
}
""",
    "Ifinish": """process Ifinish() {
    clock x;
    state Iout, id14, id15, id16;
    init id16;
    trans
        id14 -> Iout { guard x <= 0; },
        id15 -> id14 { guard x >= 298 && x <= 358; assign x = 0; },
        id16 -> id15 { sync dctq1?; assign x = 0; };
}
""",
    "Pfinish": """process Pfinish() {
    clock x;
    state Pout, id10, id11, id12;
    init id12;
    trans
        id10 -> Pout { guard x <= 0; },
        id11 -> id10 { guard x >= 378 && x <= 389 && mvp == 1; assign x = 0, mvp = 0; },
        id12 -> id11 { sync dctq?; assign x = 0; };
}
""",
    # As in the original: a clock holder with the global deadline and no transition.
    "Time": f"""process Time() {{
    clock x;
    state id0 {{ x <= {DEADLINE} }};
    init id0;
}}
""",
}


def generate_xta(num_frames: int) -> str:
    frames = [f"{i:03d}" for i in range(1, num_frames + 1)]
    ints = ["perror", "P", "mvp"] + [f"{v}{n}" for n in frames for v in ("ib", "mvb", "B", "berror")]
    procs = {f"BFrame{n}": xta_bframe(int(n)) for n in frames} | XTA_FIXED
    names = sorted(procs)
    return "\n".join([
        f"// Auto-generated broadcast mpeg2 model with {num_frames} B-frames\n",
        "broadcast chan bstart, ibx;",
        "chan dctq, dctq1;",
        f"int {', '.join(ints)};\n",
        *[procs[name] for name in names],
        f"system {', '.join(names)};\n"])


# --------------------------------------------------------------------------- Liana

def liana_bframe(i: int) -> str:
    n = f"{i:03d}"
    return f"""create automaton BFrame{n}
{{
    clocks {{ x; }}
    actions {{ tau, ibx, bstart; }}
    integers {{ ib{n}, mvb{n}, B{n}, berror{n}; }}
    locations {{
        Bout <ini: F>,
        id2  <ini: F>,
        id3  <ini: F>,
        id4  <ini: F>,
        id5  <ini: F>,
        id6  <ini: F>,
        id7  <ini: F>,
        id8  <ini: T>;
    }}
    transitions {{
        (id8, ibx??, [], [], [ib{n} = 1], id8),
        (id2, tau, [(x, <=, 0)], [], Bout),
        (id3, tau, [(x, >=, 189), (x, <=, 454)], mvb{n} == 1, [x], [mvb{n} = 0], id2),
        (id4, tau, [(x, ==, 521)], [x], id3),
        (id5, tau, [(x, <=, 0)], berror{n} == 1, [x], [berror{n} = 0], id4),
        (id6, tau, [(x, ==, 48)], [x], id5),
        (id7, tau, [(x, >=, 2399), (x, <=, 2422)], ib{n} == 1 && B{n} == 1, [x], [mvb{n} = 1, ib{n} = 0, B{n} = 0], id6),
        (id8, bstart??, [], [x], [B{n} = 1, berror{n} = 1], id7);
    }}
}}
"""

LIANA_FIXED = {
    "IandPframes": """create automaton IandPframes
{
    clocks { x; }
    actions { tau, bstart, dctq, ibx, dctq1; }
    integers { perror, P, mvp; }
    locations {
        id17 <ini: F>,
        id18 <ini: F>,
        id19 <ini: F>,
        id20 <ini: F>,
        id21 <ini: F>,
        id22 <ini: F>,
        id23 <ini: F>,
        id24 <ini: F>,
        id25 <ini: F>,
        I    <ini: T>;
    }
    transitions {
        (id18, bstart!!, [(x, <=, 0)], [], id17),
        (id19, tau, [(x, ==, 515)], [x], id18),
        (id20, dctq!, [(x, ==, 521)], [x], id19),
        (id21, tau, [(x, <=, 0)], perror == 1, [x], [perror = 0], id20),
        (id22, tau, [(x, ==, 32)], [x], id21),
        (id23, tau, [(x, >=, 1180), (x, <=, 1181)], P == 1, [x], [mvp = 1, P = 0], id22),
        (id24, ibx!!, [(x, <=, 0)], [x], [perror = 1, P = 1], id23),
        (id25, tau, [(x, ==, 673)], [x], id24),
        (I, dctq1!, [(x, ==, 595)], [x], id25);
    }
}
""",
    "Ifinish": """create automaton Ifinish
{
    clocks { x; }
    actions { tau, dctq1; }
    locations {
        Iout <ini: F>,
        id14 <ini: F>,
        id15 <ini: F>,
        id16 <ini: T>;
    }
    transitions {
        (id14, tau, [(x, <=, 0)], [], Iout),
        (id15, tau, [(x, >=, 298), (x, <=, 358)], [x], id14),
        (id16, dctq1?, [], [x], id15);
    }
}
""",
    "Pfinish": """create automaton Pfinish
{
    clocks { x; }
    actions { tau, dctq; }
    integers { mvp; }
    locations {
        Pout <ini: F>,
        id10 <ini: F>,
        id11 <ini: F>,
        id12 <ini: T>;
    }
    transitions {
        (id10, tau, [(x, <=, 0)], [], Pout),
        (id11, tau, [(x, >=, 378), (x, <=, 389)], mvp == 1, [x], [mvp = 0], id10),
        (id12, dctq?, [], [x], id11);
    }
}
""",
    # Liana needs a transition: this one is never enabled, so Time only carries the deadline.
    "Time": f"""create automaton Time
{{
    clocks {{ x; }}
    actions {{ never; }}
    locations {{
        id0 <ini: T, inv: [(x, <=, {DEADLINE})]>;
    }}
    transitions {{
        (id0, never, [], 1 < 0, [], id0);
    }}
}}
""",
}


def main():
    root = Path(__file__).resolve().parent

    for num_frames in SIZES:
        instance = f"mpeg2Broadcast_{num_frames:02d}"

        xta_dir = root / "xta" / instance
        xta_dir.mkdir(parents=True, exist_ok=True)
        (xta_dir / f"{instance}.xta").write_text(generate_xta(num_frames), encoding="utf-8")
        query = " and ".join(f"BFrame{i:03d}.Bout" for i in range(1, num_frames + 1))
        (xta_dir / "mpeg2Broadcast.q").write_text(f"E<> {query}\n", encoding="utf-8")

        liana_dir = root / "liana" / instance
        liana_dir.mkdir(parents=True, exist_ok=True)
        for i in range(1, num_frames + 1):
            (liana_dir / f"BFrame{i:03d}.txt").write_text(liana_bframe(i), encoding="utf-8")
        for name, text in LIANA_FIXED.items():
            (liana_dir / f"{name}.txt").write_text(text, encoding="utf-8")

        print(f"Generated: {instance} ({num_frames} B-frames + 4 processes)")


if __name__ == "__main__":
    main()
