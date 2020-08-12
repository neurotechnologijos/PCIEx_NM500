cmake \
  -G 'MinGW Makefiles' \
  -DCMAKE_INSTALL_PREFIX=/usr \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE=../cmake/platforms/clang.amd64-msys2.cmake \
  ..

cmake --build . --target all --config Release -- --jobs=4
