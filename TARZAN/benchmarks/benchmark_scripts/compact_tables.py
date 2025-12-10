#!/usr/bin/env python3
"""
Convert LaTeX benchmark tables from old format to new template format.
"""

import re
import sys
from pathlib import Path


def parse_table(table_text):
    """Parse a single table and extract relevant information."""
    # Extract table name from caption
    caption_match = re.search(r'\\caption\{Benchmark results for (\w+)\}', table_text)
    if not caption_match:
        return None

    table_name = caption_match.group(1)

    # Extract data rows (skip header rows)
    lines = table_text.split('\n')
    data_rows = []

    in_data = False
    for line in lines:
        # Skip until we hit the double hline (after header)
        if '\\hline\\hline' in line:
            in_data = True
            continue

        if not in_data:
            continue

        # Stop at the closing hline
        if line.strip() == '\\hline':
            break

        # Skip empty lines
        if not line.strip():
            continue

        # Parse data row
        if '&' in line:
            data_rows.append(line)

    return {
        'name': table_name,
        'rows': data_rows
    }


def parse_row(row_text):
    """Parse a single data row and extract values for each tool."""
    # Remove trailing \\ and whitespace
    row_text = row_text.strip().rstrip('\\\\').strip()

    # Split by &
    raw_parts = [p.strip() for p in row_text.split('&')]

    # Expand multicolumn entries
    parts = []
    for part in raw_parts:
        if '\\multicolumn' in part:
            # Extract the span count and content from \multicolumn{N}{...}{content}
            match = re.search(r'\\multicolumn\{(\d+)\}\{[^}]*\}\{([^}]*)\}', part)
            if match:
                span = int(match.group(1))
                content = match.group(2)
                # Add the content once, then fill with empty strings for the span
                parts.append(content)
                for _ in range(span - 1):
                    parts.append('')
            else:
                parts.append(part)
        else:
            parts.append(part)

    # Need at least instance + 12 data columns (ignoring verification columns)
    if len(parts) < 13:
        return None

    instance = parts[0]

    # Extract N from instance name
    # Handle formats like "flower_02", "fischer_02", "latch", etc.
    instance_match = re.search(r'_0*(\d+)$', instance)
    if instance_match:
        n = instance_match.group(1)
    else:
        # For single instances without number (like "latch"), use "1"
        # or extract any trailing number
        num_match = re.search(r'\d+$', instance)
        if num_match:
            n = num_match.group(0)
        else:
            # Use instance name itself if no number found
            n = instance.split('_')[-1] if '_' in instance else instance

    # TARZAN: parts[1-4] (Time, Exec, Mem, Regions)
    tarzan = {
        'vt': parts[1],
        'et': parts[2],
        'mem': parts[3],
        'regions': parts[4]
    }

    # TChecker: parts[5-8] (Time, Exec, Mem, States)
    tchecker = {
        'vt': parts[5],
        'et': parts[6],
        'mem': parts[7],
        'states': parts[8]
    }

    # UPPAAL: parts[9-12] (Time, Exec, Mem, States)
    uppaal = {
        'vt': parts[9],
        'et': parts[10],
        'mem': parts[11],
        'states': parts[12]
    }

    return {
        'n': n,
        'tarzan': tarzan,
        'tchecker': tchecker,
        'uppaal': uppaal
    }


def process_value(value, is_time=False):
    """Process a value: handle KO, OOM, <0.001, etc."""
    value = value.strip()

    # Handle multicolumn KO
    if '\\multicolumn' in value and 'KO' in value:
        return 'KO'

    # Handle direct KO or OOM
    if value == 'KO' or value == 'OOM':
        return value

    # Handle < 0.001 - use left superscript downarrow
    if value.startswith('$<$') or value.startswith('<'):
        return '$^{\\downarrow}0.001$'

    return value


def format_tool_data(tool_data, tool_name):
    """Format data for a single tool based on KO/OOM status."""
    vt = process_value(tool_data['vt'], is_time=True)
    et = process_value(tool_data['et'], is_time=True)
    mem = process_value(tool_data['mem'])
    count = process_value(tool_data.get('regions') or tool_data.get('states', ''))

    # Check if any value is KO or if it's a multicolumn
    is_ko = vt == 'KO' or et == 'KO' or mem == 'KO' or count == 'KO'
    is_oom = 'OOM' in vt or 'OOM' in et or 'OOM' in mem or 'OOM' in count

    # Handle multicolumn cases
    if '\\multicolumn' in tool_data['vt']:
        if tool_name == 'TARZAN':
            # TARZAN: OOM in Mem, -- elsewhere
            return '-- & -- & OOM & --'
        else:
            # TChecker/UPPAAL: left superscript uparrow with 600 in VT/ET, -- elsewhere
            return '$^{\\uparrow}600$ & $^{\\uparrow}600$ & -- & --'

    if is_ko or is_oom:
        if tool_name == 'TARZAN':
            # TARZAN: OOM in Mem, -- elsewhere
            return '-- & -- & OOM & --'
        else:
            # TChecker/UPPAAL: left superscript uparrow with 600 in VT/ET, -- elsewhere
            return '$^{\\uparrow}600$ & $^{\\uparrow}600$ & -- & --'

    return f'{vt} & {et} & {mem} & {count}'


