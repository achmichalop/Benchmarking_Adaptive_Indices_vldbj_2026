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

if [ ! -d "results_2d/Concentrate" ]
then
    mkdir -p "results_2d/Concentrate"
fi



printf '%% Run methods for the point data on range-based workloads %%\n\n'
printf 'Run Uniform Grid\n'
./bin_2d/ug_20M_2D ../data/points/datasets/uniform_20M_2D_norm.txt 10000 ../data/points/queries/uniform_10K_2D_knn_queries_norm.txt results_2d/Uniform/times_ug_p500.txt 500 10
./bin_2d/ug_20M_2D ../data/points/datasets/clustered_20M_2D_norm.txt 10000 ../data/points/queries/clustered_10K_2D_knn_queries_norm.txt results_2d/Clustered/times_ug_p500.txt 500 10
./bin_2d/ug_20M_2D ../data/points/datasets/skewed_20M_2D_norm.txt 10000 ../data/points/queries/skewed_10K_2D_knn_queries_norm.txt results_2d/Skewed/times_ug_p500.txt 500 10
./bin_2d/ug_20M_2D ../data/points/datasets/uniform_20M_2D_norm.txt 100 ../data/points/queries/sequential_100_2D_kNN_queries_norm.txt results_2d/Sequential/times_ug_p500.txt 500 10
./bin_2d/ug_20M_2D ../data/points/datasets/uniform_20M_2D_norm.txt 1000 ../data/points/queries/concentrate_1K_2D_kNN_queries_norm.txt results_2d/Concentrate/times_ug_p500.txt 500 10
printf 'Done\n'

printf 'Run R-tree\n'
./bin_2d/rtree_20M_2D_node1024 ../data/points/datasets/uniform_20M_2D_norm.txt 10000 ../data/points/queries/uniform_10K_2D_knn_queries_norm.txt results_2d/Uniform/times_rtree_node1024.txt 10
./bin_2d/rtree_20M_2D_node1024 ../data/points/datasets/clustered_20M_2D_norm.txt 10000 ../data/points/queries/clustered_10K_2D_knn_queries_norm.txt results_2d/Clustered/times_rtree_node1024.txt 10
./bin_2d/rtree_20M_2D_node1024 ../data/points/datasets/skewed_20M_2D_norm.txt 10000 ../data/points/queries/skewed_10K_2D_knn_queries_norm.txt results_2d/Skewed/times_rtree_node1024.txt 10
./bin_2d/rtree_20M_2D_node1024 ../data/points/datasets/uniform_20M_2D_norm.txt 100 ../data/points/queries/sequential_100_2D_kNN_queries_norm.txt results_2d/Sequential/times_rtree_node1024.txt 10
./bin_2d/rtree_20M_2D_node1024 ../data/points/datasets/uniform_20M_2D_norm.txt 1000 ../data/points/queries/concentrate_1K_2D_kNN_queries_norm.txt results_2d/Concentrate/times_rtree_node1024.txt 10
printf 'Done\n'

printf 'Run KD-tree\n'
./bin_2d/kdtree_20M_2D ../data/points/datasets/uniform_20M_2D_norm.txt 10000 ../data/points/queries/uniform_10K_2D_knn_queries_norm.txt results_2d/Uniform/times_kdtree_node512.txt 512 10
./bin_2d/kdtree_20M_2D ../data/points/datasets/clustered_20M_2D_norm.txt 10000 ../data/points/queries/clustered_10K_2D_knn_queries_norm.txt results_2d/Clustered/times_kdtree_node512.txt 512 10
./bin_2d/kdtree_20M_2D ../data/points/datasets/skewed_20M_2D_norm.txt 10000 ../data/points/queries/skewed_10K_2D_knn_queries_norm.txt results_2d/Skewed/times_kdtree_node512.txt 512 10
./bin_2d/kdtree_20M_2D ../data/points/datasets/uniform_20M_2D_norm.txt 100 ../data/points/queries/sequential_100_2D_kNN_queries_norm.txt results_2d/Sequential/times_kdtree_node512.txt 512 10
./bin_2d/kdtree_20M_2D ../data/points/datasets/uniform_20M_2D_norm.txt 1000 ../data/points/queries/concentrate_1K_2D_kNN_queries_norm.txt results_2d/Concentrate/times_kdtree_node512.txt 512 10
printf 'Done\n'


