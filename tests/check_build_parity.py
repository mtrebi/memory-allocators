#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""构建清单一致性检查（check_build_parity）

背景：本机没有 cmake（pip 源 502，装不上），CMakeLists.txt 从未在本地跑通，
回归全部走 tests/run_all.sh。两套清单靠人肉同步，每加一个用例要改两处，
是现实隐患。cmake 跑不了，就退而求其次守住"不会漂移"：

  1. run_all.sh 以 DRY_RUN=1 运行，由 bash 自己展开所有循环，输出
     TARGET / CASE 清单 —— 比用正则解析 shell 可靠得多。
  2. CMakeLists.txt 侧做结构自检（块命令配对、括号、add_test 引用的
     target 必须存在、被测源文件必须存在），并递归展开嵌套 foreach
     得到完整用例名。
  3. 两边集合比对，必须完全一致。

它不能证明 cmake 能跑，真正的验证仍依赖 CI；但"两套清单漂移"和
"CMakeLists 低级结构错误"这两类最常见的问题从此有测试守着。

用法：python tests/check_build_parity.py <dump文件>
      dump 文件由 run_all.sh 生成：DRY_RUN=1 bash tests/run_all.sh > dump.txt

为什么不由本脚本去调用 bash：Windows 上 `bash` 在 PATH 里可能解析到
System32 的 WSL bash（会被安全策略拦截），而不是当前正在用的 Git Bash。
让 run_all.sh 用 `bash "$0"` 调自己最可靠 —— 一定是同一个 bash。

