#!/usr/bin/env python3
"""
Convert UPPAAL XTA files to TChecker TCK format.

This script converts timed automata models from UPPAAL's XTA format to
TChecker's TCK format, handling process declarations, clocks, variables,
locations, and transitions.
"""

import re
import os
import sys
from pathlib import Path
from typing import Dict, List, Tuple, Set


class XTAParser:
    """Parser for UPPAAL XTA format."""

    def __init__(self, content: str):
        self.content = content
        self.constants: Dict[str, str] = {}
        self.global_vars: List[Tuple[str, str]] = []  # (type, name)
        self.global_clocks: List[str] = []  # Global clock names
        self.channels: List[str] = []
        self.processes: List[Dict] = []

    def parse(self):
        """Parse the XTA file content."""
        # Remove comments
        content = re.sub(r'//.*?\n', '\n', self.content)
        content = re.sub(r'/\*.*?\*/', '', content, flags=re.DOTALL)

        # Parse constants
        for match in re.finditer(r'const\s+int\s+(\w+)\s*=\s*([^;]+);', content):
            name, value = match.groups()
            self.constants[name] = value.strip()

        # Parse global clocks (clocks declared outside process blocks)
        # First, find all process blocks to exclude them
        process_blocks = []
        for match in re.finditer(r'process\s+\w+\s*\([^)]*\)\s*\{.*?\n\}', content, re.DOTALL):
            process_blocks.append((match.start(), match.end()))

        # Find global clock declarations (outside processes)
        for match in re.finditer(r'^clock\s+([^;]+);', content, re.MULTILINE):
            # Check if this clock declaration is outside all process blocks
            pos = match.start()
            is_global = True
            for start, end in process_blocks:
                if start <= pos <= end:
                    is_global = False
                    break

            if is_global:
                # Parse comma-separated clock names
                clock_list = match.group(1)
                clocks = [c.strip() for c in clock_list.split(',')]
                self.global_clocks.extend(clocks)

        # Parse global variables
        for match in re.finditer(r'^(bool|int)\s+(\w+)\s*;', content, re.MULTILINE):
            var_type, var_name = match.groups()
            self.global_vars.append((var_type, var_name))

        # Parse channels
        for match in re.finditer(r'chan\s+([^;]+);', content):
            channels = [ch.strip() for ch in match.group(1).split(',')]
            self.channels.extend(channels)

        # Parse processes
        process_pattern = r'process\s+(\w+)\s*\([^)]*\)\s*\{(.*?)\n\}'
        for match in re.finditer(process_pattern, content, re.DOTALL):
            process_name = match.group(1)
            process_body = match.group(2)
            process_data = self.parse_process(process_name, process_body)
            if process_data:
                self.processes.append(process_data)

        # Parse system declaration
        system_match = re.search(r'system\s+([^;]+);', content)
        if system_match:
            self.system_processes = [p.strip() for p in system_match.group(1).split(',')]
        else:
            self.system_processes = [p['name'] for p in self.processes]

    def parse_process(self, name: str, body: str) -> Dict:
        """Parse a single process definition."""
        process = {
            'name': name,
            'clocks': [],
            'states': {},
            'urgent_states': set(),
            'init_state': None,
            'transitions': []
        }

        # Parse local clocks (can be comma-separated)
        for match in re.finditer(r'clock\s+([^;]+);', body):
            clock_list = match.group(1)
            clocks = [c.strip() for c in clock_list.split(',')]
            process['clocks'].extend(clocks)

        # Parse urgent locations
        urgent_section = re.search(r'urgent\s+(.*?)(?:init|trans|state)', body, re.DOTALL)
        if urgent_section:
            urgent_text = urgent_section.group(1)
            # Parse comma-separated urgent location names
            for line in urgent_text.split(','):
                loc = line.strip().rstrip(';')
                if loc:
                    process['urgent_states'].add(loc)

        # Parse states with invariants
        state_section = re.search(r'state\s+(.*?)(?:\burgent\b|\binit\b|\btrans\b)', body, re.DOTALL)
        if state_section:
            state_text = state_section.group(1)
            # Remove any semicolons at the end
            state_text = state_text.rstrip(';').strip()

            # Parse state declarations - they are comma-separated
            # Each state can be: "name" or "name { invariant }"
            states_list = []
            current_state = ""
            brace_count = 0

            for char in state_text:
                if char == '{':
                    brace_count += 1
                    current_state += char
                elif char == '}':
                    brace_count -= 1
                    current_state += char
                elif char == ',' and brace_count == 0:
                    # End of a state declaration
                    states_list.append(current_state.strip())
                    current_state = ""
                else:
                    current_state += char

            # Don't forget the last state
            if current_state.strip():
                states_list.append(current_state.strip())

            # Now parse each state
            for state_decl in states_list:
                if not state_decl:
                    continue

                # Check for invariant
                inv_match = re.match(r'(\w+)\s*\{([^}]+)\}', state_decl)
                if inv_match:
                    state_name = inv_match.group(1)
                    invariant = inv_match.group(2).strip()
                    process['states'][state_name] = {'invariant': invariant}
                else:
                    # Remove any trailing semicolons and whitespace from state name
                    state_name = state_decl.strip().rstrip(';').strip()
                    if state_name:
                        process['states'][state_name] = {'invariant': None}

        # Parse init state
        init_match = re.search(r'init\s+(\w+);', body)
        if init_match:
            process['init_state'] = init_match.group(1)

        # Parse transitions
        trans_section = re.search(r'trans\s+(.*)', body, re.DOTALL)
        if trans_section:
            trans_text = trans_section.group(1)
            # Split by transitions (semicolon separated)
            transitions = re.findall(r'(\w+)\s*->\s*(\w+)\s*\{([^}]*)\}', trans_text)
            for source, target, attrs in transitions:
                trans = {
                    'source': source,
                    'target': target,
                    'guard': None,
                    'sync': None,
                    'sync_type': None,
                    'assign': None
                }

                # Parse guard
                guard_match = re.search(r'guard\s+([^;]+);', attrs)
                if guard_match:
                    trans['guard'] = guard_match.group(1).strip()

                # Parse sync
                sync_match = re.search(r'sync\s+(\w+)([!?])\s*;', attrs)
                if sync_match:
                    trans['sync'] = sync_match.group(1)
                    trans['sync_type'] = sync_match.group(2)

                # Parse assign
                assign_match = re.search(r'assign\s+([^;]+);', attrs)
                if assign_match:
                    trans['assign'] = assign_match.group(1).strip()

                process['transitions'].append(trans)

        return process

    def substitute_constants(self, expr: str) -> str:
        """Replace constant names with their values."""
        if not expr:
            return expr
        result = expr
        for const_name, const_value in self.constants.items():
            result = re.sub(r'\b' + const_name + r'\b', const_value, result)
        return result


