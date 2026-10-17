
# Benchmarking Adaptive Multidimensional Indices for Range and kNN Queries

This repository contains the code and resources required to reproduce the experiments presented in the paper. It extends the codebase of the VLDB 2025 paper, [*Benchmarking Adaptive Multidimensional Indices*](https://github.com/DinosL/BenchmarkingAdaptiveIndexes), with additional implementations and experiments introduced in this extended version.

## Repository Structure

The evaluated indices are organized into directories according to the query workloads they support. For range queries, both point and shape data are used, whereas kNN and mixed workloads (combining range and kNN queries) use only point data.

The directory structure is as follows:

```text
range_queries/
├── point_data/
│   ├── AAKD/
│   ├── AIR/
│   ├── AKD-based_and_QUASII/
│   ├── AVtree/
│   ├── CGI+AAKD/
│   ├── Irregular_CGI+AAKD/
│   ├── Irregular_Grid/
│   ├── KDtree/
│   ├── Rtree/
│   └── Uniform_Grid/
└── shape_data/
    ├── AAKD/
    ├── AIR/
    ├── AKD-based_and_QUASII/
    ├── CGI+AAKD/
    ├── Irregular_CGI+AAKD/
    ├── Irregular_Grid/
    ├── KDtree/
    ├── Rtree/
    └── Uniform_Grid/

kNN_queries/
├── AAKD/
├── AIR/
├── AKD_QUASII/
├── AVtree/
├── CGI+AAKD/
├── KDtree/
├── Rtree/
└── Uniform_Grid/

mixed_queries/
├── AAKD/
├── AIR/
├── AKD_QUASII/
├── CGI+AAKD/
├── KDtree/
├── Rtree/
└── Uniform_Grid/
```

## Methods and Sources

The repository includes implementations of existing indices and our own implementations and variants. The sources of the original implementations and related papers are listed below.

- **AIR**: Based on [*Adaptive Indexing of Objects with Spatial Extent*](https://www.vldb.org/pvldb/vol16/p2248-karras.pdf) and its [source code](https://gitlab.com/adaptivertree/artree).
- **AKD-based and QUASII**: Based on [*Multidimensional Adaptive & Progressive Indexes*](https://ir.cwi.nl/pub/31298/31298.pdf) and the [source code](https://github.com/pdet/MultidimensionalAdaptiveIndexing).
- **AVtree**: Based on [*Adaptive Indexing in High-Dimensional Metric Spaces*](https://www.vldb.org/pvldb/vol16/p2525-mamoulis.pdf) and its [source code](https://github.com/DinosL/AVtree).
- **KDtree**: Based on [*nanoflann: A Header-Only KD-Tree Library for Exact Nearest-Neighbor Search on Compile-Time Product Manifolds*](https://www.researchgate.net/publication/415749675_nanoflann_A_Header-Only_KD-Tree_Library_for_Exact_Nearest-Neighbor_Search_on_Compile-Time_Product_Manifolds) and the [source code](https://github.com/jlblancoc/nanoflann).
- **Rtree**: Based on the [Boost C++ Libraries](https://www.boost.org/).

### Our Implementations

The following indices are implemented by us:

- **AAKD**: A variant of AKD that uses the heuristic of AIR.
- **CGI+AAKD**: Our implementation combining CGI with AAKD.
- **Irregular_CGI+AAKD**: Our implementation combining an irregular CGI grid with AAKD.
- **Irregular_Grid**: Our implementation of an irregular grid index.
- **Uniform_Grid**: Our implementation of a uniform grid index.

## Dependencies

- **GCC/G++**: Version 13.
- **Boost**: Version 1.74.0 or higher.

## Data Preparation

We provide a script to generate the three main synthetic datasets—Uniform, Clustered, and Skewed—each containing 20 million two-dimensional records, using the parameters specified in the paper. The script also generates the range and kNN query workloads, following the access patterns described in the paper.

To generate the datasets and query workloads, run:

```bash
cd data
python3 generate_data.py
```

## Compilation

Each of the four directories (`range_queries/point_data/`, `range_queries/shape_data/`, `kNN_queries/`, and `mixed_queries/`) contains a Bash script named `make_executables.sh`. The script compiles the executables for all indices included in the corresponding directory and stores them in a common `bin_2d/` subdirectory.

For example, to build the executables for range queries on point data, run:

```bash
cd range_queries/point_data
chmod +x make_executables.sh
./make_executables.sh
```

## Running Experiments

Each of the four directories (`range_queries/point_data/`, `range_queries/shape_data/`, `kNN_queries/`, and `mixed_queries/`) contains a Bash script named `runexpe.sh`. The script uses the executables in the `bin_2d/` subdirectory to run all combinations of datasets and query access patterns for the indices involved in the corresponding workload. For each index, the parameter settings determined by the tuning process described in the paper are used.

The experimental results are stored in a `results_2d/` subdirectory, organized into subdirectories according to the experimental scenario.

For example, to run the experiments for range queries on point data, execute:

```bash
cd range_queries/point_data
chmod +x runexpe.sh
./runexpe.sh
```

After execution, the results are organized as follows:

```text
results_2d/
├── Clustered/
├── Sequential/
├── Skewed/
├── Uniform/
└── ZoomIn/
```

Each directory contains the experimental results for the corresponding scenario.
