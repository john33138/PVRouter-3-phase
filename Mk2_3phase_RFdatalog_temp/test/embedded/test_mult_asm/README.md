# Assembly Multiplication Functions Tests

This directory contains Unity-based unit tests for the assembly-optimized multiplication functions.

## Running the Tests

To run the multiplication function tests on embedded hardware (AVR):

```bash
# Run all embedded tests
pio test --environment uno

# Run only the multiplication tests
pio test --environment uno --filter test_mult_asm

# Run with verbose output
pio test --environment uno --filter test_mult_asm --verbose
```

## Test Coverage

The tests cover:

1. **Basic Functionality**
   - `mult16x16_to32`: 16×16→32 signed multiplication
   - `mult16x8_q8`: 16×8 Q8 fractional multiplication with rounding

2. **Edge Cases**
   - Zero values
   - Maximum/minimum values
   - Overflow scenarios

3. **Q8 Format Testing**
   - Conversion helpers (`float_to_q8`, `q8_to_float`)
   - Rounding behavior
   - Fractional arithmetic accuracy

4. **Assembly vs Standard Comparison**
   - Verifies assembly results match standard C multiplication
   - Tests across multiple value ranges

5. **Performance Benchmarks**
   - Measures execution time of assembly vs standard implementations
   - ISR simulation to demonstrate real-world impact

## Benchmark Results (Arduino Uno @ 16MHz)

Results measured on real Arduino Uno hardware:

| Function | Operations | Assembly Time | Standard Time | Speedup |
|----------|------------|---------------|---------------|---------|
| `mult16x16_to32` | 36,000 | 99,376μs | 146,744μs | **1.48x** |
| `mult16x8_q8` | 480,000 | 792,832μs | 1,867,320μs | **2.36x** |

**ISR Simulation (1,000 samples):**

| Metric | Assembly | Standard | Improvement |
|--------|----------|----------|-------------|
| Total time | 8,364μs | 12,076μs | **1.44x faster** |
| Time per sample | 8.36μs | 12.08μs | **3.71μs saved** |

**Why This Matters:**

The ADC ISR runs at 9.6 kHz (every 104μs). Saving 3.71μs per ISR iteration:
- Reduces ISR execution time by ~3.6%
- Provides more headroom for other processing
- Critical for maintaining real-time performance

## Test Structure

The tests follow the Unity testing framework pattern used throughout the project:
- `setUp()` and `tearDown()` for test initialization/cleanup
- Individual test functions with descriptive names
- `TEST_ASSERT_*` macros for assertions
- Arduino-style `setup()` and `loop()` structure

## Hardware Requirements

These tests require AVR hardware (Arduino Uno/compatible) since they test AVR-specific assembly optimizations.