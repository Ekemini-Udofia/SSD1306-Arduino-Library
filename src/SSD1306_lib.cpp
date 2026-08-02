#include "SSD1306_lib.hpp"

uint8_t display_buffer[DISPLAY_BUFFER_SIZE]; 

// Initialization
bool display_init(void) {
  // Ensure display is off while configuring
  if (!display_send_command(SSD1306_DISPLAYOFF)) return false;

  // Set display clock divide ratio and oscillator frequency
  if (!display_send_command(SSD1306_SETDISPLAYCLOCKDIV)) return false;
  if (!display_send_command(0x80)) return false;  // Recommended default

  // Set multiplex ratio for 128x64 panel
  if (!display_send_command(SSD1306_SETMULTIPLEX)) return false;
  if (!display_send_command(0x3F)) return false;  // 64 - 1

  // No display offset
  if (!display_send_command(SSD1306_SETDISPLAYOFFSET)) return false;
  if (!display_send_command(0x00)) return false;

  // Map RAM line 0 to display line 0
  if (!display_send_command(SSD1306_SETSTARTLINE | 0x00)) return false;

  // Enable internal charge pump (required for 3.3V operation)
  if (!display_send_command(SSD1306_CHARGEPUMP)) return false;
  if (!display_send_command(0x14)) return false;

  // Set memory addressing mode to horizontal
  if (!display_send_command(SSD1306_MEMORYMODE)) return false;
  if (!display_send_command(0x00)) return false;

  // Mirror segments for correct left-to-right orientation
  if (!display_send_command(SSD1306_SEGREMAP | 0x01)) return false;

  // Scan COM pins from high to low (fix vertical orientation)
  if (!display_send_command(SSD1306_COMSCANDEC)) return false;

  // Set COM pin hardware configuration for 128x64
  if (!display_send_command(SSD1306_SETCOMPINS)) return false;
  if (!display_send_command(0x12)) return false;

  // Set contrast control
  if (!display_send_command(SSD1306_SETCONTRAST)) return false;
  if (!display_send_command(0xCF)) return false;

  // Set pre-charge period
  if (!display_send_command(SSD1306_SETPRECHARGE)) return false;
  if (!display_send_command(0xF1)) return false;

  // Set VCOMH deselect level
  if (!display_send_command(SSD1306_SETVCOMDETECT)) return false;
  if (!display_send_command(0x40)) return false;

  // Resume display from RAM content
  if (!display_send_command(SSD1306_DISPLAYALLON_RESUME)) return false;

  // Set normal (non-inverted) display mode
  if (!display_send_command(SSD1306_NORMALDISPLAY)) return false;

  // Turn display on
  if (!display_send_command(SSD1306_DISPLAYON)) return false;

  // If every check passes, return true
  return true;
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

// Update display from buffer
bool display_update(void)
{
    // Set column address range: 0 to 127
    if (!display_send_command(SSD1306_COLUMNADDR)) return false;
    if (!display_send_command(0x00)) return false;
    if (!display_send_command(DISPLAY_WIDTH - 1)) return false;

    // Set page address range: 0 to 7 (64px / 8)
    if (!display_send_command(SSD1306_PAGEADDR)) return false;
    if (!display_send_command(0x00)) return false;
    if (!display_send_command((DISPLAY_HEIGHT / 8) - 1)) return false;

    // Send framebuffer to GDDRAM
    return display_send_data(display_buffer, DISPLAY_BUFFER_SIZE);
}


// Low level Send
/*
 * Sends raw display data to the SSD1306 controller over I2C.
 *
 * The SSD1306 distinguishes data from commands using a control byte.
 * Control byte 0x40 indicates that all following bytes are display RAM data.
 *
 * Because the Arduino Wire library has a limited internal buffer
 * (typically 32 bytes total per transmission), the data is sent in
 * small chunks. One byte is reserved for the control byte, leaving
 * room for up to 31 data bytes per transfer.
 *
 * Returns true if all chunks are transmitted successfully.
 * Returns false immediately if any I2C transmission fails.
 */
bool display_send_data(const uint8_t *data, size_t length) {
  size_t offset = 0;

  while (offset < length) {
    Wire.beginTransmission(OLED_ADDR);
    Wire.write(0x40);  // Data control byte

    size_t chunk = min((size_t)31, length - offset);

    for (size_t i = 0; i < chunk; i++) Wire.write(data[offset + i]);

    if (Wire.endTransmission() != 0) return false;

    offset += chunk;
  }

  return true;
}

/*
 * Sends a single command byte to the SSD1306 controller.
 *
 * Control byte 0x00 indicates that the following byte is a command.
 * Commands configure display behavior such as addressing mode,
 * contrast, orientation, and power state.
 *
 * Returns true if the command was acknowledged by the device.
 */
bool display_send_command(uint8_t command) {
  Wire.beginTransmission(OLED_ADDR);

  // Control byte: Co = 0, D/C# = 0 → following byte is a command
  Wire.write(0x00);

  // SSD1306 command byte
  Wire.write(command);

  return (Wire.endTransmission() == 0);
}

/*
Framebuffer and Primitive Drawing Routines (SSD1306)

This section implements an in RAM framebuffer and basic drawing primitives.
All drawing operations modify display_buffer only.
Nothing is sent to the OLED until display_update() is called.

--------------------------------------------------
BUFFER MANAGEMENT
--------------------------------------------------

display_clear_buffer()
- Clears the entire framebuffer by setting all bytes to 0x00.
- This turns all pixels off logically.
- Does not affect the display until the buffer is flushed.

display_fill_buffer(pattern)
- Fills the entire framebuffer with a constant byte pattern.
- Useful for testing, screen wipe effects, or full on (0xFF) fills.
- Operates at byte level, not pixel level.

--------------------------------------------------
PIXEL ADDRESSING MODEL
--------------------------------------------------

The SSD1306 framebuffer is arranged in pages:
- Each page is 8 vertical pixels tall.
- Memory is linear left to right across DISPLAY_WIDTH.
- Index formula:
    index = (y / 8) * DISPLAY_WIDTH + x
- Bit position inside byte:
    bit = y % 8

--------------------------------------------------
PIXEL OPERATIONS
--------------------------------------------------

display_set_pixel(x, y, colour)
- Sets or clears a single pixel in the framebuffer.
- Performs bounds checking on x and y.
- colour = true  → set pixel
- colour = false → clear pixel
- Uses bitwise OR / AND masking to avoid disturbing other pixels.

display_get_pixel(x, y)
- Reads a single pixel from the framebuffer.
- Performs bounds checking.
- Returns true if the pixel bit is set, false otherwise.

--------------------------------------------------
LINE DRAWING
--------------------------------------------------

display_draw_hline(x, y, w, colour)
- Draws a horizontal line starting at (x, y).
- Width is w pixels.
- Internally calls display_set_pixel for each x position.
- Y is bounds checked, X overflow is handled implicitly.

display_draw_vline(x, y, h, colour)
- Draws a vertical line starting at (x, y).
- Height is h pixels.
- Internally calls display_set_pixel for each y position.
- X is bounds checked.

IMPORTANT:
- In the current implementation, the loop variable uses 'h'
  but the function parameter is named 'w'.
- This is a bug and will not compile.
- The loop should iterate using the height parameter.

--------------------------------------------------
RECTANGLE DRAWING
--------------------------------------------------

display_draw_rect(x, y, w, h, colour)
- Draws an unfilled rectangle.
- Implemented using two horizontal lines and two vertical lines.
- w and h must be non zero.
- No clipping is performed beyond pixel level bounds checks.

display_fill_rect(x, y, w, h, colour)
- Draws a filled rectangle.
- Implemented by drawing h horizontal lines.
- Simple and correct but not optimized for speed.

--------------------------------------------------
DESIGN NOTES
--------------------------------------------------

- These primitives are intentionally simple and predictable.
- Performance is acceptable for small displays and low refresh rates.
- More advanced optimizations (page aligned writes, spans, masks)
  can be added later without changing the public API.
*/

// Buffer Management
void display_clear_buffer(void) {
  memset(display_buffer, 0x00, DISPLAY_BUFFER_SIZE);
}

void display_fill_buffer(uint8_t pattern){
  // Fill entire framebuffer with the given byte pattern
  memset(display_buffer, pattern, DISPLAY_BUFFER_SIZE);
}

// Pixel Drawing
void display_set_pixel(uint8_t x, uint8_t y, bool colour) {
  if (x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT)
    return;

  uint16_t index = (y / 8) * DISPLAY_WIDTH + x;
  uint8_t  mask  = 1 << (y % 8);

  if (colour)
    display_buffer[index] |= mask;
  else
    display_buffer[index] &= ~mask;
}

bool display_get_pixel(uint8_t x, uint8_t y) {
  if (x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT)
    return false;

  uint16_t index = (y / 8) * DISPLAY_WIDTH + x;
  uint8_t  mask  = 1 << (y % 8);

  return (display_buffer[index] & mask) != 0;
}

void display_draw_hline(uint8_t x, uint8_t y, uint8_t w, bool colour) {
  if (y >= DISPLAY_HEIGHT)
    return;

  for (uint8_t i = 0; i < w; i++)
    display_set_pixel(x + i, y, colour);
}

void display_draw_vline(uint8_t x, uint8_t y, uint8_t w, bool colour) {
  if (x >= DISPLAY_WIDTH)
    return;

  for (uint8_t i = 0; i < w; i++)
    display_set_pixel(x, y + i, colour);
}

void display_draw_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, bool colour) {
  if (w == 0 || h == 0)
    return;

  display_draw_hline(x, y, w, colour);
  display_draw_hline(x, y + h - 1, w, colour);
  display_draw_vline(x, y, h, colour);
  display_draw_vline(x + w - 1, y, h, colour);
}

