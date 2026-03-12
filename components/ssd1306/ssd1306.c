#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "ssd1306.h"
#include "font8x8_basic.h"
#include "i2c.h"

static const char *TAG = "SSD1306";

#define PACK8 __attribute__((aligned( __alignof__( uint8_t ) ), packed ))

static i2c_master_dev_handle_t dev_handle;
static SSD1306_t dev;

typedef union out_column_t {
	uint32_t u32;
	uint8_t  u8[4];
} PACK8 out_column_t;

esp_err_t ssd1306_i2c_init()
{   
    ESP_ERROR_CHECK(i2c_add_device(I2C_SSD1306_ADDR, &dev_handle));
	dev._address = I2C_SSD1306_ADDR;
	dev._flip = false;
	ssd1306_init(128,32);
	return ESP_OK;

}

void ssd1306_i2c_contrast(int contrast) {
	uint8_t _contrast = contrast;
	if (contrast < 0x0) _contrast = 0;
	if (contrast > 0xFF) _contrast = 0xFF;

	uint8_t out_buf[3];
	int out_index = 0;
	out_buf[out_index++] = OLED_CONTROL_BYTE_CMD_STREAM; // 00
	out_buf[out_index++] = OLED_CMD_SET_CONTRAST; // 81
	out_buf[out_index++] = _contrast;

	esp_err_t res = i2c_master_transmit(dev_handle, out_buf, 3, -1);
	if (res != ESP_OK)
		ESP_LOGE(TAG, "Could not write to device [0x%02x at %d]: %d (%s)", dev._address, dev._i2c_num, res, esp_err_to_name(res));
}


void ssd1306_i2c_display_image(int page, int seg, uint8_t * images, int width) {
	if (page >= dev._pages) return;
	if (seg >= dev._width) return;

	int _seg = seg + CONFIG_OFFSET;
	uint8_t columLow = _seg & 0x0F;
	uint8_t columHigh = (_seg >> 4) & 0x0F;

	int _page = page;
	if (dev._flip) {
		_page = (dev._pages - page) - 1;
	}

	uint8_t *out_buf;
	out_buf = malloc(width < 4 ? 4 : width + 1);
	if (out_buf == NULL) {
		ESP_LOGE(TAG, "malloc fail");
		return;
	}
	int out_index = 0;
	out_buf[out_index++] = OLED_CONTROL_BYTE_CMD_STREAM;
	// Set Lower Column Start Address for Page Addressing Mode
	out_buf[out_index++] = (0x00 + columLow);
	// Set Higher Column Start Address for Page Addressing Mode
	out_buf[out_index++] = (0x10 + columHigh);
	// Set Page Start Address for Page Addressing Mode
	out_buf[out_index++] = 0xB0 | _page;

	esp_err_t res;
	res = i2c_master_transmit(dev_handle, out_buf, out_index, -1);
	if (res != ESP_OK)
		ESP_LOGE(TAG, "Could not write to device [0x%02x at %d]: %d (%s)", dev._address, dev._i2c_num, res, esp_err_to_name(res));

	out_buf[0] = OLED_CONTROL_BYTE_DATA_STREAM;
	memcpy(&out_buf[1], images, width);

	res = i2c_master_transmit(dev_handle, out_buf, width + 1,-1);
	if (res != ESP_OK)
		ESP_LOGE(TAG, "Could not write to device [0x%02x at %d]: %d (%s)", dev._address, dev._i2c_num, res, esp_err_to_name(res));
	free(out_buf);
}


