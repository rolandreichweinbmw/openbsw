include_guard(GLOBAL)

include("${CMAKE_CURRENT_LIST_DIR}/ArmNoneEabi-header.cmake")

set(CLANG_TARGET_TRIPLE "${ARM_TARGET_TRIPLE}")

if (NOT DEFINED CMAKE_C_COMPILER)
    if (NOT DEFINED ENV{CC})
        message(FATAL_ERROR "C compiler unspecified")
    endif ()

    set(CMAKE_C_COMPILER $ENV{CC})
endif ()

cmake_path(GET CMAKE_C_COMPILER PARENT_PATH TOOLCHAIN_BIN_DIR)
cmake_path(GET TOOLCHAIN_BIN_DIR PARENT_PATH TOOLCHAIN_PREFIX)

set(CMAKE_CXX_COMPILER "${TOOLCHAIN_BIN_DIR}/clang++")
set(CMAKE_ASM_COMPILER "${TOOLCHAIN_BIN_DIR}/clang")
set(CMAKE_NM "${TOOLCHAIN_BIN_DIR}/llvm-nm")
set(CMAKE_OBJCOPY "${TOOLCHAIN_BIN_DIR}/llvm-objcopy")
set(CMAKE_OBJDUMP "${TOOLCHAIN_BIN_DIR}/llvm-objdump")
set(CMAKE_RANLIB "${TOOLCHAIN_BIN_DIR}/llvm-ranlib")
set(CMAKE_SIZE "${TOOLCHAIN_BIN_DIR}/llvm-size")
set(CMAKE_STRIP "${TOOLCHAIN_BIN_DIR}/llvm-strip")
set(CMAKE_AR "${TOOLCHAIN_BIN_DIR}/llvm-ar")
set(CMAKE_C_COMPILER_TARGET ${CLANG_TARGET_TRIPLE})
set(CMAKE_CXX_COMPILER_TARGET ${CLANG_TARGET_TRIPLE})
set(CMAKE_ASM_COMPILER_TARGET ${CLANG_TARGET_TRIPLE})

set(CMAKE_SYSROOT
    "${TOOLCHAIN_PREFIX}/lib/clang-runtimes/${CLANG_TARGET_TRIPLE}"
)

set(_CXX_FLAGS "-nostdlib")

set(_EXE_LINKER_FLAGS
    "-v -Wl,--start-group \
    -L${TOOLCHAIN_PREFIX}/lib/clang-runtimes/${CLANG_TARGET_TRIPLE}/armv7m_soft_fpv4_sp_d16_unaligned_size/lib \
    -B${TOOLCHAIN_PREFIX}/lib/clang-runtimes/${CLANG_TARGET_TRIPLE}/armv7m_soft_fpv4_sp_d16_unaligned_size/lib \
    -L /home/rr/ATfE-21.1.1-Linux-x86_64/lib/clang-runtimes/llvmlibc/arm-none-eabi/armv7m_soft_fpv4_sp_d16_unaligned_size/lib \
    /home/rr/ATfE-21.1.1-Linux-x86_64/lib/clang-runtimes/llvmlibc/arm-none-eabi/armv7m_soft_fpv4_sp_d16_unaligned_size/lib/libclang_rt.builtins.a \
    /home/rr/ATfE-21.1.1-Linux-x86_64/lib/clang-runtimes/llvmlibc/arm-none-eabi/armv7m_soft_fpv4_sp_d16_unaligned_size/lib/libc.a \
    /home/rr/ATfE-21.1.1-Linux-x86_64/lib/clang-runtimes/llvmlibc/arm-none-eabi/armv7m_soft_fpv4_sp_d16_unaligned_size/lib/libcrt0.a \
    -nostdlib \
    -Wl,--end-group")

include("${CMAKE_CURRENT_LIST_DIR}/ArmNoneEabi.cmake")
