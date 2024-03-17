//
//  ScreenSaver_Lines.ino
//  Arduino Nano ESP32, 128x64 monochrome oled I2C
//
//  Created by Peng Liu on 3/17/24.
//

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define WIDTH           128
#define HEIGHT          64
#define OLED_RESET      -1
#define DISP_ADDRESS    0x3C
Adafruit_SSD1306 disp(WIDTH, HEIGHT, &Wire, OLED_RESET);

#define MULTIPLIER      10
#define SECTIONS        4
#define REPETITION      5
#define SPEEDBASE       6

#define RADIUS          3

struct Dot {
  int16_t x, y, vx, vy;

  Dot() {
    x = random(0, WIDTH * MULTIPLIER - 1);
    y = random(0, HEIGHT * MULTIPLIER - 1);
    vx = random(0, SPEEDBASE * MULTIPLIER) - SPEEDBASE * MULTIPLIER / 2;
    vy = random(0, SPEEDBASE * MULTIPLIER) - SPEEDBASE * MULTIPLIER / 2;
  }

  void tick() {
    x += vx; y += vy;
    bool negativeV = x >= WIDTH * MULTIPLIER;
    if (negativeV || x < 0) {
      x = min(int16_t(WIDTH * MULTIPLIER - 1), max(int16_t(0), x));
      vx = random(2, SPEEDBASE * MULTIPLIER / 2) * (negativeV ? -1 : 1);
    }
    negativeV = y >= HEIGHT * MULTIPLIER;
    if (negativeV || y < 0) {
      y = min(int16_t(HEIGHT * MULTIPLIER - 1), max(int16_t(0), y));
      vy = random(2, SPEEDBASE * MULTIPLIER / 2) * (negativeV ? -1 : 1);
    }
  }
};

struct Band {
  Dot dots[SECTIONS];
  void update() {
    for (int16_t i = 0; i < SECTIONS; ++i) {
      dots[i].tick();
    }
  }

  void drawPos() {
    int16_t x, y;
    for (int16_t i = 0; i < SECTIONS; ++i) {
      x = (dots[i].x + MULTIPLIER / 2) / MULTIPLIER;
      y = (dots[i].y + MULTIPLIER / 2) / MULTIPLIER;
      disp.drawCircle(x, y, RADIUS, SSD1306_WHITE);
    }
  }

  void drawLines() {
    int16_t x[SECTIONS], y[SECTIONS], j;
    for (int16_t i = 0; i < SECTIONS; ++i) {
      x[i] = (dots[i].x + MULTIPLIER / 2) / MULTIPLIER;
      y[i] = (dots[i].y + MULTIPLIER / 2) / MULTIPLIER;
    }

    for (int16_t i = 0; i < SECTIONS; ++i) {
      j = (i + 1) % SECTIONS;
      disp.drawLine(x[i], y[i], x[j], y[j], SSD1306_WHITE);
    }
  }
};


Band bands[REPETITION];
Band band;
uint16_t currentIdx = 0;

void setup() {
  Serial.begin(115200);
  if(!disp.begin(SSD1306_SWITCHCAPVCC, DISP_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  band.update();
  bands[++currentIdx % REPETITION] = band;
  disp.clearDisplay();
  for (int16_t i = 0; i < REPETITION; ++i) {
    bands[i].drawLines();
  }
  disp.display();
}
