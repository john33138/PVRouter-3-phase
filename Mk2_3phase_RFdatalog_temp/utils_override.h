/**
 * @file utils_override.h
 * @brief Compile-time utilities for managing override pins and index-to-bitmask mapping.
 *
 * This header provides types and functions for representing and manipulating sets of override pins
 * and their associated pins, all at compile time. It enables efficient bitmask computation and
 * static configuration of pin mappings for embedded systems, such as PVRouter.
 *
 * Pin features:
 * - Compile-time conversion of index lists to bitmasks
 * - Type-safe representation of pin/index associations
 * - Static configuration of override pin mappings
 * - Template deduction guide for convenient usage
 *
 * Usage example:
 * @code
 * constexpr OverridePins overridePins{
 *     {2, {1, 3}},
 *     {3, {0, 2, 6}},
 *     {4, {5}}
 * };
 * constexpr OverridePins<3, 8> pins{pairs};
 * @endcode
 *
 * @author Frédéric Metrich (frederic.metrich@live.fr)
 * @version 0.1
 * @date 2026-01-25
 * @copyright Copyright (c) 2025-2026
 */

#ifndef UTILS_OVERRIDE_H
#define UTILS_OVERRIDE_H

#include "config.h"
#include "type_traits.hpp"

/**
 * @brief Base value for virtual pins representing remote loads.
 * @details Values >= REMOTE_PIN_BASE are virtual pins for remote loads.
 *          REMOTE_LOAD(0) returns 128, REMOTE_LOAD(1) returns 129, etc.
 */
inline constexpr uint8_t REMOTE_PIN_BASE{ 128 };

namespace override_detail
{
template< uint8_t NumLocal >
constexpr uint8_t local_load_impl(uint8_t loadNum)
{
  if constexpr (NumLocal == 0)
  {
    (void)loadNum;
    return unused_pin;
  }
  else
  {
    return (loadNum < NumLocal) ? physicalLoadPin[loadNum] : unused_pin;
  }
}
}  // namespace override_detail

/**
 * @brief Returns the physical pin number for a given LOCAL load index at compile time.
 * @param loadNum The local load index (0-based).
 * @return The physical pin number for the local load.
 * @note This only works for local loads (index < numLocalLoads).
 */
constexpr uint8_t LOCAL_LOAD(uint8_t loadNum)
{
  return override_detail::local_load_impl< NO_OF_DUMPLOADS - NO_OF_REMOTE_LOADS >(loadNum);
}

/**
 * @brief Returns a virtual pin for a given REMOTE load index at compile time.
 * @param remoteLoadNum The remote load index (0-based, relative to remote loads).
 * @return Virtual pin value (128 + remoteLoadNum).
 */
constexpr uint8_t REMOTE_LOAD(uint8_t remoteLoadNum)
{
  if constexpr (NO_OF_REMOTE_LOADS > 0)
  {
    if (remoteLoadNum < NO_OF_REMOTE_LOADS)
    {
      return REMOTE_PIN_BASE + remoteLoadNum;
    }
  }
  return unused_pin;
}

namespace override_detail
{
template< uint8_t NumLocal, uint8_t NumTotal >
constexpr uint8_t load_impl(uint8_t loadNum)
{
  if constexpr (NumLocal == 0)
  {
    // All loads are remote
    return (loadNum < NumTotal) ? REMOTE_PIN_BASE + loadNum : unused_pin;
  }
  else
  {
    // Has local loads - check local first, then remote
    if (loadNum < NumLocal)
    {
      return physicalLoadPin[loadNum];
    }
    return (loadNum < NumTotal) ? REMOTE_PIN_BASE + (loadNum - NumLocal) : unused_pin;
  }
}
}  // namespace override_detail

/**
 * @brief Returns the pin number (physical or virtual) for a given load index at compile time.
 * @param loadNum The load index (0-based).
 * @return The pin number for local loads, or virtual pin for remote loads.
 * @note Local loads return their physical pin, remote loads return virtual pins (>= 128).
 */
constexpr uint8_t LOAD(uint8_t loadNum)
{
  return override_detail::load_impl< NO_OF_DUMPLOADS - NO_OF_REMOTE_LOADS, NO_OF_DUMPLOADS >(loadNum);
}

/**
 * @brief Returns the pin number for a given relay index at compile time.
 * @param relayNum The relay index (0-based).
 * @return The pin number for the relay.
 */
