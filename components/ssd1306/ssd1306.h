#ifndef MAIN_SSD1306_H_
#define MAIN_SSD1306_H_

#include "driver/i2c_master.h"

#define OLED_CONTROL_BYTE_CMD_SINGLE    0x80
#define OLED_CONTROL_BYTE_CMD_STREAM    0x00
#define OLED_CONTROL_BYTE_DATA_SINGLE   0xC0
#define OLED_CONTROL_BYTE_DATA_STREAM   0x40

// Fundamental commands (pg.28)
#define OLED_CMD_SET_CONTRAST           0x81    // follow with 0x7F
#define OLED_CMD_DISPLAY_RAM            0xA4
#define OLED_CMD_DISPLAY_ALLON          0xA5
#define OLED_CMD_DISPLAY_NORMAL         0xA6
#define OLED_CMD_DISPLAY_INVERTED       0xA7
#define OLED_CMD_DISPLAY_OFF            0xAE
#define OLED_CMD_DISPLAY_ON             0xAF

// Addressing Command Table (pg.30)
#define OLED_CMD_SET_MEMORY_ADDR_MODE   0x20
#define OLED_CMD_SET_HORI_ADDR_MODE     0x00    // Horizontal Addressing Mode
#define OLED_CMD_SET_VERT_ADDR_MODE     0x01    // Vertical Addressing Mode
#define OLED_CMD_SET_PAGE_ADDR_MODE     0x02    // Page Addressing Mode
#define OLED_CMD_SET_COLUMN_RANGE       0x21    // can be used only in HORZ/VERT mode - follow with 0x00 and 0x7F = COL127
#define OLED_CMD_SET_PAGE_RANGE         0x22    // can be used only in HORZ/VERT mode - follow with 0x00 and 0x07 = PAGE7

// Hardware Config (pg.31)
#define OLED_CMD_SET_DISPLAY_START_LINE 0x40
#define OLED_CMD_SET_SEGMENT_REMAP_0    0xA0    
#define OLED_CMD_SET_SEGMENT_REMAP_1    0xA1    
#define OLED_CMD_SET_MUX_RATIO          0xA8    // follow with 0x3F = 64 MUX
#define OLED_CMD_SET_COM_SCAN_MODE      0xC8    
#define OLED_CMD_SET_DISPLAY_OFFSET     0xD3    // follow with 0x00
#define OLED_CMD_SET_COM_PIN_MAP        0xDA    // follow with 0x12
#define OLED_CMD_NOP                    0xE3    // NOP

// Timing and Driving Scheme (pg.32)
#define OLED_CMD_SET_DISPLAY_CLK_DIV    0xD5    // follow with 0x80
#define OLED_CMD_SET_PRECHARGE          0xD9    // follow with 0xF1
#define OLED_CMD_SET_VCOMH_DESELCT      0xDB    // follow with 0x30

// Charge Pump (pg.62)
#define OLED_CMD_SET_CHARGE_PUMP        0x8D    // follow with 0x14

// Scrolling Command
#define OLED_CMD_HORIZONTAL_RIGHT       0x26
#define OLED_CMD_HORIZONTAL_LEFT        0x27
#define OLED_CMD_CONTINUOUS_SCROLL      0x29
#define OLED_CMD_DEACTIVE_SCROLL        0x2E
#define OLED_CMD_ACTIVE_SCROLL          0x2F
#define OLED_CMD_VERTICAL               0xA3

#define I2C_SSD1306_ADDR 0x3C

#define CONFIG_OFFSET 0

typedef enum {
	SCROLL_RIGHT = 1,
	SCROLL_LEFT = 2,
	SCROLL_DOWN = 3,
	SCROLL_UP = 4,
	PAGE_SCROLL_DOWN = 5,
	PAGE_SCROLL_UP = 6,
	SCROLL_STOP = 7
} ssd1306_scroll_type_t;

typedef struct {
	bool _valid; // Not using it anymore
	int _segLen; // Not using it anymore
	uint8_t _segs[128];
} PAGE_t;

typedef struct {
	int _address;
	int _width;
	int _height;
	int _pages;
	int _dc;
	bool _scEnable;
	int _scStart;
	int _scEnd;
	int _scDirection;
	PAGE_t _page[8];
	bool _flip;
	i2c_port_t _i2c_num;
	i2c_master_bus_handle_t _i2c_bus_handle;
	i2c_master_dev_handle_t _i2c_dev_handle;
} SSD1306_t;

void ssd1306_init( int width, int height);
void ssd1306_show_buffer();
void ssd1306_display_image(int page, int seg, uint8_t * images, int width);
void ssd1306_display_text(int page, char * text, int text_len, bool invert);
void ssd1306_display_text_x3(int page, char * text, int text_len, bool invert);
void ssd1306_clear_screen(bool invert);
void ssd1306_contrast(int contrast);
void _ssd1306_pixel(int xpos, int ypos, bool invert);
void _ssd1306_line(int x1, int y1, int x2, int y2,  bool invert);
void ssd1306_clear_line(int page, bool invert);
void ssd1306_invert(uint8_t *buf, size_t blen);
void ssd1306_flip(uint8_t *buf, size_t blen);
uint8_t ssd1306_rotate_byte(uint8_t ch1);

esp_err_t ssd1306_i2c_init();
void ssd1306_i2c_display_image(int page, int seg, uint8_t * images, int width);
void ssd1306_i2c_contrast(int contrast);
esp_err_t ssd1306_i2c_delete_dev();


#endif /* MAIN_SSD1306_H_ */

