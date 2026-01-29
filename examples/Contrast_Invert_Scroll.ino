#include "ssd1306_lib.hpp"

void setup() {
  Wire.begin();
  Serial.begin(115200);

  if (!display_init()) {
    Serial.println("SSD1306 init failed!");
    while (1);
  }

  // Clear screen
  display_clear_buffer();

  // Fill entire buffer (all pixels on)
  display_fill_buffer(0xFF);
  display_update();
  delay(1000);

  // Set contrast to low
  display_set_contrast(0x30);
  delay(1000);

  // Set contrast to high
  display_set_contrast(0xFF);
  delay(1000);

  // Invert display
  display_invert(true);
  delay(1000);

  // Restore normal display
  display_invert(false);
  delay(1000);

  // Demonstrate scrolling right on top 2 pages (rows 0–15)
  display_scroll_right(0, 1, 0x00); // speed = 0 (5 frames)
}

void loop() {
  // Nothing; scrolling runs in hardware
}
