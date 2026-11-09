#include "mm.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

int main(void)
{
    char *first = my_malloc(20);
    char *second = my_malloc(37);

    /* Both requests should succeed. */
    assert(first != NULL);
    assert(second != NULL);

    /* Both returned addresses must be 16-byte aligned. */
    assert((uintptr_t)first % 16 == 0);
    assert((uintptr_t)second % 16 == 0);

    /* Data written to one allocation should remain readable. */
    strcpy(first, "Hello, Slabforge!");
    memset(second, 'A', 37);

    assert(strcmp(first, "Hello, Slabforge!") == 0);
    for (size_t i = 0; i < 37; i++) {
        assert(second[i] == 'A');
    }

    /* Our chosen behavior for a zero-byte request. */
    assert(my_malloc(0) == NULL);

    /* A request larger than the entire arena cannot fit. */
    assert(my_malloc(2 * 1024 * 1024) == NULL);

    /* Free is intentionally a no-op in this phase. */
    my_free(first);
    my_free(second);
    my_free(NULL);

    return 0;
}