constexpr uint8_t RELAY(uint8_t relayNum)
{
  return relays.get_relay(relayNum).get_pin();
}

/**
 * @brief Returns a bitmask representing all LOCAL load pins.
 *
 * This helper is used to configure an override pin to control all local loads at once.
 * Note: Only includes LOCAL loads (physical pins), not remote loads.
 *
 * @return uint32_t with lower 16 bits set for local load pins.
 */
constexpr uint32_t ALL_LOCAL_LOADS()
{
  uint32_t mask{ 0 };
  for (uint8_t i = 0; i < (NO_OF_DUMPLOADS - NO_OF_REMOTE_LOADS); ++i)
  {
    bit_set(mask, physicalLoadPin[i]);
  }
  return mask;
}

/**
 * @brief Returns a bitmask representing all REMOTE loads.
 *
 * This helper is used to configure an override pin to control all remote loads at once.
 * Remote loads are encoded in the upper 16 bits (bit 16 = remote 0, bit 17 = remote 1, etc.).
 *
 * @return uint32_t with upper 16 bits set for remote loads.
 */
constexpr uint32_t ALL_REMOTE_LOADS()
{
  uint32_t mask{ 0 };
  for (uint8_t i = 0; i < NO_OF_REMOTE_LOADS; ++i)
  {
    bit_set(mask, 16 + i);  // Set bit 16+i for remote load i
  }
  return mask;
}

/**
 * @brief Returns a bitmask representing all load pins (local + remote).
 *
 * This helper is used to configure an override pin to control all loads at once.
 * Lower 16 bits = local load pins, upper 16 bits = remote loads.
 *
 * @return uint32_t with all local and remote load bits set.
 */
constexpr uint32_t ALL_LOADS()
{
  return ALL_LOCAL_LOADS() | ALL_REMOTE_LOADS();
}

/**
 * @brief Returns a bitmask representing all relay pins.
 *
 * This helper is used to configure an override pin to control all relays at once.
 * Relays are local devices, so they use the lower 16 bits.
 *
 * @return uint32_t with lower 16 bits set for relay pins.
 */
constexpr uint32_t ALL_RELAYS()
{
  uint32_t mask{ 0 };
  for (uint8_t i = 0; i < relays.size(); ++i)
  {
    bit_set(mask, relays.get_relay(i).get_pin());
  }
  return mask;
}

/**
 * @brief Returns a bitmask representing all loads and all relays.
 *
 * This helper is used to configure an override pin to control the entire system (all loads and relays).
 * Lower 16 bits = local loads + relays, upper 16 bits = remote loads.
 *
 * @return uint32_t with all load and relay bits set.
 */
constexpr uint32_t ALL_LOADS_AND_RELAYS()
{
  return ALL_LOADS() | ALL_RELAYS();
}

// Valid pins: 2-13, so valid mask is 0b11111111111100
constexpr uint16_t validPinMask{ 0b11111111111100 };

/**
 * @brief Compile-time validation function for pin values.
 * @tparam Pins List of pins to validate.
 * @return true if all pins are valid, false otherwise.
 */
template< uint8_t... Pins >
constexpr bool are_pins_valid()
{
  return ((validPinMask & (1U << Pins)) && ...);
}

/**
 * @brief Helper macro to validate pins at compile time.
 * Usage: VALIDATE_PINS(2, 3, 5) will cause a compile error if any pin is invalid.
 */
#define VALIDATE_PINS(...) static_assert(are_pins_valid< __VA_ARGS__ >(), "Invalid pin(s) specified")

/**
 * @brief Helper to convert pins to a bitmask at compile-time.
 * @tparam Pins List of pins to set in the bitmask.
 * @return Bitmask with bits set at the specified pins.
 */
template< uint8_t... Pins >
constexpr uint16_t indicesToBitmask()
{
  return ((1U << Pins) | ...);
}

/**
 * @brief Wrapper for a list of pins, constructible from variadic arguments.
 * @tparam MaxPins Maximum number of pins supported.
 *
 * @details Supports both physical pins (0-127) and virtual pins for remote loads (128+).
 *          Virtual pins are created using REMOTE_LOAD(n) which returns REMOTE_PIN_BASE + n.
 */
template< uint8_t MaxPins >
struct PinList
{
  uint8_t pins[MaxPins];
  uint8_t count;

