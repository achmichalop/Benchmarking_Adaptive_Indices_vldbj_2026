if [ ! -d "results_2d" ]
then
    mkdir -p "results_2d"
fi

if [ ! -d "results_2d/Uniform" ]
then
    mkdir -p "results_2d/Uniform"
fi

if [ ! -d "results_2d/Clustered" ]
then
    mkdir -p "results_2d/Clustered"
fi

if [ ! -d "results_2d/Skewed" ]
then
    mkdir -p "results_2d/Skewed"
fi

if [ ! -d "results_2d/Sequential" ]
then
    mkdir -p "results_2d/Sequential"
fi

if [ ! -d "results_2d/ZoomIn" ]
then
    mkdir -p "results_2d/ZoomIn"
fi



printf '%% Run methods for the shape data on range-based workloads %%\n\n'
printf 'Run Uniform Grid\n'
./bin_2d/ug_20M_2D ../../data/shapes/datasets/uniform_20M_2D_norm.txt 10000 ../../data/shapes/queries/uniform_10K_2D_range_queries_norm.txt results_2d/Uniform/times_ug_p500.txt 500
./bin_2d/ug_20M_2D ../../data/shapes/datasets/clustered_20M_2D_norm.txt 10000 ../../data/shapes/queries/clustered_10K_2D_range_queries_norm.txt results_2d/Clustered/times_ug_p500.txt 500
./bin_2d/ug_20M_2D ../../data/shapes/datasets/skewed_20M_2D_norm.txt 10000 ../../data/shapes/queries/skewed_10K_2D_range_queries_norm.txt results_2d/Skewed/times_ug_p500.txt 500
./bin_2d/ug_20M_2D ../../data/shapes/datasets/uniform_20M_2D_norm.txt 99 ../../data/shapes/queries/sequential_100_2D_range_queries_norm.txt results_2d/Sequential/times_ug_p500.txt 500
./bin_2d/ug_20M_2D ../../data/shapes/datasets/uniform_20M_2D_norm.txt 1000 ../../data/shapes/queries/zoomin_1K_2D_range_queries_norm.txt results_2d/ZoomIn/times_ug_p500.txt 500
printf 'Done\n'

printf 'Run Irregular Grid\n'
./bin_2d/ig_20M_2D ../../data/shapes/datasets/uniform_20M_2D_norm.txt 10000 ../../data/shapes/queries/uniform_10K_2D_range_queries_norm.txt results_2d/Uniform/times_ig_s0.01_p100.txt 0.01 100
./bin_2d/ig_20M_2D ../../data/shapes/datasets/clustered_20M_2D_norm.txt 10000 ../../data/shapes/queries/clustered_10K_2D_range_queries_norm.txt results_2d/Clustered/times_ig_s0.01_p100.txt 0.01 100
./bin_2d/ig_20M_2D ../../data/shapes/datasets/skewed_20M_2D_norm.txt 10000 ../../data/shapes/queries/skewed_10K_2D_range_queries_norm.txt results_2d/Skewed/times_ig_s0.01_p100.txt 0.01 100
./bin_2d/ig_20M_2D ../../data/shapes/datasets/uniform_20M_2D_norm.txt 99 ../../data/shapes/queries/sequential_100_2D_range_queries_norm.txt results_2d/Sequential/times_ig_s0.01_p100.txt 0.01 100
./bin_2d/ig_20M_2D ../../data/shapes/datasets/uniform_20M_2D_norm.txt 1000 ../../data/shapes/queries/zoomin_1K_2D_range_queries_norm.txt results_2d/ZoomIn/times_ig_s0.01_p100.txt 0.01 100
printf 'Done\n'

