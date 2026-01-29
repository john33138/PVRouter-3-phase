/**
 * @file test_main.cpp
 * @brief Native unit tests for utils_override.h functionality
 *
 * Tests the override pin mapping system using the REAL utils_override.h header
 * with test-specific configuration values.
 */

#include <unity.h>
#include <cstdint>

#include "utils_bits.h"
#include "utils_override.h"  // Use the REAL header

// ============================================================================
// Test-specific configuration (stubs for config-dependent values)
// ============================================================================

// Test with 3 loads on specific pins
inline constexpr uint8_t NO_OF_DUMPLOADS{ 3 };
inline constexpr uint8_t physicalLoadPin[NO_OF_DUMPLOADS]{ 5, 6, 7 };

// Test relay engine stub (only need get_pin() and size())
struct TestRelayEngine
{
  struct Relay
  {
    uint8_t pin;
    constexpr uint8_t get_pin() const
    {
      return pin;
    }
  };

  Relay relayData[2] = { { 8 }, { 9 } };

  constexpr uint8_t size() const
  {
    return 2;
  }
  constexpr const Relay& get_relay(uint8_t idx) const
  {
    return relayData[idx];
  }
};

inline constexpr TestRelayEngine relays{};
inline constexpr bool RELAY_DIVERSION{ true };

// ============================================================================
// Config-dependent helpers (same logic as utils_override_helpers.h)
// ============================================================================

constexpr uint8_t LOAD(uint8_t loadNum)
{
  return physicalLoadPin[loadNum];
}

constexpr uint8_t RELAY(uint8_t relayNum)
{
  return relays.get_relay(relayNum).get_pin();
}

constexpr uint16_t ALL_LOADS()
{
  uint16_t mask{ 0 };
  for (uint8_t i = 0; i < NO_OF_DUMPLOADS; ++i)
  {
    bit_set(mask, physicalLoadPin[i]);
  }
  return mask;
}

constexpr uint16_t ALL_RELAYS()
{
  if constexpr (RELAY_DIVERSION)
  {
    uint16_t mask{ 0 };
    for (uint8_t i = 0; i < relays.size(); ++i)
    {
      bit_set(mask, relays.get_relay(i).get_pin());
    }
    return mask;
  }
  else
  {
    return 0;
  }
}

constexpr uint16_t ALL_LOADS_AND_RELAYS()
{
  return ALL_LOADS() | ALL_RELAYS();
}

// ============================================================================
// Test fixtures
// ============================================================================

void setUp(void) {}
void tearDown(void) {}

// ============================================================================
// Tests for LOAD() helper function
// ============================================================================

void test_LOAD_returns_correct_pins(void)
{
  TEST_ASSERT_EQUAL(5, LOAD(0));
  TEST_ASSERT_EQUAL(6, LOAD(1));
  TEST_ASSERT_EQUAL(7, LOAD(2));
}

void test_LOAD_matches_physicalLoadPin_array(void)
{
  for (uint8_t i = 0; i < NO_OF_DUMPLOADS; ++i)
  {
    TEST_ASSERT_EQUAL(physicalLoadPin[i], LOAD(i));
  }
}

// ============================================================================
// Tests for RELAY() helper function
// ============================================================================

void test_RELAY_returns_correct_pins(void)
{
  TEST_ASSERT_EQUAL(8, RELAY(0));
  TEST_ASSERT_EQUAL(9, RELAY(1));
}

void test_RELAY_matches_relays_config(void)
{
  for (uint8_t i = 0; i < relays.size(); ++i)
  {
    TEST_ASSERT_EQUAL(relays.get_relay(i).get_pin(), RELAY(i));
  }
}

// ============================================================================
// Tests for ALL_LOADS()
// ============================================================================

void test_ALL_LOADS_returns_correct_bitmask(void)
{
  constexpr uint16_t expected = (1U << 5) | (1U << 6) | (1U << 7);
  TEST_ASSERT_EQUAL(expected, ALL_LOADS());
}

void test_ALL_LOADS_has_correct_bit_count(void)
{
  uint16_t mask = ALL_LOADS();
  uint8_t bitCount = 0;
  while (mask)
  {
    bitCount += mask & 1;
    mask >>= 1;
  }
  TEST_ASSERT_EQUAL(NO_OF_DUMPLOADS, bitCount);
}

void test_ALL_LOADS_includes_all_load_pins(void)
{
  uint16_t mask = ALL_LOADS();
  for (uint8_t i = 0; i < NO_OF_DUMPLOADS; ++i)
  {
    TEST_ASSERT_TRUE(mask & (1U << physicalLoadPin[i]));
  }
}