printf 'Run AIR\n'
./bin_2d/air_20M_2D_thr2048 ../data/points/datasets/uniform_20M_2D_norm_air.txt 10000 ../data/points/queries/uniform_10K_2D_knn_queries_norm.txt results_2d/Uniform/times_air_thr2048.txt 10 2
./bin_2d/air_20M_2D_thr2048 ../data/points/datasets/clustered_20M_2D_norm_air.txt 10000 ../data/points/queries/clustered_10K_2D_knn_queries_norm.txt results_2d/Clustered/times_air_thr2048.txt 10 2
./bin_2d/air_20M_2D_thr2048 ../data/points/datasets/skewed_20M_2D_norm_air.txt 10000 ../data/points/queries/skewed_10K_2D_knn_queries_norm.txt results_2d/Skewed/times_air_thr2048.txt 10 2
./bin_2d/air_20M_2D_thr2048 ../data/points/datasets/uniform_20M_2D_norm_air.txt 100 ../data/points/queries/sequential_100_2D_kNN_queries_norm.txt results_2d/Sequential/times_air_thr2048.txt 10 2
./bin_2d/air_20M_2D_thr2048 ../data/points/datasets/uniform_20M_2D_norm_air.txt 1000 ../data/points/queries/concentrate_1K_2D_kNN_queries_norm.txt results_2d/Concentrate/times_air_thr2048.txt 10 2
printf 'Done\n'

printf 'Run AAKD\n'
./bin_2d/aakd_20M_2D_thr1024 ../data/points/datasets/uniform_20M_2D_norm.txt 10000 ../data/points/queries/uniform_10K_2D_knn_queries_norm.txt results_2d/Uniform/times_aakd_thr1024.txt 10 2
./bin_2d/aakd_20M_2D_thr1024 ../data/points/datasets/clustered_20M_2D_norm.txt 10000 ../data/points/queries/clustered_10K_2D_knn_queries_norm.txt results_2d/Clustered/times_aakd_thr1024.txt 10 2
./bin_2d/aakd_20M_2D_thr1024 ../data/points/datasets/skewed_20M_2D_norm.txt 10000 ../data/points/queries/skewed_10K_2D_knn_queries_norm.txt results_2d/Skewed/times_aakd_thr1024.txt 10 2
./bin_2d/aakd_20M_2D_thr1024 ../data/points/datasets/uniform_20M_2D_norm.txt 100 ../data/points/queries/sequential_100_2D_kNN_queries_norm.txt results_2d/Sequential/times_aakd_thr1024.txt 10 2
./bin_2d/aakd_20M_2D_thr1024 ../data/points/datasets/uniform_20M_2D_norm.txt 1000 ../data/points/queries/concentrate_1K_2D_kNN_queries_norm.txt results_2d/Concentrate/times_aakd_thr1024.txt 10 2
printf 'Done\n'

printf 'Run AKD\n'
./bin_2d/akd_and_quasii -d ../data/points/datasets/uniform_20M_2D_norm.txt -l 10000 -w ../data/points/queries/uniform_10K_2D_knn_queries_norm.txt -s results_2d/Uniform/times_akd_thr1024.txt -i 2 -p 1024 -x 2 -k 20000000 -r 1 -n 10 -f 2
./bin_2d/akd_and_quasii -d ../data/points/datasets/clustered_20M_2D_norm.txt -l 10000 -w ../data/points/queries/clustered_10K_2D_knn_queries_norm.txt -s  results_2d/Clustered/times_akd_thr1024.txt -i 2 -p 1024 -x 2 -k 20000000 -r 1 -n 10 -f 2
./bin_2d/akd_and_quasii -d ../data/points/datasets/skewed_20M_2D_norm.txt -l 10000 -w ../data/points/queries/skewed_10K_2D_knn_queries_norm.txt -s results_2d/Skewed/times_akd_thr1024.txt -i 2 -p 1024 -x 2 -k 20000000 -r 1 -n 10 -f 2
./bin_2d/akd_and_quasii -d ../data/points/datasets/uniform_20M_2D_norm.txt -l 100 -w ../data/points/queries/sequential_100_2D_kNN_queries_norm.txt -s results_2d/Sequential/times_akd_thr1024.txt -i 2 -p 1024 -x 2 -k 20000000 -r 1 -n 10 -f 2
./bin_2d/akd_and_quasii -d ../data/points/datasets/uniform_20M_2D_norm.txt -l 1000 -w ../data/points/queries/concentrate_1K_2D_kNN_queries_norm.txt -s results_2d/Concentrate/times_akd_thr1024.txt -i 2 -p 1024 -x 2 -k 20000000 -r 1 -n 10 -f 2
printf 'Done\n'

