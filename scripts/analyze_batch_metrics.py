import os
import sys
import glob
import argparse
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.abspath(os.path.join(SCRIPT_DIR, ".."))
BASE_LOG_DIR = os.path.join(PROJECT_ROOT, "output_logs")

def collect_batch_metrics_data(model_dir):
    """
    Scans test subdirectories for metrics_*.csv files and aggregates batch data.
    """
    csv_pattern = os.path.join(model_dir, "*", "metrics_*.csv")
    csv_files = glob.glob(csv_pattern)

    if not csv_files:
        print(f"[WARNING] No 'metrics_*.csv' files found in '{model_dir}'.")
        return pd.DataFrame()

    all_data = []

    for filepath in csv_files:
        test_dir = os.path.basename(os.path.dirname(filepath))
        filename = os.path.basename(filepath)

        mode = "UNKNOWN"
        for m in ["seq", "omp", "tbb"]:
            if filename.endswith(f"_{m}.csv") or f"_{m}_" in filename:
                mode = m.upper()
                break

        try:
            df = pd.read_csv(filepath)
            if df.empty:
                continue
            
            df["TestName"] = test_dir
            df["Mode"] = mode
            df["TotalWorkload"] = df["TotalRegions"] + df["DiscPreds"] + df["DelayPreds"]
            all_data.append(df)
        except Exception as e:
            print(f"[WARNING] Error reading {filepath}: {e}")

    if not all_data:
        return pd.DataFrame()

    return pd.concat(all_data, ignore_index=True)

