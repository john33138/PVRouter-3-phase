/**
 * @file test_main.cpp
 * @brief Embedded unit tests for utils_override.h functionality
 *
 * Tests the override pin mapping system using the REAL headers.
 * Runs on Arduino hardware with actual configuration.
 */

#include <Arduino.h>
#include <unity.h>

#include "config.h"  // Real config with physicalLoadPin, relays, etc.

// ============================================================================
// Test fixtures
// ============================================================================

void setUp(void) {}
void tearDown(void) {}

// ============================================================================
// Tests for indicesToBitmask<>() - from utils_override.h
// ============================================================================

void test_indicesToBitmask_single_index(void)
{
  constexpr uint16_t mask = indicesToBitmask< 5 >();
  TEST_ASSERT_EQUAL(1U << 5, mask);
}

void test_indicesToBitmask_multiple_indices(void)
{
  constexpr uint16_t mask = indicesToBitmask< 2, 4, 7 >();
  TEST_ASSERT_EQUAL((1U << 2) | (1U << 4) | (1U << 7), mask);
}

// ============================================================================
// Tests for are_pins_valid<>() - from utils_override.h
// ============================================================================

void test_are_pins_valid_with_valid_pins(void)
{
  constexpr bool valid = are_pins_valid< 2, 7, 10, 13 >();
  TEST_ASSERT_TRUE(valid);
}

void test_are_pins_valid_rejects_pin_0(void)
{
  constexpr bool valid = are_pins_valid< 0, 5 >();
  TEST_ASSERT_FALSE(valid);
}

void test_are_pins_valid_rejects_pin_1(void)
{
  constexpr bool valid = are_pins_valid< 1, 5 >();
  TEST_ASSERT_FALSE(valid);
}

void test_are_pins_valid_boundary_pins(void)
{
  constexpr bool valid2 = are_pins_valid< 2 >();
  constexpr bool valid13 = are_pins_valid< 13 >();
  TEST_ASSERT_TRUE(valid2);
  TEST_ASSERT_TRUE(valid13);
}

// ============================================================================
// Tests for PinList - from utils_override.h
// ============================================================================

void test_PinList_default_constructor(void)
{
  constexpr PinList< 4 > list{};
  TEST_ASSERT_EQUAL(0, list.count);
  TEST_ASSERT_EQUAL(0, list.toBitmask());
}

void test_PinList_variadic_constructor(void)
{
  constexpr PinList< 4 > list{ 3, 6, 9 };
  TEST_ASSERT_EQUAL(3, list.count);
  TEST_ASSERT_EQUAL(3, list.pins[0]);
  TEST_ASSERT_EQUAL(6, list.pins[1]);
  TEST_ASSERT_EQUAL(9, list.pins[2]);
}

void test_PinList_toBitmask(void)
{
  constexpr PinList< 4 > list{ 2, 5, 8 };
  constexpr uint16_t expected = (1U << 2) | (1U << 5) | (1U << 8);
  TEST_ASSERT_EQUAL(expected, list.toBitmask());
}

// ============================================================================
// Tests for KeyIndexPair - from utils_override.h
// ============================================================================

void test_KeyIndexPair_getBitmask(void)
{
  constexpr PinList< 4 > list{ 4, 5, 6 };
  constexpr KeyIndexPair< 4 > pair{ 2, list };

  TEST_ASSERT_EQUAL(2, pair.pin);
  TEST_ASSERT_EQUAL((1U << 4) | (1U << 5) | (1U << 6), pair.getBitmask());
}

// ============================================================================
// Tests for OverridePins class - from utils_override.h
// ============================================================================

void test_OverridePins_construction_and_size(void)
{
  constexpr OverridePins testPins{
    { KeyIndexPair< 4 >{ 2, { 4, 5 } },
      KeyIndexPair< 4 >{ 3, { 6, 7 } } }
  };

  TEST_ASSERT_EQUAL(2, testPins.size());
}

void test_OverridePins_getPin(void)
{
  constexpr OverridePins testPins{
    { KeyIndexPair< 4 >{ 10, { 4, 5 } },
      KeyIndexPair< 4 >{ 11, { 6, 7 } } }
  };

  TEST_ASSERT_EQUAL(10, testPins.getPin(0));
  TEST_ASSERT_EQUAL(11, testPins.getPin(1));
}

void test_OverridePins_getBitmask(void)
{
  constexpr OverridePins testPins{
    { KeyIndexPair< 4 >{ 10, { 4, 5 } },
      KeyIndexPair< 4 >{ 11, { 6, 7 } } }
  };

  TEST_ASSERT_EQUAL((1U << 4) | (1U << 5), testPins.getBitmask(0));
  TEST_ASSERT_EQUAL((1U << 6) | (1U << 7), testPins.getBitmask(1));
}