def create_combined_table(tables):
    """Create a combined table from multiple parsed tables."""
    # Group tables by dataset name
    datasets = {}
    for table in tables:
        if table:
            datasets[table['name']] = table

    output = []

    # Header
    output.append('\\definecolor{lightgray}{gray}{0.95}')
    output.append('\\setlength{\\aboverulesep}{1pt}')
    output.append('\\setlength{\\belowrulesep}{1pt}')
    output.append('{\\renewcommand{\\arraystretch}{0.95}')
    output.append('{\\setlength{\\tabcolsep}{3.25pt}')
    output.append('\\begin{table}[t]')
    output.append('\\centering')

    # Create caption with all dataset names
    dataset_names = sorted(datasets.keys())
    caption_text = ', '.join([f'\\texttt{{{name}}}' for name in dataset_names])
    output.append(f'\\caption{{Benchmark results for {caption_text}}}')
    output.append('\\label{tab:combined}')

    output.append('\\resizebox{\\textwidth}{!}{')
    output.append('\\begin{tabular}{%')
    output.append('    c|rrrr|rrrr|rrrr')
    output.append('}')
    output.append('\\toprule')

    # Column headers
    output.append('\\multirow{2}{*}{\\textbf{N}}')
    output.append('  & \\multicolumn{4}{c|}{\\textbf{TARZAN}}')
    output.append('  & \\multicolumn{4}{c|}{\\textbf{TChecker}}')
    output.append('  & \\multicolumn{4}{c}{\\textbf{UPPAAL}} \\\\')
    output.append('%')
    output.append('  & \\multicolumn{1}{c}{\\textbf{VT}}')
    output.append('  & \\multicolumn{1}{c}{\\textbf{ET}}')
    output.append('  & \\multicolumn{1}{c}{\\textbf{Mem}}')
    output.append('  & \\multicolumn{1}{c|}{\\textbf{Regions}}')
    output.append('  & \\multicolumn{1}{c}{\\textbf{VT}}')
    output.append('  & \\multicolumn{1}{c}{\\textbf{ET}}')
    output.append('  & \\multicolumn{1}{c}{\\textbf{Mem}}')
    output.append('  & \\multicolumn{1}{c|}{\\textbf{States}}')
    output.append('  & \\multicolumn{1}{c}{\\textbf{VT}}')
    output.append('  & \\multicolumn{1}{c}{\\textbf{ET}}')
    output.append('  & \\multicolumn{1}{c}{\\textbf{Mem}}')
    output.append('  & \\multicolumn{1}{c}{\\textbf{States}} \\\\')
    output.append('\\midrule')

    # Process each dataset
    for i, dataset_name in enumerate(dataset_names):
        table = datasets[dataset_name]

        # Dataset header
        output.append(f'% {"-" * 23} {dataset_name.upper()} DATASET {"-" * 23}')
        output.append(f'\\multicolumn{{13}}{{c}}{{\\textbf{{{dataset_name}}}}} \\\\')
        output.append('\\midrule')

        # Parse and format rows
        for row_idx, row_text in enumerate(table['rows']):
            parsed = parse_row(row_text)
            if not parsed:
                continue

            # Alternate row colors
            color = 'white' if row_idx % 2 == 0 else 'lightgray'
            output.append(f'\\rowcolor{{{color}}}')

            n = parsed['n']
            tarzan_data = format_tool_data(parsed['tarzan'], 'TARZAN')
            tchecker_data = format_tool_data(parsed['tchecker'], 'TChecker')
            uppaal_data = format_tool_data(parsed['uppaal'], 'UPPAAL')

            output.append(f'{n}  & {tarzan_data}')
            output.append(f'   & {tchecker_data}')
            output.append(f'   & {uppaal_data} \\\\')

        # Add midrule between datasets (but not after the last one)
        if i < len(dataset_names) - 1:
            output.append('\\midrule')

    # Footer
    output.append('\\bottomrule')
    output.append('\\end{tabular}')
    output.append('}')
    output.append('\\end{table}')
    output.append('}')
    output.append('}')

    return '\n'.join(output)


def convert_tables(input_file, output_file=None):
    """Convert tables from input file to output file."""
    # Read input file
    with open(input_file, 'r') as f:
        content = f.read()

    # Split into individual tables
    table_pattern = r'\\begin{table}.*?\\end{table}'
    table_matches = re.findall(table_pattern, content, re.DOTALL)

    # Parse all tables
    parsed_tables = []
    for table_text in table_matches:
        parsed = parse_table(table_text)
        if parsed:
            parsed_tables.append(parsed)

    if not parsed_tables:
        print("No tables found in input file")
        return

    # Create combined table
    output_text = create_combined_table(parsed_tables)

    # Write output
    if output_file:
        with open(output_file, 'w') as f:
            f.write(output_text)
        print(f"Converted {len(parsed_tables)} tables to {output_file}")
    else:
        print(output_text)


def main():
    if len(sys.argv) < 2:
        print("Usage: python table_converter.py <input_file> [output_file]")
        sys.exit(1)

    input_file = sys.argv[1]
    output_file = sys.argv[2] if len(sys.argv) > 2 else None

    convert_tables(input_file, output_file)


if __name__ == '__main__':
    main()