printf 'Run R-tree\n'
./bin_2d/rtree_20M_2D_node1024 ../../data/shapes/datasets/uniform_20M_2D_norm.txt 10000 ../../data/shapes/queries/uniform_10K_2D_range_queries_norm.txt results_2d/Uniform/times_rtree_node1024.txt
./bin_2d/rtree_20M_2D_node1024 ../../data/shapes/datasets/clustered_20M_2D_norm.txt 10000 ../../data/shapes/queries/clustered_10K_2D_range_queries_norm.txt results_2d/Clustered/times_rtree_node1024.txt
./bin_2d/rtree_20M_2D_node1024 ../../data/shapes/datasets/skewed_20M_2D_norm.txt 10000 ../../data/shapes/queries/skewed_10K_2D_range_queries_norm.txt results_2d/Skewed/times_rtree_node1024.txt
./bin_2d/rtree_20M_2D_node1024 ../../data/shapes/datasets/uniform_20M_2D_norm.txt 99 ../../data/shapes/queries/sequential_100_2D_range_queries_norm.txt results_2d/Sequential/times_rtree_node1024.txt
./bin_2d/rtree_20M_2D_node1024 ../../data/shapes/datasets/uniform_20M_2D_norm.txt 1000 ../../data/shapes/queries/zoomin_1K_2D_range_queries_norm.txt results_2d/ZoomIn/times_rtree_node1024.txt
printf 'Done\n'

printf 'Run KD-tree\n'
./bin_2d/kdtree_20M_2D ../../data/shapes/datasets/uniform_20M_2D_norm.txt 10000 ../../data/shapes/queries/uniform_10K_2D_range_queries_norm.txt results_2d/Uniform/times_kdtree_node512.txt 512
./bin_2d/kdtree_20M_2D ../../data/shapes/datasets/clustered_20M_2D_norm.txt 10000 ../../data/shapes/queries/clustered_10K_2D_range_queries_norm.txt results_2d/Clustered/times_kdtree_node512.txt 512
./bin_2d/kdtree_20M_2D ../../data/shapes/datasets/skewed_20M_2D_norm.txt 10000 ../../data/shapes/queries/skewed_10K_2D_range_queries_norm.txt results_2d/Skewed/times_kdtree_node512.txt 512
./bin_2d/kdtree_20M_2D ../../data/shapes/datasets/uniform_20M_2D_norm.txt 99 ../../data/shapes/queries/sequential_100_2D_range_queries_norm.txt results_2d/Sequential/times_kdtree_node512.txt 512
./bin_2d/kdtree_20M_2D ../../data/shapes/datasets/uniform_20M_2D_norm.txt 1000 ../../data/shapes/queries/zoomin_1K_2D_range_queries_norm.txt results_2d/ZoomIn/times_kdtree_node512.txt 512
printf 'Done\n'


printf 'Run AIR\n'
./bin_2d/air_20M_2D_thr2048 ../../data/shapes/datasets/uniform_20M_2D_norm.txt 10000 ../../data/shapes/queries/uniform_10K_2D_range_queries_norm.txt results_2d/Uniform/times_air_thr2048.txt
./bin_2d/air_20M_2D_thr2048 ../../data/shapes/datasets/clustered_20M_2D_norm.txt 10000 ../../data/shapes/queries/clustered_10K_2D_range_queries_norm.txt results_2d/Clustered/times_air_thr2048.txt
./bin_2d/air_20M_2D_thr2048 ../../data/shapes/datasets/skewed_20M_2D_norm.txt 10000 ../../data/shapes/queries/skewed_10K_2D_range_queries_norm.txt results_2d/Skewed/times_air_thr2048.txt
./bin_2d/air_20M_2D_thr2048 ../../data/shapes/datasets/uniform_20M_2D_norm.txt 99 ../../data/shapes/queries/sequential_100_2D_range_queries_norm.txt results_2d/Sequential/times_air_thr2048.txt
./bin_2d/air_20M_2D_thr2048 ../../data/shapes/datasets/uniform_20M_2D_norm.txt 1000 ../../data/shapes/queries/zoomin_1K_2D_range_queries_norm.txt results_2d/ZoomIn/times_air_thr2048.txt
printf 'Done\n'

