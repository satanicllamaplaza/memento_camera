// SPDX-FileCopyrightText: 2023 Limor Fried for Adafruit Industries
//
// SPDX-License-Identifier: MIT

#include "Adafruit_PyCamera.h"
#include "pins_arduino.h"
#include "sensor.h"
#include <Arduino.h>
#include <vector>


Adafruit_PyCamera pycamera;

int selectedMenu = 0; // which menu item is active
uint32_t currentLEDColor = 0x00FF0000;

framesize_t validSizes[] = {
    FRAMESIZE_QQVGA, FRAMESIZE_VGA, FRAMESIZE_HD, FRAMESIZE_QSXGA
};

struct MenuItem {
  String name;                     // Label
  int currentIndex;                     // Which option is selected
  std::vector<uint32_t> options;        // Hardware-compatible values
  std::vector<String> displayData;      // Settings converted for display text
  std::function<void(uint32_t)> apply;  // Callable function to apply hardware setting
};

std::vector<MenuItem> menu = {
  {
    "LEDC",
    0,
    {
      0x00FF0000,
      0x00FFFF00,
      0x0000FF00,
      0x0000FFFF,
      0x000000FF,
      0x00FF00FF,
      0xFF000000
    },
    {
      "RED",
      "YELLOW",
      "GREEN",
      "CYAN",
      "BLUE",
      "PINK",
      "WHITE"
    },
    [](uint32_t value) {
      currentLEDColor = value;
      pycamera.setRing(currentLEDColor);
    }
  },

  {
    "LEDB",
    0,
    {
      0,
      50,
      100,
      150,
      200,
      250
    },
    {
      "OFF",
      "20%",
      "40%",
      "60%",
      "80%",
      "100%"
    },
    [](uint32_t value) {
      pycamera.ring.setBrightness(value);
      pycamera.setRing(currentLEDColor);
    }
  },

  {
    "RESO",
    3,
    {
      0,
      1,
      2,
      3
    },
    {
      "160x120",
      "640x480",
      "1280x720",
      "2560x1920"
    },
    [](uint32_t value) { pycamera.photoSize = validSizes[value]; }
  }
};


// MenuItem& ledColors     = menu[0];   // reference ringlightcolors_RGBW menu
// MenuItem& ledBrightness = menu[1];   // reference LED Brightness menu
// MenuItem& resolutionMenu = menu[2];   // reference Resolution menu

void applyMenuSettings(int menuIndex = -1) {
  if (menuIndex == -1) menuIndex = selectedMenu;
  MenuItem& item = menu[menuIndex];
  item.apply(item.options[item.currentIndex]);
}

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

  for (int i = 0; i < menu.size(); i++) {
    applyMenuSettings(i);  // apply defaults for all menus
  }

  pinMode(IRQ, INPUT_PULLUP);
  attachInterrupt(  // probably drop maybe keep the measure for level tbd
      IRQ, [] { Serial.println("IRQ!"); }, FALLING);
}

void loop() {
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

  MenuItem& item = menu[selectedMenu];
  String menuNameStr  = item.name;
  String menuValueStr = item.displayData[item.currentIndex];

  pycamera.fb->setCursor(0, 220);
  pycamera.fb->setTextSize(2);
  pycamera.fb->setTextColor(pycamera.color565(206, 192, 144));
  pycamera.fb->print(menuNameStr);
  pycamera.fb->print(": ");
  pycamera.fb->print(menuValueStr);

  pycamera.blitFrame();
  if (pycamera.justPressed(AWEXP_BUTTON_RIGHT)) {
    selectedMenu = (selectedMenu + 1) % menu.size();
  }
  if (pycamera.justPressed(AWEXP_BUTTON_LEFT)) {
    selectedMenu = (selectedMenu - 1 + menu.size()) % menu.size();
  }
  // Cycle through options of selected menu
  if (pycamera.justPressed(AWEXP_BUTTON_UP)) {
    item.currentIndex = (item.currentIndex + 1) % item.options.size();
    applyMenuSettings();
  }
  if (pycamera.justPressed(AWEXP_BUTTON_DOWN)) {
    item.currentIndex = (item.currentIndex - 1 + item.options.size()) % item.options.size();
    applyMenuSettings();
  }

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

