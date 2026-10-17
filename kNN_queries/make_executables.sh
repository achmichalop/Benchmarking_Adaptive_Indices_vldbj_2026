if [ ! -d "bin_2d" ]
then
    mkdir -p "bin_2d"
fi


printf '%% Start Compilation %%\n\n'
printf 'Uniform Grid\n'
cd Uniform_Grid
g++ -O3 -march=native -mcmodel=large -D DIM2 -D datacount20M main_ug.cpp -o ../bin_2d/ug_20M_2D
printf '\n'

printf 'R-tree\n'
cd ../Rtree
g++ -O3 -march=native -mcmodel=large -D DIM2 -D datacount20M -D nodeSize1024 main_boost.cpp -o ../bin_2d/rtree_20M_2D_node1024
printf '\n'

printf 'KD-tree\n'
cd ../KDtree
g++ -O3 -march=native -mcmodel=large -D DIM2 -D datacount20M main_kdtree.cpp -o ../bin_2d/kdtree_20M_2D
printf '\n'

printf 'AIR\n'
cd ../AIR
g++ -O3 -march=native -mcmodel=large -D DIM2 -D datacount20M -D maxt2048 -D mint1024 -D fo16 general_testing_air.cpp -o ../bin_2d/air_20M_2D_thr2048
printf '\n'

printf 'AAKD\n'
cd ../AAKD
g++ -O3 -march=native -mcmodel=large -D DIM2 -D datacount20M -D thresh1024 main_aakd.cpp -o ../bin_2d/aakd_20M_2D_thr1024
printf '\n'

printf 'AKD and QUASII\n'
cd ../AKD_QUASII
cmake .
make -j
cp bin/main ../bin_2d/akd_and_quasii
printf '\n'

printf 'AV-tree\n'
cd ../AVtree
make main_knn distFunc=L2 datacount=20000000
cp knn ../bin_2d/avtree
printf '\n'

printf 'CGI+AAKD\n'
cd ../CGI+AAKD
g++ -O3 -march=native -mcmodel=large -D DIM2 -D datacount20M -D thresh128 main_cgi_aakd.cpp -o ../bin_2d/cgi_aakd_20M_2D_thr128
printf '\n'




