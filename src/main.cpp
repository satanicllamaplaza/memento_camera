// SPDX-FileCopyrightText: 2023 Limor Fried for Adafruit Industries
//
// SPDX-License-Identifier: MIT

#include "Adafruit_PyCamera.h"
#include <Arduino.h>
#include <iostream>
#include <string>
#include <vector>


Adafruit_PyCamera pycamera;


struct MenuItem {
  std::string name;               // label
  int currentIndex;               // which option is selected
  std::vector<uint32_t> options;  // hardware-compatible values
  std::vector<String> displayData;
};

std::vector<MenuItem> menu = {
  {
    "LED Brightness",
    0,
    {
      0,
      20,
      40,
      60,
      80,
      100
    },
    {
      "OFF",
      "20",
      "40",
      "60",
      "80",
      "100"
    }
  }, // maybe later add more like 50, 75, 100
  {
    "ringlightcolors_RGBW",
    0,
    {
      0x00000000,
      0x00ff1700,
      0x00ffab00,
      0x00fff800,
      0x00ff6d00,
      0x000083ff,
      0x00ff0072,
      0x00ffbc65
    },
    {
      "Off",
      "Orange",
      "Amber",
      "Yellow",
      "Red-Orange",
      "Blue",
      "Pink",
      "Peach"
    }
  },
  {
    "Resolution",
    4,
    {
      FRAMESIZE_QQVGA,
      FRAMESIZE_VGA,
      FRAMESIZE_HD,
      FRAMESIZE_QSXGA
    },
    {
      "160x120",
      "640x480",
      "1280x720",
      "2560x1920"
    }
  },
};

int selectedMenu = 0; // which menu item is active

#define IRQ 3 // probably drop maybe keep the measure for level tbd

void setup() {
  Serial.begin(115200);
  // while (!Serial) yield();
  delay(100);

  if (!pycamera.begin()) {
    Serial.println("Failed to initialize pyCamera interface");
    while (1)
      yield();
  }
  Serial.println("pyCamera hardware initialized!");

  pinMode(IRQ, INPUT_PULLUP);
  attachInterrupt(  // probably drop maybe keep the measure for level tbd
      IRQ, [] { Serial.println("IRQ!"); }, FALLING);
}