# ==============================================================================
# 1. PNG PLOT GENERATION (Matplotlib / Seaborn)
# ==============================================================================
def generate_png_plots(model_name, full_df, model_dir, base_out_name, font_scale, no_memory):
    # Set high-legibility theme optimized for publication figures
    sns.set_theme(style="whitegrid")
    sns.set_context("paper", font_scale=font_scale)
    
    plt.rcParams.update({
        'axes.titlesize': 14 * font_scale,
        'axes.labelsize': 12 * font_scale,
        'xtick.labelsize': 10 * font_scale,
        'ytick.labelsize': 10 * font_scale,
        'legend.fontsize': 10 * font_scale,
        'figure.titlesize': 16 * font_scale
    })

    # Global Overview Image (2x2 Grid)
    overview_path = os.path.join(model_dir, f"{base_out_name}.png")
    fig, axes = plt.subplots(2, 2, figsize=(14, 10))
    fig.suptitle(f"TARZAN Batch Profiling - Model: {model_name}", fontweight='bold')

    sns.scatterplot(
        ax=axes[0, 0], data=full_df, 
        x="TotalWorkload", y="DurationUs", 
        hue="Mode", style="Mode", alpha=0.8, s=100 * font_scale
    )
    axes[0, 0].set_title("1. Total Workload vs Time (µs)", fontweight='bold')
    axes[0, 0].set_xlabel("Total Workload (Regions + Predecessors)")
    axes[0, 0].set_ylabel("Execution Duration (µs)")

    sns.scatterplot(
        ax=axes[0, 1], data=full_df, 
        x="QueueCycles", y="DurationUs", 
        hue="Mode", style="Mode", alpha=0.8, s=100 * font_scale
    )
    axes[0, 1].set_title("2. Queue Cycles Impact vs Time (µs)", fontweight='bold')
    axes[0, 1].set_xlabel("Queue Cycles Count")
    axes[0, 1].set_ylabel("Execution Duration (µs)")

    sns.scatterplot(
        ax=axes[1, 0], data=full_df, 
        x="ToProcessBytes", y="DurationUs", 
        hue="Mode", alpha=0.8, s=100 * font_scale
    )
    axes[1, 0].set_title("3. Input Memory Size vs Time (µs)", fontweight='bold')
    axes[1, 0].set_xlabel("ToProcess Size (Bytes)")
    axes[1, 0].set_ylabel("Execution Duration (µs)")

    sns.boxplot(
        ax=axes[1, 1], data=full_df, 
        x="Mode", y="DurationUs", 
        palette="Set2"
    )
    axes[1, 1].set_title("4. Batch Duration Variability", fontweight='bold')
    axes[1, 1].set_xlabel("Execution Mode")
    axes[1, 1].set_ylabel("Execution Duration (µs)")

    plt.tight_layout()
    plt.savefig(overview_path, dpi=300)
    plt.close()
    print(f"[SUCCESS] Global overview PNG saved to: '{overview_path}'")

    # Individual Test PNG Images
    num_rows = 2 if no_memory else 3
    fig_height = 8 if no_memory else 12

    grouped_tests = full_df.groupby("TestName")
    for test_name, test_df in grouped_tests:
        test_out_dir = os.path.join(model_dir, test_name)
        os.makedirs(test_out_dir, exist_ok=True)
        
        fig, axes = plt.subplots(num_rows, 1, figsize=(12, fig_height), sharex=True)
        if num_rows == 1:
            axes = [axes]

        fig.suptitle(f"Analysis", fontweight='bold')

        # Subplot 1: Duration
        sns.lineplot(
            ax=axes[0], data=test_df, 
            x="Step", y="DurationUs", 
            hue="Mode", marker="o", linewidth=2.5, markersize=8 * font_scale
        )
        axes[0].set_title("1. Execution Duration per Step (µs)", fontweight='bold')
        axes[0].set_ylabel("Duration (µs)")
        axes[0].grid(True, linestyle="--", alpha=0.6)

        step_df = test_df.groupby("Step").mean(numeric_only=True).reset_index()

        # Subplot 2: Workload Composition
        axes[1].stackplot(
            step_df["Step"], 
            step_df["TotalRegions"], 
            step_df["DiscPreds"], 
            step_df["DelayPreds"], 
            labels=["Initial Regions (ToProcess)", "Generated DiscPreds", "Generated DelayPreds"],
            alpha=0.8, colors=["#4C72B0", "#55A868", "#C44E52"]
        )
        axes[1].set_title("2. Workload Composition", fontweight='bold')
        axes[1].set_ylabel("Number of Regions")
        axes[1].legend(loc="upper left")
        axes[1].grid(True, linestyle="--", alpha=0.6)

        # Subplot 3: Input Memory (Optional)
        if not no_memory:
            sns.lineplot(
                ax=axes[2], data=step_df, 
                x="Step", y="ToProcessBytes", 
                marker="s", color="purple", linewidth=2.5, markersize=8 * font_scale, label="ToProcess Memory"
            )
            axes[2].set_title("3. Input Memory Size per Step", fontweight='bold')
            axes[2].set_xlabel("Batch Step (piFilter Call)")
            axes[2].set_ylabel("Bytes")
            axes[2].legend(loc="upper left")
            axes[2].grid(True, linestyle="--", alpha=0.6)
        else:
            axes[1].set_xlabel("Batch Step (piFilter Call)")

        test_plot_path = os.path.join(test_out_dir, "batch_timeline_plot.png")
        plt.tight_layout()
        plt.savefig(test_plot_path, dpi=300)
        plt.close()

    print("[SUCCESS] Timeline deep-dive PNG plots generated in test subdirectories.")

# ==============================================================================
# 2. PURE LATEX / PGFPLOTS GENERATION
# ==============================================================================
def dataframe_to_pgf_coords(df, x_col, y_col):
    coords = []
    for _, row in df.iterrows():
        if pd.notna(row[x_col]) and pd.notna(row[y_col]):
            coords.append(f"({row[x_col]}, {row[y_col]})")
    return " ".join(coords)

