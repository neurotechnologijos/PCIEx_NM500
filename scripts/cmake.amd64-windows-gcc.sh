cmake \
  -G 'Unix Makefiles' \
  -DCMAKE_INSTALL_PREFIX=/usr \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE=../cmake/platforms/gcc.x86_64-w64-mingw32.cmake \
  ..

cmake --build . --target all --config Release -- --jobs=4