  /**
   * @brief Default constructor. Initializes with zero pins.
   */
  constexpr PinList()
    : pins{}, count(0) {}

  /**
   * @brief Constructor from combined bitmask. Extracts both local and remote pins.
   * @param bitmask Combined bitmask value:
   *                - Lower 16 bits (0-15): local/physical pin bitmask
   *                - Upper 16 bits (16-31): remote load bitmask (bit 16 = remote 0, etc.)
   */
  constexpr PinList(uint32_t bitmask)
    : pins{}, count(0)
  {
    // Extract local pins from lower 16 bits
    for (uint8_t pin = 0; pin < 16 && count < MaxPins; ++pin)
    {
      if (bitmask & (1UL << pin))
      {
        pins[count++] = pin;  // Store the physical pin number
      }
    }
    // Extract remote loads from upper 16 bits
    for (uint8_t i = 0; i < 8 && count < MaxPins; ++i)
    {
      if (bitmask & (1UL << (16 + i)))
      {
        pins[count++] = REMOTE_PIN_BASE + i;  // Store virtual pin for remote load
      }
    }
  }

  /**
   * @brief Variadic constructor. Initializes with provided pins.
   * @param args List of pins (can include both physical and virtual pins).
   */
  template< typename... Args >
  constexpr PinList(Args... args)
    : pins{ static_cast< uint8_t >(args)... }, count(sizeof...(args)) {}

  /**
   * @brief Converts the pin list to a bitmask for local/physical pins only.
   * @return Bitmask with bits set for pins < REMOTE_PIN_BASE.
   */
  constexpr uint16_t toLocalBitmask() const
  {
    uint16_t result{ 0 };
    for (uint8_t i = 0; i < count; ++i)
    {
      if (pins[i] < REMOTE_PIN_BASE)
      {
        result |= (1U << pins[i]);
      }
    }
    return result;
  }

  /**
   * @brief Converts the pin list to a bitmask for remote loads only.
   * @return Bitmask where bit n is set if remote load n is in the list.
   *         (Pins >= REMOTE_PIN_BASE are converted to indices 0-7)
   */
  constexpr uint8_t toRemoteBitmask() const
  {
    uint8_t result{ 0 };
    for (uint8_t i = 0; i < count; ++i)
    {
      if (pins[i] >= REMOTE_PIN_BASE)
      {
        const uint8_t remoteIndex = pins[i] - REMOTE_PIN_BASE;
        if (remoteIndex < 8)
        {
          result |= (1U << remoteIndex);
        }
      }
    }
    return result;
  }
};

/**
 * @brief Structure holding a pin and its associated index list.
 * @tparam MaxPins Maximum number of pins supported.
 */
template< uint8_t MaxPins >
struct KeyIndexPair
{
  uint8_t pin;
  PinList< MaxPins > indexList;

  /**
   * @brief Constructor.
   * @param k Pin value.
   * @param list Index list.
   */
  constexpr KeyIndexPair(uint8_t k, const PinList< MaxPins >& list)
    : pin(k), indexList(list) {}

  /**
   * @brief Returns the bitmask for local pins in the index list.
   * @return Bitmask for physical pins (< REMOTE_PIN_BASE).
   */
  constexpr uint16_t getLocalBitmask() const
  {
    return indexList.toLocalBitmask();
  }

  /**
   * @brief Returns the bitmask for remote loads in the index list.
   * @return Bitmask where bit n is set if remote load n is in the list.
   */
  constexpr uint8_t getRemoteBitmask() const
  {
    return indexList.toRemoteBitmask();
  }
};

/**
 * @class OverridePins
 * @brief Manages override pins and their associated bitmasks for forced operation.
 *
 * This class provides compile-time mapping between override pins and the loads/relays they control.
 * Each pin can be associated with a set of pins (loads/relays), represented as separate bitmasks
 * for local loads/relays and remote loads.
 *
 * @tparam N Number of pin-index pairs (entries).
 * @tparam MaxPins Maximum number of pins supported (loads + relays).
 */
