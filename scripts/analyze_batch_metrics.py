import os
import sys
import glob
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.abspath(os.path.join(SCRIPT_DIR, ".."))
BASE_LOG_DIR = os.path.join(PROJECT_ROOT, "output_logs")

def analyze_model_batch_metrics(model_name):
    model_dir = os.path.join(BASE_LOG_DIR, model_name)
    
    if not os.path.exists(model_dir):
        print(f"[ERROR] Model directory not found: '{model_dir}'")
        sys.exit(1)

    print(f"==================================================")
    print(f" Batch-Level Metrics Analysis for Model: {model_name}")
    print(f"==================================================")

    # Search for all metrics_*.csv files in test subdirectories
    csv_pattern = os.path.join(model_dir, "*", "metrics_*.csv")
    csv_files = glob.glob(csv_pattern)

    if not csv_files:
        print(f"[WARNING] No 'metrics_*.csv' files found in '{model_dir}'.")
        return

    all_data = []

    for filepath in csv_files:
        test_dir = os.path.basename(os.path.dirname(filepath))
        filename = os.path.basename(filepath)

        # Extract mode from filename (seq, omp, tbb)
        mode = "UNKNOWN"
        for m in ["seq", "omp", "tbb"]:
            if filename.endswith(f"_{m}.csv"):
                mode = m.upper()
                break

        try:
            df = pd.read_csv(filepath)
            if df.empty:
                continue
            
            df["TestName"] = test_dir
            df["Mode"] = mode
            
            # Calculate total effective workload
            df["TotalWorkload"] = df["TotalRegions"] + df["DiscPreds"] + df["DelayPreds"]
            all_data.append(df)
        except Exception as e:
            print(f"[WARNING] Error reading {filepath}: {e}")

    if not all_data:
        print("[ERROR] No valid data extracted from CSV files.")
        return

    full_df = pd.concat(all_data, ignore_index=True)

    # ---------------------------------------------------------
    # 1. GLOBAL CORRELATION MATRIX WITH DURATIONUS
    # ---------------------------------------------------------
    print("\n--- CORRELATION MATRIX (Relative to DurationUs) ---")
    corr_cols = [
        "TotalRegions", "ToProcessBytes", "SetGBytes", 
        "DiscPreds", "DelayPreds", "QueueCycles", 
        "TotalWorkload", "DurationUs"
    ]
    
    # Filter valid existing columns
    valid_cols = [c for c in corr_cols if c in full_df.columns]
    corr_matrix = full_df[valid_cols].corr()
    
    print(corr_matrix["DurationUs"].sort_values(ascending=False).to_string())
    print("---------------------------------------------------\n")

    # ---------------------------------------------------------
    # 2. OVERALL MODEL GRAPH GENERATION
    # ---------------------------------------------------------
    sns.set_theme(style="whitegrid")
    fig, axes = plt.subplots(2, 2, figsize=(16, 11))
    fig.suptitle(f"TARZAN Batch Profiling Analysis - Model: {model_name}", fontsize=16, fontweight='bold')

    # Graph 1: Total Workload Correlation vs Execution Time
    sns.scatterplot(
        ax=axes[0, 0], data=full_df, 
        x="TotalWorkload", y="DurationUs", 
        hue="Mode", style="Mode", alpha=0.7, s=70
    )
    axes[0, 0].set_title("1. Total Workload (Regions + Predecessors) vs Time (µs)")
    axes[0, 0].set_xlabel("Total Workload (Regions + DiscPreds + DelayPreds)")
    axes[0, 0].set_ylabel("Execution Duration (µs)")

    # Graph 2: Queue Cycles Impact (While-Loop Search Depth)
    sns.scatterplot(
        ax=axes[0, 1], data=full_df, 
        x="QueueCycles", y="DurationUs", 
        hue="Mode", style="Mode", alpha=0.7, s=70
    )
    axes[0, 1].set_title("2. Queue Cycles Impact (Delay Loop) vs Time (µs)")
    axes[0, 1].set_xlabel("Queue Cycles Count")
    axes[0, 1].set_ylabel("Execution Duration (µs)")

    # Graph 3: ToProcess Memory Size (Bytes) vs Time
    sns.scatterplot(
        ax=axes[1, 0], data=full_df, 
        x="ToProcessBytes", y="DurationUs", 
        hue="Mode", alpha=0.7, s=70
    )
    axes[1, 0].set_title("3. Input Memory (ToProcess Bytes) vs Time (µs)")
    axes[1, 0].set_xlabel("ToProcess Size (Bytes)")
    axes[1, 0].set_ylabel("Execution Duration (µs)")

    # Graph 4: Batch Duration Distribution by Mode
    sns.boxplot(
        ax=axes[1, 1], data=full_df, 
        x="Mode", y="DurationUs", 
        palette="Set2"
    )
    axes[1, 1].set_title("4. Batch Duration Variability by Mode")
    axes[1, 1].set_xlabel("Execution Mode")
    axes[1, 1].set_ylabel("Execution Duration (µs)")

    plt.tight_layout()
    overview_plot_path = os.path.join(model_dir, "batch_metrics_overview.png")
    plt.savefig(overview_plot_path, dpi=300)
    plt.close()
    print(f"[SUCCESS] Global model plot saved to: '{overview_plot_path}'")