printf 'Run AAKD\n'
./bin_2d/aakd_20M_2D_thr1024 ../../data/shapes/datasets/uniform_20M_2D_norm.txt 10000 ../../data/shapes/queries/uniform_10K_2D_range_queries_norm.txt ../../data/shapes/queries/uniform_10K_2D_range_queries_norm_qwe.txt results_2d/Uniform/times_aakd_thr1024.txt
./bin_2d/aakd_20M_2D_thr1024 ../../data/shapes/datasets/clustered_20M_2D_norm.txt 10000 ../../data/shapes/queries/clustered_10K_2D_range_queries_norm.txt ../../data/shapes/queries/clustered_10K_2D_range_queries_norm_qwe.txt results_2d/Clustered/times_aakd_thr1024.txt
./bin_2d/aakd_20M_2D_thr1024 ../../data/shapes/datasets/skewed_20M_2D_norm.txt 10000 ../../data/shapes/queries/skewed_10K_2D_range_queries_norm.txt ../../data/shapes/queries/skewed_10K_2D_range_queries_norm_qwe.txt results_2d/Skewed/times_aakd_thr1024.txt
./bin_2d/aakd_20M_2D_thr1024 ../../data/shapes/datasets/uniform_20M_2D_norm.txt 99 ../../data/shapes/queries/sequential_100_2D_range_queries_norm.txt ../../data/shapes/queries/sequential_100_2D_range_queries_norm_qwe.txt results_2d/Sequential/times_aakd_thr1024.txt
./bin_2d/aakd_20M_2D_thr1024 ../../data/shapes/datasets/uniform_20M_2D_norm.txt 1000 ../../data/shapes/queries/zoomin_1K_2D_range_queries_norm.txt ../../data/shapes/queries/zoomin_1K_2D_range_queries_norm_qwe.txt results_2d/ZoomIn/times_aakd_thr1024.txt
printf 'Done\n'

printf 'Run AKD\n'
./bin_2d/akd-based_and_quasii -d ../../data/shapes/datasets/uniform_20M_2D_norm.txt -l 10000 -w ../../data/shapes/queries/uniform_10K_2D_range_queries_norm.txt -b ../../data/shapes/queries/uniform_10K_2D_range_queries_norm_qwe.txt -s results_2d/Uniform/times_akd_thr1024.txt -i 2 -p 1024 -x 2 -k 20000000 -r 1
./bin_2d/akd-based_and_quasii -d ../../data/shapes/datasets/clustered_20M_2D_norm.txt -l 10000 -w ../../data/shapes/queries/clustered_10K_2D_range_queries_norm.txt -b ../../data/shapes/queries/clustered_10K_2D_range_queries_norm_qwe.txt -s  results_2d/Clustered/times_akd_thr1024.txt -i 2 -p 1024 -x 2 -k 20000000 -r 1
./bin_2d/akd-based_and_quasii -d ../../data/shapes/datasets/skewed_20M_2D_norm.txt -l 10000 -w ../../data/shapes/queries/skewed_10K_2D_range_queries_norm.txt -b ../../data/shapes/queries/skewed_10K_2D_range_queries_norm_qwe.txt -s results_2d/Skewed/times_akd_thr1024.txt -i 2 -p 1024 -x 2 -k 20000000 -r 1
./bin_2d/akd-based_and_quasii -d ../../data/shapes/datasets/uniform_20M_2D_norm.txt -l 99 -w ../../data/shapes/queries/sequential_100_2D_range_queries_norm.txt -b ../../data/shapes/queries/sequential_100_2D_range_queries_norm_qwe.txt -s results_2d/Sequential/times_akd_thr1024.txt -i 2 -p 1024 -x 2 -k 20000000 -r 1
./bin_2d/akd-based_and_quasii -d ../../data/shapes/datasets/uniform_20M_2D_norm.txt -l 1000 -w ../../data/shapes/queries/zoomin_1K_2D_range_queries_norm.txt -b ../../data/shapes/queries/zoomin_1K_2D_range_queries_norm_qwe.txt -s results_2d/ZoomIn/times_akd_thr1024.txt -i 2 -p 1024 -x 2 -k 20000000 -r 1
printf 'Done\n'