// ============================================================================
// Tests for ALL_RELAYS()
// ============================================================================

void test_ALL_RELAYS_returns_correct_bitmask(void)
{
  constexpr uint16_t expected = (1U << 8) | (1U << 9);
  TEST_ASSERT_EQUAL(expected, ALL_RELAYS());
}

void test_ALL_RELAYS_has_correct_bit_count(void)
{
  uint16_t mask = ALL_RELAYS();
  uint8_t bitCount = 0;
  while (mask)
  {
    bitCount += mask & 1;
    mask >>= 1;
  }
  TEST_ASSERT_EQUAL(relays.size(), bitCount);
}

void test_ALL_RELAYS_includes_all_relay_pins(void)
{
  uint16_t mask = ALL_RELAYS();
  for (uint8_t i = 0; i < relays.size(); ++i)
  {
    TEST_ASSERT_TRUE(mask & (1U << relays.get_relay(i).get_pin()));
  }
}

// ============================================================================
// Tests for ALL_LOADS_AND_RELAYS()
// ============================================================================

void test_ALL_LOADS_AND_RELAYS_returns_combined_bitmask(void)
{
  constexpr uint16_t expected = (1U << 5) | (1U << 6) | (1U << 7) | (1U << 8) | (1U << 9);
  TEST_ASSERT_EQUAL(expected, ALL_LOADS_AND_RELAYS());
}

void test_ALL_LOADS_AND_RELAYS_includes_loads(void)
{
  uint16_t combined = ALL_LOADS_AND_RELAYS();
  uint16_t loads = ALL_LOADS();
  TEST_ASSERT_EQUAL(loads, combined & loads);
}

void test_ALL_LOADS_AND_RELAYS_includes_relays(void)
{
  uint16_t combined = ALL_LOADS_AND_RELAYS();
  uint16_t relayMask = ALL_RELAYS();
  TEST_ASSERT_EQUAL(relayMask, combined & relayMask);
}

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