def generate_global_overview_latex(model_name, full_df, output_tex_path):
    model_escaped = model_name.replace("_", "\\_")
    modes = full_df["Mode"].unique()
    colors = {"OMP": "blue!70!black", "TBB": "orange!80!black", "SEQ": "gray!80!black"}

    latex_lines = [
        "% Require in LaTeX preamble: \\usepackage{pgfplots}, \\usepackage{tikz}",
        "\\begin{figure*}[htbp]",
        "  \\centering",
        "  \\begin{tikzpicture}",
        "    \\pgfplotsset{every axis label/.append style={font=\\large}, every tick label/.append style={font=\\large}, title style={font=\\Large\\bfseries}}",
        "    \\begin{axis}[",
        "      name=ax1,",
        "      width=0.48\\textwidth, height=7cm,",
        "      title={1. Total Workload vs Time (\\mu s)},",
        "      xlabel={Total Workload}, ylabel={Duration (\\mu s)},",
        "      grid=major, grid style={dashed,gray!30},",
        "      legend pos=north west, legend style={font=\\small}",
        "    ]"
    ]

    for m in modes:
        m_df = full_df[full_df["Mode"] == m]
        coords = dataframe_to_pgf_coords(m_df, "TotalWorkload", "DurationUs")
        color = colors.get(m, "black")
        latex_lines.append(f"      \\addplot[only marks, mark=*, mark size=2.5pt, color={color}] coordinates {{{coords}}};")
        latex_lines.append(f"      \\addlegendentry{{{m}}}")

    latex_lines.extend([
        "    \\end{axis}",
        "    \\begin{axis}[",
        "      name=ax2, at={(ax1.right of origin)}, anchor=left of origin, xshift=1.8cm,",
        "      width=0.48\\textwidth, height=7cm,",
        "      title={2. Queue Cycles Impact vs Time (\\mu s)},",
        "      xlabel={Queue Cycles Count}, ylabel={Duration (\\mu s)},",
        "      grid=major, grid style={dashed,gray!30}",
        "    ]"
    ])

    for m in modes:
        m_df = full_df[full_df["Mode"] == m]
        coords = dataframe_to_pgf_coords(m_df, "QueueCycles", "DurationUs")
        color = colors.get(m, "black")
        latex_lines.append(f"      \\addplot[only marks, mark=x, mark size=3pt, color={color}] coordinates {{{coords}}};")

    latex_lines.extend([
        "    \\end{axis}",
        "    \\begin{axis}[",
        "      name=ax3, at={(ax1.below min energy point)}, anchor=above north point, yshift=-2.2cm,",
        "      width=0.48\\textwidth, height=7cm,",
        "      title={3. Input Memory Size vs Time (\\mu s)},",
        "      xlabel={ToProcess Size (Bytes)}, ylabel={Duration (\\mu s)},",
        "      grid=major, grid style={dashed,gray!30}",
        "    ]"
    ])

    for m in modes:
        m_df = full_df[full_df["Mode"] == m]
        coords = dataframe_to_pgf_coords(m_df, "ToProcessBytes", "DurationUs")
        color = colors.get(m, "black")
        latex_lines.append(f"      \\addplot[only marks, mark=square*, mark size=2.2pt, color={color}] coordinates {{{coords}}};")

    latex_lines.extend([
        "    \\end{axis}",
        "    \\begin{axis}[",
        "      name=ax4, at={(ax2.below min energy point)}, anchor=above north point, yshift=-2.2cm,",
        "      width=0.48\\textwidth, height=7cm,",
        "      title={4. Batch Duration Variability},",
        "      xlabel={Execution Mode}, ylabel={Duration (\\mu s)},",
        "      symbolic x coords={OMP, TBB, SEQ}, xtick=data,",
        "      grid=major, grid style={dashed,gray!30}",
        "    ]"
    ])

    for m in modes:
        if m in ["OMP", "TBB", "SEQ"]:
            m_df = full_df[full_df["Mode"] == m]
            coords = " ".join([f"({m}, {val})" for val in m_df["DurationUs"].dropna()])
            color = colors.get(m, "black")
            latex_lines.append(f"      \\addplot[only marks, mark=o, mark size=2.5pt, color={color}] coordinates {{{coords}}};")

    latex_lines.extend([
        "    \\end{axis}",
        "  \\end{tikzpicture}",
        f"  \\caption{{TARZAN Batch Profiling Overview Analysis for Model: \\texttt{{{model_escaped}}}.}}",
        f"  \\label{{fig:overview_{model_name.lower().replace('-', '_')}}}",
        "\\end{figure*}"
    ])

    with open(output_tex_path, "w", encoding="utf-8") as f:
        f.write("\n".join(latex_lines))

    print(f"[SUCCESS] Global overview pure LaTeX file saved to: '{output_tex_path}'")