退出码：0 一致；1 有问题
"""
import io
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CMAKE = os.path.join(ROOT, "CMakeLists.txt")
TESTS_DIR = os.path.join(ROOT, "tests")

problems = []


def err(msg):
    problems.append(msg)


def read(path):
    with io.open(path, encoding="utf-8") as f:
        return f.read()


def first_arg(line):
    """取 CMake 命令括号内的第一个参数；失败返回 None。"""
    i = line.find("(")
    j = line.rfind(")")
    if i < 0 or j <= i:
        return None
    inner = line[i + 1:j].strip()
    if not inner:
        return None
    return inner.split()[0].strip('"')


# run_all.sh 不构建的 target：
#   main       —— 基准程序，不属于回归
#   allocators —— 静态库，被测试 target 链接，自身没有可运行的用例
NOT_IN_REGRESSION = {"main", "allocators"}


# ------------------------------------------------- CMakeLists 结构自检
OPENER = {"if": "endif", "foreach": "endforeach", "function": "endfunction",
          "macro": "endmacro", "while": "endwhile"}


def check_cmake_structure(text):
    """返回 target 集合，同时做块配对 / 括号 / add_test 引用检查。"""
    stack = []
    brace = 0
    targets = set()
    command_refs = []

    for lineno, raw in enumerate(text.splitlines(), 1):
        line = raw.split("#", 1)[0].strip()
        if not line:
            continue
        brace += line.count("(") - line.count(")")
        if brace < 0:
            err("CMakeLists.txt:%d 括号不配对（多出右括号）" % lineno)

        m = re.match(r"(\w+)\s*\(", line)
        if not m:
            continue
        cmd = m.group(1).lower()

        if cmd in OPENER:
            stack.append((cmd, lineno))
        elif cmd in OPENER.values():
            if not stack:
                err("CMakeLists.txt:%d 多余的 %s" % (lineno, cmd))
                continue
            opener, open_line = stack.pop()
            if OPENER[opener] != cmd:
                err("CMakeLists.txt:%d 的 %s 与第 %d 行的 %s 不配对"
                    % (lineno, cmd, open_line, opener))
        elif cmd in ("add_executable", "add_library"):
            first = first_arg(line)
            # 函数体里的 add_executable(${name} ...) 用的是变量，跳过；
            # 用全文 findall 会把参数名 name/source 误当成 target
            if first is None or "${" in first:
                continue
            targets.add(first)
        elif cmd.startswith("add_allocator_test"):
            # 自定义函数：调用即创建同名 target，与 add_executable 等价
            first = first_arg(line)
            if first is None or "${" in first:
                continue
            targets.add(first)
        elif cmd == "add_test":
            cm = re.search(r"COMMAND\s+(\w+)", line)
            if cm:
                command_refs.append((lineno, cm.group(1)))

    for opener, open_line in stack:
        err("CMakeLists.txt:%d 的 %s 没有闭合" % (open_line, opener))
    if brace != 0:
        err("CMakeLists.txt 括号总数不配对（差 %d）" % brace)

    for lineno, ref in command_refs:
        if ref not in targets:
            err("CMakeLists.txt:%d add_test 引用了未定义的 target: %s"
                % (lineno, ref))
    return targets


# --------------------------------------------- 展开 CMake 嵌套 foreach
FOREACH_RE = re.compile(
    r"foreach\s*\(\s*(\w+)\s+([^)]*)\)"
    r"((?:(?!foreach\b|endforeach\b).)*?)"
    r"endforeach\s*\(\s*\)", re.S)


def expand_foreach(text):
    """由内向外反复展开 foreach，直到没有为止。"""
    while True:
        m = FOREACH_RE.search(text)
        if not m:
            return text
        var, items, body = m.group(1), m.group(2).split(), m.group(3)
        expanded = "".join(body.replace("${%s}" % var, it) for it in items)
        text = text[:m.start()] + expanded + text[m.end():]


def cmake_tests(text):
    return set(re.findall(r"add_test\s*\(\s*NAME\s+([\w.$]+)", expand_foreach(text)))


# ------------------------------------------------- run_all.sh 用例清单
def parse_dump(path):
    """解析 DRY_RUN 输出：TARGET 行给出 target，CASE 行给出 (可执行名, 参数串)。"""
    text = read(path)
    targets = set(re.findall(r"^TARGET (\S+)", text, re.M))
    cases = set()
    for m in re.finditer(r"^CASE (\S+)(.*)$", text, re.M):
        cases.add((m.group(1), m.group(2).strip()))
    return targets, cases


def main(argv):
    if len(argv) < 2:
        print("缺少 dump 文件参数。请这样运行：")
        print("  DRY_RUN=1 bash tests/run_all.sh > dump.txt")
        print("  python tests/check_build_parity.py dump.txt")
        print("（run_all.sh 在一致性检查步骤里会自动完成上述两步）")
        return 1

    cmake_text = read(CMAKE)

    # 1. 结构自检 + target 收集
    targets = check_cmake_structure(cmake_text)

    # 2. 被测源文件存在性
    for m in re.finditer(r"add_allocator_test\w*\s*\(\s*(\w+)\s+tests/([\w.]+)",
                         cmake_text):
        if not os.path.isfile(os.path.join(TESTS_DIR, m.group(2))):
            err("CMakeLists.txt 引用的测试源文件不存在: tests/%s" % m.group(2))

    # 3. CMake 与 run_all.sh 的用例比对
    sh_targets, sh_cases = parse_dump(argv[1])

    expanded = expand_foreach(cmake_text)
    cmake_pairs = set()
    for m in re.finditer(
            r"add_test\s*\(\s*NAME\s+([\w.$]+)\s+COMMAND\s+(\w+)\s*(.*?)\)",
            expanded):
        cmake_pairs.add((m.group(2), m.group(3).strip()))

    only_cmake = sorted(cmake_pairs - sh_cases)
    only_sh = sorted(sh_cases - cmake_pairs)
    if only_cmake:
        err("只在 CMakeLists.txt 里、run_all.sh 没跑的用例: %s"
            % ", ".join("%s(%s)" % p for p in only_cmake))
    if only_sh:
        err("只在 run_all.sh 里、CMakeLists.txt 没登记的用例: %s"
            % ", ".join("%s(%s)" % p for p in only_sh))

    # 4. target 双向比对（豁免基准程序等不属于回归的 target）
    extra = targets - NOT_IN_REGRESSION
    if sh_targets - extra:
        err("run_all.sh 构建了但 CMakeLists.txt 没有对应 target: %s"
            % ", ".join(sorted(sh_targets - extra)))
    if extra - sh_targets:
        err("CMakeLists.txt 定义了 target 但 run_all.sh 没有构建: %s"
            % ", ".join(sorted(extra - sh_targets)))

    print("CMakeLists.txt target:      %d 个" % len(targets))
    print("CMake add_test 展开后:      %d 个用例" % len(cmake_pairs))
    print("run_all.sh 实际运行:        %d 个用例" % len(sh_cases))
    if not problems:
        print("两套清单一致，结构检查通过")
        return 0
    print("\n发现 %d 个问题:" % len(problems))
    for p in problems:
        print("  - %s" % p)
    return 1


if __name__ == "__main__":
    sys.exit(main(sys.argv))
