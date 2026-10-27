cmake_minimum_required(VERSION 3.10)
project(memory-allocators LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 未指定构建类型时默认 Release：基准测试若在 -O0 下跑，数据没有参考价值
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# D-23: 原 CMakeLists 未开启任何告警，14 条编译告警在默认构建中被静默吞掉
if(MSVC)
    set(ALLOCATOR_WARNINGS /W4)
else()
    set(ALLOCATOR_WARNINGS -Wall -Wextra -Wshadow)
endif()

# 分配器本体做成静态库，供 main 与各测试共用
set(ALLOCATOR_SOURCES
    src/Allocator.cpp
    src/CAllocator.cpp
    src/LinearAllocator.cpp
    src/StackAllocator.cpp
    src/PoolAllocator.cpp
    src/FreeListAllocator.cpp)

add_library(allocators STATIC ${ALLOCATOR_SOURCES})
target_include_directories(allocators PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/includes)
target_compile_options(allocators PRIVATE ${ALLOCATOR_WARNINGS})

add_executable(main src/main.cpp src/Benchmark.cpp)
target_link_libraries(main PRIVATE allocators)
target_compile_options(main PRIVATE ${ALLOCATOR_WARNINGS})

# ---------------------------------------------------------------- 测试

option(ALLOCATOR_BUILD_TESTS "Build the regression test suite" ON)

if(ALLOCATOR_BUILD_TESTS)
    enable_testing()

    # 测试统一以 NDEBUG 构建：模拟 Release，暴露"只靠 assert 做错误处理"的后果。
    # 其中 probe_extra / stress_validate 额外需要 ALLOCATOR_VALIDATE，
    # 它们把分配器源码直接编进可执行文件，避免与 allocators 库产生 ODR 冲突。
    set(TEST_WARNINGS ${ALLOCATOR_WARNINGS})

    function(add_allocator_test name source)
        add_executable(${name} ${source} ${ALLOCATOR_SOURCES})
        target_include_directories(${name} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/includes)
        target_compile_definitions(${name} PRIVATE NDEBUG)
        target_compile_options(${name} PRIVATE ${TEST_WARNINGS})
        # 使用相对路径定位源码，便于从任意构建目录运行
        set_target_properties(${name} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/tests)
    endfunction()

    function(add_allocator_test_validate name source)
        add_allocator_test(${name} ${source})
        target_compile_definitions(${name} PRIVATE ALLOCATOR_VALIDATE)
    endfunction()

    add_allocator_test(probe_defects   tests/probe_defects.cpp)
    add_allocator_test(probe_round3    tests/probe_round3.cpp)
    add_allocator_test_validate(probe_round6  tests/probe_round6.cpp)
    add_allocator_test(fuzz_freelist   tests/fuzz_freelist.cpp)
    add_allocator_test(fuzz_pool       tests/fuzz_pool.cpp)
    add_allocator_test_validate(probe_extra     tests/probe_extra.cpp)
    add_allocator_test_validate(probe_residual  tests/probe_residual.cpp)
    add_allocator_test_validate(stress_validate tests/stress_validate.cpp)
    add_allocator_test_validate(measure_findbest tests/measure_findbest.cpp)

    # --- 定向探针：第一轮（P0 级）---
    foreach(c linear stack pool list lost findbest)
        add_test(NAME probe_defects.${c}
                 COMMAND probe_defects ${c})
    endforeach()

    # --- 定向探针：第二轮（内存几何）---
    foreach(c overlap zeroalign rest peak)
        add_test(NAME probe_extra.${c}
                 COMMAND probe_extra ${c})
    endforeach()

    # --- 定向探针：第三轮（算术与类型契约）---
    foreach(c overflow overflow_fl poolalign doublefree copyassign)
        add_test(NAME probe_round3.${c}
                 COMMAND probe_round3 ${c})
    endforeach()

    # --- 定向探针：第四轮（Release 下的运行时契约）---
    foreach(c stack_order stack_double stack_pop used_semantics
              pool_strict align_nonpow2 fl_overflow small_overhead)
        add_test(NAME probe_residual.${c}
                 COMMAND probe_residual ${c})
    endforeach()

    # --- 定向探针：第五轮（重复释放与指针归属）---
    foreach(c pool_double fl_double foreign_ptr zero_size)
        add_test(NAME probe_residual.${c}
                 COMMAND probe_residual ${c})
    endforeach()

    # --- 定向探针：第六轮（死代码补测与容量守恒）---
    foreach(c doubly callocator pool_capacity)
        add_test(NAME probe_round6.${c}
                 COMMAND probe_round6 ${c})
    endforeach()

    # --- 随机对拍 ---
    foreach(policy first best)
        foreach(seed 1 2 3)
            add_test(NAME fuzz_freelist.${policy}.${seed}
                     COMMAND fuzz_freelist ${policy} ${seed} 20000)
        endforeach()
    endforeach()
    foreach(seed 1 2 3)
        add_test(NAME fuzz_pool.${seed}
                 COMMAND fuzz_pool ${seed} 20000)
    endforeach()

    # --- 不变量校验（CI 用规模；压力规模见 tests/run_all.sh full）---
    foreach(policy first best)
        add_test(NAME stress_validate.${policy}
                 COMMAND stress_validate ${policy} 20251027 20000 uniform 1000)
    endforeach()

    # --- FindBest 快速失败健全性与策略差距测量（计时只展示，不作为门槛）---
    add_test(NAME measure_findbest COMMAND measure_findbest)

    # 测试可能耗时数十秒，默认超时放宽
    set_tests_properties(
        fuzz_freelist.first.1 fuzz_freelist.first.2 fuzz_freelist.first.3
        fuzz_freelist.best.1  fuzz_freelist.best.2  fuzz_freelist.best.3
        fuzz_pool.1 fuzz_pool.2 fuzz_pool.3
        stress_validate.first stress_validate.best
        PROPERTIES TIMEOUT 300)
endif()
