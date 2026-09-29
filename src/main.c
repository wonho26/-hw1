/* 정렬 비교 — 병합 / 퀵 / 힙.
 *
 *   make run                 사람이 읽는 비교 표
 *   ./src/main.out --csv     같은 측정을 CSV로 (tools/plot.py가 쓴다)
 *   ./src/main.out --dups    서로 다른 key 개수를 바꿔 가며 잰 CSV
 *   ./src/main.out --pivot   퀵 정렬의 피벗을 첫 원소 / 랜덤으로 바꿔 잰 CSV
 *
 * 부르는 쪽은 정렬 이름을 하나도 적지 않는다. 구현 표(SORT_ALGORITHMS)를
 * 훑을 뿐이다. 무엇을 잴지도 아래 SPECS 한 곳에만 적는다.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bench.h"
#include "sort.h"

/* --- 무엇을 잴 것인가 -------------------------------------------------- */

typedef struct Spec {
    const char *scope; /* kinds: 입력 모양별 · growth: n을 키우며 */
    InputKind kind;
    size_t n;
    int reps;
} Spec;

static const Spec SPECS[] = {
    {"kinds", INPUT_RANDOM, 4000, 7},
    {"kinds", INPUT_SORTED, 4000, 7},
    {"kinds", INPUT_REVERSED, 4000, 7},
    {"kinds", INPUT_FEW_UNIQUE, 4000, 7},
    {"growth", INPUT_RANDOM, 1000, 7},
    {"growth", INPUT_RANDOM, 4000, 7},
    {"growth", INPUT_RANDOM, 16000, 7},
    {"growth", INPUT_RANDOM, 64000, 7},
    {"growth", INPUT_RANDOM, 256000, 7},
};

static const size_t SPEC_COUNT = sizeof(SPECS) / sizeof(SPECS[0]);

/* 측정 결과 한 줄을 받아 가는 곳. 표로 찍을지 CSV로 찍을지만 다르다 —
 * 정렬을 함수 포인터로 갈아 끼웠듯, 출력도 같은 수를 쓴다. */
typedef void (*RowSink)(const Spec *spec, const BenchResult *r);

/* SPECS를 훑으며 측정하고, 한 줄이 나올 때마다 sink에 넘긴다.
 * onSpec은 줄을 찍기 전에 불린다 (표가 소제목을 낼 자리). */
static void measureAll(RowSink sink, void (*onSpec)(const Spec *spec)) {
    for (size_t s = 0; s < SPEC_COUNT; s++) {
        const Spec *spec = &SPECS[s];
        Record *input = (Record *)malloc(spec->n * sizeof(Record));
        if (input == NULL) {
            return;
        }
        makeInput(input, spec->n, spec->kind, 20260901u);
        if (onSpec != NULL) {
            onSpec(spec);
        }
        for (size_t k = 0; k < SORT_ALGORITHM_COUNT; k++) {
            BenchResult r = benchRun(&SORT_ALGORITHMS[k], input, spec->n, spec->reps);
            sink(spec, &r);
        }
        free(input);
    }
}

/* --- 사람이 읽는 표 ---------------------------------------------------- */

#define ROW_FORMAT "%-14s %9.3f %12zu %12zu %9zu B %6zu %5s %6s\n"
#define ROW_HEADER "알고리즘        시간(ms)         비교         이동      추가메모리 재귀깊이 정렬 안정성\n"
#define ROW_RULE   "---------------------------------------------------------------------------------\n"

static void tableRow(const Spec *spec, const BenchResult *r) {
    (void)spec;
    printf(ROW_FORMAT, r->algo->name, r->millis, r->stats.compares, r->stats.moves,
           r->stats.extraBytes, r->stats.maxDepth, r->sorted ? "yes" : "NO!",
           r->stable ? "yes" : "no");
}

static void tableSpecHeader(const Spec *spec) {
    static const char *lastScope = NULL;

    if (lastScope == NULL || strcmp(lastScope, spec->scope) != 0) {
        if (strcmp(spec->scope, "kinds") == 0) {
            printf("입력 모양별 비교 (n = %zu, %d회 평균)\n", spec->n, spec->reps);
        } else {
            printf("\nn을 키우며 (무작위 입력)\n");
        }
        lastScope = spec->scope;
    }
    if (strcmp(spec->scope, "kinds") == 0) {
        printf("\n[%s]\n", inputKindName(spec->kind));
    } else {
        printf("\n[n = %zu]\n", spec->n);
    }
    printf("%s%s", ROW_HEADER, ROW_RULE);
}

/* 구현 표가 뭐라고 주장하는지 먼저 보여 준다. 아래 측정과 견줘 보라고. */
static void printDeclarations(void) {
    printf("구현 표 (SortAlgorithm이 주장하는 값)\n");
    /* 한글은 터미널에서 두 칸을 쓴다. %-14s는 바이트를 세므로 머리글은 손으로 맞춘다. */
    printf("알고리즘       시간복잡도     메모리     안정성\n");
    printf("%s", ROW_RULE);
    for (size_t k = 0; k < SORT_ALGORITHM_COUNT; k++) {
        const SortAlgorithm *algo = &SORT_ALGORITHMS[k];
        printf("%-14s %-14s %-10s %s\n", algo->name, algo->timeComplexity,
               algo->spaceComplexity, algo->stable ? "stable" : "unstable");
    }
    printf("\n");
}