# ---------------------------------------------------------
    # 3. PER-TEST TIMELINE PLOT GENERATION (ENHANCED)
    # ---------------------------------------------------------
    grouped_tests = full_df.groupby("TestName")
    
    for test_name, test_df in grouped_tests:
        test_out_dir = os.path.join(model_dir, test_name)
        
        # Creiamo un grafico a 3 livelli (Time, Workload, Memory) allineati sullo stesso asse X
        fig, axes = plt.subplots(3, 1, figsize=(14, 12), sharex=True)
        fig.suptitle(f"Deep Dive Analysis - Test: {test_name}", fontsize=16, fontweight='bold')

        # --- Livello 1: Tempo di Esecuzione ---
        sns.lineplot(
            ax=axes[0], data=test_df, 
            x="Step", y="DurationUs", 
            hue="Mode", marker="o", linewidth=2
        )
        axes[0].set_title("1. Execution Duration per Step (Who won and where?)", fontsize=12, fontweight='bold')
        axes[0].set_ylabel("Duration (µs)")
        axes[0].grid(True, linestyle="--", alpha=0.6)

        # Il grafo è lo stesso per tutte le modalità, calcoliamo la media del workload per avere una riga pulita
        step_df = test_df.groupby("Step").mean(numeric_only=True).reset_index()

        # --- Livello 2: Composizione del Workload (Area Impilata) ---
        axes[1].stackplot(
            step_df["Step"], 
            step_df["TotalRegions"], 
            step_df["DiscPreds"], 
            step_df["DelayPreds"], 
            labels=["Initial Regions (ToProcess)", "Generated DiscPreds", "Generated DelayPreds"],
            alpha=0.75, colors=["#4C72B0", "#55A868", "#C44E52"]
        )
        axes[1].set_title("2. Workload Composition (Why did it spike?)", fontsize=12, fontweight='bold')
        axes[1].set_ylabel("Number of Regions")
        axes[1].legend(loc="upper left")
        axes[1].grid(True, linestyle="--", alpha=0.6)

        # --- Livello 3: Memoria in Ingresso ---
        sns.lineplot(
            ax=axes[2], data=step_df, 
            x="Step", y="ToProcessBytes", 
            marker="s", color="purple", linewidth=2, label="ToProcess Memory"
        )
        axes[2].set_title("3. Input Memory Size per Step", fontsize=12, fontweight='bold')
        axes[2].set_xlabel("Batch Step (piFilter Call)", fontsize=11)
        axes[2].set_ylabel("Bytes")
        axes[2].legend(loc="upper left")
        axes[2].grid(True, linestyle="--", alpha=0.6)

        test_plot_path = os.path.join(test_out_dir, "batch_timeline_plot.png")
        plt.tight_layout()
        plt.savefig(test_plot_path, dpi=300)
        plt.close()

    print(f"[SUCCESS] Timeline deep-dive plots for individual tests generated in respective subdirectories.")    

def main():
    if len(sys.argv) < 2:
        print("[ERROR] You must specify the model name as an argument.")
        print("Example: python scripts/analyze_batch_metrics.py production_cell")
        sys.exit(1)

    model_name = sys.argv[1].strip("/")
    analyze_model_batch_metrics(model_name)

if __name__ == "__main__":
    main()
