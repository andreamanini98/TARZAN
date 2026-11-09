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

### Option A: Pre-built image (recommended)

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

### Option B: Build from source

Run from the **project root** (the Dockerfile is in `artifact/`, but the build context is the root):

```bash
docker build -f artifact/Dockerfile -t tarzan-artifact .
```

The image is based on Ubuntu 24.04 and compiles Boost 1.90.0 from source, so the first build is slow (~15-30 min). Later builds reuse the cached Boost layer.

On machines with 16 GB of RAM, limit parallel jobs to avoid linker out-of-memory errors:

```bash
docker build -f artifact/Dockerfile --build-arg BUILD_JOBS=15 -t tarzan-artifact .
```

---

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

---

## Compile-time configuration flags

Some behaviors are controlled by C++ macros defined in `TARZAN/regions/RTSArena.cpp`:

- `PARALLEL_THRESHOLD`: minimum region batch size ($N$) required to trigger parallel loop execution.
- `DEBUG_MEMORY`: enables fine-grained metric logging (active threads, queue cycles, exact memory sizes per `piFilter` invocation). When enabled, a CSV file with execution metrics is generated for every test run.

Changing these macros requires rebuilding the image (Option B). The Boost layer is cached and is not recompiled.

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