void loop() {
  static uint8_t loopn = 0;
  pycamera.setNeopixel(pycamera.Wheel(loopn));
  loopn += 8;

  pycamera.readButtons();
  // Serial.printf("Buttons: 0x%08X\n\r",  pycamera.readButtons());

  // pycamera.timestamp();
  pycamera.captureFrame();

  // Draw boxes on bottom and top of screen
  // NOTE: DISPLAY SIZE IS 240X240
  // Get framebuffer dimensions each loop
  int16_t BUFFER_WIDTH = pycamera.fb->width();
  int16_t BUFFER_HIGHT = pycamera.fb->height();
  // Bar is ~10% of screen height
  int TEXT_BOX_HIGHT = BUFFER_HIGHT / 10;
  // DRAW TOP BAR
  pycamera.fb->fillRect(0, 0, BUFFER_WIDTH, TEXT_BOX_HIGHT, pycamera.color565(20, 10, 10));
  // DRAW BOTTOM BAR
  pycamera.fb->fillRect(0, BUFFER_HIGHT - TEXT_BOX_HIGHT, BUFFER_WIDTH, TEXT_BOX_HIGHT, pycamera.color565(20, 10, 10));


  // once the frame is captured we can draw ontot he framebuffer
  if (pycamera.justPressed(AWEXP_SD_DET)) {

    Serial.println(F("SD Card removed"));
    pycamera.endSD();
    pycamera.fb->setCursor(0, 32);
    pycamera.fb->setTextSize(2);
    pycamera.fb->setTextColor(pycamera.color565(255, 0, 0));
    pycamera.fb->print(F("SD Card removed"));
    delay(200);
  }
  if (pycamera.justReleased(AWEXP_SD_DET)) {
    Serial.println(F("SD Card inserted!"));
    pycamera.initSD();
    pycamera.fb->setCursor(0, 32);
    pycamera.fb->setTextSize(2);
    pycamera.fb->setTextColor(pycamera.color565(255, 0, 0));
    pycamera.fb->print(F("SD Card inserted"));
    delay(200);
  }

  // probably remove this battery information.

  // float A0_voltage = analogRead(A0) / 4096.0 * 3.3;
  // if (loopn == 0) {
  //   Serial.printf("A0 = %0.1f V, Battery = %0.1f V\n\r", A0_voltage,
  //                 pycamera.readBatteryVoltage());
  // }
  // pycamera.fb->setCursor(0, 0);
  // pycamera.fb->setTextSize(2);
  // pycamera.fb->setTextColor(pycamera.color565(255, 255, 255));
  // pycamera.fb->print("A0 = ");
  // pycamera.fb->print(A0_voltage, 1);
  // pycamera.fb->print("V\nBattery = ");
  // pycamera.fb->print(pycamera.readBatteryVoltage(), 1);
  // pycamera.fb->print(" V");

  float x_ms2, y_ms2, z_ms2;
  if (pycamera.readAccelData(&x_ms2, &y_ms2, &z_ms2)) {
    // Serial.printf("X=%0.2f, Y=%0.2f, Z=%0.2f\n\r", x_ms2, y_ms2, z_ms2);
    pycamera.fb->setCursor(0, 100);
    pycamera.fb->setTextSize(2);
    pycamera.fb->setTextColor(pycamera.color565(206, 192, 144));
    pycamera.fb->print("3D: ");
    pycamera.fb->print(x_ms2, 1);
    pycamera.fb->print(", ");
    pycamera.fb->print(y_ms2, 1);
    pycamera.fb->print(", ");
    pycamera.fb->print(z_ms2, 1);
  }

  pycamera.blitFrame();
  if (pycamera.justPressed(AWEXP_BUTTON_LEFT)) {
    selectedMenu = (selectedMenu + 1) % menu.size();
  }
  if (pycamera.justPressed(AWEXP_BUTTON_RIGHT)) {
    selectedMenu = (selectedMenu - 1 + menu.size()) % menu.size();
  }
  // Cycle through options of selected menu
  MenuItem &item = menu[selectedMenu];
  if (pycamera.justPressed(AWEXP_BUTTON_UP)) {
      item.currentIndex = (item.currentIndex + 1) % item.options.size();
  }
  if (pycamera.justPressed(AWEXP_BUTTON_DOWN)) {
      item.currentIndex = (item.currentIndex - 1 + item.options.size()) % item.options.size();
  }


  // if (pycamera.justPressed(AWEXP_BUTTON_UP)) {
  //   Serial.println("Up!");
  //   for (int i = 0; i < sizeof(validSizes) / sizeof(framesize_t) - 1; ++i) {
  //     if (pycamera.photoSize == validSizes[i]) {
  //       pycamera.photoSize = validSizes[i + 1];
  //       break;
  //     }
  //   }
  // }
  // if (pycamera.justPressed(AWEXP_BUTTON_DOWN)) {
  //   Serial.println("Down!");
  //   for (int i = sizeof(validSizes) / sizeof(framesize_t) - 1; i > 0; --i) {
  //     if (pycamera.photoSize == validSizes[i]) {
  //       pycamera.photoSize = validSizes[i - 1];
  //       break;
  //     }
  //   }
  // }



  // if (pycamera.justPressed(AWEXP_BUTTON_OK)) {
  //   // iterate through all the ring light colors
  //   ringlight_i =
  //       (ringlight_i + 1) % (sizeof(ringlightcolors_RGBW) / sizeof(uint32_t));
  //   pycamera.setRing(ringlightcolors_RGBW[ringlight_i]);
  //   Serial.printf("set ringlight: 0x%08X\n\r",
  //                 (unsigned int)ringlightcolors_RGBW[ringlight_i]);
  // }
  // if (pycamera.justPressed(AWEXP_BUTTON_SEL)) {
  //   // iterate through brightness levels, incrementing 25 at a time
  //   if (ringlightBrightness >= 250)
  //     ringlightBrightness = 0;
  //   else
  //     ringlightBrightness += 50;
  //   pycamera.ring.setBrightness(ringlightBrightness);
  //   pycamera.setRing(ringlightcolors_RGBW[ringlight_i]);
  //   Serial.printf("set ringlight brightness: %d\n\r", ringlightBrightness);
  // }

  if (pycamera.justPressed(SHUTTER_BUTTON)) {
    Serial.println("Snap!");
    if (pycamera.takePhoto("IMAGE", pycamera.photoSize)) {
      pycamera.fb->setCursor(120, 100);
      pycamera.fb->setTextSize(2);
      pycamera.fb->setTextColor(pycamera.color565(255, 255, 255));
      pycamera.fb->print("Snap!");
      pycamera.speaker_tone(100, 50); // tone1 - B5
      // pycamera.blitFrame();
    }
  }

  delay(100);
}