class TCKGenerator:
    """Generator for TChecker TCK format."""

    def __init__(self, parser: XTAParser, system_name: str):
        self.parser = parser
        self.system_name = system_name
        self.events: Set[str] = set()
        self.tau_event = 'tau'  # Default event for unsynchronized transitions

    def generate(self) -> str:
        """Generate TCK format output."""
        lines = []

        # System declaration (must be first)
        lines.append(f"system:{self.system_name}\n\n")

        # Collect all events from transitions
        self.collect_events()

        # Add tau event for unsynchronized transitions
        if self.tau_event:
            self.events.add(self.tau_event)

        # Event declarations
        lines.append("# Events\n")
        for event in sorted(self.events):
            lines.append(f"event:{event}\n")
        lines.append("\n\n\n")

        # Global clock and variable declarations
        global_decls = self.generate_global_declarations()
        if global_decls:
            lines.append("# Global declarations\n")
            lines.extend(global_decls)
            lines.append("\n\n\n")

        # Process declarations
        lines.append("# Processes\n")
        for process in self.parser.processes:
            lines.append(f"process:{process['name']}\n")
        lines.append("\n")

        # Process-specific declarations (clocks, locations, edges)
        for i, process in enumerate(self.parser.processes):
            if i > 0:
                lines.append("\n\n\n")  # Three blank lines between processes (total spacing)

            lines.append(f"# Process: {process['name']}\n")

            # Local clocks
            if process['clocks']:
                lines.append(f"## Clocks for {process['name']}\n")
                for clock in process['clocks']:
                    lines.append(f"clock:1:{process['name']}_{clock}\n")
                lines.append("\n")

            # Locations
            locations = self.generate_locations(process)
            if locations:
                lines.append(f"## Locations for {process['name']}\n")
                lines.extend(locations)
                lines.append("\n")

            # Edges
            edges = self.generate_edges(process)
            if edges:
                lines.append(f"## Edges for {process['name']}\n")
                lines.extend(edges)

        # Synchronization constraints
        sync_constraints = self.generate_sync_constraints()
        if sync_constraints:
            lines.append("\n\n\n# Synchronizations\n")
            lines.extend(sync_constraints)

        return ''.join(lines)

    def collect_events(self):
        """Collect all unique events from process transitions."""
        for process in self.parser.processes:
            for trans in process['transitions']:
                if trans['sync']:
                    self.events.add(trans['sync'])

    def generate_global_declarations(self) -> List[str]:
        """Generate global clock and variable declarations."""
        lines = []

        # Constants (const int) - declare as initialized integer variables
        for const_name, const_value in self.parser.constants.items():
            # Parse the constant value (should be an integer)
            try:
                init_value = int(const_value)
                # Use a reasonable range: allow the value itself and some flexibility
                # For TChecker, we use min=0 and max as a reasonable upper bound
                max_value = max(1000, init_value * 2)
                lines.append(f"int:1:0:{max_value}:{init_value}:{const_name}\n")
            except ValueError:
                # If it's not a simple integer, use default range
                lines.append(f"int:1:0:1000:0:{const_name}\n")

        # Global clocks
        for clock_name in self.parser.global_clocks:
            lines.append(f"clock:1:{clock_name}\n")

        # Global variables
        for var_type, var_name in self.parser.global_vars:
            if var_type == 'bool':
                # bool in XTA is 0 or 1
                lines.append(f"int:1:0:1:0:{var_name}\n")
            elif var_type == 'int':
                # Default int range (can be adjusted based on model)
                lines.append(f"int:1:0:1000:0:{var_name}\n")

        return lines

    def generate_locations(self, process: Dict) -> List[str]:
        """Generate location declarations for a process."""
        lines = []

        for state_name, state_data in process['states'].items():
            attrs = []

            # Check if initial location
            if state_name == process['init_state']:
                attrs.append("initial:")

            # Check if urgent location
            if state_name in process['urgent_states']:
                attrs.append("urgent:")

            # Add invariant if present
            if state_data['invariant']:
                invariant = self.convert_expression(state_data['invariant'], process)
                attrs.append(f"invariant:{invariant}")

            # Format location declaration
            # Use " : " (space-colon-space) as separator between attributes
            # If there are attributes, use {}, otherwise no braces
            if attrs:
                attr_str = "{" + " : ".join(attrs) + "}"
            else:
                attr_str = ""

            lines.append(f"location:{process['name']}:{state_name}{attr_str}\n")

        return lines

    def generate_edges(self, process: Dict) -> List[str]:
        """Generate edge declarations for a process."""
        lines = []

        for trans in process['transitions']:
            source = trans['source']
            target = trans['target']

            # Determine event for this edge
            event = trans['sync'] if trans['sync'] else self.tau_event

            attrs = []

            # Add guard (provided)
            if trans['guard']:
                guard = self.convert_expression(trans['guard'], process)
                attrs.append(f"provided:{guard}")

            # Add assignment (do)
            if trans['assign']:
                assign = self.convert_assignment(trans['assign'], process)
                attrs.append(f"do:{assign}")

            # Format edge declaration
            # Use " : " (space-colon-space) as separator between attributes
            attr_str = "{" + " : ".join(attrs) + "}" if attrs else ""
            lines.append(f"edge:{process['name']}:{source}:{target}:{event}{attr_str}\n")

        return lines

    def generate_sync_constraints(self) -> List[str]:
        """Generate synchronization constraints based on channels."""
        lines = []

        # Build maps of senders (!) and receivers (?) for each event
        senders: Dict[str, List[str]] = {}  # event -> list of process names with !
        receivers: Dict[str, List[str]] = {}  # event -> list of process names with ?

        for process in self.parser.processes:
            for trans in process['transitions']:
                if trans['sync']:
                    event = trans['sync']
                    sync_type = trans['sync_type']

                    if sync_type == '!':
                        if event not in senders:
                            senders[event] = []
                        senders[event].append(process['name'])
                    elif sync_type == '?':
                        if event not in receivers:
                            receivers[event] = []
                        receivers[event].append(process['name'])

        # Create pairwise synchronization constraints
        # Each sender (!) pairs with each receiver (?) for the same event
        all_events = set(senders.keys()) | set(receivers.keys())

        for event in sorted(all_events):
            event_senders = sorted(set(senders.get(event, [])))
            event_receivers = sorted(set(receivers.get(event, [])))

            if not event_senders or not event_receivers:
                # Skip if no sender or no receiver for this event
                continue

            # Create pairwise synchronizations
            # Each sender synchronizes with each receiver
            for sender in event_senders:
                for receiver in event_receivers:
                    sync_constraint = f"{sender}@{event}:{receiver}@{event}"
                    lines.append(f"sync:{sync_constraint}\n")

        return lines

    def convert_expression(self, expr: str, process: Dict) -> str:
        """Convert XTA expression to TChecker expression."""
        # Do NOT substitute constants - they are now declared as variables
        result = expr

        # Replace local clock references with process-prefixed names
        # (but not global clocks)
        for clock in process['clocks']:
            if clock not in self.parser.global_clocks:
                result = re.sub(r'\b' + clock + r'\b', f"{process['name']}_{clock}", result)

        # Convert 'and' keyword to '&&' operator
        result = re.sub(r'\band\b', '&&', result)

        # Convert 'or' keyword to '||' operator
        result = re.sub(r'\bor\b', '||', result)

        # Note: Arithmetic operators (+, -, *, /) are already compatible with TChecker
        # Note: Comparison operators (==, !=, <, <=, >, >=) are already compatible

        return result.strip()

    def convert_assignment(self, assign: str, process: Dict) -> str:
        """Convert XTA assignment to TChecker assignment."""
        # Do NOT substitute constants - they are now declared as variables
        result = assign

        # Replace local clock references with process-prefixed names
        # (but not global clocks)
        for clock in process['clocks']:
            if clock not in self.parser.global_clocks:
                result = re.sub(r'\b' + clock + r'\b', f"{process['name']}_{clock}", result)

        # Normalize assignment operator to =
        result = result.replace(':=', '=')

        # Handle multiple assignments (comma-separated)
        assignments = [a.strip() for a in result.split(',')]
        return "; ".join(assignments)


