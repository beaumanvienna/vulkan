#/bin/bash

FILE=vendor/sfml/build/lib/libsfml-graphics-s.a
if [ -f $FILE ]; then
   echo "File $FILE exists"
else
   echo "building sfml"
   cd ../sfml
   mkdir -p build
   cd build
   cmake -DBUILD_SHARED_LIBS=OFF ..
   make -j$(cat /proc/cpuinfo | grep -c vendor_id)
   cd ../..
fi