template< uint8_t N, uint8_t MaxPins = NO_OF_DUMPLOADS + relays.size() >
class OverridePins
{
private:
  /**
   * @struct Entry
   * @brief Internal structure representing a pin and its associated bitmasks.
   *
   * @var Entry::pin
   * Pin number for override control.
   * @var Entry::localBitmask
   * Bitmask representing the local loads/relays (physical pins 2-13) controlled by this pin.
   * @var Entry::remoteBitmask
   * Bitmask representing the remote loads (bit 0 = remote load 0, etc.) controlled by this pin.
   */
  struct Entry
  {
    uint8_t pin;           /**< Pin value. */
    uint16_t localBitmask; /**< Bitmask for local loads/relays. */
    uint8_t remoteBitmask; /**< Bitmask for remote loads. */
  };

  const Entry entries_[N]; /**< Array of entries for all pin-index pairs. */

public:
  /**
   * @brief Constructor. Initializes the override pin mapping from pin-index pairs.
   *
   * @param pairs Array of pin-index pairs, each specifying a pin and its associated pins.
   */
  constexpr OverridePins(const KeyIndexPair< MaxPins > (&pairs)[N])
    : entries_{}  // default initialize
  {
    Entry temp[N]{};
    for (uint8_t i = 0; i < N; ++i)
    {
      temp[i] = { pairs[i].pin, pairs[i].getLocalBitmask(), pairs[i].getRemoteBitmask() };
    }
    for (uint8_t i = 0; i < N; ++i)
    {
      const_cast< Entry& >(entries_[i]) = temp[i];
    }
  }

  /**
   * @brief Returns the number of override pin entries.
   * @return Number of pin-index pairs (entries).
   */
  constexpr uint8_t size() const
  {
    return N;
  }

  /**
   * @brief Returns the pin number at the specified entry index.
   * @param index Index in the entries array.
   * @return Pin value, or 0 if out of bounds.
   */
  constexpr uint8_t getPin(uint8_t index) const
  {
    return index < N ? entries_[index].pin : 0;
  }

  /**
   * @brief Returns the local bitmask for the specified entry index.
   * @param index Index in the entries array.
   * @return Local bitmask for physical pins, or 0 if out of bounds.
   */
  constexpr uint16_t getLocalBitmask(uint8_t index) const
  {
    return index < N ? entries_[index].localBitmask : 0;
  }

  /**
   * @brief Returns the remote bitmask for the specified entry index.
   * @param index Index in the entries array.
   * @return Remote bitmask (bit n = remote load n), or 0 if out of bounds.
   */
  constexpr uint8_t getRemoteBitmask(uint8_t index) const
  {
    return index < N ? entries_[index].remoteBitmask : 0;
  }

  /**
   * @brief Finds the local bitmask associated with a given pin number.
   * @param pin Pin value to search for.
   * @return Local bitmask for the pin, or 0 if not found.
   */
  constexpr uint16_t findLocalBitmask(uint8_t pin) const
  {
    for (uint8_t i = 0; i < N; ++i)
    {
      if (entries_[i].pin == pin)
      {
        return entries_[i].localBitmask;
      }
    }
    return 0;
  }

  /**
   * @brief Finds the remote bitmask associated with a given pin number.
   * @param pin Pin value to search for.
   * @return Remote bitmask for the pin, or 0 if not found.
   */
  constexpr uint8_t findRemoteBitmask(uint8_t pin) const
  {
    for (uint8_t i = 0; i < N; ++i)
    {
      if (entries_[i].pin == pin)
      {
        return entries_[i].remoteBitmask;
      }
    }
    return 0;
  }

  /**
   * @brief Print the configured override pins and their bitmasks to Serial during startup.
   *
   * This method outputs the pin number and associated bitmasks (local and remote) for each entry.
   */
  void printOverrideConfig() const
  {
    Serial.println(F("*** Override Pins Configuration ***"));
    for (uint8_t i = 0; i < N; ++i)
    {
      Serial.print(F("\tPin: "));
      Serial.print(entries_[i].pin);
      Serial.print(F("\tLocal: 0b"));
      Serial.print(entries_[i].localBitmask, BIN);
      Serial.print(F("\tRemote: 0b"));
      Serial.println(entries_[i].remoteBitmask, BIN);
    }
  }
};

/**
 * @brief Deduction guide for OverridePins template.
 * Allows template argument deduction from constructor arguments.
 */
template< uint8_t MaxPins, uint8_t N >
OverridePins(const KeyIndexPair< MaxPins > (&)[N])
  -> OverridePins< N, MaxPins >;

#endif /* UTILS_OVERRIDE_H */