printf 'Run PKD\n'
./bin_2d/akd-based_and_quasii -d ../../data/shapes/datasets/uniform_20M_2D_norm.txt -l 10000 -w ../../data/shapes/queries/uniform_10K_2D_range_queries_norm.txt -b ../../data/shapes/queries/uniform_10K_2D_range_queries_norm_qwe.txt -s results_2d/Uniform/times_pkd_thr2048.txt -i 7 -p 2048 -x 2 -k 20000000 -a 0.2 -r 1
./bin_2d/akd-based_and_quasii -d ../../data/shapes/datasets/clustered_20M_2D_norm.txt -l 10000 -w ../../data/shapes/queries/clustered_10K_2D_range_queries_norm.txt -b ../../data/shapes/queries/clustered_10K_2D_range_queries_norm_qwe.txt -s  results_2d/Clustered/times_pkd_thr2048.txt -i 7 -p 2048 -x 2 -k 20000000 -a 0.2 -r 1
./bin_2d/akd-based_and_quasii -d ../../data/shapes/datasets/skewed_20M_2D_norm.txt -l 10000 -w ../../data/shapes/queries/skewed_10K_2D_range_queries_norm.txt -b ../../data/shapes/queries/skewed_10K_2D_range_queries_norm_qwe.txt -s results_2d/Skewed/times_pkd_thr2048.txt -i 7 -p 2048 -x 2 -k 20000000 -a 0.2 -r 1
./bin_2d/akd-based_and_quasii -d ../../data/shapes/datasets/uniform_20M_2D_norm.txt -l 99 -w ../../data/shapes/queries/sequential_100_2D_range_queries_norm.txt -b ../../data/shapes/queries/sequential_100_2D_range_queries_norm_qwe.txt -s results_2d/Sequential/times_pkd_thr2048.txt -i 7 -p 2048 -x 2 -k 20000000 -a 0.2 -r 1
./bin_2d/akd-based_and_quasii -d ../../data/shapes/datasets/uniform_20M_2D_norm.txt -l 1000 -w ../../data/shapes/queries/zoomin_1K_2D_range_queries_norm.txt -b ../../data/shapes/queries/zoomin_1K_2D_range_queries_norm_qwe.txt -s results_2d/ZoomIn/times_pkd_thr2048.txt -i 7 -p 2048 -x 2 -k 20000000 -a 0.2 -r 1
printf 'Done\n'

printf 'Run GPKD\n'
./bin_2d/akd-based_and_quasii -d ../../data/shapes/datasets/uniform_20M_2D_norm.txt -l 10000 -w ../../data/shapes/queries/uniform_10K_2D_range_queries_norm.txt -b ../../data/shapes/queries/uniform_10K_2D_range_queries_norm_qwe.txt -s results_2d/Uniform/times_gpkd_delta0.6_thr2048.txt -i 7 -p 2048 -x 2 -k 20000000 -a 0.6 -c -r 1
./bin_2d/akd-based_and_quasii -d ../../data/shapes/datasets/clustered_20M_2D_norm.txt -l 10000 -w ../../data/shapes/queries/clustered_10K_2D_range_queries_norm.txt -b ../../data/shapes/queries/clustered_10K_2D_range_queries_norm_qwe.txt -s  results_2d/Clustered/times_gpkd_delta0.6_thr2048.txt -i 7 -p 2048 -x 2 -k 20000000 -a 0.6 -c -r 1
./bin_2d/akd-based_and_quasii -d ../../data/shapes/datasets/skewed_20M_2D_norm.txt -l 10000 -w ../../data/shapes/queries/skewed_10K_2D_range_queries_norm.txt -b ../../data/shapes/queries/skewed_10K_2D_range_queries_norm_qwe.txt -s results_2d/Skewed/times_gpkd_delta0.6_thr2048.txt -i 7 -p 2048 -x 2 -k 20000000 -a 0.6 -c -r 1
./bin_2d/akd-based_and_quasii -d ../../data/shapes/datasets/uniform_20M_2D_norm.txt -l 99 -w ../../data/shapes/queries/sequential_100_2D_range_queries_norm.txt -b ../../data/shapes/queries/sequential_100_2D_range_queries_norm_qwe.txt -s results_2d/Sequential/times_gpkd_delta0.6_thr2048.txt -i 7 -p 2048 -x 2 -k 20000000 -a 0.6 -c -r 1
./bin_2d/akd-based_and_quasii -d ../../data/shapes/datasets/uniform_20M_2D_norm.txt -l 1000 -w ../../data/shapes/queries/zoomin_1K_2D_range_queries_norm.txt -b ../../data/shapes/queries/zoomin_1K_2D_range_queries_norm_qwe.txt -s results_2d/ZoomIn/times_gpkd_delta0.6_thr2048.txt -i 7 -p 2048 -x 2 -k 20000000 -a 0.6 -c -r 1
printf 'Done\n'

