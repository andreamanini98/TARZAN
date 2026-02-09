#!/usr/bin/env python3
"""
Generate LaTeX tables from benchmark summary file.

This script parses the benchmark_summary.txt file and generates LaTeX tables
for each experiment, combining timing, memory, and verification results.
It also compiles the tables into a PDF for easy viewing.
"""

import re
import sys
import subprocess
from pathlib import Path
from typing import Dict, List, Optional


class BenchmarkResult:
    """Store results for a single tool on a single sub-experiment."""

    def __init__(self):
        # TARZAN fields
        self.num_regions: Optional[str] = None
        self.total_time: Optional[str] = None
        self.goal_status: Optional[str] = None
        self.peak_memory: Optional[str] = None
        self.exec_time: Optional[str] = None

        # TChecker fields
        self.stored_states: Optional[str] = None
        self.memory_mb: Optional[str] = None
        self.reachable: Optional[str] = None
        self.running_time: Optional[str] = None
        self.exec_time_sec: Optional[str] = None

        # UPPAAL fields
        self.formula_result: Optional[str] = None
        self.states_explored: Optional[str] = None
        self.cpu_time_s: Optional[str] = None
        self.resident_memory: Optional[str] = None
        self.uppaal_exec_time: Optional[str] = None

        # Common fields
        self.timeout: bool = False
        self.failed: bool = False


class SubExperiment:
    """Store results for all tools on a single sub-experiment."""

    def __init__(self, name: str):
        self.name = name
        self.tarzan = BenchmarkResult()
        self.tchecker = BenchmarkResult()
        self.uppaal = BenchmarkResult()


class Experiment:
    """Store all sub-experiments for an experiment."""

    def __init__(self, name: str):
        self.name = name
        self.sub_experiments: List[SubExperiment] = []


def parse_benchmark_summary(file_path: str) -> List[Experiment]:
    """Parse the benchmark summary file and extract all experiments."""
    experiments: List[Experiment] = []
    current_experiment: Optional[Experiment] = None
    current_sub_experiment: Optional[SubExperiment] = None
    current_tool: Optional[str] = None
    current_result: Optional[BenchmarkResult] = None

    with open(file_path, 'r') as f:
        for line in f:
            line = line.rstrip()

            # Check for experiment header
            if line.startswith("EXPERIMENT: "):
                exp_name = line.split("EXPERIMENT: ")[1]
                current_experiment = Experiment(exp_name)
                experiments.append(current_experiment)

            # Check for sub-experiment header
            elif line.startswith("SUB-EXPERIMENT: "):
                sub_exp_name = line.split("SUB-EXPERIMENT: ")[1]
                current_sub_experiment = SubExperiment(sub_exp_name)
                if current_experiment:
                    current_experiment.sub_experiments.append(current_sub_experiment)

            # Check for tool header
            elif line.startswith("--- TOOL: "):
                tool_name = line.split("--- TOOL: ")[1].strip(" -")
                current_tool = tool_name
                if current_sub_experiment:
                    if tool_name == "TARZAN":
                        current_result = current_sub_experiment.tarzan
                    elif tool_name == "TChecker":
                        current_result = current_sub_experiment.tchecker
                    elif tool_name == "UPPAAL":
                        current_result = current_sub_experiment.uppaal

            # Check for timeout
            elif "TIMEOUT" in line and current_result:
                current_result.timeout = True

            # Check for failed status (OOM or other errors)
            elif "Status:" in line and "FAILED" in line and current_result:
                current_result.failed = True

            # Parse data fields
            elif current_result and not current_result.timeout:
                # TARZAN fields
                if "Number of regions:" in line:
                    current_result.num_regions = line.split("Number of regions:")[1].strip()
                elif "Total time:" in line and "exec" not in line.lower():
                    time_str = line.split("Total time:")[1].strip()
                    # Extract just the numeric value in seconds
                    match = re.search(r'([\d.]+)\s*s', time_str)
                    if match:
                        current_result.total_time = match.group(1)
                elif "Total exec time:" in line and current_tool == "TARZAN":
                    exec_str = line.split("Total exec time:")[1].strip()
                    # Extract the time value (could be in ms or s format)
                    match = re.search(r'([\d.]+)\s*ms', exec_str)
                    if match:
                        current_result.exec_time = match.group(1)
                    else:
                        match = re.search(r'([\d.]+)\s*s', exec_str)
                        if match:
                            # Convert seconds to milliseconds
                            current_result.exec_time = str(float(match.group(1)) * 1000)
                elif "Goal status:" in line:
                    current_result.goal_status = line.split("Goal status:")[1].strip()
                elif "Peak memory:" in line and "bytes" not in line:
                    mem_str = line.split("Peak memory:")[1].strip()
                    # Extract just the MB value
                    match = re.search(r'([\d.]+)\s*MB', mem_str)
                    if match:
                        current_result.peak_memory = match.group(1)

                # TChecker fields
                elif "Visited states:" in line:
                    current_result.stored_states = line.split("Visited states:")[1].strip()
                elif "Memory (MB):" in line:
                    current_result.memory_mb = line.split("Memory (MB):")[1].strip()
                elif "Reachable:" in line:
                    current_result.reachable = line.split("Reachable:")[1].strip()
                elif "Running time (s):" in line:
                    current_result.running_time = line.split("Running time (s):")[1].strip()
                elif "Total exec time (s):" in line:
                    current_result.exec_time_sec = line.split("Total exec time (s):")[1].strip()

                # UPPAAL fields
                elif "Formula result:" in line:
                    result = line.split("Formula result:")[1].strip()
                    # Simplify the result
                    if "satisfied" in result.lower() and "not" not in result.lower():
                        current_result.formula_result = "SAT"
                    elif "not satisfied" in result.lower():
                        current_result.formula_result = "UNSAT"
                    else:
                        current_result.formula_result = result
                elif "States explored:" in line:
                    current_result.states_explored = line.split("States explored:")[1].strip()
                elif "CPU time (s):" in line:
                    current_result.cpu_time_s = line.split("CPU time (s):")[1].strip()
                elif "Total exec time:" in line and current_tool == "UPPAAL":
                    exec_str = line.split("Total exec time:")[1].strip()
                    # Extract the time value
                    match = re.search(r'([\d.]+)\s*ms', exec_str)
                    if match:
                        current_result.uppaal_exec_time = match.group(1)
                    else:
                        match = re.search(r'([\d.]+)\s*s', exec_str)
                        if match:
                            # Convert seconds to milliseconds
                            current_result.uppaal_exec_time = str(float(match.group(1)) * 1000)
                elif "Resident memory (MB):" in line:
                    current_result.resident_memory = line.split("Resident memory (MB):")[1].strip()

    return experiments


