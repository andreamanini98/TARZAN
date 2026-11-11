# TARZAN Artifact

This artifact reproduces the experimental evaluation presented in the paper.

## Requirements

- **CPU**: x86_64 or ARM64 multi-core processor (paper results were measured on an Intel i9-13900H, x86_64).
- **RAM**: minimum 16 GB (32 GB recommended).
- **Disk**: ~10 GB free.
- **Software**: Docker (with BuildKit).

> Timings in the paper were measured natively on x86_64. Results obtained on ARM or under emulation (QEMU) are not representative.

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

The tables are produced by `scripts/analyze.py`. To regenerate or customize them for a model without rerunning the benchmarks:

```bash
python3 scripts/parse_benchmark.py <model_name> -f both
python3 scripts/parse_benchmark.py <model_name> -c seq omp tbb -f latex -o my_report
```

The script also prints a summary to the terminal, with the winner and speedup for each test.

### Reference tables from the paper

The folder `paper_output_logs/` contains exactly the reference tables reported in the paper. After the full reproduction, compare `output_logs/<model_name>/benchmark_summary.*` with the corresponding files in `paper_output_logs/`.

### Browsing the reference data inside the image

`paper_output_logs/` also contains per-iteration metrics (`metrics_*.csv`) and OMP vs TBB comparison plots (`*.png`). They are shipped inside the image, so no host checkout is needed.

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

## Compile-time configuration flags

Some behaviors are controlled by C++ macros defined in `TARZAN/regions/RTSArena.cpp`:

- `PARALLEL_THRESHOLD`: minimum region batch size ($N$) required to trigger parallel loop execution.
- `DEBUG_MEMORY`: enables fine-grained metric logging (active threads, queue cycles, exact memory sizes per `piFilter` invocation). When enabled, a CSV file with execution metrics is generated for every test run.

`PARALLEL_THRESHOLD` is a CMake cache variable (default `0`), so it can be set without editing the source:

```bash
# At image build time
docker build -f artifact/Dockerfile --build-arg PARALLEL_THRESHOLD=50 -t tarzan-artifact .

# Inside an existing image: rebuild <threshold> [jobs] [mode [flags ...]]
docker run --rm -v $(pwd)/output_logs:/tarzan/output_logs tarzan-artifact rebuild 50 8 full -t thr50
```

> [!IMPORTANT]
> **Always pass a tag (`-t`) when running with a non-default threshold.** Without a tag, logs are written as `<test>_seq.log`, `<test>_omp.log`, `<test>_tbb.log` and **overwrite** the results of any earlier run in `output_logs/`. With `-t thr50` they are saved as `<test>_omp_thr50.log`, etc., and appear as separate columns (`omp_thr50`, `tbb_thr50`, ...) in the summary tables, next to the untagged baseline.

The `rebuild` command recompiles the executables (jobs default to `nproc`) and then runs the given mode, if any. With `--rm` the rebuilt binaries are discarded when the container exits, so chain the mode in the same command, or drop `--rm` and `docker commit` the container to keep them.

Changing `DEBUG_MEMORY` requires rebuilding the image (Option B). The Boost layer is cached and is not recompiled.

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
