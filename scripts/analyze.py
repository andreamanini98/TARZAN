import os
import sys
import re
import argparse
import numpy as np
import pandas as pd

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.abspath(os.path.join(SCRIPT_DIR, ".."))
BASE_LOG_DIR = os.path.join(PROJECT_ROOT, "output_logs")

def parse_log_file(filepath):
    """
    Parses a log file to extract execution times in microseconds 
    and calculates the average duration in milliseconds.
    """
    times = []
    pattern = re.compile(r"Total time including region generation:\s*([0-9\.eE\+\-]+)\s*microseconds")
    
    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
        for line in f:
            match = pattern.search(line)
            if match:
                times.append(float(match.group(1)))
                
    if not times:
        return None
    
    mean_time_ms = np.mean(times) / 1000.0
    return round(mean_time_ms, 2)

def collect_benchmark_data(log_dir):
    """
    Recursively scans the log directory for execution logs, aggregates 
    metrics per test and execution mode, and returns a Pandas DataFrame.
    """
    data = {}
    
    if not os.path.exists(log_dir):
        print(f"[ERROR] Log directory not found: '{log_dir}'")
        return None

    known_modes = ["seq", "omp", "tbb"]

    for root, dirs, files in os.walk(log_dir):
        dirs[:] = [d for d in dirs if not d.startswith('.')]
        
        for filename in files:
            if not (filename.endswith(".log") or filename.endswith(".txt")):
                continue
            if filename.endswith("vtune_run.log"):
                continue
                
            filepath = os.path.join(root, filename)
            base_name = os.path.splitext(filename)[0]
            
            mode_found = None
            test_name = None
            
            for m in known_modes:
                pattern = re.compile(rf"^(.*)_({m}(?:_[\w\-]+)?)$")
                match = pattern.match(base_name)
                if match:
                    test_name = match.group(1)
                    mode_found = match.group(2)
                    break
                    
            if mode_found and test_name:
                avg_time = parse_log_file(filepath)
                if avg_time is not None:
                    if mode_found not in data:
                        data[mode_found] = {}
                    data[mode_found][test_name] = avg_time

    if not data:
        return None

    df = pd.DataFrame(data)
    
    standard_cols = [c for c in ["seq", "omp", "tbb"] if c in df.columns]
    tagged_cols = sorted([c for c in df.columns if c not in standard_cols])
    ordered_cols = standard_cols + tagged_cols
    df = df[ordered_cols]

    df.sort_index(key=lambda x: x.str.lower(), inplace=True)
    return df

def print_terminal_summary(df, model_name):
    """
    Prints a formatted summary table to the terminal.
    """
    cols = list(df.columns)
    col_widths = {col: max(len(col) + 5, 12) for col in cols}
    header_str = f"{'TEST NAME (' + model_name.upper() + ')':<40} | " + " | ".join([f"{col.upper() + ' (ms)':<{col_widths[col]}}" for col in cols]) + " | WINNER   | SPEEDUP "
    
    divider_len = len(header_str) + 5
    print("\n" + "=" * divider_len)
    print(header_str)
    print("-" * divider_len)
    
    for index, row in df.iterrows():
        row_str = f"{(index[:37] + '...') if len(index) > 40 else index:<40} | "
        
        vals_str = []
        for col in cols:
            val = f"{row[col]:.2f}" if not pd.isna(row[col]) else "N/A"
            vals_str.append(f"{val:<{col_widths[col]}}")
            
        row_str += " | ".join(vals_str)
        
        valid_vals = row.dropna()
        if len(valid_vals) > 0:
            winner = valid_vals.idxmin().upper()
            best_time = valid_vals.min()
            
            if 'seq' in valid_vals and winner != 'SEQ':
                speedup = f"{row['seq'] / best_time:.2f}x"
            else:
                worst_time = valid_vals.max()
                speedup = f"{worst_time / best_time:.2f}x" if worst_time != best_time else "-"
        else:
            winner = "N/A"
            speedup = "-"

        row_str += f" | {winner:<8} | {speedup:<8}"
        print(row_str)
    
    print("=" * divider_len + "\n")

def generate_color_gradient(df):
    """
    Generates CSS styles highlighting winning cells with a green color gradient.
    """
    styles = pd.DataFrame('', index=df.index, columns=df.columns)
    
    for row_idx in df.index:
        row_values = df.loc[row_idx].dropna()
        if len(row_values) == 0:
            continue
            
        best_mode = row_values.idxmin()
        best_val = row_values[best_mode]
        
        other_values = row_values[row_values.index != best_mode]
        if len(other_values) > 0:
            second_best = other_values.min()
            margin = (second_best - best_val) / second_best
        else:
            margin = 0.10
            
        intensity = min(0.90, max(0.25, margin * 1.5))
        text_color = "white" if intensity > 0.55 else "black"
        background = f"rgba(46, 125, 50, {intensity:.2f})"
        
        styles.loc[row_idx, best_mode] = (
            f"background-color: {background}; "
            f"color: {text_color}; "
            f"font-weight: bold; "
            f"border: 2px solid #1b5e20;"
        )
        
    return styles

