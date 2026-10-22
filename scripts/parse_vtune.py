import os
import sys
import re
import argparse
import pandas as pd

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.abspath(os.path.join(SCRIPT_DIR, ".."))
BASE_LOG_DIR = os.path.join(PROJECT_ROOT, "output_logs")

def parse_vtune_log(filepath):
    """
    Extracts VTune KPIs and execution time from the textual log.
    """
    data = {
        "Total Time (ms)": None,
        "CPU Util (%)": None,
        "CPUs Active": None,
        "Threads": None,
        "Wait Time (s)": None,
        "Spin Time (s)": None
    }
    
    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()

    # 1. Total Time
    m_time = re.search(r"Total time including region generation:\s*([0-9\.eE\+\-]+)\s*microseconds", content)
    if m_time:
        data["Total Time (ms)"] = round(float(m_time.group(1)) / 1000.0, 2)

    # 2. Effective CPU Utilization
    m_cpu = re.search(r"Effective CPU Utilization:\s*([\d\.]+)%\s*\(([\d\.]+)\s*out of", content)
    if m_cpu:
        data["CPU Util (%)"] = float(m_cpu.group(1))
        data["CPUs Active"] = float(m_cpu.group(2))

    # 3. Thread Count
    m_thread = re.search(r"Total Thread Count:\s*(\d+)", content)
    if m_thread:
        data["Threads"] = int(m_thread.group(1))

    # 4. Wait Time
    m_wait = re.search(r"Wait Time with poor CPU Utilization:\s*([\d\.]+)s", content)
    if m_wait:
        data["Wait Time (s)"] = float(m_wait.group(1))

    # 5. Spin and Overhead Time
    m_spin = re.search(r"Spin and Overhead Time:\s*([\d\.]+)s", content)
    if m_spin:
        data["Spin Time (s)"] = float(m_spin.group(1))

    return data

def collect_vtune_data(log_dir):
    """
    Explores the model directory to find VTune logs and builds the DataFrame.
    """
    if not os.path.exists(log_dir):
        print(f"[ERROR] Directory '{log_dir}' not found.")
        return pd.DataFrame()

    rows = []
    
    for root, dirs, files in os.walk(log_dir):
        for filename in files:
            if not filename.endswith("vtune_run.log"):
                continue
                
            filepath = os.path.join(root, filename)
            
            base = filename.replace("_vtune_run.log", "")
            mode = "UNKNOWN"
            test_name = base
            
            known_modes = ["seq", "omp", "tbb"]
            for m in known_modes:
                pattern = re.compile(rf"^(.*)_({m}(?:_[\w\-]+)?)$")
                match = pattern.match(base)
                if match:
                    test_name = match.group(1)
                    mode = match.group(2).upper()
                    break
            
            metrics = parse_vtune_log(filepath)
            metrics["Test Name"] = test_name
            metrics["Mode"] = mode
            rows.append(metrics)
            
    df = pd.DataFrame(rows)
    if not df.empty:
        cols = ["Test Name", "Mode", "Total Time (ms)", "CPU Util (%)", "CPUs Active", "Threads", "Wait Time (s)", "Spin Time (s)"]
        df = df[cols]
        df.sort_values(by=["Test Name", "Mode"], key=lambda col: col.str.lower(), inplace=True)
        df.reset_index(drop=True, inplace=True)
        
    return df

