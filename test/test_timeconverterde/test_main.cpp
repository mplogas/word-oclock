#ifdef UNIT_TEST

#include <Arduino.h>
#include <unity.h>
#include <vector>
#include "timeconverterde.h"

static void assertLedSequence(const std::vector<std::pair<int, int>>& expected,
                              const std::vector<std::pair<int, int>>& actual) {
    TEST_ASSERT_EQUAL_UINT32(expected.size(), actual.size());
    for (size_t i = 0; i < expected.size(); ++i) {
        TEST_ASSERT_EQUAL_INT32(expected[i].first, actual[i].first);
        TEST_ASSERT_EQUAL_INT32(expected[i].second, actual[i].second);
    }
}

void test_standard_viertel_nach_drei(void) {
    TimeConverterDE converter;
    const auto actual = converter.convertTime(3, 15, false, false);
    const std::vector<std::pair<int, int>> expected = {
        {110, 2}, // ES
        {113, 3}, // IST
        {92, 7},  // VIERTEL
        {82, 4},  // NACH
        {45, 4}   // DREI
    };
    assertLedSequence(expected, actual);
}

void test_regional_viertel_eins(void) {
    TimeConverterDE converter;
    const auto actual = converter.convertTime(12, 15, true, false);
    const std::vector<std::pair<int, int>> expected = {
        {110, 2}, // ES
        {113, 3}, // IST
        {92, 7},  // VIERTEL
        {60, 4}   // EINS ("viertel eins")
    };
    assertLedSequence(expected, actual);
}

void test_standard_viertel_vor_vier(void) {
    TimeConverterDE converter;
    const auto actual = converter.convertTime(3, 45, false, false);
    const std::vector<std::pair<int, int>> expected = {
        {110, 2}, // ES
        {113, 3}, // IST
        {92, 7},  // VIERTEL
        {79, 3},  // VOR
        {33, 4}   // VIER
    };
    assertLedSequence(expected, actual);
}

void test_fuenf_vor_halb_vier(void) {
    TimeConverterDE converter;
    const auto actual = converter.convertTime(3, 25, false, false);
    const std::vector<std::pair<int, int>> expected = {
        {110, 2}, // ES
        {113, 3}, // IST
        {117, 4}, // FÜNF
        {79, 3},  // VOR
        {66, 4},  // HALB
        {33, 4}   // VIER
    };
    assertLedSequence(expected, actual);
}

void test_option_minutes_append(void) {
    TimeConverterDE converter;
    const auto actual = converter.convertTime(3, 17, false, true);
    const std::vector<std::pair<int, int>> expected = {
        {110, 2}, // ES
        {113, 3}, // IST
        {92, 7},  // VIERTEL
        {82, 4},  // NACH
        {45, 4},  // DREI
        {2, 1},   // Option LED 1
        {4, 1}    // Option LED 2
    };
    assertLedSequence(expected, actual);
}

void setUp(void) {}
void tearDown(void) {}

int runUnityTests(void) {
    UNITY_BEGIN();
    RUN_TEST(test_standard_viertel_nach_drei);
    RUN_TEST(test_regional_viertel_eins);
    RUN_TEST(test_standard_viertel_vor_vier);
    RUN_TEST(test_fuenf_vor_halb_vier);
    RUN_TEST(test_option_minutes_append);
    return UNITY_END();
}

void setup() {
    delay(2000);
    runUnityTests();
}

void loop() {
    // not used
}

#endif