printf 'Run QUASII\n'
./bin_2d/akd-based_and_quasii -d ../../data/shapes/datasets/uniform_20M_2D_norm.txt -l 10000 -w ../../data/shapes/queries/uniform_10K_2D_range_queries_norm.txt -b ../../data/shapes/queries/uniform_10K_2D_range_queries_norm_qwe.txt -s results_2d/Uniform/times_quasii_thr2048.txt -i 6 -p 2048 -x 2 -k 20000000 -r 1
./bin_2d/akd-based_and_quasii -d ../../data/shapes/datasets/clustered_20M_2D_norm.txt -l 10000 -w ../../data/shapes/queries/clustered_10K_2D_range_queries_norm.txt -b ../../data/shapes/queries/clustered_10K_2D_range_queries_norm_qwe.txt -s  results_2d/Clustered/times_quasii_thr2048.txt -i 6 -p 2048 -x 2 -k 20000000 -r 1
./bin_2d/akd-based_and_quasii -d ../../data/shapes/datasets/skewed_20M_2D_norm.txt -l 10000 -w ../../data/shapes/queries/skewed_10K_2D_range_queries_norm.txt -b ../../data/shapes/queries/skewed_10K_2D_range_queries_norm_qwe.txt -s results_2d/Skewed/times_quasii_thr2048.txt -i 6 -p 2048 -x 2 -k 20000000 -r 1
./bin_2d/akd-based_and_quasii -d ../../data/shapes/datasets/uniform_20M_2D_norm.txt -l 99 -w ../../data/shapes/queries/sequential_100_2D_range_queries_norm.txt -b ../../data/shapes/queries/sequential_100_2D_range_queries_norm_qwe.txt -s results_2d/Sequential/times_quasii_thr2048.txt -i 6 -p 2048 -x 2 -k 20000000 -r 1
./bin_2d/akd-based_and_quasii -d ../../data/shapes/datasets/uniform_20M_2D_norm.txt -l 1000 -w ../../data/shapes/queries/zoomin_1K_2D_range_queries_norm.txt -b ../../data/shapes/queries/zoomin_1K_2D_range_queries_norm_qwe.txt -s results_2d/ZoomIn/times_quasii_thr2048.txt -i 6 -p 2048 -x 2 -k 20000000 -r 1
printf 'Done\n'

printf 'Run CGI+AAKD\n'
./bin_2d/cgi_aakd_20M_2D_thr128 ../../data/shapes/datasets/uniform_20M_2D_norm.txt 10000 ../../data/shapes/queries/uniform_10K_2D_range_queries_norm.txt ../../data/shapes/queries/uniform_10K_2D_range_queries_norm_qwe.txt results_2d/Uniform/times_cgi_aakd_thr128.txt
./bin_2d/cgi_aakd_20M_2D_thr128 ../../data/shapes/datasets/clustered_20M_2D_norm.txt 10000 ../../data/shapes/queries/clustered_10K_2D_range_queries_norm.txt ../../data/shapes/queries/clustered_10K_2D_range_queries_norm_qwe.txt results_2d/Clustered/times_cgi_aakd_thr128.txt
./bin_2d/cgi_aakd_20M_2D_thr128 ../../data/shapes/datasets/skewed_20M_2D_norm.txt 10000 ../../data/shapes/queries/skewed_10K_2D_range_queries_norm.txt ../../data/shapes/queries/skewed_10K_2D_range_queries_norm_qwe.txt results_2d/Skewed/times_cgi_aakd_thr128.txt
./bin_2d/cgi_aakd_20M_2D_thr128 ../../data/shapes/datasets/uniform_20M_2D_norm.txt 99 ../../data/shapes/queries/sequential_100_2D_range_queries_norm.txt ../../data/shapes/queries/sequential_100_2D_range_queries_norm_qwe.txt results_2d/Sequential/times_cgi_aakd_thr128.txt
./bin_2d/cgi_aakd_20M_2D_thr128 ../../data/shapes/datasets/uniform_20M_2D_norm.txt 1000 ../../data/shapes/queries/zoomin_1K_2D_range_queries_norm.txt ../../data/shapes/queries/zoomin_1K_2D_range_queries_norm_qwe.txt results_2d/ZoomIn/times_cgi_aakd_thr128.txt
printf 'Done\n'