def format_value(value: Optional[str], default: str = "KO") -> str:
    """Format a value for LaTeX, handling None and N/A."""
    if value is None or value == "N/A":
        return default
    return value


def format_time_value(value: Optional[str], default: str = "KO") -> str:
    """Format a time value for LaTeX, showing epsilon for very small values and truncating to 3 decimal places."""
    if value is None or value == "N/A":
        return default

    try:
        time_val = float(value)

        # Truncate to 3 decimal places without rounding
        # Multiply by 1000, truncate to integer, divide by 1000
        import math
        truncated = math.floor(time_val * 1000) / 1000

        # If truncated value is 0.000 (or original was less than 0.001), show epsilon
        if truncated == 0.000:
            return r"$\epsilon$"

        return f"{truncated:.3f}"
    except ValueError:
        return default


def extract_n_from_instance(instance_name: str) -> str:
    """Extract N value from instance name (e.g., 'flower_02' -> '2') and adjust with k_adj.txt."""
    # Handle formats like "flower_02", "fischer_02", "latch", etc.
    instance_match = re.search(r'_0*(\d+)$', instance_name)
    if instance_match:
        n = int(instance_match.group(1))
    else:
        # For single instances without number (like "latch"), use "1"
        # or extract any trailing number
        num_match = re.search(r'\d+$', instance_name)
        if num_match:
            n = int(num_match.group(0))
        else:
            # Default to 1 if no number found
            n = 1

    # Read k_adj.txt from the instance directory (using relative path)
    k_adj_path = Path(__file__).parent / "../models" / instance_name / "k_adj.txt"
    try:
        with open(k_adj_path, 'r') as f:
            k_adj = int(f.read().strip())
        n += k_adj
    except (FileNotFoundError, ValueError):
        # If k_adj.txt doesn't exist or is invalid, just use n as is
        pass

    return str(n)