def generate_test_timeline_latex(test_name, test_df, output_tex_path, no_memory):
    test_escaped = test_name.replace("_", "\\_")
    modes = test_df["Mode"].unique()
    colors = {"OMP": "blue!70!black", "TBB": "orange!80!black", "SEQ": "gray!80!black"}

    step_df = test_df.groupby("Step").mean(numeric_only=True).reset_index()

    step_df["Stack1"] = step_df["TotalRegions"]
    step_df["Stack2"] = step_df["Stack1"] + step_df["DiscPreds"]
    step_df["Stack3"] = step_df["Stack2"] + step_df["DelayPreds"]

    latex_lines = [
        "% Require in LaTeX preamble: \\usepackage{pgfplots}, \\usepackage{tikz}",
        "\\begin{figure*}[htbp]",
        "  \\centering",
        "  \\begin{tikzpicture}",
        "    \\pgfplotsset{every axis label/.append style={font=\\large}, every tick label/.append style={font=\\large}, title style={font=\\Large\\bfseries}}",
        "    % Level 1: Duration per Step",
        "    \\begin{axis}[",
        "      name=ax1,",
        "      width=0.95\\textwidth, height=5.5cm,",
        f"      title={{Deep Dive Analysis --- Test: \\texttt{{{test_escaped}}} (Execution Duration per Step)}},",
        "      ylabel={Duration (\\mu s)},",
        "      grid=major, grid style={dashed,gray!30},",
        "      legend pos=north west, legend style={font=\\medium}",
        "    ]"
    ]

    for m in modes:
        m_df = test_df[test_df["Mode"] == m].sort_values("Step")
        coords = dataframe_to_pgf_coords(m_df, "Step", "DurationUs")
        color = colors.get(m, "black")
        latex_lines.append(f"      \\addplot[very thick, color={color}, mark=*] coordinates {{{coords}}};")
        latex_lines.append(f"      \\addlegendentry{{{m}}}")

    xlabel_attr = "xlabel={Batch Step (piFilter Call)}," if no_memory else ""

    latex_lines.extend([
        "    \\end{axis}",
        "    % Level 2: Workload Composition Stacked Area",
        "    \\begin{axis}[",
        "      name=ax2, at={(ax1.below min energy point)}, anchor=above north point, yshift=-1.5cm,",
        "      width=0.95\\textwidth, height=5.5cm,",
        "      title={2. Workload Composition (Why did it spike?)},",
        f"      {xlabel_attr}",
        "      ylabel={Number of Regions},",
        "      grid=major, grid style={dashed,gray!30},",
        "      legend pos=north west, legend style={font=\\small}",
        "    ]"
    ])

    coords_s1 = dataframe_to_pgf_coords(step_df, "Step", "Stack1")
    coords_s2 = dataframe_to_pgf_coords(step_df, "Step", "Stack2")
    coords_s3 = dataframe_to_pgf_coords(step_df, "Step", "Stack3")

    latex_lines.append(f"      \\addplot[stack plots=y, area legend, fill=blue!40, draw=blue!70!black] coordinates {{{coords_s1}}} \\closedcycle;")
    latex_lines.append("      \\addlegendentry{Initial Regions (ToProcess)}")
    latex_lines.append(f"      \\addplot[stack plots=y, area legend, fill=green!40, draw=green!70!black] coordinates {{{coords_s2}}} \\closedcycle;")
    latex_lines.append("      \\addlegendentry{Generated DiscPreds}")
    latex_lines.append(f"      \\addplot[stack plots=y, area legend, fill=red!40, draw=red!70!black] coordinates {{{coords_s3}}} \\closedcycle;")
    latex_lines.append("      \\addlegendentry{Generated DelayPreds}")

    if not no_memory:
        latex_lines.extend([
            "    \\end{axis}",
            "    % Level 3: Input Memory Size",
            "    \\begin{axis}[",
            "      name=ax3, at={(ax2.below min energy point)}, anchor=above north point, yshift=-1.5cm,",
            "      width=0.95\\textwidth, height=4.8cm,",
            "      title={3. Input Memory Size per Step},",
            "      xlabel={Batch Step (piFilter Call)}, ylabel={Bytes},",
            "      grid=major, grid style={dashed,gray!30},",
            "      legend pos=north west, legend style={font=\\small}",
            "    ]"
        ])
        coords_mem = dataframe_to_pgf_coords(step_df, "Step", "ToProcessBytes")
        latex_lines.append(f"      \\addplot[very thick, color=purple!80!black, mark=square*] coordinates {{{coords_mem}}};")
        latex_lines.append("      \\addlegendentry{ToProcess Memory}")

    latex_lines.extend([
        "    \\end{axis}",
        "  \\end{tikzpicture}",
        f"  \\caption{{Step-by-step workload timeline deep dive for test \\texttt{{{test_escaped}}}.}}",
        f"  \\label{{fig:timeline_{test_name.lower().replace('-', '_')}}}",
        "\\end{figure*}"
    ])

    with open(output_tex_path, "w", encoding="utf-8") as f:
        f.write("\n".join(latex_lines))

