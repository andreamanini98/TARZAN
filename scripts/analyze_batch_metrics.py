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
    Scans test subdirectories for metrics_*.csv files and aggregates step-by-step batch metrics.
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
            if filename.endswith(f"_{m}.csv") or f"_{m}_" in filename or f"_{m}." in filename:
                mode = m.upper()
                break

        try:
            df = pd.read_csv(filepath)
            if df.empty:
                continue

            df["TestName"] = test_dir
            df["Mode"] = mode
            if "ActiveThreads" not in df.columns:
                df["ActiveThreads"] = 1 if mode == "SEQ" else np.nan

            df["TotalWorkload"] = df["TotalRegions"] + df["DiscPreds"] + df["DelayPreds"]
            all_data.append(df)
        except Exception as e:
            print(f"[WARNING] Error reading {filepath}: {e}")

    if not all_data:
        return pd.DataFrame()

    return pd.concat(all_data, ignore_index=True)

# ==============================================================================
# 1. PER-TEST PNG PLOTS (Matplotlib / Seaborn)
# ==============================================================================
def generate_png_plots(full_df, model_dir, font_scale):
    sns.set_theme(style="whitegrid")
    sns.set_context("paper", font_scale=font_scale)
    
    plt.rcParams.update({
        'axes.titlesize': 13 * font_scale,
        'axes.labelsize': 11 * font_scale,
        'xtick.labelsize': 10 * font_scale,
        'ytick.labelsize': 10 * font_scale,
        'legend.fontsize': 9.5 * font_scale,
        'figure.titlesize': 15 * font_scale
    })

    grouped_tests = full_df.groupby("TestName")

    for test_name, test_df in grouped_tests:
        test_out_dir = os.path.join(model_dir, test_name)
        os.makedirs(test_out_dir, exist_ok=True)
        
        fig, axes = plt.subplots(4, 1, figsize=(13, 15), sharex=True)
        fig.suptitle(f"Step-by-Step Profile Deep Dive — Test: {test_name}", fontweight='bold')

        # Panel 1: Duration per Step (µs)
        sns.lineplot(
            ax=axes[0], data=test_df, 
            x="Step", y="DurationUs", 
            hue="Mode", marker="o", linewidth=2.2, markersize=7 * font_scale
        )
        axes[0].set_title("1. Execution Duration per Step (µs)", fontweight='bold')
        axes[0].set_ylabel("Duration (µs)")
        axes[0].grid(True, linestyle="--", alpha=0.6)

        # ----------------------------------------------------------------------
        # Panel 2: Active Threads per Step (con offset visivo anti-sovrapposizione)
        # ----------------------------------------------------------------------
        test_df_threads = test_df.copy()
        offset_map = {"OMP": 0.06, "TBB": -0.06, "SEQ": 0.0}
        test_df_threads["ActiveThreadsPlot"] = test_df_threads.apply(
            lambda r: r["ActiveThreads"] + offset_map.get(r["Mode"], 0.0), axis=1
        )

        sns.lineplot(
            ax=axes[1], data=test_df_threads, 
            x="Step", y="ActiveThreadsPlot", 
            hue="Mode", style="Mode",
            markers={"OMP": "o", "TBB": "s", "SEQ": "^"},
            dashes={"OMP": "", "TBB": (3, 2), "SEQ": (1, 1)},
            linewidth=2.5, alpha=0.85, markersize=7.5 * font_scale
        )
        axes[1].set_title("2. Active Threads per Step (Micro-offset per linee sovrapposte)", fontweight='bold')
        axes[1].set_ylabel("Active Threads")
        axes[1].grid(True, linestyle="--", alpha=0.6)

        # Panel 3: Workload Composition Stackplot
        step_df = test_df.groupby("Step").mean(numeric_only=True).reset_index()
        axes[2].stackplot(
            step_df["Step"], 
            step_df["TotalRegions"], 
            step_df["DiscPreds"], 
            step_df["DelayPreds"], 
            labels=["Initial Regions (ToProcess)", "Discrete Predecessors", "Delay Predecessors"],
            alpha=0.8, colors=["#4C72B0", "#55A868", "#C44E52"]
        )
        axes[2].set_title("3. Workload Composition per Step", fontweight='bold')
        axes[2].set_ylabel("Number of Regions")
        axes[2].legend(loc="upper left")
        axes[2].grid(True, linestyle="--", alpha=0.6)

        # Panel 4: Memory Sizes (ToProcessBytes & SetGBytes)
        sns.lineplot(
            ax=axes[3], data=step_df, 
            x="Step", y="ToProcessBytes", 
            marker="^", color="purple", linewidth=2.0, label="ToProcess Memory (Bytes)"
        )
        sns.lineplot(
            ax=axes[3], data=step_df, 
            x="Step", y="SetGBytes", 
            marker="v", color="teal", linewidth=2.0, label="SetG Memory (Bytes)"
        )
        axes[3].set_title("4. Input & Target Memory Sizes (Bytes)", fontweight='bold')
        axes[3].set_xlabel("Batch Step (piFilter Call)")
        axes[3].set_ylabel("Memory (Bytes)")
        axes[3].legend(loc="upper left")
        axes[3].grid(True, linestyle="--", alpha=0.6)

        test_plot_path = os.path.join(test_out_dir, "batch_timeline_plot.png")
        plt.tight_layout()
        plt.savefig(test_plot_path, dpi=300)
        plt.close()

    print(f"[SUCCESS] Per-test 4-panel PNG plots generated in test subdirectories.")