def generate_latex_table(experiment: Experiment) -> str:
    """Generate a LaTeX table for a single experiment."""

    # Start building the LaTeX code
    latex = []
    latex.append(r"\definecolor{lightgray}{gray}{0.95}")
    latex.append(r"\setlength{\aboverulesep}{1pt}")
    latex.append(r"\setlength{\belowrulesep}{1pt}")
    latex.append(r"{\renewcommand{\arraystretch}{0.95}")
    latex.append(r"{\setlength{\tabcolsep}{3.25pt}")
    latex.append(r"\begin{table}[t]")
    latex.append(r"\centering")
    latex.append(r"\caption{Benchmark results for \texttt{" + experiment.name.replace("_", r"\_") + r"}}")
    latex.append(r"\label{tab:" + experiment.name + r"}")
    latex.append(r"\resizebox{\textwidth}{!}{")
    latex.append(r"\begin{tabular}{%")
    latex.append(r"    c|rrrr|rrrr|rrrr")
    latex.append(r"}")
    latex.append(r"\toprule")

    # Header row 1: Tool names
    latex.append(r"\multirow{2}{*}{\textbf{K}}")
    latex.append(r"  & \multicolumn{4}{c|}{\textbf{TARZAN}}")
    latex.append(r"  & \multicolumn{4}{c|}{\textbf{TChecker}}")
    latex.append(r"  & \multicolumn{4}{c}{\textbf{UPPAAL}} \\")
    latex.append(r"%")

    # Header row 2: Column names
    latex.append(r"  & \multicolumn{1}{c}{\textbf{VT}}")
    latex.append(r"  & \multicolumn{1}{c}{\textbf{ET}}")
    latex.append(r"  & \multicolumn{1}{c}{\textbf{Mem}}")
    latex.append(r"  & \multicolumn{1}{c|}{\textbf{Regions}}")
    latex.append(r"  & \multicolumn{1}{c}{\textbf{VT}}")
    latex.append(r"  & \multicolumn{1}{c}{\textbf{ET}}")
    latex.append(r"  & \multicolumn{1}{c}{\textbf{Mem}}")
    latex.append(r"  & \multicolumn{1}{c|}{\textbf{States}}")
    latex.append(r"  & \multicolumn{1}{c}{\textbf{VT}}")
    latex.append(r"  & \multicolumn{1}{c}{\textbf{ET}}")
    latex.append(r"  & \multicolumn{1}{c}{\textbf{Mem}}")
    latex.append(r"  & \multicolumn{1}{c}{\textbf{States}} \\")
    latex.append(r"\midrule")

    # Data rows
    for row_idx, sub_exp in enumerate(experiment.sub_experiments):
        # Alternate row colors
        color = 'white' if row_idx % 2 == 0 else 'lightgray'
        latex.append(r"\rowcolor{" + color + r"}")

        # Extract N from instance name
        n = extract_n_from_instance(sub_exp.name)

        # Build row data
        tarzan_data = []
        tchecker_data = []
        uppaal_data = []

        # TARZAN columns
        if sub_exp.tarzan.timeout:
            # TARZAN timeout: TO in VT/ET, --- elsewhere
            tarzan_data = ["TO", "TO", "---", "---"]
        elif sub_exp.tarzan.failed:
            # TARZAN failure (OOM): --- in VT/ET, OOM in Mem, --- in Regions
            tarzan_data = ["---", "---", "OOM", "---"]
        else:
            tarzan_data.append(format_time_value(sub_exp.tarzan.total_time, "---"))
            # Convert exec time from ms to seconds if available
            exec_time_s = "---"
            if sub_exp.tarzan.exec_time:
                try:
                    # Convert from ms to seconds, then format
                    exec_time_s = format_time_value(str(float(sub_exp.tarzan.exec_time) / 1000), "---")
                except:
                    exec_time_s = "---"
            tarzan_data.append(exec_time_s)
            tarzan_data.append(format_value(sub_exp.tarzan.peak_memory, "---"))
            tarzan_data.append(format_value(sub_exp.tarzan.num_regions, "---"))

        # TChecker columns
        if sub_exp.tchecker.timeout:
            # TChecker timeout: TO in VT/ET, --- elsewhere
            tchecker_data = ["TO", "TO", "---", "---"]
        elif sub_exp.tchecker.failed:
            # TChecker failure (OOM): --- in VT/ET, OOM in Mem, --- in States
            tchecker_data = ["---", "---", "OOM", "---"]
        else:
            tchecker_data.append(format_time_value(sub_exp.tchecker.running_time, "---"))
            tchecker_data.append(format_time_value(sub_exp.tchecker.exec_time_sec, "---"))
            tchecker_data.append(format_value(sub_exp.tchecker.memory_mb, "---"))
            tchecker_data.append(format_value(sub_exp.tchecker.stored_states, "---"))

        # UPPAAL columns
        if sub_exp.uppaal.timeout:
            # UPPAAL timeout: TO in VT/ET, --- elsewhere
            uppaal_data = ["TO", "TO", "---", "---"]
        elif sub_exp.uppaal.failed:
            # UPPAAL failure (OOM): --- in VT/ET, OOM in Mem, --- in States
            uppaal_data = ["---", "---", "OOM", "---"]
        else:
            uppaal_data.append(format_time_value(sub_exp.uppaal.cpu_time_s, "---"))
            # Convert exec time from ms to seconds if available
            exec_time_s = "---"
            if sub_exp.uppaal.uppaal_exec_time:
                try:
                    # Convert from ms to seconds, then format
                    exec_time_s = format_time_value(str(float(sub_exp.uppaal.uppaal_exec_time) / 1000), "---")
                except:
                    exec_time_s = "---"
            uppaal_data.append(exec_time_s)
            uppaal_data.append(format_value(sub_exp.uppaal.resident_memory, "---"))
            uppaal_data.append(format_value(sub_exp.uppaal.states_explored, "---"))

        # Format the row
        latex.append(f"{n}  & {' & '.join(tarzan_data)}")
        latex.append(f"   & {' & '.join(tchecker_data)}")
        latex.append(f"   & {' & '.join(uppaal_data)} \\\\")

    # Table footer
    latex.append(r"\bottomrule")
    latex.append(r"\end{tabular}")
    latex.append(r"}")
    latex.append(r"\end{table}")
    latex.append(r"}")
    latex.append(r"}")
    latex.append("")

    return "\n".join(latex)


