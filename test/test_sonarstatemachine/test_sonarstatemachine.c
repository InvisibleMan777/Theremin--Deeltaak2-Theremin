#include "sonarStateMachine.h"
#include <unity.h>

void setUp(void) {
}

void tearDown(void) {
}

void test_sonarStateMachineStep(void) {
    //Test case 1: Transition from READY_FOR_TRIGGER to SENDING_TRIGGER
    enum SonarState sonarState = READY_FOR_TRIGGER;
    echoReceivedFlag = 0;
    latestMeasurement = 0; 

    sonarStateMachineStep(&sonarState, 1, NULL, NULL);
    TEST_ASSERT_EQUAL(SENDING_TRIGGER, sonarState);

    //Test case 2: Transition from SENDING_TRIGGER to WAITING_FOR_ECHO when time is not reached yet
    sonarState = SENDING_TRIGGER;
    echoReceivedFlag = 0;
    latestMeasurement = 0; 

    timeSinceTriggerStart = 0; // reset trigger start time for testing
    sonarStateMachineStep(&sonarState, 3, NULL, NULL);
    TEST_ASSERT_EQUAL(SENDING_TRIGGER, sonarState);

    //Test case 3: Transition from SENDING_TRIGGER to WAITING_FOR_ECHO when time is reached
    sonarState = SENDING_TRIGGER;
    echoReceivedFlag = 0;
    latestMeasurement = 0;

    sonarStateMachineStep(&sonarState, 12, NULL, NULL);
    TEST_ASSERT_EQUAL(WAITING_FOR_ECHO, sonarState);

    //Test case 4: Transition from WAITING_FOR_ECHO to ECHO_RECEIVED when echo not yet received
    sonarState = WAITING_FOR_ECHO;
    echoReceivedFlag = 0;
    latestMeasurement = 0;

    sonarStateMachineStep(&sonarState, 15, NULL, NULL);
    TEST_ASSERT_EQUAL(WAITING_FOR_ECHO, sonarState);

    //Test case 5: Transition from WAITING_FOR_ECHO to ECHO_RECEIVED when echo received
    sonarState = WAITING_FOR_ECHO;
    echoReceivedFlag = 1;
    latestMeasurement = 0;

    sonarStateMachineStep(&sonarState, 18, NULL, NULL);
    TEST_ASSERT_EQUAL(ECHO_RECEIVED, sonarState);

    //Test case 6: Transition from ECHO_RECEIVED to READY_FOR_TRIGGER
    sonarState = ECHO_RECEIVED;
    echoReceivedFlag = 0;
    latestMeasurement = 0;

    sonarStateMachineStep(&sonarState, 20, NULL, NULL);
    TEST_ASSERT_EQUAL(READY_FOR_TRIGGER, sonarState);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_sonarStateMachineStep);
    return UNITY_END();
}