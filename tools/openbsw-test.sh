#!/bin/bash

TARGET=$1

set -e

CORES=$(nproc)

#export CC=clang
#export CXX=clang++
#export CPP_STANDARD=17
export CPP_STANDARD=17

#export MY_CC=arm-none-eabi-gcc
#export MY_CXX=arm-none-eabi-g++
if [ -e /home/ernie ] ; then

export MY_CC=/usr/local/arm-gnu-toolchain-14.3.rel1-x86_64-arm-none-eabi/bin/arm-none-eabi-gcc
export MY_CXX=/usr/local/arm-gnu-toolchain-14.3.rel1-x86_64-arm-none-eabi/bin/arm-none-eabi-g++
export MY_CLANG=/home/ernie/llvm-arm/bin/clang
export MY_CLANGXX=/home/ernie/llvm-arm/bin/clang++

else

export MY_CC=/home/rr/arm-gnu-toolchain-14.3.rel1-x86_64-arm-none-eabi/bin/arm-none-eabi-gcc
export MY_CXX=/home/rr/arm-gnu-toolchain-14.3.rel1-x86_64-arm-none-eabi/bin/arm-none-eabi-g++
export MY_CLANG=/home/rr/llvm-arm/bin/clang
export MY_CLANGXX=/home/rr/llvm-arm/bin/clang++

fi

if false ; then
if [[ "$TARGET" = "" || "$TARGET" == "stm32" ]] ; then
echo "Building STM32 NUCLEO ..."
#CC=/home/rr/gcc-arm-none-eabi-10.3-2021.10/bin/arm-none-eabi-gcc \
#CXX=/home/rr/gcc-arm-none-eabi-10.3-2021.10/bin/arm-none-eabi-g++
CC=$MY_CC \
CXX=$MY_CXX \
cmake --preset nucleo-g474re-freertos-gcc -DCMAKE_CXX_STANDARD=$CPP_STANDARD
cmake --build --preset nucleo-g474re-freertos-gcc --verbose -j $CORES
fi
fi

if [[ "$TARGET" = "" || "$TARGET" = "pi" ]] ; then
# Pi Pico
CC=$MY_CC \
CXX=$MY_CXX \
cmake --preset pipico-freertos-gcc -DCMAKE_CXX_STANDARD=$CPP_STANDARD
cmake --build --preset pipico-freertos-gcc --verbose -j $CORES
fi

if [[ "$TARGET" = "" || "$TARGET" = "pi-clang" ]] ; then
# Pi Pico
CC=$MY_CLANG \
CXX=$MY_CLANGXX \
cmake --preset pipico-freertos-clang -DCMAKE_CXX_STANDARD=$CPP_STANDARD
cmake --build --preset pipico-freertos-clang --verbose -j $CORES
fi

if [[ "$TARGET" = "" || "$TARGET" = "pi-threadx" ]] ; then
# Pi Pico
CC=$MY_CC \
CXX=$MY_CXX \
cmake --preset pipico-threadx-gcc -DCMAKE_CXX_STANDARD=$CPP_STANDARD
cmake --build --preset pipico-threadx-gcc --verbose -j $CORES
fi

if [[ "$TARGET" = "" || "$TARGET" = "pi-threadx-clang" ]] ; then
# Pi Pico
CC=$MY_CLANG \
CXX=$MY_CLANGXX \
cmake --preset pipico-threadx-clang -DCMAKE_CXX_STANDARD=$CPP_STANDARD
cmake --build --preset pipico-threadx-clang --verbose -j $CORES
fi

if [[ "$TARGET" = "" || "$TARGET" = "pi2" ]] ; then
# Pi Pico
CC=$MY_CC \
CXX=$MY_CXX \
cmake --preset pico2-freertos-gcc -DCMAKE_CXX_STANDARD=$CPP_STANDARD
cmake --build --preset pico2-freertos-gcc --verbose -j $CORES
fi

if [[ "$TARGET" = "" || "$TARGET" = "pi2-clang" ]] ; then
# Pi Pico
CC=$MY_CLANG \
CXX=$MY_CLANGXX \
cmake --preset pico2-freertos-clang -DCMAKE_CXX_STANDARD=$CPP_STANDARD
cmake --build --preset pico2-freertos-clang --verbose -j $CORES
fi

if [[ "$TARGET" = "" || "$TARGET" = "pi2-threadx" ]] ; then
# Pi Pico
CC=$MY_CC \
CXX=$MY_CXX \
cmake --preset pico2-threadx-gcc -DCMAKE_CXX_STANDARD=$CPP_STANDARD
cmake --build --preset pico2-threadx-gcc --verbose -j $CORES
fi

if [[ "$TARGET" = "" || "$TARGET" = "pi2-threadx-clang" ]] ; then
# Pi Pico
CC=$MY_CLANG \
CXX=$MY_CLANGXX \
cmake --preset pico2-threadx-clang -DCMAKE_CXX_STANDARD=$CPP_STANDARD
cmake --build --preset pico2-threadx-clang --verbose -j $CORES
fi

