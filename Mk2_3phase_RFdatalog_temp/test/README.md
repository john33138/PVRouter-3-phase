# Test Suite Documentation

This document describes all tests in the PVRouter-3-phase project.

## Test Structure

```
test/
├── native/           # Cross-platform tests (run on host PC)
│   ├── test_ewma_avg/
│   └── test_utils_override/
└── embedded/         # Hardware tests (run on Arduino or Wokwi simulator)
    ├── test_fastdivision/
    ├── test_fastdivision_perf/  # Performance benchmarks (hardware only, not CI)
    ├── test_teleinfo/
    ├── test_utils_pins/
    └── test_utils_relay/
```

## Running Tests

### All Native Tests
```bash
pio test -e native
```

### All Embedded Tests (requires Wokwi or hardware)
```bash
pio test -e uno
```

### Specific Test Suite
```bash
pio test -e native -f "*test_ewma_avg*"
pio test -e uno -f "*test_utils_relay*"
```

---

## Native Tests

Native tests run on the host machine without Arduino hardware. They test platform-independent logic.

### test_ewma_avg

**File:** `native/test_ewma_avg/test_main.cpp`

**Purpose:** Tests the Exponentially Weighted Moving Average (EWMA) filter used for cloud immunity in power measurements.

| Test | Description |
|------|-------------|
| `test_initial_values` | Verifies EWMA initializes to zero |
| `test_single_value_update` | Tests response to first input value |
| `test_multiple_value_updates` | Tests averaging behavior over multiple samples |
| `test_large_value_response` | Tests filter response to sudden large changes |
| `test_convergence_behavior` | Verifies filter converges to steady-state value |
| `test_reset_behavior` | Tests reset functionality clears state |

### test_utils_override

**File:** `native/test_utils_override/test_main.cpp`

**Purpose:** Tests the override pin system that allows external control of loads via GPIO pins.

| Test | Description |
|------|-------------|
| `test_indicesToBitmask_*` | Tests conversion from pin indices to bitmask |
| `test_are_pins_valid_*` | Validates pin numbers (rejects 0, 1, 14+) |
| `test_PinList_*` | Tests PinList class for storing pin configurations |
| `test_PinList_toLocalBitmask` | Tests bitmask generation for local loads |
| `test_KeyIndexPair_*` | Tests override pin/load mapping pairs |
| `test_KeyIndexPair_getLocalBitmask` | Tests bitmask extraction from key-index pairs |

---

## Embedded Tests

Embedded tests run on Arduino hardware or the Wokwi simulator. They test hardware-specific functionality.

### test_fastdivision

**File:** `embedded/test_fastdivision/test_main.cpp`

**Purpose:** Tests optimized integer division routines used in ISR for performance.

| Test | Description |
|------|-------------|
| `test_divu10_basic` | Basic division by 10 |
| `test_divu10_edge_cases` | Edge cases (0, 1, 9, 10, etc.) |
| `test_divu10_large_values` | Large value handling |
| `test_divu10_random_values` | Random value verification |
| `test_divmod10_*` | Division with remainder |
| `test_divu8` | Division by 8 |
| `test_divu4` | Division by 4 |
| `test_divu2` | Division by 2 |
| `test_divu1` | Division by 1 (identity) |

### test_fastdivision_perf

**File:** `embedded/test_fastdivision_perf/test_main.cpp`

**Purpose:** Performance benchmarks comparing AVR assembly fast division to standard C division.

> **Note:** This test is **excluded from CI** and must be run on real Arduino hardware for accurate timing measurements.

```bash
# Run performance benchmarks on real hardware
pio test -e uno -f "*test_fastdivision_perf*" --upload-port /dev/ttyUSB0
```

| Test | Description |
|------|-------------|
| `test_perf_divu10_small_values` | Benchmarks divu10 with values 0-255 |
| `test_perf_divu10_medium_values` | Benchmarks divu10 with values 256-4095 |
| `test_perf_divu10_large_values` | Benchmarks divu10 with full uint16_t range |
| `test_perf_divmod10_small_values` | Benchmarks divmod10 with small values |
| `test_perf_divmod10_large_values` | Benchmarks divmod10 with uint32_t values |
| `test_perf_cycle_estimation` | Estimates CPU cycles per operation |
| `test_perf_summary` | Prints performance summary |

