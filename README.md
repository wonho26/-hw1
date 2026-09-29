# 과제 1 — 정렬 비교: 병합 · 퀵 · 힙

2026-2 **고급알고리즘**(SIT2001-01) 과제 1. 정렬 세 가지를 하나의 공통 인터페이스로
묶고, 같은 잣대로 비교 횟수 · 이동 횟수 · 시간 · 추가 메모리 · 재귀 깊이 · 안정성을 쟀다.

| 정렬 | 출처 | 평균 | 최악 | 추가 메모리 | 안정 |
| --- | --- | --- | --- | --- | --- |
| 병합 정렬 `mergeSort` | 수업 (주제 03) | O(n log n) | O(n log n) | O(n) | 예 |
| 퀵 정렬 `quickSort` (랜덤 피벗) | 수업 (주제 04) | O(n log n) | O(n²) | 스택 O(log n) | 아니오 |
| 힙 정렬 `heapSort` | **수업 밖** (Wikipedia 비교표) | O(n log n) | O(n log n) | O(1) | 아니오 |

- 보고서: [report/REPORT.md](report/REPORT.md) (제출본은 PDF)
- 이 저장소는 [lec-algorithm/algorithm-env](https://github.com/lec-algorithm/algorithm-env)를
  **Use this template**으로 만들었고, 뼈대(공통 인터페이스 · 측정 도구 · 그래프 도구)는
  교수님 샘플 [hw1-sample-2026](https://github.com/lec-algorithm/hw1-sample-2026)의 구조를 따랐다.

## 돌려보기

Codespaces(**Code → Codespaces → Create codespace on main**)나 `docker compose`
컨테이너 안에서:

```bash
make test     # 유닛 테스트 45개
make run      # 비교 표
make charts   # 측정을 다시 돌려 report/ 아래 CSV와 SVG 그래프를 새로 만든다
./src/main.out --dups   # 중복 key 실험만 CSV로
./src/main.out --pivot  # 퀵 정렬 피벗(첫 원소 / 랜덤) 실험만 CSV로
```

편집기에서는 파일을 열고 오른쪽 위 **▶ 버튼**(Code Runner)으로 실행한다.

## 구조

```text
src/
├── sort.h · sortctx.h · sort.c   # 공통 인터페이스(SortAlgorithm) · 공용 도구 · 구현 표
├── mergeSort.c                   # 병합 정렬 (temp 배열 O(n))
├── quickSort.c                   # 퀵 정렬 (Lomuto 파티션 + 시드 고정 랜덤 피벗)
├── heapSort.c                    # 힙 정렬 (최대 힙, 반복문 sift-down)
├── bench.h · bench.c             # 입력 생성 · 시간 · 안정성 측정
└── main.c                        # 비교 표 (--csv, --dups, --pivot)
tests/test_sort.c                 # 유닛 테스트 (표준 C만)
tools/plot.py · svgchart.py       # CSV → SVG 그래프 (표준 모듈만)
report/                           # 보고서 · 그래프 · 측정값 원본(results.csv, duplicates.csv, pivot.csv)
```

정렬을 하나 더 넣으려면 파일을 하나 두고 `src/sort.c`의 `SORT_ALGORITHMS` 표에
한 줄, `Makefile`의 `SORT_SRC`에 한 단어를 더하면 된다. 테스트와 측정은 표를 훑으므로
자동으로 따라온다.

## 규약

- 외부 라이브러리를 쓰지 않는다. C는 표준 라이브러리, Python은 표준 모듈만.
- 실행 파일은 `*.out`으로 만든다. `.gitignore`가 그것만 걸러낸다.
- `-Wall -Wextra` 경고 없이 빌드된다.

## 정리

```bash
docker compose down
```
