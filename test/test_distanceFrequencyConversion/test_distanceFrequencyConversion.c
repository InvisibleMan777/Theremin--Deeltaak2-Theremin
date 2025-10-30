#include "distanceToFrequency.h"
#include "constants.h"

#include <unity.h>

void setUp(void) {
}

void tearDown(void) {
}

static void test_DistanceToFrequency(void) {
    // Test case 1: Minimum distance
    double freq1 = DistanceToFrequency(MIN_DISTANCE_MM, MIN_DISTANCE_MM, MAX_DISTANCE_MM, MIN_FREQ_HZ, MAX_FREQ_HZ);
    TEST_ASSERT_EQUAL_FLOAT(MAX_FREQ_HZ, freq1);

    // Test case 2: Maximum distance
    double freq2 = DistanceToFrequency(MAX_DISTANCE_MM, MIN_DISTANCE_MM, MAX_DISTANCE_MM, MIN_FREQ_HZ, MAX_FREQ_HZ);
    TEST_ASSERT_EQUAL_FLOAT(MIN_FREQ_HZ, freq2);

    // Test case 3: Midpoint distance
    double midpoint = (MIN_DISTANCE_MM + MAX_DISTANCE_MM) / 2.0;
    double expectedFreqMid = (MIN_FREQ_HZ + MAX_FREQ_HZ) / 2.0;
    double freq3 = DistanceToFrequency(midpoint, MIN_DISTANCE_MM, MAX_DISTANCE_MM, MIN_FREQ_HZ, MAX_FREQ_HZ);
    TEST_ASSERT_EQUAL_FLOAT(expectedFreqMid, freq3);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_DistanceToFrequency);
    return UNITY_END();
}