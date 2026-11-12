# TARZAN Artifact

This artifact reproduces the experimental evaluation presented in the paper.

## Requirements

- **CPU**: x86_64 or ARM64 multi-core processor (paper results were measured on an Intel i9-13900H, x86_64).
- **RAM**: minimum 16 GB (32 GB recommended).
- **Disk**: ~10 GB free.
- **Software**: Docker (with BuildKit).

> [!NOTE]
> Timings in the paper were measured natively on x86_64. Results obtained on ARM or under emulation (QEMU) are not representative.
> Notably, the performance of parallel runtimes may vary depending on their corresponding version, as experienced in our own tests when building the images.
> However, the results and the claims of the paper remain unaffected.

---

## Step 0: Get the Docker image

### Pre-built image

Two archives are provided, one per architecture:

| Architecture | File | Loaded as |
|---|---|---|
| x86_64 | `tarzan-artifact-x86_64.tar.gz` | `tarzan-artifact:amd64` |
| ARM64 (e.g. Apple Silicon) | `tarzan-artifact-arm64.tar.gz` | `tarzan-artifact:arm64` |

Load the archive matching your machine and tag it as `tarzan-artifact`:

```bash
# x86_64
docker load -i tarzan-artifact-x86_64.tar.gz
docker tag tarzan-artifact:amd64 tarzan-artifact:latest

# ARM64
docker load -i tarzan-artifact-arm64.tar.gz
docker tag tarzan-artifact:arm64 tarzan-artifact:latest
```

> [!NOTE]
> **macOS users:** Safari may automatically decompress the `.gz` archive after download, leaving a file named `tarzan-artifact-arm64.tar`. In that case, drop the `.gz` from the command: `docker load -i tarzan-artifact-arm64.tar`. `docker load` accepts both compressed and uncompressed archives.

All following commands use the `tarzan-artifact` name.


## Step 1: Kick-the-tires (~1 minute)

Verify that the container runs correctly:

```bash
docker run --rm tarzan-artifact kick-the-tires
```

## Step 2: Full reproduction (~60-240 minutes)

Generate the complete set of tables and metrics. The output directory is mounted so results are preserved on the host:

```bash
docker run --rm -v $(pwd)/output_logs:/tarzan/output_logs tarzan-artifact full
```

LaTeX and HTML tables and aggregated summary reports are saved in `./output_logs/`.

### Passing extra flags to `kick-the-tires` and `full`