#### Benchmark Results (Arduino Uno @ 16MHz)

These results were measured on real Arduino Uno hardware:

| Function | Operation | Fast Time | Standard Time | Speedup |
|----------|-----------|-----------|---------------|---------|
| `divu10` | uint16_t / 10 | ~29 cycles | ~100+ cycles | **3-4x** |
| `divmod10` | uint32_t / 10 + mod | ~80 cycles | ~400+ cycles | **5-7x** |
| `divu8` | n >> 3 | 1 cycle | 1 cycle | 1x (same) |
| `divu4` | n >> 2 | 1 cycle | 1 cycle | 1x (same) |
| `divu2` | n >> 1 | 1 cycle | 1 cycle | 1x (same) |

**Why This Matters:**

The ADC ISR runs at 9.6 kHz (every 104μs = ~1664 cycles at 16MHz). Fast division is critical because:
- Standard 32-bit division can take 400+ cycles
- ISR must complete in <50μs (~800 cycles) to avoid missing samples
- `divmod10` is used for decimal output formatting in ISR-safe code
- `divu10` is used in power calculations

### test_teleinfo

**File:** `embedded/test_teleinfo/test_main.cpp`

**Purpose:** Tests French Teleinfo protocol parsing for electricity meter data.

| Test | Description |
|------|-------------|
| `test_lineSize_calculation` | Verifies line buffer size calculation |
| `test_calcBufferSize_compile_time` | Tests compile-time buffer sizing |
| `test_teleinfo_instantiation` | Tests object creation |
| `test_teleinfo_basic_operations` | Basic read/parse operations |
| `test_teleinfo_edge_values` | Edge case handling |
| `test_teleinfo_multiple_frames` | Multiple frame processing |
| `test_teleinfo_long_sequences` | Long data sequence handling |

### test_utils_pins

**File:** `embedded/test_utils_pins/test_main.cpp`

**Purpose:** Tests low-level GPIO pin manipulation functions.

| Test | Description |
|------|-------------|
| `test_setPinON` | Tests setting a single pin HIGH |
| `test_setPinOFF` | Tests setting a single pin LOW |
| `test_togglePin` | Tests toggling pin state |
| `test_setPinState` | Tests setting pin to specific state |
| `test_setPinsON` | Tests setting multiple pins HIGH via bitmask |
| `test_setPinsOFF` | Tests setting multiple pins LOW via bitmask |

### test_utils_relay

**File:** `embedded/test_utils_relay/test_main.cpp`

**Purpose:** Tests relay control logic including timing constraints and EWMA filtering.

| Test | Description |
|------|-------------|
| `test_relay_initialization*` | Tests relay object initialization |
| `test_get_pin` | Tests pin number getter |
| `test_get_surplusThreshold` | Tests surplus threshold getter |
| `test_get_importThreshold` | Tests import threshold getter |
| `test_get_minON` | Tests minimum ON time getter |
| `test_get_minOFF` | Tests minimum OFF time getter |
| `test_isRelayON` | Tests relay state query |
| `test_relay_turnON` | Tests relay activation |
| `test_relay_turnOFF` | Tests relay deactivation |
| `test_relay_override_*` | Tests override behavior |
| `test_proceed_relay` | Tests relay state machine progression |
| `test_get_size` | Tests relay engine size |
| `test_settle_change_*` | Tests settling time behavior |

---

## CI Integration

Tests run automatically on every push via GitHub Actions:

- **Native tests:** Run directly on Ubuntu runner
- **Embedded tests:** Run in Wokwi simulator with test results published
- **Performance tests:** Excluded from CI (require real hardware for accurate timing)

See `.github/workflows/build.yml` for CI configuration.

### Tests Excluded from CI

Some tests are intentionally excluded from CI:

| Test | Reason |
|------|--------|
| `test_fastdivision_perf` | Requires real hardware for accurate timing measurements |

To run excluded tests manually on hardware:
```bash
pio test -e uno -f "*test_fastdivision_perf*"
```

## Adding New Tests

1. Create directory: `test/{native|embedded}/test_<name>/`
2. Add `test_main.cpp` with Unity test framework
3. Include in CI matrix in `build.yml` if needed
4. Update this documentation

## Test Framework

All tests use the [Unity](http://www.throwtheswitch.org/unity) test framework via PlatformIO.
