#include "sonarStateMachine.h"
#include <unity.h>

void setUp(void) {
    // set stuff up here
}

void tearDown(void) {
    // clean stuff up here
}

void test_sonarStateMachineStep(void) {
    enum SonarState sonarState = READY_FOR_TRIGGER;
    echoReceivedFlag = 0;
    latestMeasurement = 0; // example measurement

    sonarStateMachineStep(&sonarState, 1, NULL, NULL);
    TEST_ASSERT_EQUAL(SENDING_TRIGGER, sonarState);

    sonarState = SENDING_TRIGGER;
    echoReceivedFlag = 0;
    latestMeasurement = 0; // example measurement

    timeSinceTriggerStart = 0; // reset trigger start time for testing
    sonarStateMachineStep(&sonarState, 3, NULL, NULL);
    TEST_ASSERT_EQUAL(SENDING_TRIGGER, sonarState);

    sonarState = SENDING_TRIGGER;
    echoReceivedFlag = 0;
    latestMeasurement = 0; // example measurement

    sonarStateMachineStep(&sonarState, 12, NULL, NULL);
    TEST_ASSERT_EQUAL(WAITING_FOR_ECHO, sonarState);

    sonarState = WAITING_FOR_ECHO;
    echoReceivedFlag = 0;
    latestMeasurement = 0; // example measurement

    sonarStateMachineStep(&sonarState, 15, NULL, NULL);
    TEST_ASSERT_EQUAL(WAITING_FOR_ECHO, sonarState);

    sonarState = WAITING_FOR_ECHO;
    echoReceivedFlag = 1;
    latestMeasurement = 0; // example measurement

    sonarStateMachineStep(&sonarState, 18, NULL, NULL);
    TEST_ASSERT_EQUAL(ECHO_RECEIVED, sonarState);

    sonarState = ECHO_RECEIVED;
    echoReceivedFlag = 0;
    latestMeasurement = 0; // example measurement

    sonarStateMachineStep(&sonarState, 20, NULL, NULL);
    TEST_ASSERT_EQUAL(READY_FOR_TRIGGER, sonarState);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_sonarStateMachineStep);
    return UNITY_END();
}