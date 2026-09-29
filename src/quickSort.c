/* 퀵 정렬 (주제 04) — 피벗을 제자리에 놓고, 양쪽을 각각 정렬한다.
 *
 * 병합 정렬의 반대편이다. 나누기(파티션)에서 일하고, 합치기는 공짜다.
 * 파티션은 강의의 Lomuto 방식을 그대로 옮겼다: 첫 원소를 피벗으로 삼고
 * 피벗보다 작은 것을 왼쪽으로 모은다.
 *
 * 강의와 다른 점은 하나다. 첫 원소를 그냥 쓰면 정렬된 입력에서 O(n^2)이
 * 되므로, 파티션마다 **무작위로 고른 원소를 먼저 첫 자리로 옮겨** 피벗으로
 * 쓴다(강의 옵션 1. 랜덤). 난수는 강의의 minstd 생성기를 직접 만들고
 * 시드를 고정해, 비교 횟수가 매번 똑같이 재현되게 했다.
 */
#include <stdint.h>

#include "sortctx.h"

/* 1이면 랜덤 피벗, 0이면 강의 그대로 첫 원소 피벗. 피벗 실험(main.c --pivot)만
 * 이 값을 0으로 바꿔 본다. */
int quickSortRandomPivot = 1;

/* minstd — 강의 01_random/의 생성기와 같다. */
static int64_t quickState = 1;

static size_t nextRandom(void) {
    quickState = quickState * 16807 % 2147483647;
    return (size_t)quickState;
}

/* a[lo..hi]를 가른다. 돌아올 때 피벗은 제자리에 있고 그 인덱스를 돌려준다. */
static size_t partition(SortCtx *c, size_t lo, size_t hi) {
    size_t r = quickSortRandomPivot ? lo + nextRandom() % (hi - lo + 1) : lo;
    if (r != lo) {
        sortSwap(c, lo, r); /* 무작위로 고른 피벗을 첫 자리로 */
    }
    size_t i = lo;
    for (size_t j = lo + 1; j <= hi; j++) {
        /* 피벗보다 '작은' 것만 왼쪽으로 보낸다. 피벗과 같은 값은 오른쪽에
         * 남는다. 중복이 많은 입력에서 이것이 약점이 된다(보고서 3.2). */
        if (sortCompareAt(c, j, lo) < 0) {
            i++;
            if (i != j) {
                sortSwap(c, i, j);
            }
        }
    }
    if (i != lo) {
        sortSwap(c, lo, i);
    }
    return i;
}

static void quickSortRange(SortCtx *c, size_t lo, size_t hi, size_t depth) {
    if (c->stats != NULL && depth > c->stats->maxDepth) {
        c->stats->maxDepth = depth;
    }
    if (lo >= hi) {
        return; /* 원소 하나면 이미 정렬 */
    }
    size_t p = partition(c, lo, hi);
    if (p > lo) {
        quickSortRange(c, lo, p - 1, depth + 1); /* 피벗 왼쪽 */
    }
    quickSortRange(c, p + 1, hi, depth + 1);     /* 피벗 오른쪽 */
}

void quickSort(void *base, size_t n, size_t size, SortCompare cmp, SortStats *stats) {
    SortCtx c;
    if (!sortBegin(&c, base, n, size, cmp, stats)) {
        return;
    }
    quickState = 20260930; /* 시드 고정: 같은 입력이면 같은 피벗, 같은 횟수 */
    quickSortRange(&c, 0, n - 1, 1);
    sortEnd(&c);
}