def generate_custom_html_report(df, model_name, output_filepath):
    """
    Generates an HTML report containing a separate table for each test.
    """
    html_lines = [
        "<!DOCTYPE html>",
        "<html>",
        "<head>",
        f"<title>VTune Profiling Report - {model_name}</title>",
        "<style>",
        "body { font-family: Arial, sans-serif; margin: 20px; background-color: #f4f6f8; color: #333; }",
        "h1 { text-align: center; color: #263238; margin-bottom: 30px; }",
        ".test-card { background: white; padding: 20px; margin-bottom: 30px; border-radius: 8px; box-shadow: 0 2px 5px rgba(0,0,0,0.1); border: 1px solid #cfd8dc; }",
        ".test-header { display: flex; justify-content: space-between; align-items: center; border-bottom: 2px solid #263238; padding-bottom: 10px; margin-bottom: 15px; }",
        ".test-title { font-size: 20px; font-weight: bold; color: #263238; }",
        ".winner-badge { background-color: #2e7d32; color: white; padding: 6px 14px; font-weight: bold; border-radius: 20px; font-size: 14px; }",
        "table { width: 100%; border-collapse: collapse; margin-top: 10px; }",
        "th { background-color: #263238; color: white; padding: 10px; text-align: center; border: 1px solid #455a64; }",
        "td { padding: 8px; text-align: center; border: 1px solid #cfd8dc; color: black; }",
        ".best-row { background-color: rgba(46, 125, 50, 0.15); font-weight: bold; }",
        "</style>",
        "</head>",
        "<body>",
        f"<h1>VTune Profiling Analysis - Model: <span style='color: #00838f;'>{model_name}</span></h1>"
    ]

    grouped = df.groupby("Test Name")
    sorted_test_names = sorted(grouped.groups.keys(), key=lambda x: x.lower())

    for test_name in sorted_test_names:
        group_df = grouped.get_group(test_name).copy()
        
        valid_times = group_df.dropna(subset=["Total Time (ms)"])
        if not valid_times.empty:
            winner_row = valid_times.loc[valid_times["Total Time (ms)"].idxmin()]
            winner_mode = winner_row["Mode"]
            winner_time = f"{winner_row['Total Time (ms)']:.2f} ms"
            winner_text = f"🏆 WINNER: {winner_mode} ({winner_time})"
        else:
            winner_mode = None
            winner_text = "WINNER: N/A"

        html_lines.append('<div class="test-card">')
        html_lines.append('<div class="test-header">')
        html_lines.append(f'<div class="test-title">TEST: {test_name}</div>')
        html_lines.append(f'<div class="winner-badge">{winner_text}</div>')
        html_lines.append('</div>')

        html_lines.append('<table>')
        html_lines.append('<thead><tr>')
        html_lines.append('<th>Mode</th><th>Total Time (ms)</th><th>CPU Util (%)</th><th>CPUs Active</th><th>Threads</th><th>Wait Time (s)</th><th>Spin Time (s)</th>')
        html_lines.append('</tr></thead>')
        html_lines.append('<tbody>')

        for _, row in group_df.iterrows():
            is_winner = (row["Mode"] == winner_mode)
            row_class = ' class="best-row"' if is_winner else ''
            
            total_time = f"{row['Total Time (ms)']:.2f}" if pd.notna(row['Total Time (ms)']) else "-"
            cpu_util = f"{row['CPU Util (%)']:.1f}%" if pd.notna(row['CPU Util (%)']) else "-"
            cpus_act = f"{row['CPUs Active']:.2f}" if pd.notna(row['CPUs Active']) else "-"
            threads = f"{row['Threads']:.0f}" if pd.notna(row['Threads']) else "-"
            wait_time = f"{row['Wait Time (s)']:.3f}" if pd.notna(row['Wait Time (s)']) else "-"
            spin_time = f"{row['Spin Time (s)']:.3f}" if pd.notna(row['Spin Time (s)']) else "-"

            html_lines.append(f'<tr{row_class}>')
            html_lines.append(f'<td><strong>{row["Mode"]}</strong></td>')
            html_lines.append(f'<td>{total_time}</td>')
            html_lines.append(f'<td>{cpu_util}</td>')
            html_lines.append(f'<td>{cpus_act}</td>')
            html_lines.append(f'<td>{threads}</td>')
            html_lines.append(f'<td>{wait_time}</td>')
            html_lines.append(f'<td>{spin_time}</td>')
            html_lines.append('</tr>')

        html_lines.append('</tbody>')
        html_lines.append('</table>')
        html_lines.append('</div>')

    html_lines.append('</body></html>')

    with open(output_filepath, 'w', encoding='utf-8') as f:
        f.write('\n'.join(html_lines))
        
    print(f"[SUCCESS] HTML VTune report saved to: '{output_filepath}'")

