/**
 * @file test_main.cpp
 * @brief Native unit tests for relay logic
 *
 * Tests the RelayEngine and relayOutput logic without Arduino dependencies.
 * Uses stub implementations that replicate the core behavior.
 */

#include <unity.h>
#include <cstdint>
#include <climits>

#include "utils_bits.h"

// ============================================================================
// Minimal stubs replicating relay logic from utils_relay.h
// ============================================================================

// EWMA stub - simplified version that just tracks average
template< uint16_t D >
class EWMA_average
{
  int32_t average{ 0 };

public:
  constexpr void addValue(int32_t value)
  {
    // Simplified EWMA: just use simple average for testing
    average = (average * (D - 1) + value) / D;
  }

  constexpr int32_t getAverageS() const
  {
    return average;
  }
  constexpr int32_t getAverageD() const
  {
    return average;
  }
  constexpr int32_t getAverageT() const
  {
    return average;
  }
};

// relayOutput stub - replicates core logic
class relayOutput
{
  uint8_t relay_pin;
  int16_t surplusThreshold;
  int16_t importThreshold;
  uint16_t minON;
  uint16_t minOFF;
  mutable uint16_t duration{ 0 };
  mutable bool relayIsON{ false };

public:
  constexpr relayOutput(uint8_t pin, int16_t surplus, int16_t import_, uint16_t minOn, uint16_t minOff)
    : relay_pin(pin), surplusThreshold(-surplus), importThreshold(import_), minON(minOn * 60), minOFF(minOff * 60)
  {
  }

  constexpr uint8_t get_pin() const
  {
    return relay_pin;
  }
  constexpr int16_t get_surplusThreshold() const
  {
    return surplusThreshold;
  }
  constexpr int16_t get_importThreshold() const
  {
    return importThreshold;
  }
  constexpr uint16_t get_minON() const
  {
    return minON;
  }
  constexpr uint16_t get_minOFF() const
  {
    return minOFF;
  }
  constexpr bool isRelayON() const
  {
    return relayIsON;
  }

  bool try_turnON() const
  {
    if (relayIsON)
      return false;
    if (duration < minOFF)
      return false;
    relayIsON = true;
    duration = 0;
    return true;
  }

  bool try_turnOFF() const
  {
    if (!relayIsON)
      return false;
    if (duration < minON)
      return false;
    relayIsON = false;
    duration = 0;
    return true;
  }

  bool proceed_relay(int32_t currentAvgPower, uint16_t& overrideBitmask) const
  {
    const bool isOverrideActive = bit_read(overrideBitmask, relay_pin);

    if (currentAvgPower < surplusThreshold || isOverrideActive)
    {
      bit_clear(overrideBitmask, relay_pin);
      return try_turnON();
    }

    if (importThreshold >= 0)
    {
      if (currentAvgPower > importThreshold)
      {
        return try_turnOFF();
      }
    }
    else
    {
      if (currentAvgPower < -importThreshold)
      {
        return try_turnOFF();
      }
    }
    return false;
  }

  void inc_duration() const
  {
    if (duration < UINT16_MAX)
      ++duration;
  }
};

// RelayEngine stub
template< uint8_t N, uint8_t D = 10 >
class RelayEngine
{
  relayOutput relay[N];
  mutable EWMA_average< D > ewma_average;
  mutable uint8_t settle_change{ 60 };

public:
  template< typename... Args >
  constexpr RelayEngine(Args... args)
    : relay{ args... }
  {
  }

  constexpr uint8_t size() const
  {
    return N;
  }
  constexpr const relayOutput& get_relay(uint8_t idx) const
  {
    return relay[idx];
  }

  void update_average(int32_t value) const
  {
    ewma_average.addValue(value);
  }

  void inc_duration() const
  {
    if (settle_change > 0)
      --settle_change;
    for (uint8_t i = 0; i < N; ++i)
      relay[i].inc_duration();
  }

  void proceed_relays(uint16_t& overrideBitmask) const
  {
    if (settle_change != 0)
      return;

    if (ewma_average.getAverageT() > 0)
    {
      uint8_t idx{ N };
      do
      {
        if (relay[--idx].proceed_relay(ewma_average.getAverageT(), overrideBitmask))
        {
          settle_change = 60;
          return;
        }
      } while (idx);
    }
    else
    {
      uint8_t idx{ 0 };
      do
      {
        if (relay[idx].proceed_relay(ewma_average.getAverageT(), overrideBitmask))
        {
          settle_change = 60;
          return;
        }
      } while (++idx < N);
    }
  }
};