void display_fill_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, bool colour) {
  for (uint8_t i = 0; i < h; i++)
    display_draw_hline(x, y + i, w, colour);
}

// Text - To Implement Later
// void display_draw_bitmap(const uint8_t *bitmap, uint8_t w, uint8_t h, uint8_t
// x, uint8_t y, bool color);
// void display_draw_char(char c, uint8_t x, uint8_t y, const uint8_t *font,
//                        uint8_t font_w, uint8_t font_h, bool color);
// void display_draw_string(const char *str, uint8_t x, uint8_t y,
//                          const uint8_t *font, uint8_t font_w, uint8_t font_h,
//                          bool color);

// Display Control
/*
Display Control Functions (SSD1306)

These functions wrap common SSD1306 control commands and provide a
clean software interface for display configuration. They do NOT draw
pixels directly; they only configure how the controller interprets and
renders display RAM.

1. display_set_contrast(contrast)
   - Sets the panel drive contrast level.
   - Valid range is 0x00 (very dim) to 0xFF (maximum brightness).
   - This affects perceived brightness, not pixel data.
   - Actual visible range depends on panel quality and supply voltage.

2. display_invert(inv)
   - Toggles display inversion mode.
   - Normal mode: 0 = black, 1 = lit pixel.
   - Inverted mode: 1 = black, 0 = lit pixel.
   - This is a hardware operation; framebuffer data is unchanged.

3. display_set_column_address(start, end)
   - Defines the horizontal column range for subsequent data writes.
   - Used in horizontal and page addressing modes.
   - For a 128px wide display, valid values are 0 to 127.
   - Data writes will wrap only within this column window.

4. display_set_page_address(start, end)
   - Defines the vertical page range for subsequent data writes.
   - Each page represents 8 vertical pixels.
   - For a 64px tall display, valid pages are 0 to 7.
   - Data writes advance page by page within this window.

Important Notes:
- Column and page address commands define WHERE data goes, not WHAT is drawn.
- These functions are typically called before bulk framebuffer transfers.
- Incorrect address ranges will result in clipped or wrapped rendering.
*/
void display_set_contrast(uint8_t contrast) {
  // SSD1306 contrast range: 0x00 (dim) to 0xFF (bright)
  display_send_command(SSD1306_SETCONTRAST);
  display_send_command(contrast);
}

