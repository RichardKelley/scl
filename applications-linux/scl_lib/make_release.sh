rm -rf release &&
mkdir -p release &&
mkdir -p build_rel &&
cd build_rel &&
cmake .. -DCMAKE_BUILD_TYPE=Release &&
make -j8 &&
mv lib* ../release/ &&
cd ..
