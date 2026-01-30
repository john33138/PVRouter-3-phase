/**
 * @file test_main.cpp
 * @brief Unity-based unit tests for assembly multiplication functions
 * @version 0.1
 * @date 2026-01-30
 *
 * This file contains comprehensive unit tests for the assembly-optimized
 * multiplication functions using the Unity testing framework.
 * 
 * Based on:
 * - florentbr's optimization suggestions for PVRouter
 * - avrfreertos multiplication optimizations by feilipu
 * - OpenMusicLabs AVR assembly techniques
 */

#include <Arduino.h>
#include <unity.h>

#include "mult_asm.h"

// Global volatile to prevent optimization without affecting timing
volatile uint8_t g_optimizer_defeat = 0;

// ============================================================================
// Test data structures for data-driven testing
// ============================================================================

// Test case for signed 16x16->32 multiplication
struct TestCaseS16x16
{
  int16_t a;
  int16_t b;
  int32_t expected;
};

// Test case for unsigned 16x16->32 multiplication
struct TestCaseU16x16
{
  uint16_t a;
  uint16_t b;
  uint32_t expected;
};

// Test case for Q8 fractional multiplication
struct TestCaseQ8
{
  int16_t value;
  uint8_t fraction;
  int16_t expected;
};

// ============================================================================
// Test data arrays
// ============================================================================

// Signed multiplication test cases
const TestCaseS16x16 testDataS16x16[] = {
  // Basic positive × positive
  { 2, 3, 6 },
  { 100, 100, 10000 },
  { 1000, 1000, 1000000 },
  // Positive × negative
  { 2, -3, -6 },
  { 100, -100, -10000 },
  { -1000, 1000, -1000000 },
  // Negative × negative
  { -2, -3, 6 },
  // Zero cases
  { 0, 1000, 0 },
  { 1000, 0, 0 },
  // Maximum/minimum values
  { 32767, 1, 32767 },
  { -32768, 1, -32768 },
  { 32767, 2, 65534 },
  { -32768, 2, -65536 },
  { 32767, -1, -32767 },
  { -32768, -1, 32768 },
  // Squared values (max positive/negative)
  { 32767, 32767, 1073676289L },
  { -32768, -32768, 1073741824L },
  { 32767, -32768, -1073709056L },
  { -32768, 32767, -1073709056L },
  // Typical ADC range values
  { 1648, 512, 843776 },
  { -1648, 512, -843776 },
  { 1648, -512, -843776 },
  { -32640, 257, -8388480 },
};
const uint8_t numTestsS16x16 = sizeof(testDataS16x16) / sizeof(testDataS16x16[0]);

// Unsigned multiplication test cases
const TestCaseU16x16 testDataU16x16[] = {
  // Basic multiplication
  { 2, 3, 6 },
  { 100, 100, 10000 },
  { 1000, 1000, 1000000 },
  // Zero cases
  { 0, 1000, 0 },
  { 1000, 0, 0 },
  // Boundary values
  { 1, 65535, 65535 },
  { 65535, 1, 65535 },
  // Maximum value (65535²)
  { 65535, 65535, 4294836225UL },
  // Typical ADC values
  { 32768, 32768, 1073741824UL },
  { 1648, 1648, 2715904 },
  // Powers of 2
  { 256, 256, 65536 },
  { 512, 512, 262144 },
  { 1024, 1024, 1048576 },
};
const uint8_t numTestsU16x16 = sizeof(testDataU16x16) / sizeof(testDataU16x16[0]);

// Q8 fractional multiplication test cases
const TestCaseQ8 testDataQ8[] = {
  // Basic fractions (128=0.5, 64=0.25, 192=0.75, 255≈1.0)
  { 100, 128, 50 },   // 100 * 0.5 = 50
  { 100, 64, 25 },    // 100 * 0.25 = 25
  { 100, 192, 75 },   // 100 * 0.75 = 75
  { 100, 255, 100 },  // 100 * ~1.0 ≈ 100
  // Negative values
  { -100, 128, -50 },
  { -100, 64, -25 },
  // Zero cases
  { 100, 0, 0 },
  { 0, 128, 0 },
  { 32767, 0, 0 },
  { -32768, 0, 0 },
  { 0, 255, 0 },
  // Maximum values
  { 32767, 128, 16384 },
  { -32768, 128, -16384 },
  { 32767, 255, 32639 },
  { -32768, 255, -32640 },
  { 32767, 1, 128 },
  { -32768, 1, -128 },
  // Rounding tests
  { 100, 127, 50 },   // 49.609... rounds to 50
  { 100, 129, 50 },   // 50.390... rounds to 50
  { 256, 128, 128 },  // Exact: 256 * 0.5 = 128
  { 512, 64, 128 },   // Exact: 512 * 0.25 = 128
  // Small fractions
  { 1000, 1, 4 },  // 1000 * (1/256) ≈ 4
  { 1000, 2, 8 },  // 1000 * (2/256) ≈ 8
  { 256, 1, 1 },   // 256 * (1/256) = 1
  { 128, 1, 1 },   // 128 * (1/256) = 0.5 → rounds to 1
  { 127, 1, 0 },   // 127 * (1/256) = 0.496... → rounds to 0
  // Near-overflow scenarios
  { 30000, 200, 23438 },    // Large value * large fraction
  { -30000, 200, -23437 },  // Large negative * large fraction
};
const uint8_t numTestsQ8 = sizeof(testDataQ8) / sizeof(testDataQ8[0]);

