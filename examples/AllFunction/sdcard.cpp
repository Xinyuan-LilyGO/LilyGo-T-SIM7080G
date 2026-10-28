/**
 * @file      sdcard.cpp
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2022  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2022-09-23
 *
 */
#include "FS.h"
#include "SD_MMC.h"
#include "utilities.h"

constexpr char TEST_FILE[] = "/sd_mount_rw_test.txt";
constexpr char TEST_DATA[] =
    "LilyGo T-SIM7080G SD card test\n"
    "This file verifies mount, write, read, and data integrity.\n";
    
// Try conservative clocks first for marginal cards, then move up to the
// normal and high-speed clocks. The sequence is repeated once before the
// longer periodic retry in loop(). Values are in kHz, as required by begin().
constexpr int MOUNT_FREQUENCIES_KHZ[] = {
    SDMMC_FREQ_PROBING,
    SDMMC_FREQ_DEFAULT,
    SDMMC_FREQ_26M,
    SDMMC_FREQ_HIGHSPEED,
    SDMMC_FREQ_PROBING,
    SDMMC_FREQ_DEFAULT,
    SDMMC_FREQ_26M,
    SDMMC_FREQ_HIGHSPEED,
};
constexpr uint32_t MOUNT_RETRY_DELAYS_MS[] = {
    250, 750, 1500, 3000, 6000, 10000, 10000, 10000
};
constexpr size_t MOUNT_RETRY_COUNT = sizeof(MOUNT_FREQUENCIES_KHZ) /
                                     sizeof(MOUNT_FREQUENCIES_KHZ[0]);
constexpr uint32_t REMOUNT_PERIOD_MS = 30000;

bool sdReady = false;
bool testCompleted = false;
uint32_t nextMountAttemptAt = 0;

const char *cardTypeName(uint8_t cardType)
{
    switch (cardType) {
    case CARD_MMC:
        return "MMC";
    case CARD_SD:
        return "SDSC";
    case CARD_SDHC:
        return "SDHC";
    default:
        return "UNKNOWN";
    }
}

void printCardInfo()
{
    const uint8_t cardType = SD_MMC.cardType();
    const uint64_t cardSizeMb = SD_MMC.cardSize() / (1024 * 1024);
    const uint64_t totalMb = SD_MMC.totalBytes() / (1024 * 1024);
    const uint64_t usedMb = SD_MMC.usedBytes() / (1024 * 1024);

    Serial.printf("[SD] Card: %s, reported size: %llu MB\n",
                  cardTypeName(cardType), cardSizeMb);
    Serial.printf("[SD] Filesystem: %llu MB total, %llu MB used\n",
                  totalMb, usedMb);
}

bool mountSdCard()
{
    SD_MMC.setPins(SDMMC_CLK, SDMMC_CMD, SDMMC_DATA);

    for (size_t attempt = 0; attempt < MOUNT_RETRY_COUNT; ++attempt) {
        const int frequencyKHz = MOUNT_FREQUENCIES_KHZ[attempt];
        Serial.printf("[SD] Mount attempt %u/%u at %d kHz...\n",
                      static_cast<unsigned>(attempt + 1),
                      static_cast<unsigned>(MOUNT_RETRY_COUNT),
                      frequencyKHz);

        // End any previous failed session before trying a fresh mount.
        SD_MMC.end();
        if (SD_MMC.begin("/sdcard", true, false, frequencyKHz)) {
            const uint8_t cardType = SD_MMC.cardType();
            if (cardType != CARD_NONE) {
                Serial.println("[SD] Mount OK");
                printCardInfo();
                return true;
            }
            Serial.println("[SD] Mount returned no card");
        } else {
            Serial.println("[SD] Mount failed");
        }

        SD_MMC.end();
        if (attempt + 1 < MOUNT_RETRY_COUNT) {
            const uint32_t waitMs = MOUNT_RETRY_DELAYS_MS[attempt];
            Serial.printf("[SD] Retrying in %lu ms\n",
                          static_cast<unsigned long>(waitMs));
            delay(waitMs);
        }
    }

    Serial.println("[SD] All mount attempts failed");
    return false;
}


bool writeTestFile()
{
    Serial.printf("[TEST] Writing %s (%u bytes)...\n",
                  TEST_FILE, static_cast<unsigned>(strlen(TEST_DATA)));

    File file = SD_MMC.open(TEST_FILE, FILE_WRITE);
    if (!file) {
        Serial.println("[TEST] FAIL: cannot open file for writing");
        return false;
    }

    const size_t expectedBytes = strlen(TEST_DATA);
    const size_t writtenBytes = file.print(TEST_DATA);
    file.close();

    if (writtenBytes != expectedBytes) {
        Serial.printf("[TEST] FAIL: wrote %u of %u bytes\n",
                      static_cast<unsigned>(writtenBytes),
                      static_cast<unsigned>(expectedBytes));
        return false;
    }

    Serial.printf("[TEST] Write OK: %u bytes\n",
                  static_cast<unsigned>(writtenBytes));
    return true;
}

bool readAndVerifyTestFile()
{
    Serial.printf("[TEST] Reading %s and verifying contents...\n", TEST_FILE);

    File file = SD_MMC.open(TEST_FILE, FILE_READ);
    if (!file) {
        Serial.println("[TEST] FAIL: cannot open file for reading");
        return false;
    }

    const String actual = file.readString();
    file.close();

    if (actual != TEST_DATA) {
        Serial.println("[TEST] FAIL: read data does not match written data");
        Serial.printf("[TEST] Expected %u bytes, got %u bytes\n",
                      static_cast<unsigned>(strlen(TEST_DATA)),
                      static_cast<unsigned>(actual.length()));
        return false;
    }

    Serial.printf("[TEST] Read/verify OK: %u bytes\n",
                  static_cast<unsigned>(actual.length()));
    return true;
}

bool runReadWriteTest()
{
    Serial.println();
    Serial.println("[TEST] Starting SD read/write test");

    if (!writeTestFile()) {
        return false;
    }
    if (!readAndVerifyTestFile()) {
        return false;
    }

    File file = SD_MMC.open(TEST_FILE, FILE_READ);
    const size_t fileSize = file ? file.size() : 0;
    if (file) {
        file.close();
    }
    Serial.printf("[TEST] Final file size: %u bytes\n",
                  static_cast<unsigned>(fileSize));
    Serial.println("[TEST] PASS: SD mount, write, read, and verify completed");
    return true;
}

bool setupSdcard()
{
    sdReady = mountSdCard();
    if (sdReady) {
        testCompleted = runReadWriteTest();
        if (!testCompleted) {
            Serial.println("[TEST] FAIL: check the card, wiring, and filesystem");
        }
    } else {
        Serial.printf("[SD] Will retry the complete mount sequence every %lu ms\n",
                      static_cast<unsigned long>(REMOUNT_PERIOD_MS));
        nextMountAttemptAt = millis() + REMOUNT_PERIOD_MS;
    }

    return sdReady;
}
