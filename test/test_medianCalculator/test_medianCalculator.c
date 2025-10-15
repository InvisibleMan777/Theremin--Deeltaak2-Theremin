#include "medianCalculator.h"
#include <unity.h>

void setUp(void) {
    // set stuff up here
}

void tearDown(void) {
    // clean stuff up here
}

// Test function for calculateMedian_uint32
void test_calculateMedian_uint32() {
    // Test case 1: Odd number of elements
    uint32_t samples1[] = {1, 2, 3, 4, 5, 6, 7};
    uint8_t size1 = sizeof(samples1) / sizeof(samples1[0]); // size of array is totalsize / size of one element
    TEST_ASSERT_EQUAL(4, calculateMedian_uint32(samples1, size1));

    // Test case 2: Even number of elements
    uint32_t samples2[] = {1, 2, 3, 4, 5, 6};
    uint8_t size2 = sizeof(samples2) / sizeof(samples2[0]);
    TEST_ASSERT_EQUAL(3, calculateMedian_uint32(samples2, size2));

    // Test case 3: Single element
    uint32_t samples3[] = {42};
    uint8_t size3 = sizeof(samples3) / sizeof(samples3[0]);
    TEST_ASSERT_EQUAL(42, calculateMedian_uint32(samples3, size3));

    // Test case 4: Empty array
    uint32_t samples4[] = {};
    uint8_t size4 = sizeof(samples4) / sizeof(samples4[0]);
    TEST_ASSERT_EQUAL(0, calculateMedian_uint32(samples4, size4));

    // Test case 5: Unsorted array
    uint32_t samples5[] = {7, 1, 3, 5, 2, 6, 4};
    uint8_t size5 = sizeof(samples5) / sizeof(samples5[0]);
    TEST_ASSERT_EQUAL(4, calculateMedian_uint32(samples5, size5));

    // Test case 6: Array with duplicate elements
    uint32_t samples6[] = {1, 2, 2, 3, 4, 4, 5};
    uint8_t size6 = sizeof(samples6) / sizeof(samples6[0]);
    TEST_ASSERT_EQUAL(3, calculateMedian_uint32(samples6, size6));

    // Test case 7: Large numbers
    uint32_t samples7[] = {1000000, 2000000, 3000000, 4000000, 5000000};
    uint8_t size7 = sizeof(samples7) / sizeof(samples7[0]);
    TEST_ASSERT_EQUAL(3000000, calculateMedian_uint32(samples7, size7));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_calculateMedian_uint32);
    return UNITY_END();
}