void setUp(void)
{
  // Set up code here (if needed)
}

void tearDown(void)
{
  // Clean up code here (if needed)
}

/**
 * @brief Test basic functionality of multS16x16_to32
 */
void test_multS16x16_to32_basic(void)
{
  int32_t result;

  for (uint8_t i = 0; i < numTestsS16x16; ++i)
  {
    multS16x16_to32(result, testDataS16x16[i].a, testDataS16x16[i].b);
    TEST_ASSERT_EQUAL_MESSAGE(testDataS16x16[i].expected, result, "Failed at test index");
  }
}

/**
 * @brief Test basic functionality of multU16x16_to32 (unsigned)
 */
void test_multU16x16_to32_basic(void)
{
  uint32_t result;

  for (uint8_t i = 0; i < numTestsU16x16; ++i)
  {
    multU16x16_to32(result, testDataU16x16[i].a, testDataU16x16[i].b);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(testDataU16x16[i].expected, result, "Failed at test index");
  }
}

/**
 * @brief Test basic functionality of mult16x8_q8
 */
void test_mult16x8_q8_basic(void)
{
  int16_t result;

  for (uint8_t i = 0; i < numTestsQ8; ++i)
  {
    mult16x8_q8(result, testDataQ8[i].value, testDataQ8[i].fraction);
    TEST_ASSERT_EQUAL_MESSAGE(testDataQ8[i].expected, result, "Failed at test index");
  }
}

/**
 * @brief Test Q8 format conversion helpers
 */
void test_q8_conversion_helpers(void)
{
  // Test float_to_q8 conversion
  TEST_ASSERT_EQUAL(0, float_to_q8(0.0f));
  TEST_ASSERT_EQUAL(64, float_to_q8(0.25f));
  TEST_ASSERT_EQUAL(128, float_to_q8(0.5f));
  TEST_ASSERT_EQUAL(192, float_to_q8(0.75f));
  TEST_ASSERT_EQUAL(255, float_to_q8(1.0f));

  // Test q8_to_float conversion (approximate due to floating-point precision)
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, q8_to_float(0));
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.25f, q8_to_float(64));
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.5f, q8_to_float(128));
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.75f, q8_to_float(192));
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.996f, q8_to_float(255));  // 255/256 ≈ 0.996
}


/**
 * @brief Compare assembly results with standard multiplication
 */
void test_assembly_vs_standard(void)
{
  int32_t asm_result, std_result;
  uint32_t asm_result_u, std_result_u;
  int16_t asm_result16, std_result16;

  // Test values for signed multiplication
  int16_t test_vals[] = { 100, -200, 1000, -1500, 32767, -32768 };
  uint8_t test_fracs[] = { 64, 128, 192, 255 };  // 0.25, 0.5, 0.75, ~1.0

  // Compare multS16x16_to32 results
  for (uint8_t i = 0; i < 6; i++)
  {
    for (uint8_t j = 0; j < 6; j++)
    {
      multS16x16_to32(asm_result, test_vals[i], test_vals[j]);
      std_result = (int32_t)test_vals[i] * test_vals[j];
      TEST_ASSERT_EQUAL(std_result, asm_result);
    }
  }

  // Test values for unsigned multiplication
  uint16_t test_vals_u[] = { 0, 1, 100, 1000, 32767, 32768, 65535 };
  const uint8_t num_vals_u = sizeof(test_vals_u) / sizeof(test_vals_u[0]);

  // Compare multU16x16_to32 results
  for (uint8_t i = 0; i < num_vals_u; i++)
  {
    for (uint8_t j = 0; j < num_vals_u; j++)
    {
      multU16x16_to32(asm_result_u, test_vals_u[i], test_vals_u[j]);
      std_result_u = (uint32_t)test_vals_u[i] * test_vals_u[j];
      TEST_ASSERT_EQUAL_UINT32(std_result_u, asm_result_u);
    }
  }

  // Compare mult16x8_q8 results
  for (uint8_t i = 0; i < 6; i++)
  {
    for (uint8_t j = 0; j < 4; j++)
    {
      mult16x8_q8(asm_result16, test_vals[i], test_fracs[j]);
      std_result16 = ((int32_t)test_vals[i] * test_fracs[j] + 0x80) >> 8;
      TEST_ASSERT_EQUAL(std_result16, asm_result16);
    }
  }
}

void setup()
{
  delay(1000);    // Wait for Serial to initialize
  UNITY_BEGIN();  // Start Unity test framework
}

uint8_t i = 0;
uint8_t max_blinks = 1;

void loop()
{
  if (i < max_blinks)
  {
    // Functional tests
    RUN_TEST(test_multS16x16_to32_basic);
    delay(100);
    RUN_TEST(test_multU16x16_to32_basic);
    delay(100);
    RUN_TEST(test_mult16x8_q8_basic);
    delay(100);
    RUN_TEST(test_q8_conversion_helpers);
    delay(100);
    RUN_TEST(test_assembly_vs_standard);
    delay(100);

    ++i;
  }
  else if (i == max_blinks)
  {
    UNITY_END();  // End Unity test framework
    ++i;
  }
}