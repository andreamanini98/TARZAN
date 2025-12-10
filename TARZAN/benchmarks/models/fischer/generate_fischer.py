# fischer_generator_inline.py
# Generates Fischer protocol networks (2 to 6 automata)
# Inlines literal values for k and pid (no const declarations)

def generate_fischer_automaton(pid):
    name = f"fischer{pid:02d}"
    k = 2
    return f"""process {name}() {{
    clock x;
    state
        wait,
        req {{ x <= {k} }},
        A,
        cs;
    init
        A;
    trans
        A -> req {{ guard id == 0; assign x = 0; }},
        req -> wait {{ guard x <= {k}; assign x = 0, id = {pid}; }},
        wait -> req {{ guard id == 0; assign x = 0; }},
        wait -> cs {{ guard x > {k} && id == {pid}; }},
        cs -> A {{ assign id = 0; }};
}}"""

def generate_system(n):
    processes = [generate_fischer_automaton(i+1) for i in range(n)]
    system_decl = f"system {', '.join(f'P{i+1:02d}' for i in range(n))};"
    header = "int id;\n\n"
    return header + "\n\n".join(processes) + "\n\n" + system_decl + "\n"

def main():
    for n in range(2, 7):
        content = generate_system(n)
        filename = f"fischer{n:02d}.xta"
        with open(filename, "w") as f:
            f.write(content)
        print(f"✅ Generated {filename}")

if __name__ == "__main__":
    main()