void ssd1306_init(int width, int height)
{
	
	dev._width = width;
	dev._height = height;
	dev._pages = 8;
	if (dev._height == 32) dev._pages = 4;
	
	uint8_t out_buf[27];
	int out_index = 0;
	out_buf[out_index++] = OLED_CONTROL_BYTE_CMD_STREAM;
	out_buf[out_index++] = OLED_CMD_DISPLAY_OFF;				// AE
	out_buf[out_index++] = OLED_CMD_SET_MUX_RATIO;			 // A8
	if (dev._height == 64) out_buf[out_index++] = 0x3F;
	if (dev._height == 32) out_buf[out_index++] = 0x1F;
	out_buf[out_index++] = OLED_CMD_SET_DISPLAY_OFFSET;		 // D3
	out_buf[out_index++] = 0x00;
	//out_buf[out_index++] = OLED_CONTROL_BYTE_DATA_STREAM;	// 40
	out_buf[out_index++] = OLED_CMD_SET_DISPLAY_START_LINE;	// 40
	//out_buf[out_index++] = OLED_CMD_SET_SEGMENT_REMAP;		// A1
	if (dev._flip) {
		out_buf[out_index++] = OLED_CMD_SET_SEGMENT_REMAP_0; // A0
	} else {
		out_buf[out_index++] = OLED_CMD_SET_SEGMENT_REMAP_1;	// A1
	}
	out_buf[out_index++] = OLED_CMD_SET_COM_SCAN_MODE;		// C8
	out_buf[out_index++] = OLED_CMD_SET_DISPLAY_CLK_DIV;		// D5
	out_buf[out_index++] = 0x80;
	out_buf[out_index++] = OLED_CMD_SET_COM_PIN_MAP;			// DA
	if (dev._height == 64) out_buf[out_index++] = 0x12;
	if (dev._height == 32) out_buf[out_index++] = 0x02;
	out_buf[out_index++] = OLED_CMD_SET_CONTRAST;			// 81
	out_buf[out_index++] = 0xFF;
	out_buf[out_index++] = OLED_CMD_DISPLAY_RAM;				// A4
	out_buf[out_index++] = OLED_CMD_SET_VCOMH_DESELCT;		// DB
	out_buf[out_index++] = 0x40;
	out_buf[out_index++] = OLED_CMD_SET_MEMORY_ADDR_MODE;	// 20
	//out_buf[out_index++] = OLED_CMD_SET_HORI_ADDR_MODE;	// 00
	out_buf[out_index++] = OLED_CMD_SET_PAGE_ADDR_MODE;		// 02
	// Set Lower Column Start Address for Page Addressing Mode
	out_buf[out_index++] = 0x00;
	// Set Higher Column Start Address for Page Addressing Mode
	out_buf[out_index++] = 0x10;
	out_buf[out_index++] = OLED_CMD_SET_CHARGE_PUMP;			// 8D
	out_buf[out_index++] = 0x14;
	out_buf[out_index++] = OLED_CMD_DEACTIVE_SCROLL;			// 2E
	out_buf[out_index++] = OLED_CMD_DISPLAY_NORMAL;			// A6
	out_buf[out_index++] = OLED_CMD_DISPLAY_ON;				// AF

	esp_err_t res;
	res = i2c_master_transmit(dev_handle, out_buf, out_index, -1);
	if (res == ESP_OK) {
		ESP_LOGI(TAG, "OLED configured successfully");
	} else {
		ESP_LOGE(TAG, "Could not write to device [0x%02x at %d]: %d (%s)", dev._address, dev._i2c_num, res, esp_err_to_name(res));
	}

	// Initialize internal buffer
	for (int i=0;i<dev._pages;i++) {
		memset(dev._page[i]._segs, 0, 128);
	}
}

void ssd1306_display_image(int page, int seg, uint8_t * images, int width)
{
	ssd1306_i2c_display_image(page, seg, images, width);

	// Set to internal buffer
	memcpy(&dev._page[page]._segs[seg], images, width);
}

void ssd1306_show_buffer()
{
	for (int page=0; page<dev._pages;page++) {
		ssd1306_i2c_display_image(page, 0, dev._page[page]._segs, dev._width);
	}
}

void ssd1306_display_text(int page, char * text, int text_len, bool invert)
{
	if (page >= dev._pages) return;
	int _text_len = text_len;
	if (_text_len > 16) _text_len = 16;

	int seg = 0;
	uint8_t image[8];
	for (int i = 0; i < _text_len; i++) {
		memcpy(image, font8x8_basic_tr[(uint8_t)text[i]], 8);
		if (invert) ssd1306_invert(image, 8);
		if (dev._flip) ssd1306_flip(image, 8);
		ssd1306_display_image(page, seg, image, 8);
		seg = seg + 8;
	}
}