# ==============================================================================
# 2. PER-TEST LATEX / PGFPLOTS GENERATION
# ==============================================================================
def dataframe_to_pgf_coords(df, x_col, y_col):
    coords = []
    for _, row in df.iterrows():
        if pd.notna(row[x_col]) and pd.notna(row[y_col]):
            coords.append(f"({row[x_col]}, {row[y_col]})")
    return " ".join(coords)

def generate_test_timeline_latex(test_name, test_df, output_tex_path):
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
        "    % Panel 1: Duration per Step",
        "    \\begin{axis}[",
        "      name=ax1,",
        "      width=0.95\\textwidth, height=4.8cm,",
        f"      title={{Step-by-Step Deep Dive --- Test: \\texttt{{{test_escaped}}} (Execution Duration)}},",
        "      ylabel={Duration (\\mu s)},",
        "      grid=major, grid style={dashed,gray!30},",
        "      legend pos=north west, legend style={font=\\small}",
        "    ]"
    ]

    for m in modes:
        m_df = test_df[test_df["Mode"] == m].sort_values("Step")
        coords = dataframe_to_pgf_coords(m_df, "Step", "DurationUs")
        color = colors.get(m, "black")
        latex_lines.append(f"      \\addplot[very thick, color={color}, mark=*] coordinates {{{coords}}};")
        latex_lines.append(f"      \\addlegendentry{{{m}}}")

    latex_lines.extend([
        "    \\end{axis}",
        "    % Panel 2: Active Threads per Step",
        "    \\begin{axis}[",
        "      name=ax2, at={(ax1.below min energy point)}, anchor=above north point, yshift=-1.4cm,",
        "      width=0.95\\textwidth, height=4.5cm,",
        "      title={2. Active Threads per Step},",
        "      ylabel={Active Threads},",
        "      grid=major, grid style={dashed,gray!30},",
        "      legend pos=north west, legend style={font=\\small}",
        "    ]"
    ])

    for m in modes:
        m_df = test_df[test_df["Mode"] == m].sort_values("Step")
        coords = dataframe_to_pgf_coords(m_df, "Step", "ActiveThreads")
        color = colors.get(m, "black")
        latex_lines.append(f"      \\addplot[very thick, color={color}, mark=square*] coordinates {{{coords}}};")
        latex_lines.append(f"      \\addlegendentry{{{m}}}")

    latex_lines.extend([
        "    \\end{axis}",
        "    % Panel 3: Workload Composition Stacked Area",
        "    \\begin{axis}[",
        "      name=ax3, at={(ax2.below min energy point)}, anchor=above north point, yshift=-1.4cm,",
        "      width=0.95\\textwidth, height=4.8cm,",
        "      title={3. Workload Composition per Step},",
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
    latex_lines.append("      \\addlegendentry{Discrete Predecessors}")
    latex_lines.append(f"      \\addplot[stack plots=y, area legend, fill=red!40, draw=red!70!black] coordinates {{{coords_s3}}} \\closedcycle;")
    latex_lines.append("      \\addlegendentry{Delay Predecessors}")

    latex_lines.extend([
        "    \\end{axis}",
        "    % Panel 4: Memory Sizes",
        "    \\begin{axis}[",
        "      name=ax4, at={(ax3.below min energy point)}, anchor=above north point, yshift=-1.4cm,",
        "      width=0.95\\textwidth, height=4.5cm,",
        "      title={4. Memory Footprint per Step},",
        "      xlabel={Batch Step (piFilter Call)}, ylabel={Bytes},",
        "      grid=major, grid style={dashed,gray!30},",
        "      legend pos=north west, legend style={font=\\small}",
        "    ]"
    ])
    
    coords_toprocess = dataframe_to_pgf_coords(step_df, "Step", "ToProcessBytes")
    coords_setg = dataframe_to_pgf_coords(step_df, "Step", "SetGBytes")

    latex_lines.append(f"      \\addplot[very thick, color=purple!80!black, mark=triangle*] coordinates {{{coords_toprocess}}};")
    latex_lines.append("      \\addlegendentry{ToProcess Bytes}")
    latex_lines.append(f"      \\addplot[very thick, color=teal!80!black, mark=diamond*] coordinates {{{coords_setg}}};")
    latex_lines.append("      \\addlegendentry{SetG Bytes}")

    latex_lines.extend([
        "    \\end{axis}",
        "  \\end{tikzpicture}",
        f"  \\caption{{Step-by-step metrics timeline deep dive for test \\texttt{{{test_escaped}}}.}}",
        f"  \\label{{fig:timeline_{test_name.lower().replace('-', '_')}}}",
        "\\end{figure*}"
    ])

    with open(output_tex_path, "w", encoding="utf-8") as f:
        f.write("\n".join(latex_lines))

def generate_pure_latex_files(full_df, model_dir):
    grouped_tests = full_df.groupby("TestName")
    for test_name, test_df in grouped_tests:
        test_out_dir = os.path.join(model_dir, test_name)
        os.makedirs(test_out_dir, exist_ok=True)
        test_tex_path = os.path.join(test_out_dir, "batch_timeline_plot.tex")
        generate_test_timeline_latex(test_name, test_df, test_tex_path)

    print("[SUCCESS] Pure LaTeX (.tex) 4-panel timeline files generated in test subdirectories.")

def main():
    parser = argparse.ArgumentParser(
        description="TARZAN Per-Test Batch Metrics Timeline Generator",
        formatter_class=argparse.RawTextHelpFormatter,
        epilog="""Examples:
  python scripts/analyze_batch_metrics.py med_app -f png -s 1.5
  python scripts/analyze_batch_metrics.py production_cell -f latex
  python scripts/analyze_batch_metrics.py med_app -t single_patient_win -f both
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
        help="Filter specific test subdirectories (e.g., -t single_patient_win). If omitted, all tests are processed."
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
        default=1.4,
        help="Scale factor for text/labels/titles for paper publication (default: 1.4)."
    )

    args = parser.parse_args()

    model_name = args.model.strip("/")
    model_dir = os.path.join(BASE_LOG_DIR, model_name)

    if not os.path.exists(model_dir):
        print(f"[ERROR] Model directory not found: '{model_dir}'")
        sys.exit(1)

    print("==================================================")
    print(f" Per-Test Metrics Timeline Generator — Model: {model_name}")
    print(f" Font Scale Factor: {args.font_scale} | Format: {args.format}")
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

    if args.format in ["png", "both"]:
        generate_png_plots(full_df, model_dir, args.font_scale)

    if args.format in ["latex", "both"]:
        generate_pure_latex_files(full_df, model_dir)

if __name__ == "__main__":
    main()