void test_indicesToBitmask_consecutive_indices(void)
{
  constexpr uint16_t mask = indicesToBitmask< 3, 4, 5, 6 >();
  TEST_ASSERT_EQUAL(0b01111000, mask);
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

void test_are_pins_valid_rejects_pin_14_and_above(void)
{
  constexpr bool valid = are_pins_valid< 5, 14 >();
  TEST_ASSERT_FALSE(valid);
}

void test_are_pins_valid_boundary_pin_2(void)
{
  constexpr bool valid = are_pins_valid< 2 >();
  TEST_ASSERT_TRUE(valid);
}

void test_are_pins_valid_boundary_pin_13(void)
{
  constexpr bool valid = are_pins_valid< 13 >();
  TEST_ASSERT_TRUE(valid);
}

void test_are_pins_valid_all_valid_pins(void)
{
  constexpr bool valid = are_pins_valid< 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13 >();
  TEST_ASSERT_TRUE(valid);
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

void test_PinList_from_bitmask(void)
{
  constexpr PinList< 4 > list{ static_cast< uint16_t >(0b10100100) };
  TEST_ASSERT_EQUAL(3, list.count);
  TEST_ASSERT_EQUAL(0b10100100, list.toBitmask());
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
  TEST_ASSERT_EQUAL(0, testPins.getPin(2));
}

void test_OverridePins_getBitmask(void)
{
  constexpr OverridePins testPins{
    { KeyIndexPair< 4 >{ 10, { 4, 5 } },
      KeyIndexPair< 4 >{ 11, { 6, 7 } } }
  };

  TEST_ASSERT_EQUAL((1U << 4) | (1U << 5), testPins.getBitmask(0));
  TEST_ASSERT_EQUAL((1U << 6) | (1U << 7), testPins.getBitmask(1));
  TEST_ASSERT_EQUAL(0, testPins.getBitmask(2));
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

void test_OverridePins_single_entry(void)
{
  constexpr OverridePins testPins{
    { KeyIndexPair< 4 >{ 9, { 2, 3, 4 } } }
  };

  TEST_ASSERT_EQUAL(1, testPins.size());
  TEST_ASSERT_EQUAL(9, testPins.getPin(0));
  TEST_ASSERT_EQUAL((1U << 2) | (1U << 3) | (1U << 4), testPins.getBitmask(0));
}

// ============================================================================
// Tests for OverridePins with helper functions
// ============================================================================

void test_OverridePins_with_LOAD_helper(void)
{
  constexpr OverridePins testPins{
    { KeyIndexPair< 4 >{ 10, { LOAD(0), LOAD(1) } } }
  };

  TEST_ASSERT_EQUAL((1U << 5) | (1U << 6), testPins.getBitmask(0));
}

void test_OverridePins_with_RELAY_helper(void)
{
  constexpr OverridePins testPins{
    { KeyIndexPair< 4 >{ 11, { RELAY(0), RELAY(1) } } }
  };

  TEST_ASSERT_EQUAL((1U << 8) | (1U << 9), testPins.getBitmask(0));
}

void test_OverridePins_with_ALL_LOADS(void)
{
  constexpr OverridePins testPins{
    { KeyIndexPair< 4 >{ 12, ALL_LOADS() } }
  };

  TEST_ASSERT_EQUAL(ALL_LOADS(), testPins.getBitmask(0));
}

void test_OverridePins_with_ALL_LOADS_AND_RELAYS(void)
{
  constexpr OverridePins testPins{
    { KeyIndexPair< 8 >{ 13, ALL_LOADS_AND_RELAYS() } }
  };

  TEST_ASSERT_EQUAL(ALL_LOADS_AND_RELAYS(), testPins.getBitmask(0));
}

// ============================================================================
// Main
// ============================================================================

int main(void)
{
  UNITY_BEGIN();

  // LOAD() tests
  RUN_TEST(test_LOAD_returns_correct_pins);
  RUN_TEST(test_LOAD_matches_physicalLoadPin_array);

  // RELAY() tests
  RUN_TEST(test_RELAY_returns_correct_pins);
  RUN_TEST(test_RELAY_matches_relays_config);

  // ALL_LOADS() tests
  RUN_TEST(test_ALL_LOADS_returns_correct_bitmask);
  RUN_TEST(test_ALL_LOADS_has_correct_bit_count);
  RUN_TEST(test_ALL_LOADS_includes_all_load_pins);

  // ALL_RELAYS() tests
  RUN_TEST(test_ALL_RELAYS_returns_correct_bitmask);
  RUN_TEST(test_ALL_RELAYS_has_correct_bit_count);
  RUN_TEST(test_ALL_RELAYS_includes_all_relay_pins);

  // ALL_LOADS_AND_RELAYS() tests
  RUN_TEST(test_ALL_LOADS_AND_RELAYS_returns_combined_bitmask);
  RUN_TEST(test_ALL_LOADS_AND_RELAYS_includes_loads);
  RUN_TEST(test_ALL_LOADS_AND_RELAYS_includes_relays);

  // indicesToBitmask tests (REAL from utils_override.h)
  RUN_TEST(test_indicesToBitmask_single_index);
  RUN_TEST(test_indicesToBitmask_multiple_indices);
  RUN_TEST(test_indicesToBitmask_consecutive_indices);

  // are_pins_valid tests (REAL from utils_override.h)
  RUN_TEST(test_are_pins_valid_with_valid_pins);
  RUN_TEST(test_are_pins_valid_rejects_pin_0);
  RUN_TEST(test_are_pins_valid_rejects_pin_1);
  RUN_TEST(test_are_pins_valid_rejects_pin_14_and_above);
  RUN_TEST(test_are_pins_valid_boundary_pin_2);
  RUN_TEST(test_are_pins_valid_boundary_pin_13);
  RUN_TEST(test_are_pins_valid_all_valid_pins);

  // PinList tests (REAL from utils_override.h)
  RUN_TEST(test_PinList_default_constructor);
  RUN_TEST(test_PinList_variadic_constructor);
  RUN_TEST(test_PinList_toBitmask);
  RUN_TEST(test_PinList_from_bitmask);

  // KeyIndexPair tests (REAL from utils_override.h)
  RUN_TEST(test_KeyIndexPair_getBitmask);

  // OverridePins tests (REAL from utils_override.h)
  RUN_TEST(test_OverridePins_construction_and_size);
  RUN_TEST(test_OverridePins_getPin);
  RUN_TEST(test_OverridePins_getBitmask);
  RUN_TEST(test_OverridePins_findBitmask);
  RUN_TEST(test_OverridePins_single_entry);

  // OverridePins with helper functions tests
  RUN_TEST(test_OverridePins_with_LOAD_helper);
  RUN_TEST(test_OverridePins_with_RELAY_helper);
  RUN_TEST(test_OverridePins_with_ALL_LOADS);
  RUN_TEST(test_OverridePins_with_ALL_LOADS_AND_RELAYS);

  return UNITY_END();
}