def generate_complete_latex_document(tables_content: str) -> str:
    """Generate a complete LaTeX document with all tables."""
    document = []
    document.append(r"\documentclass[11pt,a4paper]{article}")
    document.append(r"\usepackage[utf8]{inputenc}")
    document.append(r"\usepackage[T1]{fontenc}")
    document.append(r"\usepackage{graphicx}")
    document.append(r"\usepackage{multirow}")
    document.append(r"\usepackage{booktabs}")
    document.append(r"\usepackage{xcolor}")
    document.append(r"\usepackage{colortbl}")
    document.append(r"\usepackage{geometry}")
    document.append(r"\geometry{margin=1in}")
    document.append(r"\usepackage{longtable}")
    document.append(r"")
    document.append(r"\title{Benchmark Results Summary}")
    document.append(r"\author{TARZAN Benchmark Script}")
    document.append(r"\date{\today}")
    document.append(r"")
    document.append(r"\begin{document}")
    document.append(r"")
    document.append(r"\maketitle")
    document.append(r"")
    document.append(r"\section*{Legend}")
    document.append(r"\begin{itemize}")
    document.append(r"  \item \textbf{VT} = Verification Time (seconds)")
    document.append(r"  \item \textbf{ET} = Execution Time (seconds)")
    document.append(r"  \item \textbf{Mem} = Memory usage (MB)")
    document.append(r"  \item \textbf{---} = Not available")
    document.append(r"  \item \textbf{OOM} = Out of memory")
    document.append(r"  \item \textbf{TO} = Timeout")
    document.append(r"  \item $\epsilon$ = Value less than 0.001")
    document.append(r"\end{itemize}")
    document.append(r"")
    document.append(r"\clearpage")
    document.append(r"")
    document.append(tables_content)
    document.append(r"")
    document.append(r"\end{document}")

    return "\n".join(document)


