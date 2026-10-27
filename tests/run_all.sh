#!/usr/bin/env bash
# 一键构建并运行全部回归用例（不依赖 CMake，只用 g++）。
# 用法：bash tests/run_all.sh [quick|full]
#   quick —— 定向探针 + 小规模对拍（默认，约 10 秒）
#   full  —— 追加百万级压力校验（约 1~2 分钟）
set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

MODE="${1:-quick}"
CXX="${CXX:-g++}"
OUT="${TMPDIR:-/tmp}/alloc-tests"
mkdir -p "$OUT"

SRCS="src/Allocator.cpp src/CAllocator.cpp src/LinearAllocator.cpp src/StackAllocator.cpp src/PoolAllocator.cpp src/FreeListAllocator.cpp"
FLAGS="-std=c++11 -Iincludes -DNDEBUG -O2 -g -Wall -Wextra -Wshadow"

pass=0
fail=0

build() {   # build <输出名> <源文件> [额外宏]
    local name="$1" src="$2" extra="${3:-}"
    # DRY_RUN=1 时不真正编译，只报告 target 名，供 check_build_parity.py
    # 解析用例清单 —— 让 bash 自己展开循环，比用正则解析 shell 可靠得多。
    if [ "${DRY_RUN:-0}" = "1" ]; then
        printf 'TARGET %s\n' "$name"
        return 0
    fi
    # shellcheck disable=SC2086
    if ! $CXX $FLAGS $extra -o "$OUT/$name" "$src" $SRCS 2>"$OUT/$name.log"; then
        echo "[BUILD FAIL] $name"
        sed -n '1,20p' "$OUT/$name.log"
        fail=$((fail + 1))
        return 1
    fi
    return 0
}

run() {     # run <可执行名> <参数串>
    local bin="$1" args="$2"
    if [ "${DRY_RUN:-0}" = "1" ]; then
        printf 'CASE %s %s\n' "$bin" "$args"
        return 0
    fi
    # shellcheck disable=SC2086
    if "$OUT/$bin" $args >"$OUT/out.txt" 2>&1; then
        printf '  [PASS] %-16s %s\n' "$bin" "$args"
        pass=$((pass + 1))
    else
        printf '  [FAIL] %-16s %s\n' "$bin" "$args"
        sed -n '1,30p' "$OUT/out.txt" | sed 's/^/         /'
        fail=$((fail + 1))
    fi
}

echo "== 构建 =="
build probe_defects    tests/probe_defects.cpp                     || true
build probe_extra      tests/probe_extra.cpp      -DALLOCATOR_VALIDATE || true
build probe_round3     tests/probe_round3.cpp                      || true
build probe_residual   tests/probe_residual.cpp   -DALLOCATOR_VALIDATE || true
build probe_round6     tests/probe_round6.cpp     -DALLOCATOR_VALIDATE || true
build fuzz_freelist    tests/fuzz_freelist.cpp                     || true
build fuzz_pool        tests/fuzz_pool.cpp                         || true
build stress_validate  tests/stress_validate.cpp  -DALLOCATOR_VALIDATE || true
build measure_findbest tests/measure_findbest.cpp -DALLOCATOR_VALIDATE || true

echo "== 定向探针：第一轮（P0 级）=="
for c in linear stack pool list lost findbest; do run probe_defects "$c"; done

echo "== 定向探针：第二轮（内存几何）=="
for c in overlap zeroalign rest peak; do run probe_extra "$c"; done

echo "== 定向探针：第三轮（算术与类型契约）=="
for c in overflow overflow_fl poolalign doublefree copyassign; do run probe_round3 "$c"; done

echo "== 定向探针：第四轮（Release 下的运行时契约）=="
for c in stack_order stack_double stack_pop used_semantics pool_strict align_nonpow2 fl_overflow small_overhead; do
    run probe_residual "$c"
done

echo "== 定向探针：第五轮（重复释放与指针归属）=="
for c in pool_double fl_double foreign_ptr zero_size; do
    run probe_residual "$c"
done

echo "== 定向探针：第六轮（死代码补测与容量守恒）=="
for c in doubly callocator pool_capacity; do
    run probe_round6 "$c"
done

echo "== 随机对拍（3 万次/组）=="
for p in first best; do
    for s in 1 2 3; do run fuzz_freelist "$p $s 20000"; done
done
for s in 1 2 3; do run fuzz_pool "$s 20000"; done

echo "== 不变量校验（2 万次/组，逐步校验）=="
for p in first best; do run stress_validate "$p 20251027 20000 uniform 1000"; done

echo "== FindBest 快速失败健全性与策略差距测量 =="
run measure_findbest ""

echo "== 构建清单一致性（CMakeLists 与 run_all.sh 的用例必须同构）=="
# 注意：本检查自身会以 DRY_RUN=1 重跑本脚本，所以在 DRY_RUN 下必须跳过，
# 否则无限递归。
if [ "${DRY_RUN:-0}" = "1" ]; then
    echo "  （DRY_RUN 下跳过，避免递归）"
elif DRY_RUN=1 bash "$0" >"$OUT/parity_dump.txt" 2>&1 \
     && python tests/check_build_parity.py "$OUT/parity_dump.txt"; then
    printf '  [PASS] %-16s %s\n' "check_build_parity" ""
    pass=$((pass + 1))
else
    printf '  [FAIL] %-16s %s\n' "check_build_parity" "两套构建清单不一致"
    tail -n 25 "$OUT/parity_dump.txt" | sed 's/^/         /'
    fail=$((fail + 1))
fi

if [ "$MODE" = "full" ]; then
    echo "== 百万级压力（含逐步不变量校验）=="
    for p in first best; do run stress_validate "$p 20251027 1000000 uniform 5000"; done
    echo "== 大规模随机对拍（40 万次/组）=="
    for p in first best; do
        for s in 11 22; do run fuzz_freelist "$p $s 400000"; done
    done
    for s in 11 22; do run fuzz_pool "$s 400000"; done
fi

echo
echo "通过 $pass 项，失败 $fail 项"
[ "$fail" -eq 0 ] || exit 1