printf 'Run Irregular CGI+AAKD\n'
./bin_2d/irreg_cgi_aakd_20M_2D_thr2048 ../../data/shapes/datasets/uniform_20M_2D_norm.txt 10000 ../../data/shapes/queries/uniform_10K_2D_range_queries_norm.txt ../../data/shapes/queries/uniform_10K_2D_range_queries_norm_qwe.txt results_2d/Uniform/times_irreg_cgi_aakd_s0.01_thr2048.txt 0.01
./bin_2d/irreg_cgi_aakd_20M_2D_thr2048 ../../data/shapes/datasets/clustered_20M_2D_norm.txt 10000 ../../data/shapes/queries/clustered_10K_2D_range_queries_norm.txt ../../data/shapes/queries/clustered_10K_2D_range_queries_norm_qwe.txt results_2d/Clustered/times_irreg_cgi_aakd_s0.01_thr2048.txt 0.01
./bin_2d/irreg_cgi_aakd_20M_2D_thr2048 ../../data/shapes/datasets/skewed_20M_2D_norm.txt 10000 ../../data/shapes/queries/skewed_10K_2D_range_queries_norm.txt ../../data/shapes/queries/skewed_10K_2D_range_queries_norm_qwe.txt results_2d/Skewed/times_irreg_cgi_aakd_s0.01_thr2048.txt 0.01
./bin_2d/irreg_cgi_aakd_20M_2D_thr2048 ../../data/shapes/datasets/uniform_20M_2D_norm.txt 99 ../../data/shapes/queries/sequential_100_2D_range_queries_norm.txt ../../data/shapes/queries/sequential_100_2D_range_queries_norm_qwe.txt results_2d/Sequential/times_irreg_cgi_aakd_s0.01_thr2048.txt 0.01
./bin_2d/irreg_cgi_aakd_20M_2D_thr2048 ../../data/shapes/datasets/uniform_20M_2D_norm.txt 1000 ../../data/shapes/queries/zoomin_1K_2D_range_queries_norm.txt ../../data/shapes/queries/zoomin_1K_2D_range_queries_norm_qwe.txt results_2d/ZoomIn/times_irreg_cgi_aakd_s0.01_thr2048.txt 0.01
printf 'Done\n'

printf 'Run CGI+AIR\n'
./bin_2d/cgi_air_20M_2D_thr256 ../../data/shapes/datasets/uniform_20M_2D_norm.txt 10000 ../../data/shapes/queries/uniform_10K_2D_range_queries_norm.txt ../../data/shapes/queries/uniform_10K_2D_range_queries_norm_qwe.txt results_2d/Uniform/times_cgi_air_thr256.txt
./bin_2d/cgi_air_20M_2D_thr256 ../../data/shapes/datasets/clustered_20M_2D_norm.txt 10000 ../../data/shapes/queries/clustered_10K_2D_range_queries_norm.txt ../../data/shapes/queries/clustered_10K_2D_range_queries_norm_qwe.txt results_2d/Clustered/times_cgi_air_thr256.txt
./bin_2d/cgi_air_20M_2D_thr256 ../../data/shapes/datasets/skewed_20M_2D_norm.txt 10000 ../../data/shapes/queries/skewed_10K_2D_range_queries_norm.txt ../../data/shapes/queries/skewed_10K_2D_range_queries_norm_qwe.txt results_2d/Skewed/times_cgi_air_thr256.txt
./bin_2d/cgi_air_20M_2D_thr256 ../../data/shapes/datasets/uniform_20M_2D_norm.txt 99 ../../data/shapes/queries/sequential_100_2D_range_queries_norm.txt ../../data/shapes/queries/sequential_100_2D_range_queries_norm_qwe.txt results_2d/Sequential/times_cgi_air_thr256.txt
./bin_2d/cgi_air_20M_2D_thr256 ../../data/shapes/datasets/uniform_20M_2D_norm.txt 1000 ../../data/shapes/queries/zoomin_1K_2D_range_queries_norm.txt ../../data/shapes/queries/zoomin_1K_2D_range_queries_norm_qwe.txt results_2d/ZoomIn/times_cgi_air_thr256.txt
printf 'Done\n'