printf 'Run QUASII\n'
./bin_2d/akd_and_quasii -d ../data/points/datasets/uniform_20M_2D_norm.txt -l 10000 -w ../data/points/queries/uniform_10K_2D_knn_queries_norm.txt -s results_2d/Uniform/times_quasii_thr2048.txt -i 6 -p 2048 -x 2 -k 20000000 -r 1 -n 10 -f 2
./bin_2d/akd_and_quasii -d ../data/points/datasets/clustered_20M_2D_norm.txt -l 10000 -w ../data/points/queries/clustered_10K_2D_knn_queries_norm.txt -s  results_2d/Clustered/times_quasii_thr2048.txt -i 6 -p 2048 -x 2 -k 20000000 -r 1 -n 10 -f 2
./bin_2d/akd_and_quasii -d ../data/points/datasets/skewed_20M_2D_norm.txt -l 10000 -w ../data/points/queries/skewed_10K_2D_knn_queries_norm.txt -s results_2d/Skewed/times_quasii_thr2048.txt -i 6 -p 2048 -x 2 -k 20000000 -r 1 -n 10 -f 2
./bin_2d/akd_and_quasii -d ../data/points/datasets/uniform_20M_2D_norm.txt -l 100 -w ../data/points/queries/sequential_100_2D_kNN_queries_norm.txt -s results_2d/Sequential/times_quasii_thr2048.txt -i 6 -p 2048 -x 2 -k 20000000 -r 1 -n 10 -f 2
./bin_2d/akd_and_quasii -d ../data/points/datasets/uniform_20M_2D_norm.txt -l 1000 -w ../data/points/queries/concentrate_1K_2D_kNN_queries_norm.txt -s results_2d/Concentrate/times_quasii_thr2048.txt -i 6 -p 2048 -x 2 -k 20000000 -r 1 -n 10 -f 2
printf 'Done\n'


printf 'Run AV-tree\n'
./bin_2d/avtree -m -t 256 -n 10 -d 2 -q 10000 ../data/points/datasets/uniform_20M_2D_norm.txt ../data/points/queries/uniform_10K_2D_knn_queries_norm.txt > results_2d/Uniform/times_avtree_thr256.txt
./bin_2d/avtree -m -t 256 -n 10 -d 2 -q 10000 ../data/points/datasets/clustered_20M_2D_norm.txt ../data/points/queries/clustered_10K_2D_knn_queries_norm.txt > results_2d/Clustered/times_avtree_thr256.txt
./bin_2d/avtree -m -t 256 -n 10 -d 2 -q 10000 ../data/points/datasets/skewed_20M_2D_norm.txt ../data/points/queries/skewed_10K_2D_knn_queries_norm.txt > results_2d/Skewed/times_avtree_thr256.txt
./bin_2d/avtree -m -t 256 -n 10 -d 2 -q 100 ../data/points/datasets/uniform_20M_2D_norm.txt ../data/points/queries/sequential_100_2D_kNN_queries_norm.txt > results_2d/Sequential/times_avtree_thr256.txt
./bin_2d/avtree -m -t 256 -n 10 -d 2 -q 1000 ../data/points/datasets/uniform_20M_2D_norm.txt ../data/points/queries/concentrate_1K_2D_kNN_queries_norm.txt > results_2d/Concentrate/times_avtree_thr256.txt
printf 'Done\n'

printf 'Run CGI+AAKD\n'
./bin_2d/cgi_aakd_20M_2D_thr128 ../data/points/datasets/uniform_20M_2D_norm.txt 10000 ../data/points/queries/uniform_10K_2D_knn_queries_norm.txt results_2d/Uniform/times_cgi_aakd_thr128.txt 10 2
./bin_2d/cgi_aakd_20M_2D_thr128 ../data/points/datasets/clustered_20M_2D_norm.txt 10000 ../data/points/queries/clustered_10K_2D_knn_queries_norm.txt results_2d/Clustered/times_cgi_aakd_thr128.txt 10 2
./bin_2d/cgi_aakd_20M_2D_thr128 ../data/points/datasets/skewed_20M_2D_norm.txt 10000 ../data/points/queries/skewed_10K_2D_knn_queries_norm.txt results_2d/Skewed/times_cgi_aakd_thr128.txt 10 2
./bin_2d/cgi_aakd_20M_2D_thr128 ../data/points/datasets/uniform_20M_2D_norm.txt 100 ../data/points/queries/sequential_100_2D_kNN_queries_norm.txt results_2d/Sequential/times_cgi_aakd_thr128.txt 10 2
./bin_2d/cgi_aakd_20M_2D_thr128 ../data/points/datasets/uniform_20M_2D_norm.txt 1000 ../data/points/queries/concentrate_1K_2D_kNN_queries_norm.txt results_2d/Concentrate/times_cgi_aakd_thr128.txt 10 2
printf 'Done\n'