Any flag written after the mode is forwarded to `entrypoint.sh` for every run (see [Advanced usage](#advanced-usage) for the full list). Flags given here override the defaults of the mode, e.g. `-r 3` replaces the `-r 10` of `full`:

```bash
docker run --rm -v $(pwd)/output_logs:/tarzan/output_logs tarzan-artifact full -t mytag -r 3
```

---

## Container modes

The image entrypoint is `artifact/run_artifact.sh`. The first argument after the image name selects the mode:

```text
docker run [docker options] tarzan-artifact [mode] [args ...]
```

| Mode | Syntax | What it does |
|---|---|---|
| `kick-the-tires` | `kick-the-tires [flags ...]` | Quick check (default when no mode is given) |
| `full` | `full [flags ...]` | Full reproduction of all models |
| `paper-logs` | `paper-logs [file]` | Browse the reference data shipped in the image |
| `rebuild` | `rebuild <threshold> [jobs] [mode [args ...]]` | Recompile with a different `PARALLEL_THRESHOLD`, then optionally run a mode |
| _direct_ | `<--seq\|--omp\|--tbb> <path> [flags ...]` | Call `entrypoint.sh` directly |

### `kick-the-tires`

Runs `seq`, `omp` and `tbb` once on `autoclave_sterilization_system`, then generates its summary table. Default flags: `-f -i`.

```bash
docker run --rm tarzan-artifact kick-the-tires
docker run --rm tarzan-artifact            # same thing
```

Mount `output_logs/` (as in Step 2) if you want to keep the logs.

### `full`

Runs `seq`, `omp` and `tbb` on every model (see [Models](#models)), then generates one summary table per model. Default flags: `-f -i -r 10 --min-sleep 1 --max-sleep 10`.

```bash
docker run --rm -v $(pwd)/output_logs:/tarzan/output_logs tarzan-artifact full
```

For both `kick-the-tires` and `full`, any flag after the mode is appended to the defaults and overrides them (e.g. `full -r 3 -t mytag`). See [Advanced usage](#advanced-usage) for the list of flags.

### `paper-logs`

Reads `paper_output_logs/` inside the image, without running anything.

```bash
# List every CSV/PNG file
docker run --rm tarzan-artifact paper-logs

# Print a CSV as an aligned table (path relative to paper_output_logs/)
docker run --rm tarzan-artifact paper-logs med_app/drug_always_expire_lose/metrics_drug_always_expire_lose_omp.csv

# Print any other text file (e.g. a LaTeX table)
docker run --rm tarzan-artifact paper-logs med_app/results.tex
```

To open images, copy the folder to the host as shown in [Browsing the reference data](#browsing-the-reference-data-inside-the-image).

### `rebuild`

Recompiles the executables with a new `PARALLEL_THRESHOLD` and then runs the given mode, if any.

- `<threshold>`: non-negative integer, the minimum region batch size that triggers parallel execution.
- `[jobs]`: number of parallel compile jobs (default: `nproc`). Lower it if the linker runs out of memory.
- `[mode [args ...]]`: any of the modes above, with its flags.

```bash
# Threshold 50, 8 compile jobs, then the full reproduction tagged as thr50
docker run --rm -v $(pwd)/output_logs:/tarzan/output_logs tarzan-artifact rebuild 50 8 full -t thr50

# Threshold 50, default jobs, then kick-the-tires
docker run --rm tarzan-artifact rebuild 50 kick-the-tires
```

With `--rm` the rebuilt binaries are lost when the container exits, so chain the mode in the same command. Always pass a tag (`-t`) so the results do not overwrite the baseline; see [Compile-time configuration](#compile-time-configuration-parallel_threshold).

### Direct call to `entrypoint.sh`

Any first argument that is not one of the modes above is passed straight to `/tarzan/entrypoint.sh`. Use this to run one variant on one model or test folder:

```bash
docker run --rm -v $(pwd)/output_logs:/tarzan/output_logs tarzan-artifact \
  --tbb executables/games_executables/production_cell -f -i -r 5
```

No default flags are added in this mode, so pass `-f` to save the logs. No summary table is generated either; build it afterwards with `scripts/analyze.py` (see [Running the tools without `run_artifact.sh`](#running-the-tools-without-run_artifactsh)).

---

## Models

The benchmarked models live in `executables/games_executables/` inside the image. Use these names as `<model_name>` in the commands of this README:

| Model name | Run by |
|---|---|
| `autoclave_sterilization_system` | `kick-the-tires`, `full` |
| `synth_reactor` | `full` |
| `production_cell` | `full` |
| `med_app` | `full` |

To run a single model instead of all of them, call the entrypoint directly with a variant (`--seq`, `--omp` or `--tbb`) and the model path:

```bash
docker run --rm -v $(pwd)/output_logs:/tarzan/output_logs tarzan-artifact \
  --omp executables/games_executables/med_app -f -i -r 10
```

Repeat it for each variant you need, then regenerate the table with `scripts/analyze.py` (see [Running the tools without `run_artifact.sh`](#running-the-tools-without-run_artifactsh)).

---

## Reproduced tables

The full reproduction (Step 2) generates **one table per model**. Each row is a test of the model, and the columns report the average execution time of each implementation:

| Column | Description |
|---|---|
| `seq` | Sequential baseline (no OpenMP, no TBB) |
| `omp` | Parallel version based on OpenMP |
| `tbb` | Parallel version based on Intel TBB |

If extra variants were run with a tag (`-t`, e.g. `omp_dynamic`, `tbb_treshold50`), they appear as additional columns after `seq`, `omp` and `tbb`.

Tables are written inside each model's log folder, in both HTML (quick inspection) and LaTeX (paper-ready, `booktabs`):

```text
output_logs/<model_name>/
├── <base_name>/<log_name>.log     # raw log of each run
├── benchmark_summary.html         # styled table
└── benchmark_summary.tex          # LaTeX table
```

The tables are produced by `scripts/analyze.py`. To regenerate or customize them for a model without rerunning the benchmarks (inside the container, see [Running the tools without `run_artifact.sh`](#running-the-tools-without-run_artifactsh)):

```bash
python3 scripts/analyze.py <model_name> -f both
python3 scripts/analyze.py <model_name> -c seq omp tbb -f latex -o my_report
```

The script also prints a summary to the terminal, with the winner and speedup for each test.

### Reference tables from the paper

The folder `paper_output_logs/` contains exactly the reference tables reported in the paper. After the full reproduction, compare `output_logs/<model_name>/benchmark_summary.*` with the corresponding files in `paper_output_logs/`.

How each table of the paper relates to the artifact:

| Paper table | Source in `paper_output_logs/` | How to reproduce |
|---|---|---|
| Table 2 (execution times) | `<model_name>/results.tex` of every model, combined into one table | `full` (one `benchmark_summary.*` per model) |
| Table 3 (threshold comparison) | — | `rebuild` with each threshold, see [Compile-time configuration](#compile-time-configuration-parallel_threshold) |
| Table 4 (per-iteration metrics) | `<model_name>/<test>/metrics_<test>_<variant>.csv` | Not reproducible with the artifact build: precomputed only (see below) |

### Browsing the reference data inside the image

`paper_output_logs/` also contains per-iteration metrics (`metrics_*.csv`) and OMP vs TBB comparison plots (`*.png`). They are shipped inside the image, so no host checkout is needed.

These metrics were produced with the internal `DEBUG_MEMORY` build flag, which logs fine-grained data for every `piFilter` invocation (active threads, queue cycles, exact memory sizes). The flag is disabled in the artifact build because it slows execution; all the data it generates for the paper's experiments is already precomputed in `paper_output_logs/` and can be browsed as shown below.

Table 4 of the paper reports the data of one `production_cell` test, taken from these CSV files (`paper_output_logs/production_cell/<test>/metrics_<test>_<variant>.csv`).

**In the terminal** (CSV and text files):

```bash
# List all CSV/PNG files
docker run --rm tarzan-artifact paper-logs

# Print a CSV as an aligned table (path relative to paper_output_logs/)
docker run --rm tarzan-artifact paper-logs med_app/drug_always_expire_lose/metrics_drug_always_expire_lose_omp.csv
```

**With your own programs** (images, spreadsheets): copy the folder from the image to the host, then open the files with any viewer:

```bash
docker create --name tarzan-tmp tarzan-artifact
docker cp tarzan-tmp:/tarzan/paper_output_logs ./paper_output_logs
docker rm tarzan-tmp

xdg-open paper_output_logs/med_app/patient_doc_in_pulse_win/batch_timeline_plot.png # Linux
open paper_output_logs/med_app/patient_doc_in_pulse_win/batch_timeline_plot.png  # macOS
```

`docker cp` creates the files as your user, so no `sudo` is needed to edit or delete them.

## Compile-time configuration: `PARALLEL_THRESHOLD`

`PARALLEL_THRESHOLD` (C++ macro in `TARZAN/regions/RTSArena.cpp`) is the minimum region batch size ($N$) required to trigger parallel loop execution. It is a CMake cache variable (default `0`), so it can be set without editing the source:

```bash
# At image build time
docker build -f artifact/Dockerfile --build-arg PARALLEL_THRESHOLD=50 -t tarzan-artifact .

# Inside an existing image: rebuild <threshold> [jobs] [mode [flags ...]]
docker run --rm -v $(pwd)/output_logs:/tarzan/output_logs tarzan-artifact rebuild 50 8 full -t thr50
```

> [!IMPORTANT]
> **Always pass a tag (`-t`) when running with a non-default threshold.** Without a tag, logs are written as `<test>_seq.log`, `<test>_omp.log`, `<test>_tbb.log` and **overwrite** the results of any earlier run in `output_logs/`. With `-t thr50` they are saved as `<test>_omp_thr50.log`, etc., and appear as separate columns (`omp_thr50`, `tbb_thr50`, ...) in the summary tables, next to the untagged baseline.

The `rebuild` command recompiles the executables (jobs default to `nproc`) and then runs the given mode, if any. With `--rm` the rebuilt binaries are discarded when the container exits, so chain the mode in the same command, or drop `--rm` and `docker commit` the container to keep them.

### Table 3 of the paper

Table 3 compares the parallel versions under different values of `PARALLEL_THRESHOLD`. To obtain it, recompile with each threshold and rerun `full` with a different tag into the same `output_logs/` folder: each tag becomes a separate column of the model's `benchmark_summary.*`, next to the untagged baseline.

```bash
for thr in 100 200 400; do
  docker run --rm -v $(pwd)/output_logs:/tarzan/output_logs tarzan-artifact rebuild $thr full -t thr$thr
done
```

---

## Advanced usage

`artifact/run_artifact.sh` wraps the root entrypoint `/tarzan/entrypoint.sh`, which supports the following options:

```text
Usage: /tarzan/entrypoint.sh <--seq|--omp|--tbb> <file_or_folder_path> [options]

Modes:
  --seq                Sequential version (no OpenMP, no TBB)
  --omp                OpenMP version
  --tbb                TBB version

Benchmark options:
  -r, --repeats NUM    Repeat each test NUM times (default: 1)
  -s, --sleep SEC      Fixed sleep time in seconds (default: 0)
  --min-sleep SEC      Lower bound for dynamic cooldown based on run time
  --max-sleep SEC      Upper bound for dynamic cooldown based on run time
  -f, --file-output    Save outputs to output_logs/<model_name>/<base_name>/<log_name>.[EXT]
  -t, --tag STR        Append a tag to the log name (e.g. 'dynamic' -> _omp_dynamic.log)
  -i, --ignore-errors  Continue batch execution on exception/crash
  -k, --skip-existing  Skip tests that already have an output log
```

### Running the tools without `run_artifact.sh`

The image entrypoint is `run_artifact.sh`, but every tool in the image can be called directly by overriding it with Docker's `--entrypoint` flag. The working directory is `/tarzan`, so the relative paths of this README work as is. Mount `output_logs/` so that logs and tables are kept on the host.

**`entrypoint.sh`** (one variant on one model or test folder):

```bash
docker run --rm -v $(pwd)/output_logs:/tarzan/output_logs \
  --entrypoint /tarzan/entrypoint.sh tarzan-artifact \
  --omp executables/games_executables/med_app -f -i -r 10

# Show all options
docker run --rm --entrypoint /tarzan/entrypoint.sh tarzan-artifact --help
```

**`scripts/analyze.py`** (rebuild the tables from the logs already in `output_logs/`, without rerunning the benchmarks):

```bash
docker run --rm -v $(pwd)/output_logs:/tarzan/output_logs \
  --entrypoint python3 tarzan-artifact \
  scripts/analyze.py med_app -f both

docker run --rm -v $(pwd)/output_logs:/tarzan/output_logs \
  --entrypoint python3 tarzan-artifact \
  scripts/analyze.py med_app -c seq omp tbb -f latex -o my_report
```

**`artifact/rebuild.sh`** (recompile only, see [Compile-time configuration](#compile-time-configuration-parallel_threshold)). With `--rm` the new binaries are discarded on exit, so use it from the interactive shell below.

**Interactive shell**, to chain several commands in the same container (for example, rebuild and then run without losing the new binaries):

```bash
docker run --rm -it -v $(pwd)/output_logs:/tarzan/output_logs \
  --entrypoint bash tarzan-artifact

# then, inside the container:
artifact/rebuild.sh 50 8
./entrypoint.sh --tbb executables/games_executables/production_cell -f -i -r 5 -t thr50
python3 scripts/analyze.py production_cell -f both
exit
```

### Editing `run_artifact.sh` without rebuilding

Bind-mount the script at runtime:

```bash
docker run --rm \
  -v $(pwd)/artifact/run_artifact.sh:/tarzan/artifact/run_artifact.sh \
  -v $(pwd)/output_logs:/tarzan/output_logs \
  tarzan-artifact full
```

---

## Troubleshooting

- **Linker out-of-memory during build**: lower `--build-arg BUILD_JOBS=N` (e.g. 8).
- **`exec format error`**: the loaded image does not match your CPU architecture; use the archive for your machine.
- **`Unable to find image 'tarzan-artifact:latest'`**: you skipped the `docker tag` step after `docker load`.
