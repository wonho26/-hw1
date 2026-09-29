/* 힙 정렬 (수업에서 다루지 않은 정렬) — 배열을 최대 힙으로 만든 뒤,
 * 맨 위(최댓값)를 하나씩 배열 끝으로 보낸다.
 *
 * 힙: 부모가 늘 자식 이상인 완전 이진 트리. 배열에 그대로 담는다.
 *     인덱스 i의 자식은 2i+1, 2i+2 이고, 부모는 (i-1)/2 다.
 *
 * 1단계 (힙 만들기): 맨 아래 부모부터 거꾸로 올라가며 sift-down. O(n)
 * 2단계 (꺼내기)   : 루트(최댓값)를 끝과 바꾸고, 힙을 한 칸 줄이고,
 *                    새 루트를 sift-down. 이걸 n-1번. O(n log n)
 *
 * 재귀도 보조 배열도 없다. 추가 메모리는 교환에 쓰는 원소 한 칸뿐이고,
 * 최악도 O(n log n)이다. 대신 멀리 떨어진 원소끼리 바꾸므로 안정하지 않다.
 */
#include "sortctx.h"

/* a[i]를 아래로 내려 보내 a[i..n-1]이 다시 힙이 되게 한다. 반복문으로 쓴다. */
static void siftDown(SortCtx *c, size_t i, size_t n) {
    for (;;) {
        size_t largest = i;
        size_t left = 2 * i + 1;
        size_t right = left + 1;
        if (left < n && sortCompareAt(c, left, largest) > 0) {
            largest = left;
        }
        if (right < n && sortCompareAt(c, right, largest) > 0) {
            largest = right;
        }
        if (largest == i) {
            return; /* 부모가 두 자식 이상이다. 힙 조건을 만족 */
        }
        sortSwap(c, i, largest);
        i = largest;
    }
}

void heapSort(void *base, size_t n, size_t size, SortCompare cmp, SortStats *stats) {
    SortCtx c;
    if (!sortBegin(&c, base, n, size, cmp, stats)) {
        return;
    }
    /* 1단계: 자식이 있는 마지막 부모(n/2 - 1)부터 루트까지 거꾸로. */
    for (size_t i = n / 2; i-- > 0;) {
        siftDown(&c, i, n);
    }
    /* 2단계: 최댓값을 끝으로 보내고 힙을 한 칸씩 줄인다. */
    for (size_t end = n - 1; end > 0; end--) {
        sortSwap(&c, 0, end);
        siftDown(&c, 0, end);
    }
    sortEnd(&c);
}
