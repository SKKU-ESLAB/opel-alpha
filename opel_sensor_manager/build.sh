
cd ./BUILD_SO
mkdir build
cd ./build
cmake ../
make
cp libsensors.so ../../
cd ../../
mkdir build
cd ./build
cmake ../
make