def compile_latex_to_pdf(tex_file: Path, output_dir: Path) -> bool:
    """Compile a LaTeX file to PDF using pdflatex."""
    print(f"\nCompiling LaTeX to PDF...")

    # Check if pdflatex is available
    try:
        subprocess.run(["pdflatex", "--version"],
                      capture_output=True,
                      check=True)
    except (subprocess.CalledProcessError, FileNotFoundError):
        print("Warning: pdflatex not found. PDF compilation skipped.")
        print("Install a LaTeX distribution (e.g., TeX Live, MiKTeX) to enable PDF generation.")
        return False

    # Run pdflatex twice to resolve references
    for i in range(2):
        try:
            result = subprocess.run(
                ["pdflatex", "-interaction=nonstopmode", tex_file.name],
                cwd=output_dir,
                capture_output=True,
                text=True,
                timeout=60
            )

            if result.returncode != 0:
                print(f"Warning: pdflatex run {i+1} failed with return code {result.returncode}")
                if i == 0:
                    # Continue to second run even if first fails
                    continue
                else:
                    return False

        except subprocess.TimeoutExpired:
            print(f"Error: pdflatex compilation timed out")
            return False
        except Exception as e:
            print(f"Error during PDF compilation: {e}")
            return False

    # Clean up auxiliary files
    aux_extensions = ['.aux', '.log', '.out']
    for ext in aux_extensions:
        aux_file = output_dir / f"{tex_file.stem}{ext}"
        if aux_file.exists():
            aux_file.unlink()

    pdf_file = output_dir / f"{tex_file.stem}.pdf"
    if pdf_file.exists():
        print(f"PDF successfully generated: {pdf_file}")
        return True
    else:
        print("Warning: PDF file was not generated")
        return False


def main():
    """Main function to generate all LaTeX tables."""

    # Paths
    summary_file = "../../../output/benchmark_summary.txt"
    output_dir = Path("../../../output/latex_results")
    tables_file = output_dir / "benchmark_tables.tex"
    complete_doc_file = output_dir / "benchmark_results.tex"

    # Check if the summary file exists
    if not Path(summary_file).exists():
        print(f"Error: Summary file not found: {summary_file}")
        sys.exit(1)

    # Create output directory
    output_dir.mkdir(parents=True, exist_ok=True)
    print(f"Output directory: {output_dir}")

    print(f"\nParsing benchmark summary from: {summary_file}")
    experiments = parse_benchmark_summary(summary_file)
    print(f"Found {len(experiments)} experiments")

    # Generate LaTeX tables
    latex_tables = []
    latex_tables.append("% LaTeX tables generated from benchmark results")
    latex_tables.append("% Generated by generate_latex_tables.py")
    latex_tables.append("")
    latex_tables.append("% Legend:")
    latex_tables.append("% VT = Verification Time (seconds)")
    latex_tables.append("% ET = Execution Time (seconds)")
    latex_tables.append("% Mem = Memory usage MaxRSS (MB)")
    latex_tables.append("% --- = Not available")
    latex_tables.append("% OOM = Out of memory")
    latex_tables.append("% TO = Timeout")
    latex_tables.append("% \\epsilon = Value less than 0.001")
    latex_tables.append("")

    for experiment in experiments:
        print(f"  Generating table for: {experiment.name} ({len(experiment.sub_experiments)} instances)")
        latex_tables.append(generate_latex_table(experiment))

    tables_content = "\n".join(latex_tables)

    # Write tables-only file
    with open(tables_file, 'w') as f:
        f.write(tables_content)
    print(f"\nLaTeX tables written to: {tables_file}")

    # Generate complete document
    complete_document = generate_complete_latex_document(tables_content)
    with open(complete_doc_file, 'w') as f:
        f.write(complete_document)
    print(f"Complete LaTeX document written to: {complete_doc_file}")

    print(f"Total tables generated: {len(experiments)}")

    # Compile to PDF
    pdf_success = compile_latex_to_pdf(complete_doc_file, output_dir)

    if pdf_success:
        print(f"\n{'='*60}")
        print("SUCCESS: All files generated successfully!")
        print(f"{'='*60}")
        print(f"Tables only: {tables_file}")
        print(f"Full document: {complete_doc_file}")
        print(f"PDF: {output_dir / 'benchmark_results.pdf'}")
    else:
        print(f"\n{'='*60}")
        print("LaTeX files generated successfully (PDF compilation failed or skipped)")
        print(f"{'='*60}")
        print(f"Tables only: {tables_file}")
        print(f"Full document: {complete_doc_file}")
        print("\nYou can manually compile the PDF using:")
        print(f"  cd {output_dir} && pdflatex benchmark_results.tex")


if __name__ == "__main__":
    main()