static void reportTable(void) {
    printf("=== 정렬 비교: 병합 · 퀵 · 힙 ===\n");
    printf("원소는 (key, tag) %zu바이트. key로 정렬하고 tag로 안정성을 본다.\n\n",
           sizeof(Record));
    printDeclarations();
    measureAll(tableRow, tableSpecHeader);

    printf("\n읽는 법\n");
    printf("  시간     : 같은 기계에서만 견준다. 비교·이동 횟수가 더 믿을 만하다.\n");
    printf("  추가메모리: 입력 배열 밖에 잡은 바이트. 병합만 temp 배열(n칸)을 쓴다.\n");
    printf("  재귀깊이 : 스택 사용량의 대리 지표. 힙은 반복문뿐이라 늘 1이다.\n");
    printf("  안정성   : 표의 주장이 아니라 tag 순서로 실측한 값이다.\n");
}

/* --- 기계가 읽는 CSV --------------------------------------------------- */

/* CSV에는 ASCII 키를 쓴다. 표에 찍는 한글 이름(inputKindName)과 따로 둔다. */
static const char *inputKindKey(InputKind kind) {
    switch (kind) {
        case INPUT_RANDOM:     return "random";
        case INPUT_SORTED:     return "sorted";
        case INPUT_REVERSED:   return "reversed";
        case INPUT_FEW_UNIQUE: return "few-unique";
        default:               return "unknown";
    }
}

static void csvRow(const Spec *spec, const BenchResult *r) {
    printf("%s,%s,%zu,%s,%.3f,%zu,%zu,%zu,%zu,%d,%d\n", spec->scope,
           inputKindKey(spec->kind), spec->n, r->algo->name, r->millis,
           r->stats.compares, r->stats.moves, r->stats.extraBytes,
           r->stats.maxDepth, r->sorted, r->stable);
}

static void reportCsv(void) {
    printf("scope,input,n,algo,millis,compares,moves,extraBytes,maxDepth,sorted,stable\n");
    measureAll(csvRow, NULL);
}

/* --- 중복 key 실험 ---------------------------------------------------- */

/* 서로 다른 key가 몇 개뿐인 입력을 만든다. k가 작을수록 중복이 많다. */
static void makeDupInput(Record *a, size_t n, int k, unsigned seed) {
    srand(seed);
    for (size_t i = 0; i < n; i++) {
        a[i].key = rand() % k;
        a[i].tag = (int)i;
    }
}

/* 서로 다른 key 개수(k)를 바꿔 가며 세 정렬을 잰다. 퀵 정렬의 파티션이
 * 같은 값을 한쪽으로 몰아 버리는 약점을 드러내는 실험이다. */
static void reportDupSweep(void) {
    static const int KEYS[] = {2, 8, 64, 512, 4096, 32768};
    const size_t n = 32000;
    const size_t keyCount = sizeof(KEYS) / sizeof(KEYS[0]);
    Record *input = (Record *)malloc(n * sizeof(Record));

    if (input == NULL) {
        return;
    }
    printf("n,keys,algo,compares,moves,millis,maxDepth,sorted,stable\n");
    for (size_t t = 0; t < keyCount; t++) {
        makeDupInput(input, n, KEYS[t], 20260901u);
        for (size_t k = 0; k < SORT_ALGORITHM_COUNT; k++) {
            BenchResult r = benchRun(&SORT_ALGORITHMS[k], input, n, 3);
            printf("%zu,%d,%s,%zu,%zu,%.3f,%zu,%d,%d\n", n, KEYS[t], r.algo->name,
                   r.stats.compares, r.stats.moves, r.millis, r.stats.maxDepth,
                   r.sorted, r.stable);
        }
    }
    free(input);
}

/* --- 피벗 실험 -------------------------------------------------------- */

/* 퀵 정렬만, 피벗을 강의 그대로(첫 원소) 두었을 때와 랜덤으로 골랐을 때를 잰다. */
static void reportPivot(void) {
    const size_t n = 4000;
    const SortAlgorithm *quick = NULL;
    Record *input = (Record *)malloc(n * sizeof(Record));

    for (size_t k = 0; k < SORT_ALGORITHM_COUNT; k++) {
        if (strcmp(SORT_ALGORITHMS[k].name, "quickSort") == 0) {
            quick = &SORT_ALGORITHMS[k];
        }
    }
    if (input == NULL || quick == NULL) {
        free(input);
        return;
    }
    printf("pivot,input,n,compares,moves,millis,maxDepth,sorted\n");
    for (int random = 0; random <= 1; random++) {
        quickSortRandomPivot = random;
        for (int kind = 0; kind < INPUT_KIND_COUNT; kind++) {
            makeInput(input, n, (InputKind)kind, 20260901u);
            BenchResult r = benchRun(quick, input, n, 7);
            printf("%s,%s,%zu,%zu,%zu,%.3f,%zu,%d\n", random ? "random" : "first",
                   inputKindKey((InputKind)kind), n, r.stats.compares, r.stats.moves,
                   r.millis, r.stats.maxDepth, r.sorted);
        }
    }
    quickSortRandomPivot = 1;
    free(input);
}

int main(int argc, char **argv) {
    if (argc > 1 && strcmp(argv[1], "--pivot") == 0) {
        reportPivot();
        return 0;
    }
    if (argc > 1 && strcmp(argv[1], "--csv") == 0) {
        reportCsv();
        return 0;
    }
    if (argc > 1 && strcmp(argv[1], "--dups") == 0) {
        reportDupSweep();
        return 0;
    }
    reportTable();
    return 0;
}
