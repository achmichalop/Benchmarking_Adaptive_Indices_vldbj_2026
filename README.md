# Benchmarking Adaptive Multidimensional Indices for Range and kNN queries
This repository contains the code and resources required to reproduce the experiments presented in the paper. It extends the codebase of the VLDB 2025 paper, [Benchmarking Adaptive Multidimensional Indices](https://github.com/DinosL/BenchmarkingAdaptiveIndexes), with additional implementations and experiments introduced in this extended version.

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

- **AIR**: Based on the [Adaptive Indexing of Objects with Spatial Extent](https://www.vldb.org/pvldb/vol16/p2248-karras.pdf) and [source code](https://gitlab.com/adaptivertree/artree).
- **AKD-based and QUASII**: Based on the [Multidimensional Adaptive & Progressive Indexes](https://ir.cwi.nl/pub/31298/31298.pdf) and [source code](https://github.com/pdet/MultidimensionalAdaptiveIndexing).
- **AVtree**: Based on the [AdaptiveIndexinginHigh-DimensionalMetricSpaces](https://www.vldb.org/pvldb/vol16/p2525-mamoulis.pdf) and [source code](https://github.com/DinosL/AVtree).
- **KDtree**: Based on the [nanoﬂann: A Header-Only KD-Tree Library for ExactNearest-Neighbor Search on Compile-Time Product Manifolds](https://www.researchgate.net/publication/415749675_nanoflann_A_Header-Only_KD-Tree_Library_for_Exact_Nearest-Neighbor_Search_on_Compile-Time_Product_Manifolds) and [source code](https://github.com/jlblancoc/nanoflann).
- **Rtree**: Based on the [Boost C++ Libraries](http://www.boost.org/).

### Our Implementations

The following indices are implemented by us:

- **AAKD**: A variant of AKD that uses the heuristic of AIR.
- **CGI+AAKD**: Our implementation combining CGI with AAKD.
- **Irregular CGI+AAKD**: Our implementation combining an irregular CGI grid with AAKD.
- **Irregular Grid**: Our implementation of an irregular grid index.
- **Uniform Grid**: Our implementation of a uniform grid index.

## Dependencies
- **gcc/g++**: Version 13
- **Boost**: Version 1.74.0 or higher

## Data Preparation
We provide a script to generate the three main synthetic datasets —Uniform, Clustered, and Skewed— each containing 20 million two-dimensional records, using the parameters specified in the paper. The script also generates the range and kNN query workloads, following the access patterns described in the paper.

```bash
cd data
python3 generate_data.py
```

### Compilation
Once all dependencies are installed and `CMakeLists.txt` is configured, you can build the benchmark with the following commands:

```bash
# Create a build directory and navigate into it
mkdir build && cd build

# Generate the build files with CMake
cmake ..

# Compile the project
make
```
## Run Experiments
### 1. Prepare Data
## Data Preparation

We provide a script to generate the three main synthetic datasets—Uniform, Clustered, and Skewed—each containing 20 million two-dimensional records. 
```bash
cd data
python3 create_data.py
```
The script generates the following folders:
- **datasets/** - containing the initial datasets
- **range_queries/** and **knn_queries/** - each containing 20 queries for every dataset

### 2. Run Queries
Once the datasets and benchmark executable are ready, you can run queries for each index.

#### 2.1 Range and kNN Queries
For **range queries** the selectivity is set to 0.001%. For **knn queries** each point is evaluated for different k values (k=1,5,10,50,100,500).

The general command format is:

```bash
# General command format
./bin/bench_<n>d <index> <dataset_fname> <num_points> <queries_fname> <num_queries> range
./bin/bench_<n>d <index> <dataset_fname> <num_points> <queries_fname> <num_queries> knn
```

Example usage:

```bash
# Run range queries for a 4-dimensional dataset
./bin/bench_4d skdtree ../data/datasets/20M_4D_uniform.bin 20000000 ../data/range_queries/20M_4D_uniform_range_queries.txt 20 range

# Run kNN queries for a 4-dimensional dataset
./bin/bench_4d skdtree ../data/datasets/20M_4D_uniform.bin 20000000 ../data/knn_queries/20M_4D_uniform_knn_queries.txt 20 knn
```
##### Notes:
- Replace `<n>` with the dataset dimensionality
- Replace `<index>` with the index you want to test (e.g., skdtree)
- Adjust `<num_points>` and `<num_queries>` if using custom datasets/queries.

#### 2.2 Mixed Workload
The benchmark also supports a **mixed workload**, which combines dynamic updates (**insertions** and **deletions**) with an equal number of **range** and **knn queries**, as described in the paper.

The general command format is:
```bash
./bin/bench_<n>d_mixed <index> <dataset_fname> <num_points> <update_ratio> <range_queries> <knn_queries> <num_queries> 
```
Example usage:
```bash
# Run a mixed workload for a 4-dimensional dataset
./bin/bench_4d_mixed skdtree ../data/datasets/20M_4D_uniform.bin 20000000 0.1 ../data/range_queries/20M_4D_uniform_range_queries.txt ../data/knn_queries/20M_4D_uniform_knn_queries.txt 20 
```






# Benchmarking_Adaptive_Indices_vldbj_2026

