//
//  ScreenSaver_Lines.ino
//  Arduino Nano ESP32, 128x64 monochrome oled I2C
//  Pin: SDA 20 SCL 21
//  Created by Peng Liu on 3/17/24.
//

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define WIDTH           128
#define HEIGHT          64
#define OLED_RESET      -1
#define I2C_SDA         20
#define I2C_SCL         21
#define DISP_ADDRESS    0x3C

#define MULTIPLIER      10
#define SECTIONS        4
#define REPETITION      5
#define SPEEDBASE       6

#define RADIUS          3

#define DEBUG           0

struct Dot {
    int16_t x, y, vx, vy;
    
    Dot() {
        x = random(0, WIDTH * MULTIPLIER - 1);
        y = random(0, HEIGHT * MULTIPLIER - 1);
#if DEBUG
        vx = 1 * MULTIPLIER;
        vy = 1 * MULTIPLIER;
#else
        vx = random(0, SPEEDBASE * MULTIPLIER) - SPEEDBASE * MULTIPLIER / 2;
        vy = random(0, SPEEDBASE * MULTIPLIER) - SPEEDBASE * MULTIPLIER / 2;
#endif
    }
    
    void updateV(int16_t& v, int16_t& p, int16_t bound) {
        bool negativeV = p >= bound;
        if (negativeV || p < 0) {
            p = min(int16_t(bound - 1), max(int16_t(0), p));
#if DEBUG
            v = -v;
#else
            v = random(2, SPEEDBASE * MULTIPLIER / 2) * (negativeV ? -1 : 1);
#endif
        }
    }
    
    void tick() {
        x += vx; y += vy;
        updateV(vx, x, WIDTH * MULTIPLIER);
        updateV(vy, y, HEIGHT * MULTIPLIER);
    }
};

struct Band {
    Dot dots[SECTIONS];
    void update() {
        for (int16_t i = 0; i < SECTIONS; ++i) {
            dots[i].tick();
        }
    }
    
    void drawPos(Adafruit_SSD1306* disp) {
        int16_t x, y;
        for (int16_t i = 0; i < SECTIONS; ++i) {
            x = (dots[i].x + MULTIPLIER / 2) / MULTIPLIER;
            y = (dots[i].y + MULTIPLIER / 2) / MULTIPLIER;
            disp->drawCircle(x, y, RADIUS, SSD1306_WHITE);
        }
    }
    
    void drawLines(Adafruit_SSD1306* disp) {
        int16_t x[SECTIONS], y[SECTIONS], j;
        for (int16_t i = 0; i < SECTIONS; ++i) {
            x[i] = (dots[i].x + MULTIPLIER / 2) / MULTIPLIER;
            y[i] = (dots[i].y + MULTIPLIER / 2) / MULTIPLIER;
        }
        
        for (int16_t i = 0; i < SECTIONS; ++i) {
            j = (i + 1) % SECTIONS;
            disp->drawLine(x[i], y[i], x[j], y[j], SSD1306_WHITE);
        }
    }
};


Band bands[REPETITION];
Band band;
uint16_t currentIdx = 0;

TwoWire* i2c;
Adafruit_SSD1306* disp;

void setup() {
    Serial.begin(115200);
    
    i2c = new TwoWire(0);
    
    if (i2c == nullptr) {
        Serial.println(F("i2c init failed"));
        for(;;);
    }
    i2c->setPins(I2C_SDA, I2C_SCL);
    disp = new Adafruit_SSD1306(WIDTH, HEIGHT, i2c, OLED_RESET);
    
    if (disp == nullptr) {
        Serial.println(F("disp init failed"));
        for(;;);
    }
    
    if(!disp->begin(SSD1306_SWITCHCAPVCC, DISP_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
        for(;;); // Don't proceed, loop forever
    }
}

void loop() {
    band.update();
    bands[++currentIdx % REPETITION] = band;
    disp->clearDisplay();
    for (int16_t i = 0; i < REPETITION; ++i) {
        bands[i].drawLines(disp);
    }
    disp->display();
}