if [[ "$TARGET" = "" || "$TARGET" == "s32" ]] ; then
echo "Building S32K148 ..."
#CC=/home/rr/gcc-arm-none-eabi-10.3-2021.10/bin/arm-none-eabi-gcc \
#CXX=/home/rr/gcc-arm-none-eabi-10.3-2021.10/bin/arm-none-eabi-g++
CC=$MY_CC \
CXX=$MY_CXX \
cmake --preset s32k148-freertos-gcc -DCMAKE_CXX_STANDARD=$CPP_STANDARD
cmake --build --preset s32k148-freertos-gcc --verbose -j $CORES
fi

if [[ "$TARGET" = "" || "$TARGET" == "s32-clang" ]] ; then
echo "Building S32K148 clang ..."
#CC=/home/rr/LLVM-ET-Arm-19.1.5-Linux-x86_64/bin/clang \
#CXX=/home/rr/LLVM-ET-Arm-19.1.5-Linux-x86_64/bin/clang++
#CC=/home/rr/ATfE-21.1.1-Linux-x86_64/bin/clang \
#CXX=/home/rr/ATfE-21.1.1-Linux-x86_64/bin/clang++ \
CC=$MY_CLANG \
CXX=$MY_CLANGXX \
cmake --preset s32k148-freertos-clang -DCMAKE_CXX_STANDARD=$CPP_STANDARD
cmake --build --preset s32k148-freertos-clang --verbose -j $CORES
fi

if [[ "$TARGET" = "" || "$TARGET" == "s32-threadx" ]] ; then
echo "Building S32K148 threadx ..."
CC=$MY_CC \
CXX=$MY_CXX \
cmake --preset s32k148-threadx-gcc -DCMAKE_CXX_STANDARD=$CPP_STANDARD
cmake --build --preset s32k148-threadx-gcc --verbose -j $CORES
fi

if [[ "$TARGET" = "" || "$TARGET" == "s32-threadx-clang" ]] ; then
echo "Building S32K148 threadx clang ..."
CC=$MY_CLANG \
CXX=$MY_CLANGXX \
cmake --preset s32k148-threadx-clang -DCMAKE_CXX_STANDARD=$CPP_STANDARD
cmake --build --preset s32k148-threadx-clang --verbose -j $CORES
fi

if [[ "$TARGET" = "" || "$TARGET" == "posix" ]] ; then
echo "Building POSIX ..."
cmake --preset posix-freertos -DCMAKE_CXX_STANDARD=$CPP_STANDARD
cmake --build --preset posix-freertos --verbose -j $CORES
fi

if [[ "$TARGET" = "" || "$TARGET" == "posix-threadx" ]] ; then
echo "Building POSIX with threadx ..."
cmake --preset posix-threadx -DCMAKE_CXX_STANDARD=$CPP_STANDARD
cmake --build --preset posix-threadx --verbose -j $CORES
fi

if [[ "$TARGET" = "" || "$TARGET" == "ut" ]] ; then
echo "Building Unit Tests ..."
for i in posix s32k1xx ; do
cmake --preset tests-$i-debug -DCMAKE_CXX_STANDARD=$CPP_STANDARD
#-DCMAKE_CXX_FLAGS="-DOPENBSW_NO_IPV6=1"
cmake --build --preset tests-$i-debug --verbose -j $CORES
ctest --preset tests-$i-debug --parallel
done
fi

if [[ "$TARGET" = "" || "$TARGET" = "all" || "$TARGET" == "ut-clang" ]] ; then
echo "Building Unit Tests with clang ..."
for i in posix s32k1xx ; do
CC=clang \
CXX=clang++ \
cmake --preset tests-$i-debug -DCMAKE_CXX_STANDARD=$CPP_STANDARD
cmake --build --preset tests-$i-debug --verbose -j $CORES
ctest --preset tests-$i-debug --parallel
done
fi

#if [[ "$TARGET" = "" || "$TARGET" == "posix-rust" ]] ; then
#echo "Building POSIX GCC FreeRTOS Rust..."
#cmake --preset posix-rust -DCMAKE_CXX_STANDARD=$CPP_STANDARD
#cmake --build --preset posix-rust --verbose -j $CORES
#fi

#if [[ "$TARGET" = "" || "$TARGET" == "s32-rust" ]] ; then
#echo "Building S32K148 GCC FreeRTOS Rust ..."
#CC=/home/rr/arm-gnu-toolchain-14.3.rel1-x86_64-arm-none-eabi/bin/arm-none-eabi-gcc \
#CXX=/home/rr/arm-gnu-toolchain-14.3.rel1-x86_64-arm-none-eabi/bin/arm-none-eabi-g++ \
#cmake --preset s32k148-rust-gcc -DCMAKE_CXX_STANDARD=$CPP_STANDARD
#cmake --build --preset s32k148-rust-gcc --verbose
#fi

if [[ "$TARGET" = "" || "$TARGET" = "bazel" ]] ; then
bazel run //:format_check
bazel query //...
bazel build //...
#bazel build --config=s32k148 //...
#bazel test //...
fi

echo "All tests successful."