def generate_latex_report(df, model_name, output_filepath):
    """
    Generates a clean, compact LaTeX table report using booktabs.
    """
    model_escaped = model_name.replace("_", "\\_")
    
    latex_lines = [
        "\\begin{table}[htbp]",
        "\\centering",
        f"\\caption{{VTune Profiling Analysis - Model: \\texttt{{{model_escaped}}}}}",
        f"\\label{{tab:vtune_{model_name.lower().replace('-', '_')}}}",
        "\\resizebox{\\textwidth}{!}{%",
        "\\begin{tabular}{llrrrrrr}",
        "\\toprule",
        "\\textbf{Test Name} & \\textbf{Mode} & \\textbf{Time (ms)} & \\textbf{CPU Util (\\%)} & \\textbf{CPUs Active} & \\textbf{Threads} & \\textbf{Wait (s)} & \\textbf{Spin (s)} \\\\"
    ]

    grouped = df.groupby("Test Name")
    sorted_test_names = sorted(grouped.groups.keys(), key=lambda x: x.lower())

    for t_idx, test_name in enumerate(sorted_test_names):
        group_df = grouped.get_group(test_name).copy()
        test_escaped = f"\\texttt{{{test_name.replace('_', '\\_')}}}"
        
        valid_times = group_df.dropna(subset=["Total Time (ms)"])
        winner_mode = valid_times.loc[valid_times["Total Time (ms)"].idxmin()]["Mode"] if not valid_times.empty else None

        latex_lines.append("\\midrule")
        
        for r_idx, (_, row) in enumerate(group_df.iterrows()):
            is_winner = (row["Mode"] == winner_mode)
            
            mode_str = f"\\textbf{{{row['Mode']}}}" if is_winner else row['Mode']
            time_val = f"{row['Total Time (ms)']:.2f}" if pd.notna(row['Total Time (ms)']) else "-"
            if is_winner and time_val != "-":
                time_val = f"\\textbf{{{time_val}}}"

            cpu_val = f"{row['CPU Util (%)']:.1f}" if pd.notna(row['CPU Util (%)']) else "-"
            act_val = f"{row['CPUs Active']:.2f}" if pd.notna(row['CPUs Active']) else "-"
            th_val = f"{row['Threads']:.0f}" if pd.notna(row['Threads']) else "-"
            wait_val = f"{row['Wait Time (s)']:.3f}" if pd.notna(row['Wait Time (s)']) else "-"
            spin_val = f"{row['Spin Time (s)']:.3f}" if pd.notna(row['Spin Time (s)']) else "-"

            test_col = test_escaped if r_idx == 0 else ""
            
            row_str = f"{test_col} & {mode_str} & {time_val} & {cpu_val} & {act_val} & {th_val} & {wait_val} & {spin_val} \\\\"
            latex_lines.append(row_str)

    latex_lines.extend([
        "\\bottomrule",
        "\\end{tabular}%",
        "}",
        "\\end{table}"
    ])

    with open(output_filepath, 'w', encoding='utf-8') as f:
        f.write('\n'.join(latex_lines))

    print(f"[SUCCESS] LaTeX VTune report saved to: '{output_filepath}'")

def main():
    parser = argparse.ArgumentParser(
        description="TARZAN VTune Profiling Log Parser & Report Generator",
        formatter_class=argparse.RawTextHelpFormatter,
        epilog="""Examples:
  python scripts/parse_vtune.py med_app
  python scripts/parse_vtune.py med_app -t patient_doc_after_three_int_time_win single_patient_time_win
  python scripts/parse_vtune.py production_cell -m OMP TBB -f latex -o vtune_prod_cell
  python scripts/parse_vtune.py med_app -t single_patient_win -f both -o custom_vtune_summary
"""
    )

    parser.add_argument(
        "model", 
        type=str, 
        help="Target model folder name inside 'output_logs' (e.g., med_app, production_cell)."
    )

    parser.add_argument(
        "-t", "--tests",
        nargs="+",
        default=None,
        help="Filter specific test names (e.g., -t pc_F_win pc_G_lose). If omitted, all tests are included."
    )

    parser.add_argument(
        "-m", "--modes",
        nargs="+",
        default=None,
        help="Filter specific execution modes (e.g., -m SEQ OMP TBB OMP_DYNAMIC). Case-insensitive."
    )

    parser.add_argument(
        "-f", "--format", 
        choices=["html", "latex", "both"], 
        default="html", 
        help="Select output report format: 'html', 'latex', or 'both' (default: html)."
    )

    parser.add_argument(
        "-o", "--output",
        type=str,
        default=None,
        help="Custom output filename base (default: vtune_summary)."
    )

    args = parser.parse_args()

    model_name = args.model.strip("/")
    target_log_dir = os.path.join(BASE_LOG_DIR, model_name)

    print(f"[INFO] Searching VTune files for model: '{model_name}' inside '{target_log_dir}'...")
    df = collect_vtune_data(target_log_dir)

    if df.empty:
        print(f"[WARNING] No '*_vtune_run.log' files found in '{target_log_dir}'.")
        return

    # 1. Filter by Test Names if requested
    if args.tests:
        test_pattern = [t.lower() for t in args.tests]
        df = df[df["Test Name"].str.lower().isin(test_pattern)]
        if df.empty:
            print(f"[ERROR] None of the requested tests {args.tests} were found in log data.")
            sys.exit(1)

    # 2. Filter by Modes if requested
    if args.modes:
        mode_pattern = [m.upper() for m in args.modes]
        df = df[df["Mode"].str.upper().isin(mode_pattern)]
        if df.empty:
            print(f"[ERROR] None of the requested modes {args.modes} were found in log data.")
            sys.exit(1)

    # Print terminal output summary
    print("\n========================= VTUNE SUMMARY =========================")
    print(df.to_string(index=False))
    print("=================================================================\n")

    # Determine base output filename
    base_out_name = os.path.splitext(args.output)[0] if args.output else "vtune_summary"

    # Export requested file format(s)
    if args.format in ["html", "both"]:
        html_path = os.path.join(target_log_dir, f"{base_out_name}.html")
        generate_custom_html_report(df, model_name, html_path)

    if args.format in ["latex", "both"]:
        latex_path = os.path.join(target_log_dir, f"{base_out_name}.tex")
        generate_latex_report(df, model_name, latex_path)

if __name__ == "__main__":
    main()