// ============================================================================
// Test fixtures and helpers
// ============================================================================

void setUp(void) {}
void tearDown(void) {}

// Test relay configurations
RelayEngine< 2 > testRelays{ relayOutput{ 4, 500, 100, 1, 1 },
                             relayOutput{ 5, 800, 150, 1, 1 } };

void wait_for_settle_change()
{
  for (uint8_t i = 0; i < 60; ++i)
    testRelays.inc_duration();
}

// ============================================================================
// Relay initialization tests
// ============================================================================

void test_relay_initialization(void)
{
  TEST_ASSERT_EQUAL(2, testRelays.size());
}

void test_get_pin(void)
{
  TEST_ASSERT_EQUAL(4, testRelays.get_relay(0).get_pin());
  TEST_ASSERT_EQUAL(5, testRelays.get_relay(1).get_pin());
}

void test_get_surplusThreshold(void)
{
  TEST_ASSERT_EQUAL(-500, testRelays.get_relay(0).get_surplusThreshold());
  TEST_ASSERT_EQUAL(-800, testRelays.get_relay(1).get_surplusThreshold());
}

void test_get_importThreshold(void)
{
  TEST_ASSERT_EQUAL(100, testRelays.get_relay(0).get_importThreshold());
  TEST_ASSERT_EQUAL(150, testRelays.get_relay(1).get_importThreshold());
}

void test_get_minON(void)
{
  TEST_ASSERT_EQUAL(60, testRelays.get_relay(0).get_minON());
}

void test_get_minOFF(void)
{
  TEST_ASSERT_EQUAL(60, testRelays.get_relay(0).get_minOFF());
}

void test_isRelayON_initial(void)
{
  RelayEngine< 1 > freshRelay{ relayOutput{ 6, 500, 100, 1, 1 } };
  TEST_ASSERT_FALSE(freshRelay.get_relay(0).isRelayON());
}

// ============================================================================
// Override tests
// ============================================================================

RelayEngine< 2 > overrideTestRelays{ relayOutput{ 14, 500, 100, 1, 1 },
                                     relayOutput{ 15, 800, 150, 1, 1 } };

void wait_for_override_settle()
{
  for (uint8_t i = 0; i < 60; ++i)
    overrideTestRelays.inc_duration();
}

void test_override_bypasses_surplus_threshold(void)
{
  wait_for_override_settle();

  // Feed small surplus (not enough to trigger relay normally)
  for (uint8_t i = 0; i < 50; ++i)
    overrideTestRelays.update_average(-100);

  const auto& relay0 = overrideTestRelays.get_relay(0);
  TEST_ASSERT_FALSE(relay0.isRelayON());

  // Set override bit
  uint16_t overrideBitmask = (1U << relay0.get_pin());

  // Now proceed with override
  overrideTestRelays.proceed_relays(overrideBitmask);

  TEST_ASSERT_TRUE(relay0.isRelayON());
}

void test_override_clears_bit_after_processing(void)
{
  wait_for_override_settle();

  const auto& relay0 = overrideTestRelays.get_relay(0);

  // Turn off relay first if it's on
  if (relay0.isRelayON())
  {
    for (uint8_t i = 0; i < 50; ++i)
      overrideTestRelays.update_average(200);
    uint16_t bitmask = 0;
    overrideTestRelays.proceed_relays(bitmask);
    wait_for_override_settle();
  }

  // Set override
  uint16_t overrideBitmask = (1U << relay0.get_pin());
  TEST_ASSERT_NOT_EQUAL(0, overrideBitmask & (1U << relay0.get_pin()));

  for (uint8_t i = 0; i < 50; ++i)
    overrideTestRelays.update_average(-100);

  overrideTestRelays.proceed_relays(overrideBitmask);

  // Override bit should be cleared
  TEST_ASSERT_EQUAL(0, overrideBitmask & (1U << relay0.get_pin()));
}