void test_OverridePins_findBitmask(void)
{
  constexpr OverridePins testPins{
    { KeyIndexPair< 4 >{ 10, { 4, 5 } },
      KeyIndexPair< 4 >{ 11, { 6, 7 } } }
  };

  TEST_ASSERT_EQUAL((1U << 4) | (1U << 5), testPins.findBitmask(10));
  TEST_ASSERT_EQUAL((1U << 6) | (1U << 7), testPins.findBitmask(11));
  TEST_ASSERT_EQUAL(0, testPins.findBitmask(99));
}

// ============================================================================
// Tests for helper functions with REAL config - from utils_override_helpers.h
// ============================================================================

void test_LOAD_returns_valid_pins(void)
{
  for (uint8_t i = 0; i < NO_OF_DUMPLOADS; ++i)
  {
    TEST_ASSERT_EQUAL(physicalLoadPin[i], LOAD(i));
  }
}

void test_ALL_LOADS_matches_config(void)
{
  uint16_t mask = ALL_LOADS();
  uint8_t bitCount = 0;

  // Count bits
  uint16_t temp = mask;
  while (temp)
  {
    bitCount += temp & 1;
    temp >>= 1;
  }

  TEST_ASSERT_EQUAL(NO_OF_DUMPLOADS, bitCount);

  // Verify each load pin is set
  for (uint8_t i = 0; i < NO_OF_DUMPLOADS; ++i)
  {
    TEST_ASSERT_TRUE(mask & (1U << physicalLoadPin[i]));
  }
}

#if RELAY_DIVERSION
void test_RELAY_returns_valid_pins(void)
{
  for (uint8_t i = 0; i < relays.size(); ++i)
  {
    TEST_ASSERT_EQUAL(relays.get_relay(i).get_pin(), RELAY(i));
  }
}

void test_ALL_RELAYS_matches_config(void)
{
  uint16_t mask = ALL_RELAYS();
  uint8_t bitCount = 0;

  // Count bits
  uint16_t temp = mask;
  while (temp)
  {
    bitCount += temp & 1;
    temp >>= 1;
  }

  TEST_ASSERT_EQUAL(relays.size(), bitCount);

  // Verify each relay pin is set
  for (uint8_t i = 0; i < relays.size(); ++i)
  {
    TEST_ASSERT_TRUE(mask & (1U << relays.get_relay(i).get_pin()));
  }
}
#endif

void test_ALL_LOADS_AND_RELAYS_combines_both(void)
{
  uint16_t combined = ALL_LOADS_AND_RELAYS();
  uint16_t loads = ALL_LOADS();
  uint16_t relayMask = ALL_RELAYS();

  TEST_ASSERT_EQUAL(loads | relayMask, combined);
}

// ============================================================================
// Unity setup
// ============================================================================

void setup()
{
  delay(2000);  // Wait for serial connection

  UNITY_BEGIN();

  // Generic utility tests (from utils_override.h)
  RUN_TEST(test_indicesToBitmask_single_index);
  RUN_TEST(test_indicesToBitmask_multiple_indices);
  RUN_TEST(test_are_pins_valid_with_valid_pins);
  RUN_TEST(test_are_pins_valid_rejects_pin_0);
  RUN_TEST(test_are_pins_valid_rejects_pin_1);
  RUN_TEST(test_are_pins_valid_boundary_pins);
  RUN_TEST(test_PinList_default_constructor);
  RUN_TEST(test_PinList_variadic_constructor);
  RUN_TEST(test_PinList_toBitmask);
  RUN_TEST(test_KeyIndexPair_getBitmask);
  RUN_TEST(test_OverridePins_construction_and_size);
  RUN_TEST(test_OverridePins_getPin);
  RUN_TEST(test_OverridePins_getBitmask);
  RUN_TEST(test_OverridePins_findBitmask);

  // Helper function tests with REAL config
  RUN_TEST(test_LOAD_returns_valid_pins);
  RUN_TEST(test_ALL_LOADS_matches_config);
#if RELAY_DIVERSION
  RUN_TEST(test_RELAY_returns_valid_pins);
  RUN_TEST(test_ALL_RELAYS_matches_config);
#endif
  RUN_TEST(test_ALL_LOADS_AND_RELAYS_combines_both);

  UNITY_END();
}

void loop()
{
  // Nothing to do
}
