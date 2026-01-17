#include "SSD1306_lib.hpp"

// Initialization
bool display_init(void) {
  // Ensure display is off while configuring
  display_send_command(SSD1306_DISPLAYOFF);

  // Set display clock divide ratio and oscillator frequency
  display_send_command(SSD1306_SETDISPLAYCLOCKDIV);
  display_send_command(0x80);  // Recommended default

  // Set multiplex ratio for 128x64 panel
  display_send_command(SSD1306_SETMULTIPLEX);
  display_send_command(0x3F);  // 64 - 1

  // No display offset
  display_send_command(SSD1306_SETDISPLAYOFFSET);
  display_send_command(0x00);

  // Map RAM line 0 to display line 0
  display_send_command(SSD1306_SETSTARTLINE | 0x00);

  // Enable internal charge pump (required for 3.3V operation)
  display_send_command(SSD1306_CHARGEPUMP);
  display_send_command(0x14);

  // Set memory addressing mode to horizontal
  display_send_command(SSD1306_MEMORYMODE);
  display_send_command(0x00);

  // Mirror segments for correct left-to-right orientation
  display_send_command(SSD1306_SEGREMAP | 0x01);

  // Scan COM pins from high to low (fix vertical orientation)
  display_send_command(SSD1306_COMSCANDEC);

  // Set COM pin hardware configuration for 128x64
  display_send_command(SSD1306_SETCOMPINS);
  display_send_command(0x12);

  // Set contrast control
  display_send_command(SSD1306_SETCONTRAST);
  display_send_command(0xCF);

  // Set pre-charge period
  display_send_command(SSD1306_SETPRECHARGE);
  display_send_command(0xF1);

  // Set VCOMH deselect level
  display_send_command(SSD1306_SETVCOMDETECT);
  display_send_command(0x40);

  // Resume display from RAM content
  display_send_command(SSD1306_DISPLAYALLON_RESUME);

  // Set normal (non-inverted) display mode
  display_send_command(SSD1306_NORMALDISPLAY);

  // Turn display on
  display_send_command(SSD1306_DISPLAYON);
}

void set_power_state(bool on) {
  if (on) {
    // Turn display on
    display_send_command(SSD1306_DISPLAYON);
  } else {
    // Turn display off
    display_send_command(SSD1306_DISPLAYOFF);
  }
}

// Low level Sned
void display_send_data(const uint8_t data, size_t length);

void display_send_command(uint8_t command) {
  Wire.beginTransmission(OLED_ADDR);

  // Control byte: Co = 0, D/C# = 0 → following byte is a command
  Wire.write(0x00);

  // SSD1306 command byte
  Wire.write(command);

  Wire.endTransmission();
}

// Buffer Management
void display_clear_buffer(void);
void display_fill_buffer(uint8_t pattern);

// Pixel Drawing
void display_set_pixel(uint8_t x, uint8_t y, bool colour);
bool display_get_pixel(uint8_t x, uint8_t y);
void display_draw_hline(uint8_t x, uint8_t y, uint8_t w, bool colour);
void display_draw_vline(uint8_t x, uint8_t y, uint8_t w, bool colour);
void display_draw_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, bool colour);
void display_fill_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, bool colour);

// Text
// void display_draw_bitmap(const uint8_t *bitmap, uint8_t w, uint8_t h, uint8_t
// x, uint8_t y, bool color);
void display_draw_char(char c, uint8_t x, uint8_t y, const uint8_t *font,
                       uint8_t font_w, uint8_t font_h, bool color);
void display_draw_string(const char *str, uint8_t x, uint8_t y,
                         const uint8_t *font, uint8_t font_w, uint8_t font_h,
                         bool color);

// Display Control
void display_set_contrast(uint8_t contrast);
void display_invert(bool inv);
void display_set_column_address(uint8_t start, uint8_t end);
void display_set_page_address(uint8_t start, uint8_t end);

// Scrolling Helpers (Might do Later)
void display_scroll_right(uint8_t start_page, uint8_t end_page, uint8_t speed);
void display_scroll_left(uint8_t start_page, uint8_t end_page, uint8_t speed);
