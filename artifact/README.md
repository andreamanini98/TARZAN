# TARZAN Artifact

This artifact reproduces the experimental evaluation presented in the paper.

## Hardware Requirements
- **CPU**: x86_64 multi-core processor (benchmarked on Intel i9-13900H).
- **RAM**: Minimum 16 GB (32 GB recommended).
  - *Note on RAM constraints:* On systems with 16 GB of RAM and high logical thread counts (e.g., 20 threads), it is recommended to limit the Docker build parallel jobs by passing `--build-arg BUILD_JOBS=15` to prevent C++ linker out-of-memory errors during compilation.

---

## Step 1: Kick-the-tires Phase (~2 minutes)
To verify that the container builds and runs properly:

```bash
# Build the Docker image (limiting to 15 build jobs if running on 16GB RAM)
sudo docker build -f artifact/Dockerfile --build-arg BUILD_JOBS=15 -t tarzan-artifact . 

# Run quick verification
sudo docker run --rm tarzan-artifact kick-the-tires
```

---

## Step 2: Full Reproduction (~30-60 minutes)
To generate the complete set of experimental tables and metrics, mounting local volumes to preserve output logs on the host machine:

```bash
sudo docker run --rm -v $(pwd)/output_logs:/tarzan/output_logs tarzan-artifact full
```

The generated LaTeX and HTML tables and aggregated summary reports will be saved directly into `./output_logs/`.

---

## Compile-time Configuration Flags

Certain core behaviors and optimization parameters of TARZAN are controlled via C++ preprocessor macros defined at compile time in the `TARZAN/regions/RTSArena.cpp` file:

- `PARALLEL_THRESHOLD`: Defines the minimum region batch size ($N$) required to trigger parallel loop execution.
- `DEBUG_MEMORY`: Enables fine-grained step-by-step metric logging (active thread counts, queue cycles, and exact memory byte sizes per `piFilter` invocation). When enabled, a CSV file detailing execution metrics will be generated for every test run.

> **Note on Re-compilation:**  
> Modifying any of these compile-time macros in the source code or via CMake arguments requires rebuilding the C++ executable. You can trigger a rebuild inside the container:
> 
> ```bash
> sudo docker build -f artifact/Dockerfile -t tarzan-artifact . 
> ```

---

## Advanced Usage & Customizing `run_artifact.sh`

The coordinator wrapper script `artifact/run_artifact.sh` wraps the root entrypoint (`/tarzan/entrypoint.sh`), which supports various execution modes and options.

### Original Entrypoint (`entrypoint.sh`) Options
```text
Usage: /tarzan/entrypoint.sh <--seq|--omp|--tbb> <file_or_folder_path> [options]

Modes:
  --seq                Runs Sequential version (No OpenMP, No TBB)
  --omp                Runs OpenMP version
  --tbb                Runs TBB version

Benchmark Options:
  -r, --repeats NUM    Number of times to repeat each test (default: 1)
  -s, --sleep SEC      Fixed sleep time in seconds (default: 0)
  --min-sleep SEC      Lower bound for dynamic cooldown based on run time
  --max-sleep SEC      Upper bound for dynamic cooldown based on run time
  -f, --file-output    Save outputs to 'output_logs/<model_name>/<base_name>/<log_name>.[EXT]'
  -t, --tag STR        Append a tag to log name (e.g., 'dynamic' -> _omp_dynamic.log)
  -i, --ignore-errors  Continue batch execution on exception/crash
  -k, --skip-existing  Skip tests that already have an output log
```

### Hot-reloading `run_artifact.sh` (Without Rebuilding)
If you wish to modify execution logic in `run_artifact.sh` (such as tweaking benchmark parameters or mode execution), you can test changes instantly by bind-mounting the script at runtime without rebuilding the Docker container:

```bash
sudo docker run --rm \
  -v $(pwd)/artifact/run_artifact.sh:/tarzan/artifact/run_artifact.sh \
  -v $(pwd)/output_logs:/tarzan/output_logs \
  tarzan-artifact full
```
