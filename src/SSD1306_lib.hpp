#ifndef SSD1306_LIB_HPP_
#define SSD1306_LIB_HPP_

#include "headers.hpp"
#include <stdint.h>
#include <Arduino.h>
#include <Wire.h>

#define DISPLAY_HEIGHT 64
#define DISPLAY_WIDTH 128
#define DISPLAY_BUFFER_SIZE ((DISPLAY_WIDTH * DISPLAY_HEIGHT) / 8)
#define OLED_ADDR 0X3C

extern uint8_t display_buffer[DISPLAY_BUFFER_SIZE];

// Initialization
bool display_init(void);
void set_power_state(bool on);

// Update display from buffer
bool display_update(void);

// Low level Send 
bool display_send_data(const uint8_t *data, size_t length);
bool display_send_command(uint8_t command);

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
// void display_draw_bitmap(const uint8_t *bitmap, uint8_t w, uint8_t h, uint8_t x, uint8_t y, bool color);
void display_draw_char(char c, uint8_t x, uint8_t y, const uint8_t *font, uint8_t font_w, uint8_t font_h, bool color);
void display_draw_string(const char *str, uint8_t x, uint8_t y, const uint8_t *font, uint8_t font_w, uint8_t font_h, bool color);

// Display Control
void display_set_contrast(uint8_t contrast);
void display_invert(bool inv);
void display_set_column_address(uint8_t start, uint8_t end);
void display_set_page_address(uint8_t start, uint8_t end);

// Scrolling Helpers (Might do Later)
void display_scroll_right(uint8_t start_page, uint8_t end_page, uint8_t speed);
void display_scroll_left(uint8_t start_page, uint8_t end_page, uint8_t speed);


#endif // SSD1306_LIB_HPP_