def export_html_report(df, model_name, output_path):
    """
    Generates and saves a styled HTML report.
    """
    try:
        styled_df = df.style.apply(generate_color_gradient, axis=None)\
                            .format("{:.2f} ms", na_rep="-")\
                            .set_caption(f"TARZAN Benchmark Report - Model: {model_name} (Average Execution Time in ms — Darker Green = Greater Margin)")\
                            .set_table_styles([
                                {'selector': 'table', 'props': [('border-collapse', 'collapse'), ('font-family', 'Arial, sans-serif'), ('width', '100%')]},
                                {'selector': 'th', 'props': [('background-color', '#263238'), ('color', 'white'), ('padding', '12px'), ('text-align', 'center'), ('border', '1px solid #455a64')]},
                                {'selector': 'td', 'props': [('padding', '10px'), ('text-align', 'center'), ('border', '1px solid #cfd8dc')]},
                                {'selector': 'caption', 'props': [('font-size', '16px'), ('font-weight', 'bold'), ('margin-bottom', '12px')]}
                            ])

        styled_df.to_html(output_path)
        print(f"[SUCCESS] HTML report successfully saved to: '{output_path}'")
    except AttributeError:
        print("\n[WARNING] Failed to generate HTML report.")
        print("Please install jinja2 using: pip install jinja2")

def export_latex_report(df, model_name, output_path):
    """
    Generates and saves a publication-ready LaTeX table report using booktabs format.
    """
    formatted_df = pd.DataFrame(index=df.index, columns=df.columns)
    
    for row_idx in df.index:
        row_vals = df.loc[row_idx]
        valid_vals = row_vals.dropna()
        best_col = valid_vals.idxmin() if not valid_vals.empty else None
        
        for col in df.columns:
            val = row_vals[col]
            if pd.isna(val):
                formatted_df.loc[row_idx, col] = "-"
            elif col == best_col:
                formatted_df.loc[row_idx, col] = f"\\textbf{{{val:.2f}}}"
            else:
                formatted_df.loc[row_idx, col] = f"{val:.2f}"

    escaped_index = [f"\\texttt{{{idx.replace('_', '\\_')}}}" for idx in formatted_df.index]
    escaped_cols = [col.replace('_', '\\_') for col in formatted_df.columns]
    
    col_spec = "l" + "r" * len(df.columns)
    model_escaped = model_name.replace('_', '\\_')
    
    latex_lines = [
        "\\begin{table}[htbp]",
        "\\centering",
        f"\\caption{{TARZAN Benchmark Report - Model: \\texttt{{{model_escaped}}} (Average Execution Time in ms)}}",
        f"\\label{{tab:{model_name.lower().replace('-', '_')}_benchmark}}",
        f"\\begin{{tabular}}{{{col_spec}}}",
        "\\toprule",
        "\\textbf{Test / Mode} & " + " & ".join([f"\\textbf{{{c}}}" for c in escaped_cols]) + " \\\\",
        "\\midrule"
    ]
    
    for idx_str, (_, row) in zip(escaped_index, formatted_df.iterrows()):
        row_str = f"{idx_str} & " + " & ".join([str(v) for v in row.values]) + " \\\\"
        latex_lines.append(row_str)
        
    latex_lines.extend([
        "\\bottomrule",
        "\\end{tabular}",
        "\\end{table}"
    ])
    
    latex_code = "\n".join(latex_lines)
    
    with open(output_path, "w", encoding="utf-8") as f:
        f.write(latex_code)
        
    print(f"[SUCCESS] LaTeX report successfully saved to: '{output_path}'")

def main():
    parser = argparse.ArgumentParser(
        description="TARZAN Benchmark Log Parser & Report Generator",
        formatter_class=argparse.RawTextHelpFormatter,
        epilog="""Examples:
  python scripts/parse_benchmark.py med_app
  python scripts/parse_benchmark.py production_cell --cols seq omp tbb --format html
  python scripts/parse_benchmark.py autoclave_sterilization_system -f latex -o my_report
  python scripts/parse_benchmark.py med_app -c seq tbb omp_dynamic -f both -o med_app_summary
"""
    )
    
    parser.add_argument(
        "model", 
        type=str, 
        help="Target model folder name inside 'output_logs' (e.g., med_app, production_cell)."
    )
    
    parser.add_argument(
        "-c", "--cols", "--columns",
        nargs="+", 
        default=None,
        help="Specify column modes to include (e.g., seq omp tbb tbb_treshold50). If omitted, all detected modes are included."
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
        help="Custom output filename or base name (default: benchmark_summary)."
    )

    args = parser.parse_args()

    model_name = args.model.strip("/")
    target_log_dir = os.path.join(BASE_LOG_DIR, model_name)

    print(f"[INFO] Analyzing benchmark data for model: '{model_name}'")
    print(f"[INFO] Target log directory: '{target_log_dir}'")

    df = collect_benchmark_data(target_log_dir)
    
    if df is None or df.empty:
        print(f"[WARNING] No valid log data found in '{target_log_dir}'.")
        return

    # Filter columns if requested
    if args.cols:
        requested_cols = [c for c in args.cols if c in df.columns]
        missing_cols = [c for c in args.cols if c not in df.columns]
        
        if missing_cols:
            print(f"[WARNING] The following requested columns were not found in data: {missing_cols}")
            print(f"[INFO] Available columns are: {list(df.columns)}")
            
        if not requested_cols:
            print("[ERROR] None of the requested columns were found in the dataset.")
            sys.exit(1)
            
        df = df[requested_cols]

    # Print terminal output summary
    print_terminal_summary(df, model_name)

    # Determine base output filename
    if args.output:
        base_out_name = os.path.splitext(args.output)[0]
    else:
        base_out_name = "benchmark_summary"

    # Export requested file format(s)
    if args.format in ["html", "both"]:
        html_path = os.path.join(target_log_dir, f"{base_out_name}.html")
        export_html_report(df, model_name, html_path)
        
    if args.format in ["latex", "both"]:
        latex_path = os.path.join(target_log_dir, f"{base_out_name}.tex")
        export_latex_report(df, model_name, latex_path)

if __name__ == "__main__":
    main()