void display_invert(bool inv) {
  if (inv)
    display_send_command(SSD1306_INVERTDISPLAY);
  else
    display_send_command(SSD1306_NORMALDISPLAY);
}

void display_set_column_address(uint8_t start, uint8_t end) {
  // Valid range for 128px wide display: 0–127
  display_send_command(SSD1306_COLUMNADDR);
  display_send_command(start);
  display_send_command(end);
}

void display_set_page_address(uint8_t start, uint8_t end) {
  // Each page = 8 vertical pixels (64px display → pages 0–7)
  display_send_command(SSD1306_PAGEADDR);
  display_send_command(start);
  display_send_command(end);
}

// Scrolling Helpers 
/*
Scrolling on SSD1306 type displays is HARDWARE ASSISTED, not pixel based.

Key points:

1. The controller can only scroll FULL PAGES (8 pixel tall rows).
   You cannot scroll individual pixels using the built in scroll commands.

2. Scrolling works by telling the display controller to:
   - Select a start page
   - Select an end page
   - Choose direction (left or right)
   - Choose speed (frame interval)

3. Once enabled, the controller automatically shifts the display RAM
   horizontally. No framebuffer modification happens in software.

4. This means:
   - You must already have content written to the display RAM
   - Scrolling does NOT modify the buffer
   - When scrolling stops, the content snaps back to its original position

5. Because of this behavior:
   - Scrolling is best for banners or static text regions
   - It is NOT suitable for smooth animations or pixel precise effects

6. Vertical scrolling is even more limited and often requires:
   - Combined vertical + horizontal scroll commands
   - Display specific configuration bits
   - Careful page alignment

7. If true pixel level scrolling is required:
   - You must implement software scrolling
   - This means shifting your framebuffer manually and re uploading it
   - This is slower but fully controllable
*/
void display_scroll_right(uint8_t start_page, uint8_t end_page, uint8_t speed) {
  // Disable any existing scroll
  display_send_command(SSD1306_DEACTIVATE_SCROLL);

  // Right horizontal scroll setup
  display_send_command(SSD1306_RIGHT_HORIZONTAL_SCROLL);
  display_send_command(0x00);          // Dummy byte (per datasheet)
  display_send_command(start_page);    // Start page
  display_send_command(speed);         // Frame interval
  display_send_command(end_page);      // End page
  display_send_command(0x00);          // Dummy byte
  display_send_command(0xFF);          // Dummy byte

  // Activate scroll
  display_send_command(SSD1306_ACTIVATE_SCROLL);
}

void display_scroll_left(uint8_t start_page, uint8_t end_page, uint8_t speed) {
  // Disable any existing scroll
  display_send_command(SSD1306_DEACTIVATE_SCROLL);

  // Left horizontal scroll setup
  display_send_command(SSD1306_LEFT_HORIZONTAL_SCROLL);
  display_send_command(0x00);          // Dummy byte
  display_send_command(start_page);    // Start page
  display_send_command(speed);         // Frame interval
  display_send_command(end_page);      // End page
  display_send_command(0x00);          // Dummy byte
  display_send_command(0xFF);          // Dummy byte

  // Activate scroll
  display_send_command(SSD1306_ACTIVATE_SCROLL);
}
