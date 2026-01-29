/**
 * @file test_main.cpp
 * @brief Embedded unit tests for relay logic
 *
 * Tests the RelayEngine and relayOutput using the real implementation.
 * Runs on Arduino hardware.
 */

#include <Arduino.h>
#include <unity.h>

#include "utils_relay.h"

// Test relay configuration
static const relayOutput testRelays[] = {
  relayOutput(8, 1000, 200, 5, 5),  // pin 8, surplus 1000W, import 200W, 5min ON/OFF
  relayOutput(9, 1500, 300, 3, 3),  // pin 9, surplus 1500W, import 300W, 3min ON/OFF
};

static RelayEngine< 2 > relayEngine(testRelays);

// ============================================================================
// Basic relay configuration tests
// ============================================================================

void test_relay_initialization()
{
  TEST_ASSERT_EQUAL(2, relayEngine.size());
}

void test_get_pin()
{
  TEST_ASSERT_EQUAL(8, relayEngine.get_relay(0).get_pin());
  TEST_ASSERT_EQUAL(9, relayEngine.get_relay(1).get_pin());
}

void test_get_surplusThreshold()
{
  // Note: internally stored as negative
  TEST_ASSERT_EQUAL(1000, relayEngine.get_relay(0).get_surplusThreshold());
  TEST_ASSERT_EQUAL(1500, relayEngine.get_relay(1).get_surplusThreshold());
}

void test_get_importThreshold()
{
  TEST_ASSERT_EQUAL(200, relayEngine.get_relay(0).get_importThreshold());
  TEST_ASSERT_EQUAL(300, relayEngine.get_relay(1).get_importThreshold());
}

void test_get_minON()
{
  TEST_ASSERT_EQUAL(5 * 60, relayEngine.get_relay(0).get_minON());
  TEST_ASSERT_EQUAL(3 * 60, relayEngine.get_relay(1).get_minON());
}

void test_get_minOFF()
{
  TEST_ASSERT_EQUAL(5 * 60, relayEngine.get_relay(0).get_minOFF());
  TEST_ASSERT_EQUAL(3 * 60, relayEngine.get_relay(1).get_minOFF());
}

void test_isRelayON_initial()
{
  TEST_ASSERT_FALSE(relayEngine.get_relay(0).isRelayON());
  TEST_ASSERT_FALSE(relayEngine.get_relay(1).isRelayON());
}

// ============================================================================
// EWMA average tests
// ============================================================================

void test_ewma_initial_average()
{
  // Fresh engine should have zero average
  TEST_ASSERT_EQUAL(0, relayEngine.get_average());
}

void test_ewma_update_average()
{
  // Update with some values
  for (int i = 0; i < 100; ++i)
  {
    relayEngine.update_average(1000);
  }
  // Average should be moving towards 1000
  TEST_ASSERT_TRUE(relayEngine.get_average() > 0);
}

// ============================================================================
// Unity setup
// ============================================================================

void setUp()
{
  // Called before each test
}

void tearDown()
{
  // Called after each test
}

void setup()
{
  delay(2000);  // Wait for serial connection

  UNITY_BEGIN();

  // Configuration tests
  RUN_TEST(test_relay_initialization);
  RUN_TEST(test_get_pin);
  RUN_TEST(test_get_surplusThreshold);
  RUN_TEST(test_get_importThreshold);
  RUN_TEST(test_get_minON);
  RUN_TEST(test_get_minOFF);
  RUN_TEST(test_isRelayON_initial);

  // EWMA tests
  RUN_TEST(test_ewma_initial_average);
  RUN_TEST(test_ewma_update_average);

  UNITY_END();
}

void loop()
{
  // Nothing to do
}
