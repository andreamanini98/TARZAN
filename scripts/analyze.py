import os
import re
import numpy as np
import pandas as pd

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.abspath(os.path.join(SCRIPT_DIR, ".."))

LOG_DIR = os.path.join(PROJECT_ROOT, "output_logs")
HTML_OUTPUT = os.path.join(PROJECT_ROOT, "benchmark_summary.html")

def parse_log_file(filepath):
    times = []
    # Espressione regolare per catturare interi, decimali e notazione scientifica (es. 6.44863e+07)
    pattern = re.compile(r"Total time including region generation:\s*([0-9\.eE\+\-]+)\s*microseconds")
    
    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
        for line in f:
            match = pattern.search(line)
            if match:
                times.append(float(match.group(1)))
                
    if not times:
        return None
    
    # Calcola il tempo medio in MILLISECONDI (ms)
    mean_time_ms = np.mean(times) / 1000.0
    return round(mean_time_ms, 2)

def collect_benchmark_data(log_dir):
    data = {}
    
    if not os.path.exists(log_dir):
        print(f"[ERROR] Directory dei log non trovata: '{log_dir}'")
        return None

    # Prefissi di modalità conosciuti
    known_modes = ["seq", "omp", "tbb"]

    # Esplorazione ricorsiva per supportare la struttura a sottocartelle
    for root, dirs, files in os.walk(log_dir):
        # Ignora le cartelle nascoste (come .vtune_...) per velocizzare la ricerca
        dirs[:] = [d for d in dirs if not d.startswith('.')]
        
        for filename in files:
            # Ignora i file che non sono log/txt o che sono report di VTune
            if not (filename.endswith(".log") or filename.endswith(".txt")):
                continue
            if filename.endswith("vtune_run.log"):
                continue
                
            filepath = os.path.join(root, filename)
            base_name = os.path.splitext(filename)[0]
            
            mode_found = None
            test_name = None
            
            # Cerca corrispondenze con i prefissi base o varianti taggate (es. _omp_dynamic)
            for m in known_modes:
                # Cerca pattern tipo: _omp oppure _omp_QUALSIASI_TAG
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

    # Costruiamo il DataFrame usando i test come righe e le modalità (inclusi i tag) come colonne
    df = pd.DataFrame(data)
    
    # Ordiniamo le colonne: prima le modalità standard (seq, omp, tbb), poi i vari tag
    standard_cols = [c for c in ["seq", "omp", "tbb"] if c in df.columns]
    tagged_cols = sorted([c for c in df.columns if c not in standard_cols])
    ordered_cols = standard_cols + tagged_cols
    df = df[ordered_cols]

    # Ordinamento alfabetico rigoroso (case-insensitive) delle righe (i Test)
    df.sort_index(key=lambda x: x.str.lower(), inplace=True)

    return df

def print_terminal_summary(df):
    """
    Stampa una tabella formattata e dinamica nel terminale.
    """
    cols = list(df.columns)
    
    # Calcolo dinamicamente le larghezze delle colonne
    col_widths = {col: max(len(col) + 5, 12) for col in cols}
    header_str = f"{'TEST NAME':<40} | " + " | ".join([f"{col.upper() + ' (ms)':<{col_widths[col]}}" for col in cols]) + " | WINNER   | SPEEDUP "
    
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
        
        # Determina il vincitore
        valid_vals = row.dropna()
        if len(valid_vals) > 0:
            winner = valid_vals.idxmin().upper()
            best_time = valid_vals.min()
            
            # Calcola lo speedup rispetto a SEQ (se disponibile), altrimenti rispetto al peggiore
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

def main():
    df = collect_benchmark_data(LOG_DIR)
    
    if df is None or df.empty:
        print("[WARNING] Nessun dato estratto dai file di log.")
        return

    # 1. Output Testuale a Tabella Leggibile
    print_terminal_summary(df)

    # 2. Output HTML
    try:
        styled_df = df.style.apply(generate_color_gradient, axis=None)\
                            .format("{:.2f} ms", na_rep="-")\
                            .set_caption("Benchmark TARZAN (Tempi medi in ms — Verde più scuro = Vittoria più netta)")\
                            .set_table_styles([
                                {'selector': 'table', 'props': [('border-collapse', 'collapse'), ('font-family', 'Arial, sans-serif'), ('width', '100%')]},
                                {'selector': 'th', 'props': [('background-color', '#263238'), ('color', 'white'), ('padding', '12px'), ('text-align', 'center'), ('border', '1px solid #455a64')]},
                                {'selector': 'td', 'props': [('padding', '10px'), ('text-align', 'center'), ('border', '1px solid #cfd8dc')]},
                                {'selector': 'caption', 'props': [('font-size', '16px'), ('font-weight', 'bold'), ('margin-bottom', '12px')]}
                            ])

        styled_df.to_html(HTML_OUTPUT)
        print(f"[SUCCESS] Report HTML generato in: '{HTML_OUTPUT}'")
    except AttributeError as e:
        print("\n[WARNING] Generazione HTML fallita.")
        print("Per visualizzare il report HTML a colori, installa jinja2 eseguendo:")
        print("pip install jinja2")

if __name__ == "__main__":
    main()
