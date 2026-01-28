/**
 * @file test_main.cpp
 * @brief Native tests for override helpers with RELAY_DIVERSION disabled
 *
 * Tests that ALL_RELAYS() returns 0 when RELAY_DIVERSION is false.
 * This is a minimal test that avoids Arduino dependencies.
 */

#include <unity.h>
#include <cstdint>

// ============================================================================
// Test-specific configuration with RELAY_DIVERSION = false
// ============================================================================

inline constexpr uint8_t NO_OF_DUMPLOADS{ 2 };
inline constexpr uint8_t physicalLoadPin[NO_OF_DUMPLOADS]{ 5, 6 };

// Minimal RelayEngine stub
template< uint8_t N, uint8_t D = 10 >
class RelayEngine
{
public:
  constexpr RelayEngine() = default;
  constexpr uint8_t size() const
  {
    return N;
  }

  struct Relay
  {
    uint8_t pin;
    constexpr uint8_t get_pin() const
    {
      return pin;
    }
  };

  constexpr Relay get_relay(uint8_t) const
  {
    return Relay{ 0 };
  }
};

inline constexpr bool RELAY_DIVERSION{ false };  // KEY: Disabled
inline constexpr RelayEngine< 0 > relays{};      // Empty relays

// ============================================================================
// Inline the helper functions here (same logic as utils_override_helpers.h)
// ============================================================================

// Mock bit_set
template< typename T >
constexpr void bit_set(T& value, uint8_t bit)
{
  value |= (static_cast< T >(1) << bit);
}

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
// Tests
// ============================================================================

void setUp(void) {}
void tearDown(void) {}

void test_ALL_RELAYS_returns_zero_when_disabled(void)
{
  // When RELAY_DIVERSION is false, ALL_RELAYS() should return 0
  constexpr uint16_t result = ALL_RELAYS();
  TEST_ASSERT_EQUAL(0, result);
}

void test_ALL_LOADS_still_works_when_relay_disabled(void)
{
  // ALL_LOADS() should still work normally
  constexpr uint16_t expected = (1U << 5) | (1U << 6);
  constexpr uint16_t result = ALL_LOADS();
  TEST_ASSERT_EQUAL(expected, result);
}

void test_ALL_LOADS_AND_RELAYS_equals_ALL_LOADS_when_relay_disabled(void)
{
  // With no relays, ALL_LOADS_AND_RELAYS() should equal ALL_LOADS()
  constexpr uint16_t loads = ALL_LOADS();
  constexpr uint16_t combined = ALL_LOADS_AND_RELAYS();
  TEST_ASSERT_EQUAL(loads, combined);
}

void test_LOAD_helper_works(void)
{
  constexpr uint8_t load0 = LOAD(0);
  constexpr uint8_t load1 = LOAD(1);
  TEST_ASSERT_EQUAL(5, load0);
  TEST_ASSERT_EQUAL(6, load1);
}

void test_if_constexpr_compiles_both_branches(void)
{
  // This test verifies that even though RELAY_DIVERSION is false,
  // the code inside the if constexpr (RELAY_DIVERSION) branch
  // is still syntactically valid and compiles.
  // If there was a syntax error in that branch, this file wouldn't compile.
  TEST_ASSERT_TRUE(true);  // If we got here, both branches compiled
}

int main(void)
{
  UNITY_BEGIN();

  RUN_TEST(test_ALL_RELAYS_returns_zero_when_disabled);
  RUN_TEST(test_ALL_LOADS_still_works_when_relay_disabled);
  RUN_TEST(test_ALL_LOADS_AND_RELAYS_equals_ALL_LOADS_when_relay_disabled);
  RUN_TEST(test_LOAD_helper_works);
  RUN_TEST(test_if_constexpr_compiles_both_branches);

  return UNITY_END();
}
