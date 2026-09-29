/* 병합 정렬 (주제 03) — 반으로 자르고, 각각 정렬한 뒤, 합친다.
 *
 * 나누기는 공짜(가운데에서 자른다)이고 일은 합치는 쪽에서 한다.
 * 합칠 때 두 쪽을 번갈아 꺼내 담을 자리가 필요해서, 배열 크기만큼의
 * 보조 배열(temp)을 처음에 한 번 잡아 두고 끝까지 돌려쓴다.
 * 이것이 병합 정렬이 치르는 값, 추가 메모리 O(n)이다.
 */
#include <stdlib.h>

#include "sortctx.h"

/* 병합 정렬만 쓰는 문맥. 공통 문맥에 보조 배열 하나를 더 들고 다닌다. */
typedef struct MergeCtx {
    SortCtx *c;
    char *temp; /* n칸짜리 보조 배열 */
} MergeCtx;

static char *tempAt(const MergeCtx *m, size_t i) {
    return m->temp + i * m->c->size;
}

/* 정렬된 두 구간 a[lo..mid-1], a[mid..hi-1]을 합쳐 a[lo..hi-1]에 되돌려 놓는다. */
static void merge(MergeCtx *m, size_t lo, size_t mid, size_t hi) {
    SortCtx *c = m->c;
    size_t i = lo;
    size_t j = mid;
    size_t k = lo;

    while (i < mid && j < hi) {
        /* '<=' 가 안정성을 만든다. 같으면 왼쪽(먼저 온 것)을 먼저 꺼낸다.
         * '<' 로 바꾸면 결과는 여전히 정렬되지만 같은 값의 순서가 뒤집힌다. */
        if (sortCompareAt(c, i, j) <= 0) {
            sortMove(c, tempAt(m, k++), sortElemAt(c, i++));
        } else {
            sortMove(c, tempAt(m, k++), sortElemAt(c, j++));
        }
    }
    while (i < mid) {
        sortMove(c, tempAt(m, k++), sortElemAt(c, i++));
    }
    while (j < hi) {
        sortMove(c, tempAt(m, k++), sortElemAt(c, j++));
    }
    for (k = lo; k < hi; k++) {
        sortMove(c, sortElemAt(c, k), tempAt(m, k));
    }
}

static void mergeSortRange(MergeCtx *m, size_t lo, size_t hi, size_t depth) {
    if (m->c->stats != NULL && depth > m->c->stats->maxDepth) {
        m->c->stats->maxDepth = depth;
    }
    if (hi - lo < 2) {
        return; /* 원소 하나면 이미 정렬 */
    }
    size_t mid = lo + (hi - lo) / 2;
    mergeSortRange(m, lo, mid, depth + 1);
    mergeSortRange(m, mid, hi, depth + 1);
    /* 이미 이어 붙어 있으면 합칠 일이 없다. 정렬된 입력에서 비교가 n-1번쯤으로 준다. */
    if (sortCompareAt(m->c, mid - 1, mid) <= 0) {
        return;
    }
    merge(m, lo, mid, hi);
}

void mergeSort(void *base, size_t n, size_t size, SortCompare cmp, SortStats *stats) {
    SortCtx c;
    if (!sortBegin(&c, base, n, size, cmp, stats)) {
        return;
    }
    MergeCtx m = {&c, (char *)malloc(n * size)};
    if (m.temp != NULL) {
        if (stats != NULL) {
            stats->extraBytes += n * size; /* temp 몫. 이것이 O(n) 공간이다 */
        }
        mergeSortRange(&m, 0, n, 1);
        free(m.temp);
    }
    sortEnd(&c);
}
