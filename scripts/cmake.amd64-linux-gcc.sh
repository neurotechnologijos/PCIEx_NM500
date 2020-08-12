cmake \
  -G 'Unix Makefiles' \
  -DCMAKE_INSTALL_PREFIX=/usr \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE=../cmake/platforms/gcc.amd64-linux.cmake \
  ..

cmake --build . --target all --config Release -- --jobs=4
