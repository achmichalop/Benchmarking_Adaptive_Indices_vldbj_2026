if [ ! -d "bin_2d" ]
then
    mkdir -p "bin_2d"
fi


printf '%% Start Compilation %%\n\n'
printf 'Uniform Grid\n'
cd Uniform_Grid
g++ -O3 -march=native -mcmodel=large -D DIM2 -D datacount20M main_ug.cpp -o ../bin_2d/ug_20M_2D
printf '\n'

printf 'Irregular Grid\n'
cd ../Irregular_Grid
g++ -O3 -march=native -mcmodel=large -D DIM2 -D datacount20M main_ig.cpp -o ../bin_2d/ig_20M_2D
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

printf 'AKD-based and QUASII\n'
cd ../AKD-based_and_QUASII
cmake .
make -j
cp bin/main ../bin_2d/akd-based_and_quasii
printf '\n'

printf 'CGI+AAKD\n'
cd ../CGI+AAKD
g++ -O3 -march=native -mcmodel=large -D DIM2 -D datacount20M -D thresh128 main_cgi_aakd.cpp -o ../bin_2d/cgi_aakd_20M_2D_thr128
printf '\n'

printf 'Irregular CGI+AAKD\n'
cd ../Irregular_CGI+AAKD
g++ -O3 -march=native -mcmodel=large -D DIM2 -D datacount20M -D thresh2048 main_irregular_cgi_aakd.cpp -o ../bin_2d/irreg_cgi_aakd_20M_2D_thr2048
printf '\n'

printf 'CGI+AIR\n'
cd ../CGI+AIR
g++ -O3 -march=native -mcmodel=large -D DIM2 -D datacount20M -D maxt256 -D mint128 -D fo16 main_cgi_air.cpp -o ../bin_2d/cgi_air_20M_2D_thr256
printf '\n'


