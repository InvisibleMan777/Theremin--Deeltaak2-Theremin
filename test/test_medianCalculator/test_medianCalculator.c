#include "medianCalculator.h"
#include <unity.h>

void setUp(void) {
    // set stuff up here
}

void tearDown(void) {
    // clean stuff up here
}

// Test function for calculateMedian_uint32
static void test_calculateMedian_uint32() {
    uint32_t median;

    // Test case 1: Odd number of elements
    uint32_t samples1[] = {1, 2, 3, 4, 5, 6, 7};
    uint8_t size1 = sizeof(samples1) / sizeof(samples1[0]); // size of array is totalsize / size of one element
    calculateMedian_uint32(samples1, size1, &median);
    TEST_ASSERT_EQUAL(4, median);

    // Test case 2: Even number of elements
    uint32_t samples2[] = {1, 3, 5, 7, 9, 11};
    uint8_t size2 = sizeof(samples2) / sizeof(samples2[0]);
    calculateMedian_uint32(samples2, size2, &median);
    TEST_ASSERT_EQUAL(6, median); 

    // Test case 3: Unsorted array
    uint32_t samples3[] = {7, 1, 3, 9, 5};
    uint8_t size3 = sizeof(samples3) / sizeof(samples3[0]);
    calculateMedian_uint32(samples3, size3, &median);
    TEST_ASSERT_EQUAL(5, median);

    // Test case 4: Array with duplicate values
    uint32_t samples4[] = {4, 2, 2, 8, 6, 4};
    uint8_t size4 = sizeof(samples4) / sizeof(samples4[0]);
    calculateMedian_uint32(samples4, size4, &median);
    TEST_ASSERT_EQUAL(4, median);

    // Test case 5: Single element array
    uint32_t samples5[] = {42};
    uint8_t size5 = sizeof(samples5) / sizeof(samples5[0]);
    calculateMedian_uint32(samples5, size5, &median);
    TEST_ASSERT_EQUAL(42, median);

    // Test case 6: Empty array
    uint32_t samples6[] = {};
    uint8_t size6 = sizeof(samples6) / sizeof(samples6[0]);
    calculateMedian_uint32(samples6, size6, &median);
    TEST_ASSERT_EQUAL(0, median); // Assuming function returns 0 for empty array
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_calculateMedian_uint32);
    return UNITY_END();
}