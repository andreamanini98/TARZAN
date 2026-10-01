import os
import re
import pandas as pd

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.abspath(os.path.join(SCRIPT_DIR, ".."))

LOG_DIR = os.path.join(PROJECT_ROOT, "output_logs")
HTML_OUTPUT = os.path.join(PROJECT_ROOT, "vtune_summary.html")

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
    Explores the directories to find VTune logs and builds the DataFrame sorted alphabetically.
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
            mode = "unknown"
            test_name = base
            
            for m in ["_seq", "_omp", "_tbb"]:
                if base.endswith(m):
                    mode = m.replace("_", "")
                    test_name = base[:-len(m)]
                    break
            
            metrics = parse_vtune_log(filepath)
            metrics["Test Name"] = test_name
            metrics["Mode"] = mode.upper()
            rows.append(metrics)
            
    df = pd.DataFrame(rows)
    if not df.empty:
        cols = ["Test Name", "Mode", "Total Time (ms)", "CPU Util (%)", "CPUs Active", "Threads", "Wait Time (s)", "Spin Time (s)"]
        df = df[cols]
        # Ordinamento Alfabetico rigoroso su 'Test Name' (e secondariamente su 'Mode')
        df.sort_values(by=["Test Name", "Mode"], key=lambda col: col.str.lower(), inplace=True)
        df.reset_index(drop=True, inplace=True)
        
    return df

def generate_custom_html_style(df):
    """
    Generates inline CSS colors manually without requiring matplotlib.
    """
    styles = pd.DataFrame('', index=df.index, columns=df.columns)
    
    max_wait = df["Wait Time (s)"].max() if "Wait Time (s)" in df and not df["Wait Time (s)"].empty else 1.0
    
    for idx in df.index:
        # 1. CPU Utilization (Higher is better -> Green, Lower -> Red)
        cpu = df.loc[idx, "CPU Util (%)"]
        if pd.notna(cpu):
            ratio = min(1.0, max(0.0, cpu / 100.0))
            red = int(255 * (1 - ratio))
            green = int(200 * ratio)
            styles.loc[idx, "CPU Util (%)"] = f"background-color: rgba({red}, {green}, 50, 0.35); font-weight: bold;"

        # 2. Wait Time (Lower is better -> Green, Higher -> Red)
        wait = df.loc[idx, "Wait Time (s)"]
        if pd.notna(wait) and max_wait > 0:
            ratio = min(1.0, max(0.0, wait / max_wait))
            red = int(255 * ratio)
            green = int(200 * (1 - ratio))
            styles.loc[idx, "Wait Time (s)"] = f"background-color: rgba({red}, {green}, 50, 0.35); font-weight: bold;"

    return styles

def main():
    print("Searching for VTune files...")
    df = collect_vtune_data(LOG_DIR)
    
    if df.empty:
        print("[WARNING] No '*_vtune_run.log' files found with valid data.")
        return

    # Compact terminal output
    print("\n========================= VTUNE SUMMARY =========================")
    print(df.to_string(index=False))
    print("=================================================================\n")

    # Pure HTML/CSS output generation
    try:
        format_dict = {
            "Total Time (ms)": "{:.2f}",
            "CPU Util (%)": "{:.1f}%",
            "CPUs Active": "{:.2f}",
            "Threads": "{:.0f}",
            "Wait Time (s)": "{:.3f}",
            "Spin Time (s)": "{:.3f}"
        }

        styled_df = df.style.apply(generate_custom_html_style, axis=None)\
                            .format(format_dict, na_rep="-")\
                            .set_caption("VTune Profiling Report - Threading Efficiency and Load Balancing Analysis")\
                            .set_table_styles([
                                {'selector': 'table', 'props': [('border-collapse', 'collapse'), ('font-family', 'Arial, sans-serif'), ('width', '100%')]},
                                {'selector': 'th', 'props': [('background-color', '#263238'), ('color', 'white'), ('padding', '10px'), ('text-align', 'center'), ('border', '1px solid #455a64')]},
                                {'selector': 'td', 'props': [('padding', '8px'), ('text-align', 'center'), ('border', '1px solid #cfd8dc'), ('color', 'black')]},
                                {'selector': 'caption', 'props': [('font-size', '18px'), ('font-weight', 'bold'), ('margin-bottom', '15px')]}
                            ])

        styled_df.to_html(HTML_OUTPUT)
        print(f"[SUCCESS] HTML VTune report generated at: '{HTML_OUTPUT}'")
    except AttributeError:
        print("[WARNING] Cannot generate HTML. Please check Jinja2 installation.")

if __name__ == "__main__":
    main()