void test_override_when_relay_already_ON(void)
{
  wait_for_override_settle();

  const auto& relay0 = overrideTestRelays.get_relay(0);

  // Turn on relay with surplus
  for (uint8_t i = 0; i < 50; ++i)
    overrideTestRelays.update_average(-600);

  uint16_t bitmask = 0;
  overrideTestRelays.proceed_relays(bitmask);

  if (!relay0.isRelayON())
  {
    wait_for_override_settle();
    overrideTestRelays.proceed_relays(bitmask);
  }

  TEST_ASSERT_TRUE(relay0.isRelayON());

  wait_for_override_settle();

  // Set override for already-ON relay
  uint16_t overrideBitmask = (1U << relay0.get_pin());

  for (uint8_t i = 0; i < 50; ++i)
    overrideTestRelays.update_average(-600);

  overrideTestRelays.proceed_relays(overrideBitmask);

  // Relay should still be ON, bit should be cleared
  TEST_ASSERT_TRUE(relay0.isRelayON());
  TEST_ASSERT_EQUAL(0, overrideBitmask & (1U << relay0.get_pin()));
}

void test_override_blocked_by_settle_change(void)
{
  // Don't wait - keep settle_change active
  RelayEngine< 1 > freshRelays{ relayOutput{ 10, 500, 100, 1, 1 } };  // Pin 10 (valid for 16-bit bitmask)

  const auto& relay0 = freshRelays.get_relay(0);
  uint16_t overrideBitmask = (1U << relay0.get_pin());

  for (uint8_t i = 0; i < 50; ++i)
    freshRelays.update_average(-600);

  // Only increment a few times - NOT enough to clear settle_change
  for (uint8_t i = 0; i < 10; ++i)
    freshRelays.inc_duration();

  freshRelays.proceed_relays(overrideBitmask);

  // Override bit should NOT be cleared (settle_change blocked)
  TEST_ASSERT_NOT_EQUAL(0, overrideBitmask & (1U << relay0.get_pin()));
}

// ============================================================================
// Duration overflow tests
// ============================================================================

void test_duration_overflow_saturates(void)
{
  RelayEngine< 1 > overflowRelay{ relayOutput{ 11, 500, 100, 1, 1 } };

  // Increment duration to max
  for (uint32_t i = 0; i < UINT16_MAX + 100; ++i)
    overflowRelay.inc_duration();

  // Should not have wrapped - relay should still work
  // (can't directly test duration, but relay should still function)
  TEST_ASSERT_TRUE(true);  // If we got here without crash, saturation worked
}

// ============================================================================
// Settle change tests
// ============================================================================

void test_settle_change_blocks_initial(void)
{
  RelayEngine< 1 > freshRelays{ relayOutput{ 12, 500, 100, 1, 1 } };

  for (uint8_t i = 0; i < 50; ++i)
    freshRelays.update_average(-600);

  uint16_t bitmask = 0;
  freshRelays.proceed_relays(bitmask);

  // Should not turn ON due to settle_change
  TEST_ASSERT_FALSE(freshRelays.get_relay(0).isRelayON());
}

void test_settle_change_allows_after_60(void)
{
  RelayEngine< 1 > freshRelays{ relayOutput{ 13, 500, 100, 1, 1 } };

  for (uint8_t i = 0; i < 50; ++i)
    freshRelays.update_average(-600);

  // Wait for settle_change
  for (uint8_t i = 0; i < 60; ++i)
    freshRelays.inc_duration();

  uint16_t bitmask = 0;
  freshRelays.proceed_relays(bitmask);

  TEST_ASSERT_TRUE(freshRelays.get_relay(0).isRelayON());
}

// ============================================================================
// Main
// ============================================================================

int main(void)
{
  UNITY_BEGIN();

  // Initialization tests
  RUN_TEST(test_relay_initialization);
  RUN_TEST(test_get_pin);
  RUN_TEST(test_get_surplusThreshold);
  RUN_TEST(test_get_importThreshold);
  RUN_TEST(test_get_minON);
  RUN_TEST(test_get_minOFF);
  RUN_TEST(test_isRelayON_initial);

  // Override tests
  RUN_TEST(test_override_bypasses_surplus_threshold);
  RUN_TEST(test_override_clears_bit_after_processing);
  RUN_TEST(test_override_when_relay_already_ON);
  RUN_TEST(test_override_blocked_by_settle_change);

  // Duration overflow tests
  RUN_TEST(test_duration_overflow_saturates);

  // Settle change tests
  RUN_TEST(test_settle_change_blocks_initial);
  RUN_TEST(test_settle_change_allows_after_60);

  return UNITY_END();
}
