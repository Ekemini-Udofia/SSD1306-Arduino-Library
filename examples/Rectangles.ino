#include "SSD1306_lib.hpp"

void setup() {
  Wire.begin();
  Serial.begin(115200);

  if (!display_init()) {
    Serial.println("SSD1306 init failed!");
    while (1);
  }

  // Clear framebuffer
  display_clear_buffer();

  // Draw pixels at corners
  display_set_pixel(0, 0, true);
  display_set_pixel(DISPLAY_WIDTH - 1, 0, true);
  display_set_pixel(0, DISPLAY_HEIGHT - 1, true);
  display_set_pixel(DISPLAY_WIDTH - 1, DISPLAY_HEIGHT - 1, true);

  // Draw horizontal and vertical lines
  display_draw_hline(10, 10, 50, true);
  display_draw_vline(20, 5, 40, true);

  // Draw a rectangle
  display_draw_rect(60, 20, 50, 30, true);

  // Draw a filled rectangle
  display_fill_rect(10, 40, 30, 20, true);

  // Push framebuffer to OLED
  display_update();
}

void loop() {
  // Blink top-left pixel every second
  static bool state = true;
  display_set_pixel(0, 0, state);
  display_update();
  state = !state;
  delay(1000);
}
