/**
 * @file RemoteLoadReceiver.ino
 * @brief Main sketch for remotely controlled loads via RF using RFM69
 * @version 2.0
 * @date 2025-10-26
 * @author Based on remoteUnit_fasterControl_1 by Robin Emley
 *
 * @details This sketch receives RF commands from the main PV Router and controls
 *          local loads (TRIACs or SSRs) accordingly. Uses RFM69 library (maintained)
 *          instead of deprecated JeeLib.
 *
 *          Supports up to 8 loads via a bitmask protocol that minimizes data transmission.
 *
 *          Features:
 *          - Receives compact bitmask commands (1 byte for up to 8 loads)
 *          - Detects RF link failures (no messages for > 500ms)
 *          - LED indicators for RF status and system watchdog
 *          - Automatic safety: turns all loads OFF on RF link loss
 *
 *          RF Status Indication:
 *          - Green LED (D5): Blinks at 1Hz as watchdog (system alive)
 *          - Red LED (D7): Fast blink (~4Hz) when RF link is lost
 *
 * Hardware:
 * - Arduino UNO or compatible
 * - RFM69W/CW or RFM69HW/HCW RF module
 * - TRIAC/SSR outputs for loads
 * - Status LEDs (green and red)
 *
 *      www.Mk2PVrouter.co.uk
 */

#include "config.h"

// Verify C++17 is enabled
static_assert(__cplusplus >= 201703L, "This project requires C++17 or later");
