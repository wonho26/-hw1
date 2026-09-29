"""비교 결과를 그래프(SVG)로 그린다.

    make charts          # 또는
    python3 tools/plot.py

src/main.out --csv 와 --dups 를 돌려 측정값을 받아 report/ 아래에 SVG와
CSV를 쓴다. 사람이 읽는 표를 파싱하지 않고 CSV를 쓰는 이유는, 표의 모양이
바뀌어도 그래프가 깨지지 않게 하려는 것이다.

표준 모듈만 쓴다. 그림은 tools/svgchart.py가 직접 찍어 낸다.
"""

import csv
import io
import math
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import svgchart  # noqa: E402

ROOT = Path(__file__).resolve().parents[1]
BINARY = ROOT / "src" / "main.out"
OUT_DIR = ROOT / "report"
ALGOS = ["mergeSort", "quickSort", "heapSort"]
KIND_KEYS = ["random", "sorted", "reversed", "few-unique"]
KIND_LABEL = {
    "random": "무작위",
    "sorted": "정렬됨",
    "reversed": "역순",
    "few-unique": "중복많음",
}
INT_FIELDS = ("n", "keys", "compares", "moves", "extraBytes", "maxDepth")


def run_csv(flag):
    """측정 프로그램을 돌려 CSV를 읽는다. 없으면 make가 만들게 한다."""
    if not BINARY.exists():
        subprocess.run(["make", "src/main.out"], cwd=ROOT, check=True)
    result = subprocess.run([str(BINARY), flag], cwd=ROOT, check=True,
                            capture_output=True, text=True)
    rows = list(csv.DictReader(io.StringIO(result.stdout)))
    for row in rows:
        for key in INT_FIELDS:
            if key in row:
                row[key] = int(row[key])
        row["millis"] = float(row["millis"])
    return rows


def save_csv(rows, name):
    with open(OUT_DIR / name, "w", encoding="utf-8", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)


def pick(rows, **conditions):
    return [r for r in rows if all(r[k] == v for k, v in conditions.items())]


def by_algo(rows, field, keys, key_field):
    """{알고리즘: [키 순서대로의 값]} 으로 모은다."""
    table = {}
    for algo in ALGOS:
        values = []
        for key in keys:
            match = [r for r in rows if r["algo"] == algo and r[key_field] == key]
            values.append(match[0][field] if match else 0)
        table[algo] = values
    return table


def main():
    OUT_DIR.mkdir(exist_ok=True)
    rows = run_csv("--csv")
    dups = run_csv("--dups")
    # 그래프와 보고서의 표가 같은 실행에서 나오도록 측정값을 그대로 남긴다.
    save_csv(rows, "results.csv")
    save_csv(dups, "duplicates.csv")
    save_csv(run_csv("--pivot"), "pivot.csv")

    growth = pick(rows, scope="growth")
    kinds = pick(rows, scope="kinds")
    sizes = sorted({r["n"] for r in growth})
    shape_labels = [KIND_LABEL[k] for k in KIND_KEYS]
    made = []

    # 1. n에 따라 자라는 모양 — 로그-로그에서 기울기가 곧 복잡도 지수다
    made.append(svgchart.line_chart(
        OUT_DIR / "growth-compares-log.svg",
        "n이 커질 때 비교 횟수 — 로그-로그 축",
        "무작위 입력 · 기울기가 곧 복잡도 지수다 (1.0이면 n, 2.0이면 n^2)",
        sizes, by_algo(growth, "compares", sizes, "n"),
        "n (원소 개수)", "비교 횟수", annotate_slope=False))
    made.append(svgchart.line_chart(
        OUT_DIR / "growth-time.svg",
        "n이 커질 때 걸린 시간 — 로그-로그 축",
        "무작위 입력 · 7회 평균 · 세 선이 나란하다(같은 증가율). 간격이 곧 상수배다",
        sizes, by_algo(growth, "millis", sizes, "n"),
        "n (원소 개수)", "시간 (ms)", annotate_slope=False))

    # 2. 비교 횟수를 n log2 n으로 나눈 값 — Big-O가 지운 상수를 되살린다
    ratio = {}
    for algo in ALGOS:
        ratio[algo] = [
            round(pick(growth, algo=algo, n=n)[0]["compares"] / (n * math.log2(n)), 2)
            for n in sizes]
    made.append(svgchart.grouped_bar_chart(
        OUT_DIR / "growth-constant.svg",
        "비교 횟수 ÷ n log₂ n — Big-O가 지운 상수",
        "무작위 입력 · 막대가 n이 커져도 평평하면 n log n이다. 높이가 곧 상수배다",
        [f"{n:,}" for n in sizes], ratio, "비교 ÷ n log₂ n",
        value_label=lambda v: f"{v:.2f}"))

    # 3. 입력 모양별 (n = 4,000)
    made.append(svgchart.grouped_bar_chart(
        OUT_DIR / "input-shapes-compares-log.svg",
        "입력 모양에 따른 비교 횟수 — 로그 축",
        "n = 4,000 · 눈금 한 칸이 10배다. 퀵 정렬만 중복많음에서 한 자릿수 이상 뛴다",
        shape_labels, by_algo(kinds, "compares", KIND_KEYS, "input"),
        "비교 횟수", log_scale=True))
    made.append(svgchart.grouped_bar_chart(
        OUT_DIR / "input-shapes-time.svg",
        "입력 모양에 따른 걸린 시간 — 선형 축",
        "n = 4,000 · 7회 평균 · 퀵 정렬은 중복많음에서만 느려진다",
        shape_labels, by_algo(kinds, "millis", KIND_KEYS, "input"),
        "시간 (ms)", value_label=svgchart.ms))

    # 4. 중복 key 실험 — 퀵 정렬의 약점
    keys = sorted({r["keys"] for r in dups})
    made.append(svgchart.line_chart(
        OUT_DIR / "duplicates-compares.svg",
        "서로 다른 key 개수에 따른 비교 횟수 — 로그-로그 축",
        "n = 32,000 · key 종류가 적을수록(왼쪽) 퀵 정렬만 n²으로 무너진다",
        keys, by_algo(dups, "compares", keys, "keys"),
        "서로 다른 key 개수", "비교 횟수", annotate_slope=False))
    made.append(svgchart.line_chart(
        OUT_DIR / "duplicates-depth.svg",
        "서로 다른 key 개수에 따른 재귀 깊이 — 로그-로그 축",
        "n = 32,000 · 병합은 log n으로 고정, 힙은 1, 퀵은 key가 2종이면 16,000까지 간다",
        keys, by_algo(dups, "maxDepth", keys, "keys"),
        "서로 다른 key 개수", "재귀 깊이", annotate_slope=False))

    for path in made:
        print(f"wrote {Path(path).relative_to(ROOT)}")


if __name__ == "__main__":
    main()
