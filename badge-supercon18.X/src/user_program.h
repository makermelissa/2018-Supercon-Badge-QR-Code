/************************************
 * This is the framework for those
 * who wish to write their own C
 * code for the basic badge
 ************************************/
#define MAX_QR_STRING_LENGTH    80
#define	USERPROG_BASEADDR       0x040000
#define BACKGROUND_COLOR        0x00880088  //0x00RRGGBB
#define FOREGROUND_COLOR        0x00FFFF00  //0x00RRGGBB
#define QR_BACKGROUND_COLOR     0x00000000  //0x00RRGGBB
#define QR_FOREGROUND_COLOR     0xFFFFFFFF  //0x00RRGGBB

void user_program_init(void);
void user_program_loop(void);
static void printQr(const uint8_t qrcode[]);
void printString(const char * string, uint16_t x_pixel, uint8_t y_pixel, uint32_t fgcolor, uint32_t bgcolor);
void backspace(void);
void saveQrText(void);
void loadQrText(void);
void updateQrCode(void);
void updateText(void);
void updateScreen(void);
void setup_screen(void);

char qrText[MAX_QR_STRING_LENGTH]; // Text to Encode into QR Code
int lastQrSize = 0;
int lastPixelSize = 0;
int maxQrHeight = 200;