def convert_xta_to_tck(xta_path: str, tck_path: str):
    """Convert a single XTA file to TCK format."""
    # Read XTA file
    with open(xta_path, 'r') as f:
        xta_content = f.read()

    # Parse XTA
    parser = XTAParser(xta_content)
    parser.parse()

    # Generate system name from filename
    system_name = Path(xta_path).stem

    # Generate TCK
    generator = TCKGenerator(parser, system_name)
    tck_content = generator.generate()

    # Write TCK file
    os.makedirs(os.path.dirname(tck_path), exist_ok=True)
    with open(tck_path, 'w') as f:
        f.write(tck_content)

    print(f"Converted: {xta_path} -> {tck_path}")


def convert_all_benchmarks(benchmarks_dir: str):
    """Convert all XTA files in the benchmarks directory."""
    benchmarks_path = Path(benchmarks_dir)

    # Clear all existing tck directories first
    print("Clearing existing tck directories...")
    tck_dirs = list(benchmarks_path.glob('**/tck'))
    for tck_dir in tck_dirs:
        if tck_dir.is_dir():
            import shutil
            shutil.rmtree(tck_dir)
            print(f"Removed: {tck_dir}")

    if tck_dirs:
        print(f"Cleared {len(tck_dirs)} tck directories\n")

    # Find all .xta files
    xta_files = list(benchmarks_path.glob('**/xta/**/*.xta'))

    if not xta_files:
        print(f"No .xta files found in {benchmarks_dir}")
        return

    print(f"Found {len(xta_files)} .xta files to convert")

    converted = 0
    failed = 0

    for xta_file in xta_files:
        try:
            # Determine the output path
            # Replace 'xta' directory with 'tck' directory
            relative_path = xta_file.relative_to(benchmarks_path)

            # Convert path: replace /xta/ with /tck/
            path_parts = list(relative_path.parts)
            if 'xta' in path_parts:
                xta_index = path_parts.index('xta')
                path_parts[xta_index] = 'tck'

            tck_relative = Path(*path_parts).with_suffix('.tck')
            tck_file = benchmarks_path / tck_relative

            # Convert
            convert_xta_to_tck(str(xta_file), str(tck_file))
            converted += 1

        except Exception as e:
            print(f"Error converting {xta_file}: {e}")
            failed += 1

    print(f"\nConversion complete:")
    print(f"  Successfully converted: {converted}")
    print(f"  Failed: {failed}")


def main():
    """Main entry point."""
    if len(sys.argv) < 2:
        print("Usage:")
        print("  Convert all benchmarks:")
        print(f"    {sys.argv[0]} <benchmarks_dir>")
        print("  Convert single file:")
        print(f"    {sys.argv[0]} <input.xta> <output.tck>")
        sys.exit(1)

    if len(sys.argv) == 2:
        # Convert all benchmarks
        benchmarks_dir = sys.argv[1]
        convert_all_benchmarks(benchmarks_dir)
    else:
        # Convert single file
        xta_file = sys.argv[1]
        tck_file = sys.argv[2]
        convert_xta_to_tck(xta_file, tck_file)


if __name__ == '__main__':
    main()
