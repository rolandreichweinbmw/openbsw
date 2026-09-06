#!/bin/bash

TARGET=$1

set -e

CORES=8

#export CC=clang
#export CXX=clang++
#export CPP_STANDARD=17
export CPP_STANDARD=17

if false ; then
if [[ "$TARGET" = "" || "$TARGET" == "stm32" ]] ; then
echo "Building STM32 NUCLEO ..."
#CC=/home/rr/gcc-arm-none-eabi-10.3-2021.10/bin/arm-none-eabi-gcc \
#CXX=/home/rr/gcc-arm-none-eabi-10.3-2021.10/bin/arm-none-eabi-g++
CC=/home/rr/arm-gnu-toolchain-14.3.rel1-x86_64-arm-none-eabi/bin/arm-none-eabi-gcc \
CXX=/home/rr/arm-gnu-toolchain-14.3.rel1-x86_64-arm-none-eabi/bin/arm-none-eabi-g++ \
cmake --preset nucleo-g474re-freertos-gcc -DCMAKE_CXX_STANDARD=$CPP_STANDARD
cmake --build --preset nucleo-g474re-freertos-gcc --verbose
fi
fi

if [[ "$TARGET" = "" || "$TARGET" == "s32" ]] ; then
echo "Building S32K148 ..."
#CC=/home/rr/gcc-arm-none-eabi-10.3-2021.10/bin/arm-none-eabi-gcc \
#CXX=/home/rr/gcc-arm-none-eabi-10.3-2021.10/bin/arm-none-eabi-g++
CC=/home/rr/arm-gnu-toolchain-14.3.rel1-x86_64-arm-none-eabi/bin/arm-none-eabi-gcc \
CXX=/home/rr/arm-gnu-toolchain-14.3.rel1-x86_64-arm-none-eabi/bin/arm-none-eabi-g++ \
cmake --preset s32k148-freertos-gcc -DCMAKE_CXX_STANDARD=$CPP_STANDARD
cmake --build --preset s32k148-freertos-gcc --verbose
fi

if [[ "$TARGET" = "" || "$TARGET" == "s32-clang" ]] ; then
echo "Building S32K148 clang ..."
#CC=/home/rr/LLVM-ET-Arm-19.1.5-Linux-x86_64/bin/clang \
#CXX=/home/rr/LLVM-ET-Arm-19.1.5-Linux-x86_64/bin/clang++
#CC=/home/rr/ATfE-21.1.1-Linux-x86_64/bin/clang \
#CXX=/home/rr/ATfE-21.1.1-Linux-x86_64/bin/clang++ \
CC=/home/rr/llvm-arm/bin/clang \
CXX=/home/rr/llvm-arm/bin/clang++ \
cmake --preset s32k148-freertos-clang -DCMAKE_CXX_STANDARD=$CPP_STANDARD
cmake --build --preset s32k148-freertos-clang --verbose
fi

if [[ "$TARGET" = "" || "$TARGET" == "s32-threadx" ]] ; then
echo "Building S32K148 threadx ..."
CC=/home/rr/arm-gnu-toolchain-14.3.rel1-x86_64-arm-none-eabi/bin/arm-none-eabi-gcc \
CXX=/home/rr/arm-gnu-toolchain-14.3.rel1-x86_64-arm-none-eabi/bin/arm-none-eabi-g++ \
cmake --preset s32k148-threadx-gcc -DCMAKE_CXX_STANDARD=$CPP_STANDARD
cmake --build --preset s32k148-threadx-gcc --verbose
fi

if [[ "$TARGET" = "" || "$TARGET" == "s32-threadx-clang" ]] ; then
echo "Building S32K148 threadx clang ..."
CC=/home/rr/llvm-arm/bin/clang \
CXX=/home/rr/llvm-arm/bin/clang++ \
cmake --preset s32k148-threadx-clang -DCMAKE_CXX_STANDARD=$CPP_STANDARD
cmake --build --preset s32k148-threadx-clang --verbose
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
cmake --build --preset tests-$i-debug --verbose
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