def generate_pure_latex_files(model_name, full_df, model_dir, base_out_name, no_memory):
    global_tex_path = os.path.join(model_dir, f"{base_out_name}.tex")
    generate_global_overview_latex(model_name, full_df, global_tex_path)

    grouped_tests = full_df.groupby("TestName")
    for test_name, test_df in grouped_tests:
        test_out_dir = os.path.join(model_dir, test_name)
        os.makedirs(test_out_dir, exist_ok=True)
        test_tex_path = os.path.join(test_out_dir, "batch_timeline_plot.tex")
        generate_test_timeline_latex(test_name, test_df, test_tex_path, no_memory)

    print("[SUCCESS] Pure LaTeX (.tex) files generated in root model folder and test subdirectories.")

def main():
    parser = argparse.ArgumentParser(
        description="TARZAN Batch Profiling Plot & LaTeX Code Generator",
        formatter_class=argparse.RawTextHelpFormatter,
        epilog="""Examples:
  python scripts/analyze_batch_metrics.py med_app --no-memory -f png -s 1.6
  python scripts/analyze_batch_metrics.py production_cell --no-memory -f latex
  python scripts/analyze_batch_metrics.py autoclave_sterilization_system -f both
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
        help="Filter specific test subdirectories (e.g., -t ass_wc1 ass_wc2). If omitted, all tests are processed."
    )

    parser.add_argument(
        "-f", "--format", 
        choices=["png", "latex", "both"], 
        default="png", 
        help="Select output format: 'png' (image files), 'latex' (pure PGFPlots TikZ .tex files), or 'both' (default: png)."
    )

    parser.add_argument(
        "-s", "--font-scale",
        type=float,
        default=1.5,
        help="Scale factor for text/labels/titles for paper publication (default: 1.5)."
    )

    parser.add_argument(
        "--no-memory",
        action="store_true",
        help="Disable the 3rd subplot (Input Memory Size) in per-test timeline deep dives, keeping only 2 levels."
    )

    parser.add_argument(
        "-o", "--output",
        type=str,
        default=None,
        help="Custom output base filename for global overview file (default: batch_metrics_overview)."
    )

    args = parser.parse_args()

    model_name = args.model.strip("/")
    model_dir = os.path.join(BASE_LOG_DIR, model_name)

    if not os.path.exists(model_dir):
        print(f"[ERROR] Model directory not found: '{model_dir}'")
        sys.exit(1)

    print("==================================================")
    print(f" Batch Metrics Analysis for Model: {model_name}")
    print(f" Font Scale Factor: {args.font_scale} | Skip Memory Plot: {args.no_memory}")
    print("==================================================")

    full_df = collect_batch_metrics_data(model_dir)

    if full_df.empty:
        print(f"[ERROR] No valid data extracted from CSV files in '{model_dir}'.")
        return

    if args.tests:
        tests_lower = [t.lower() for t in args.tests]
        full_df = full_df[full_df["TestName"].str.lower().isin(tests_lower)]
        if full_df.empty:
            print(f"[ERROR] None of the requested tests {args.tests} were found.")
            sys.exit(1)

    base_out_name = os.path.splitext(args.output)[0] if args.output else "batch_metrics_overview"

    # 1. Generate PNG images if requested
    if args.format in ["png", "both"]:
        generate_png_plots(model_name, full_df, model_dir, base_out_name, args.font_scale, args.no_memory)

    # 2. Generate Pure LaTeX .tex files if requested
    if args.format in ["latex", "both"]:
        generate_pure_latex_files(model_name, full_df, model_dir, base_out_name, args.no_memory)

if __name__ == "__main__":
    main()
