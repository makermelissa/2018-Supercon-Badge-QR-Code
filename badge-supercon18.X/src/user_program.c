/************************************
 * Badge QR Code Generator 
 * 
 * By Melissa LeBlanc-Williams
 * https://makermelissa.com
 * 
 * This allows you to type out a string of up to 80 characters and a QR Code
 * is generated on the fly. The string is stored in memory as you type so 
 * resetting the badge retains the last string entered.
 * 
 * If the string is longer than 40 characters, a 2nd line is used and everything
 * is adjusted automatically.
 * 
 * Oh yeah, and you can change the background and foreground colors in
 * user_program.h if you like, though some color combos may be less effective
 * than others.
 * 
 ************************************/


#include "badge_user.h"
#include "qrcodegen.h"
#include <math.h>
#include <string.h>

void user_program_init(void)
{
    start_after_wake = &setup_screen;	//Set function to run when waking from sleep
    clr_buffer();
    enable_display_scanning(0);
    loadQrText();
    setup_screen();
}

void setup_screen(void)
{
    // Clear and redraw the screen after starting or waking up
    tft_fill_area (0, 0, 319, 239, BACKGROUND_COLOR);
    updateScreen();
}

void user_program_loop(void)
{   
    // Input a string and store in qrText
    uint8_t get_stat, char_out;
    get_stat = stdio_get(&char_out);
    if (get_stat != 0) {

        char buffer[2];
        buffer[0] = (char) char_out; // Set First Character to input
        buffer[1] = 0; // Terminate the Buffer
        
        if (char_out > 31 && char_out != K_DEL) { // Make sure it was a printable character that isn't delete
            if (strlen(qrText) < MAX_QR_STRING_LENGTH - 1) {
                if (strlen(qrText) == 40) {
                    // About to go to 2 lines
                    tft_fill_area (0, maxQrHeight, 319, 239 - maxQrHeight, BACKGROUND_COLOR);
                }
                // Append to qrText
                strcat(qrText, buffer);
                updateScreen();
                saveQrText();
            }
		} else if (char_out == BACKSPACE) {
            backspace();
            updateScreen();
        }
    }
    
    // Waste some time but less than 1 ms
    uint16_t quickdelay = 2000;
    while (quickdelay) {
        --quickdelay;
    }
}

void updateScreen(void)
{
    maxQrHeight = strlen(qrText) > 40 ? 185 : 200;
    updateText();
    updateQrCode();
}

void updateText(void)
{
    char qrTextFragment[41];
    
    // Display the current String
    printString("Type in a QR Code String:", 10, maxQrHeight + 5, FOREGROUND_COLOR, BACKGROUND_COLOR);
    if (strlen(qrText) <= 40) {
        // Only one line
        printString(qrText, 0, maxQrHeight + 20, FOREGROUND_COLOR, BACKGROUND_COLOR);
    } else {
        // Draw first line
        memcpy(qrTextFragment, &qrText[0], 40);
        qrTextFragment[40] = '\0';
        printString(qrTextFragment, 0, maxQrHeight + 20, FOREGROUND_COLOR, BACKGROUND_COLOR);

        // Draw second line
        memcpy(qrTextFragment, &qrText[40], strlen(qrText) - 40);
        qrTextFragment[strlen(qrText) - 40] = '\0';        
        printString(qrTextFragment, 0, maxQrHeight + 35, FOREGROUND_COLOR, BACKGROUND_COLOR);
    }    
}

void updateQrCode(void)
{
    enum qrcodegen_Ecc errCorLvl = qrcodegen_Ecc_QUARTILE;  // Error correction level   

    // Make and print the QR Code symbol
    uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX];
    uint8_t tempBuffer[qrcodegen_BUFFER_LEN_MAX];
    bool ok = qrcodegen_encodeText(qrText, tempBuffer, qrcode, errCorLvl,
        qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX, qrcodegen_Mask_AUTO, true);
    if (ok) {
        printQr(qrcode);
    }    
}

void backspace(void)
{
    if (strlen(qrText) > 0) {
        // Move the Terminator up one space and save
        qrText[strlen(qrText) - 1] = '\0';
        saveQrText();
        
        // Determine where the last character was
        int charHeight = maxQrHeight + 20;
        int charLeft = strlen(qrText) * 8;
        if (strlen(qrText) > 40) {
            charHeight += 15;
            charLeft -= 320;
        }
        
        // Write a space over the last character
        printString(" ", charLeft, charHeight, FOREGROUND_COLOR, BACKGROUND_COLOR);
        if (strlen(qrText) == 40) {
            tft_fill_area (0, maxQrHeight, 319, 239 - maxQrHeight, BACKGROUND_COLOR);
        }
    }
}

void printString(const char * string, uint16_t x_pixel, uint8_t y_pixel, uint32_t fgcolor, uint32_t bgcolor)
{
	uint8_t i = 0;

    // Print out the string one character at a time
	while (i < strlen(string)) {
		tft_print_char((uint8_t)string[i], x_pixel+(i*8), y_pixel, fgcolor, bgcolor);
		++i;
	}
}

// Prints the given QR Code to the console.
static void printQr(const uint8_t qrcode[])
{
	int size = qrcodegen_getSize(qrcode);
	int border = 2; // Size in QR Pixels
    int qrSize = size + (border * 2);
    int pixelSize = floor(maxQrHeight / qrSize);
    int leftOffset = floor(160 - (qrSize * pixelSize / 2));
    int x, y;

    // Erase the last QR Size if different
    if (lastQrSize != qrSize || lastPixelSize != pixelSize) {
        int lastLeftOffset = floor(160 - (lastQrSize * lastPixelSize / 2));
        tft_fill_area (lastLeftOffset, 0, lastQrSize * lastPixelSize, lastQrSize * lastPixelSize, BACKGROUND_COLOR);
    }
    
    // Draw the new QR Code segment by segment
	for (y = -border; y < size + border; y++) {
		for (x = -border; x < size + border; x++) {
            tft_fill_area (leftOffset + (x + border) * pixelSize, (y + border) * pixelSize, pixelSize, pixelSize, qrcodegen_getModule(qrcode, x, y) ? QR_BACKGROUND_COLOR : QR_FOREGROUND_COLOR);
		}
	}
    
    // Store the value so we can tell if it's different next time
    lastQrSize = qrSize;
    lastPixelSize = pixelSize;
}

void saveQrText(void)
{
    // Save qrText to memory at location USERPROG_BASEADDR
    uint8_t * data = (uint8_t *) qrText;
	uint32_t addr = USERPROG_BASEADDR;
    fl_erase_4k(addr);
    fl_write_4k(addr, data);	
}

void loadQrText(void)
{
    // Retrieve qrText from memory at location USERPROG_BASEADDR
    uint8_t data[BPROG_SECSIZ];
	fl_read_4k(USERPROG_BASEADDR, data);	

    // If no data stored, load empty string
	if (data[0] == 0xFF) {
        strcpy(qrText, "");
    } else {
        strcpy(qrText, data);
    }
}