void ssd1306_display_text_x3(int page, char * text, int text_len, bool invert)
{
	if (page >= dev._pages) return;
	int _text_len = text_len;
	if (_text_len > 5) _text_len = 5;

	int seg = 0;

	for (int nn = 0; nn < _text_len; nn++) {

		uint8_t const * const in_columns = font8x8_basic_tr[(uint8_t)text[nn]];

		// make the character 3x as high
		out_column_t out_columns[8];
		memset(out_columns, 0, sizeof(out_columns));

		for (int xx = 0; xx < 8; xx++) { // for each column (x-direction)

			uint32_t in_bitmask = 0b1;
			uint32_t out_bitmask = 0b111;

			for (int yy = 0; yy < 8; yy++) { // for pixel (y-direction)
				if (in_columns[xx] & in_bitmask) {
					out_columns[xx].u32 |= out_bitmask;
				}
				in_bitmask <<= 1;
				out_bitmask <<= 3;
			}
		}

		// render character in 8 column high pieces, making them 3x as wide
		for (int yy = 0; yy < 3; yy++)	{ // for each group of 8 pixels high (y-direction)

			uint8_t image[24];
			for (int xx = 0; xx < 8; xx++) { // for each column (x-direction)
				image[xx*3+0] = 
				image[xx*3+1] = 
				image[xx*3+2] = out_columns[xx].u8[yy];
			}
			if (invert) ssd1306_invert(image, 24);
			if (dev._flip) ssd1306_flip(image, 24);
				ssd1306_i2c_display_image(page+yy, seg, image, 24);
			memcpy(&dev._page[page+yy]._segs[seg], image, 24);
		}
		seg = seg + 24;
	}
}



void ssd1306_clear_screen(bool invert)
{
	char space[16];
	memset(space, 0x00, sizeof(space));
	for (int page = 0; page < dev._pages; page++) {
		ssd1306_display_text(page, space, sizeof(space), invert);
	}
}

void ssd1306_contrast(int contrast)
{
	ssd1306_i2c_contrast(contrast);
}

void ssd1306_invert(uint8_t *buf, size_t blen)
{
	uint8_t wk;
	for(int i=0; i<blen; i++){
		wk = buf[i];
		buf[i] = ~wk;
	}
}

void ssd1306_flip(uint8_t *buf, size_t blen)
{
	for(int i=0; i<blen; i++){
		buf[i] = ssd1306_rotate_byte(buf[i]);
	}
}

// Set pixel to internal buffer. Not show it.
void _ssd1306_pixel(int xpos, int ypos, bool invert)
{
	uint8_t _page = (ypos / 8);
	uint8_t _bits = (ypos % 8);
	uint8_t _seg = xpos;
	uint8_t wk0 = dev._page[_page]._segs[_seg];
	uint8_t wk1 = 1 << _bits;
	ESP_LOGD(TAG, "ypos=%d _page=%d _bits=%d wk0=0x%02x wk1=0x%02x", ypos, _page, _bits, wk0, wk1);
	if (invert) {
		wk0 = wk0 & ~wk1;
	} else {
		wk0 = wk0 | wk1;
	}
	if (dev._flip) wk0 = ssd1306_rotate_byte(wk0);
	ESP_LOGD(TAG, "wk0=0x%02x wk1=0x%02x", wk0, wk1);
	dev._page[_page]._segs[_seg] = wk0;
}

void _ssd1306_line(int x1, int y1, int x2, int y2,  bool invert)
{
	int i;
	int dx,dy;
	int sx,sy;
	int E;

	/* distance between two points */
	dx = ( x2 > x1 ) ? x2 - x1 : x1 - x2;
	dy = ( y2 > y1 ) ? y2 - y1 : y1 - y2;

	/* direction of two point */
	sx = ( x2 > x1 ) ? 1 : -1;
	sy = ( y2 > y1 ) ? 1 : -1;

	/* inclination < 1 */
	if ( dx > dy ) {
		E = -dx;
		for ( i = 0 ; i <= dx ; i++ ) {
			_ssd1306_pixel(x1, y1, invert);
			x1 += sx;
			E += 2 * dy;
			if ( E >= 0 ) {
			y1 += sy;
			E -= 2 * dx;
		}
	}

	/* inclination >= 1 */
	} else {
		E = -dy;
		for ( i = 0 ; i <= dy ; i++ ) {
			_ssd1306_pixel(x1, y1, invert);
			y1 += sy;
			E += 2 * dx;
			if ( E >= 0 ) {
				x1 += sx;
				E -= 2 * dy;
			}
		}
	}
}

void ssd1306_clear_line(int page, bool invert)
{
	char space[16];
	memset(space, 0x00, sizeof(space));
	ssd1306_display_text(page, space, sizeof(space), invert);
}


uint8_t ssd1306_rotate_byte(uint8_t ch1) {
	uint8_t ch2 = 0;
	for (int j=0;j<8;j++) {
		ch2 = (ch2 << 1) + (ch1 & 0x01);
		ch1 = ch1 >> 1;
	}
	return ch2;
}

esp_err_t ssd1306_i2c_delete_dev(){
    return i2c_master_bus_rm_device(dev_handle); 
}

