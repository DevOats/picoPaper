#include "EPD_7in5_V2.h"
#include "pico/time.h"
#include "pico/stdlib.h"
#include "pico/unique_id.h"
#include <math.h>

#include "DEV_Config.h"
#include "GUI_Paint.h"
#include "Debug.h"
#include <stdlib.h>
#include "picoDisplay.h"

typedef enum rxByteStateEnum{
    WAITING_FOR_START,
    WAITING_FOR_MSGBYTE
} rxByteStates;

typedef enum rxFunctionStateEnum{
    RX_FUNCTION_IDLE,
    RX_FUNCTION_IMAGERX,
    RX_FUNCTION_DRAWSTRING,
} rxFunctionStates;

rxByteStates rxByteState = WAITING_FOR_START; 
rxFunctionStates rxFunctionState = RX_FUNCTION_IDLE;

const char BYTE_START_CHAR = ':';
const char UART_RESET_CHAR = '/';

char drawStringRxBuffer[255];
int drawStringRxBufferIndex = 0;

const char* ACK_IMAGE_RECEIVED_MSG = "IMG_RCVD\0";
const char* ACK_CLEAR_DISPLAY_MSG = "CLR_SCR\0";
const char* ACK_SPLASH_SCREEN_MSG = "SPLASH\0";
const char* ACK_DISPLAY_IMG_BUFFER = "DISPLAY\0";

const char* ACK_MESSAGE_START = "~ACK#\0";
const char* ERROR_MESSAGE_START = "~ERR#\0";
const char* DEBUG_MESSAGE_START = "~DBG#\0";
const char* MESSAGE_END = "^\n\0";

const UBYTE CMD_DEVICE_IDENT = 0x01;
const UBYTE CMD_IMG_RX = 0x02;
const UBYTE CMD_IMG_DISPLAY = 0x03;
const UBYTE CMD_CLEAR_DISPLAY = 0x04;
const UBYTE CMD_DISPLAY_SPLASH = 0x05;
const UBYTE CMD_DRAW_STRING = 0x06;

//
// Draw string command: 
// [0x06][XXX][YYY][font][color][bgColor][stringLength][^\n]
//
// XXX: 3 characters X position
// YYY: 3 characters Y position
// 
// Font: 2 characters: 08, 12, 16, 20, 24
//
// color: 1 Character: Text color. 1 for black. 0 for white
//
// bgColor: 1 Character: Backgroud color. 1 for black. 0 for white
//
// Stringlength: 3 characters. The length of the string.
//
// String: The string to display. 
// The length of the string is defined by the stringLength field. 
// Does NOT need to be null terminated. NewLines inside the string are not supported.
//
// [^\n]: End of message character. This is a caret (^) followed by a newline character.
//



const char* ident_device = "PicoPaper\0";
const char* ident_version = "1.0.0\0";
const char* ident_display_type = "ePaper\0";
const char* ident_display_size = "7.5\0";
const int ident_display_width = 800;
const int ident_display_height = 480;
const char* ident_display_color = "BW\0";
const char* ident_display_format = "1bpp\0";
const char* ident_board = "Raspberry Pi Pico 2W\0";

char* identString = "{"
"\"device\":\"%s\","
"\"version\":\"%s\","
"\"id\":\"%s\","
"\"board\":\"%s\","
"\"Display\":{"
"\"type\":\"%s\","
"\"size\":\"%s\","
"\"Resolution"
"\":{"
"\"width\":%d,"
"\"height\":%d"
"},"
"\"color\":\"%s\","
"\"format\":\"%s\""
"}}\0";


char messageByteString[3];
int msgByteIndex = 0;
UBYTE *BlackImage;
int imageRxIndex;
UDOUBLE ImagesizeInBytes;

void initialize(void);
void listenOnUart(void);
void listenForStartCharacter(char character);
void receveiveMsgByte(char character);
void ProcessByteReceived(UBYTE msg);
void selectNewRxFuntion(UBYTE msg);
void receiveNextImageByte(UBYTE msg);
void runDisplayImageCommand(void);
void runClearDisplayCommand(void);
void runDisplaySplashScreenCommand(void);
void resetByteMsgRx(void);
void startByteMsgRx(void);
void resetByteMsgRx(void);
void resetUartStateMachine(void);
void sendAckMessage(const char* message);
void sendErrorMessage(const char* message);
void sendDebugMessage(const char* message);
void showSplashScreen(void);
void runIdentCommand(void);
void getPicoSerialNumber(char* idChars);
int createIdentJson(char* identJson, int maxLength);
int buildIdentString(char* targetString, int length, char* serial);
void receiveNextDrawStringByte(UBYTE msg);
void processCompleteDrawStringCommand(void);

void picoDisplay_run(void)
{
    initialize();
    showSplashScreen();
    listenOnUart();
}


// idChars should contain room for 16 chars + 1 null terminator (17 bytes)
void getPicoSerialNumber(char* idChars) {
    pico_unique_board_id_t id;
    pico_get_unique_board_id(&id);

    char hexByte[3];
    hexByte[2] = '\0';

    for (int i = 0; i < 8; i++) {
        snprintf(hexByte, 3, "%02X", id.id[i]);
        idChars[i*2] = hexByte[0];
        idChars[i*2+1] = hexByte[1];
    }
    idChars[16] = '\0';
}


 int createIdentJson(char* identJson, int maxLength){

    char serialString[17];
    getPicoSerialNumber(serialString);

    int stringLength = buildIdentString(NULL, 0, serialString);
    stringLength += 1;  // For the null terminator

    if(stringLength > maxLength){
        return -1;
    }
    buildIdentString(identJson, stringLength, serialString);
    return stringLength;
}


int buildIdentString(char* targetString, int length, char* serial){
    
    int strLen = snprintf(targetString, length, identString, 
        ident_device, 
        ident_version, 
        serial, 
        ident_board, 
        ident_display_type, 
        ident_display_size, 
        ident_display_width, 
        ident_display_height, 
        ident_display_color, 
        ident_display_format);

        return strLen;
}


void listenOnUart(void){

    char character;

    while(true){

        scanf("%c", &character);

        if(character == UART_RESET_CHAR){
            resetUartStateMachine();
        }
        else{
            switch (rxByteState)
            {
                case WAITING_FOR_START:
                    //printf("WAITING_FOR_START\n");
                    listenForStartCharacter(character);
                    break;
                
                case WAITING_FOR_MSGBYTE:
                    //printf("WAITING_FOR_MSGBYTE\n");
                    receveiveMsgByte(character);
                break;
                default:
                    printf(ERROR_MESSAGE_START);
                    printf("Unsupported RxByteState");
                    printf(MESSAGE_END);
                    break;
            }
        }
    }
}


void resetUartStateMachine(void){
    //printf("Resetting UART state machine\n");
    resetByteMsgRx();
    imageRxIndex = 0;
    rxFunctionState = RX_FUNCTION_IDLE;
}


void resetByteMsgRx(){
    rxByteState = WAITING_FOR_START;
    msgByteIndex = 0;
    //printf("resetByteMsgRx. rxByteState: %d\n", rxByteState);
}


void startByteMsgRx(void){
    rxByteState = WAITING_FOR_MSGBYTE;
    msgByteIndex = 0;
    //printf("startByteMsgRx. rxByteState: %d\n", rxByteState);
}


void listenForStartCharacter(char character){

    if(character == BYTE_START_CHAR){
        //printf("StartChar received\n");
        startByteMsgRx();
    }
    else{
        // Unexpected character
        printf(ERROR_MESSAGE_START);
        printf("Unexpected Character: %c ", character);
        printf(MESSAGE_END);
    }
}


void receveiveMsgByte(char character){

    messageByteString[msgByteIndex] = character;
    msgByteIndex++;
    //printf("RX: %c\n", character);
    UBYTE msg = 0;

    // Reset the byte index if we unexpectedly receive a start character
    if(character == BYTE_START_CHAR){
        startByteMsgRx();
        //printf("Unexpecedly received startCharacter\n");
    }
    else{
        if(msgByteIndex == 2){
            
            int success = sscanf(messageByteString, "%2x", &msg);
            //printf("Message: %.2s --> %d  (success: %d)\n", messageByteString, msg, success);

            if(success == 1){
                ProcessByteReceived(msg);
            }
            else{
                // Failed to parse the received characters into a byte
                printf(ERROR_MESSAGE_START);
                printf("Failed to parse hex characters to a byte");
                printf(MESSAGE_END);
            }
            resetByteMsgRx();            
        }
    }
}


void ProcessByteReceived(UBYTE msg){

    switch(rxFunctionState){
        case RX_FUNCTION_IDLE:
            selectNewRxFuntion(msg);
            break;
        
        case RX_FUNCTION_IMAGERX:
            receiveNextImageByte(msg);
            break;

        case RX_FUNCTION_DRAWSTRING:
            receiveNextDrawStringByte(msg);
            break;

        default:
            // Unsupported RxFunctionState 
            printf(ERROR_MESSAGE_START);
            printf("Unsupported RxFunctionState");
            printf(MESSAGE_END);
            break;
    }
}


void selectNewRxFuntion(UBYTE msg){
    switch(msg){
        case CMD_DEVICE_IDENT:
            runIdentCommand();
            rxFunctionState = RX_FUNCTION_IDLE;
            break;
        case CMD_IMG_RX:
            //printf("Starting Image RX\n");
            imageRxIndex = 0;
            rxFunctionState = RX_FUNCTION_IMAGERX;
            break;
        case CMD_IMG_DISPLAY:
            runDisplayImageCommand();
            rxFunctionState = RX_FUNCTION_IDLE;
            break;
        case CMD_CLEAR_DISPLAY:
            runClearDisplayCommand();
            rxFunctionState = RX_FUNCTION_IDLE;
            break;
        case CMD_DISPLAY_SPLASH:
            runDisplaySplashScreenCommand();
            rxFunctionState = RX_FUNCTION_IDLE;
            break;
        case CMD_DRAW_STRING:
            //printf("Received command to draw string\n");
            rxFunctionState = RX_FUNCTION_DRAWSTRING;
            drawStringRxBufferIndex = 0;
            break;
        default:
            // Unsuppported command
            sendErrorMessage("Unsupported command: 0x%2x");
            rxFunctionState = RX_FUNCTION_IDLE;
            break;
    }
}

void receiveNextDrawStringByte(UBYTE msg)
{
    drawStringRxBuffer[drawStringRxBufferIndex] = msg;
    drawStringRxBufferIndex++;

    if(drawStringRxBufferIndex >= 255){
        sendErrorMessage("DrawString buffer overflow");
        rxFunctionState = RX_FUNCTION_IDLE;
    }

    if(msg == '\n' && drawStringRxBuffer[drawStringRxBufferIndex -2] == '^'){
        processCompleteDrawStringCommand();
    }

}


void processCompleteDrawStringCommand(void){


    char xString[4];
    char yString[4];
    char fontString[3];
    char colorString[2];
    char bgColorString[2];
    char stringLengthString[4];
    char contentString[255];
    
    strncpy(xString, drawStringRxBuffer, 3);
    xString[3] = '\0';

    strncpy(yString, drawStringRxBuffer + 3, 3);
    yString[3] = '\0';

    strncpy(fontString, drawStringRxBuffer + 6, 2);
    fontString[2] = '\0';

    strncpy(colorString, drawStringRxBuffer + 8, 1);
    colorString[1] = '\0';

    strncpy(bgColorString, drawStringRxBuffer + 9, 1);
    bgColorString[1] = '\0';

    strncpy(stringLengthString, drawStringRxBuffer + 10, 3);
    stringLengthString[3] = '\0';


    int x = atoi(xString);
    int y = atoi(yString);
    int font = atoi(fontString);
    int color = atoi(colorString);
    int bgColor = atoi(bgColorString);
    int stringLength = atoi(stringLengthString);

    // Font = 
    // foreColor = //
    // bgColor = //
    
    // Todo: Figure out string length before copying the content

    if(stringLength < 240){

        strncpy(contentString, drawStringRxBuffer + 13, stringLength);
        contentString[stringLength] = '\0';

        Paint_DrawString_EN(x, y, contentString, &Font24, color, bgColor);

        sendAckMessage(ACK_DISPLAY_IMG_BUFFER);
    }
    else{
        sendErrorMessage("String too long");
    }

    rxFunctionState = RX_FUNCTION_IDLE;
}


void sendDebugMessage(const char* message){
    printf("%s%s%s", DEBUG_MESSAGE_START, message, MESSAGE_END);
}


void sendAckMessage(const char* message){
    printf("%s%s%s", ACK_MESSAGE_START, message, MESSAGE_END);
}


void sendErrorMessage(const char* message){
    printf("%s%s%s", ERROR_MESSAGE_START, message, MESSAGE_END);
}


void receiveNextImageByte(UBYTE msg){
    //printf("BlackImage[%d] = 0x%2x\n", imageRxIndex, msg);

    BlackImage[imageRxIndex] = msg;
    imageRxIndex++;
    
    if(imageRxIndex >= ImagesizeInBytes){
        //printf("Image received\n");
        sendAckMessage(ACK_IMAGE_RECEIVED_MSG);
        rxFunctionState = RX_FUNCTION_IDLE;
    }
}


void runIdentCommand(){

    int jsonMaxLength = 256;
    char identJson[jsonMaxLength];

    int written = createIdentJson(identJson, jsonMaxLength);

    if(written > 1){
        sendAckMessage(identJson);
    }
    else{
        sendErrorMessage("Failed to create identJson");
    }
}




void runClearDisplayCommand(void){
    //printf("Received command to clear display\n");

    EPD_7IN5_V2_Init();
    EPD_7IN5_V2_Clear();
    EPD_7IN5_V2_Sleep();
    DEV_Delay_ms(50);
    sendAckMessage(ACK_CLEAR_DISPLAY_MSG);
}

void runDisplaySplashScreenCommand(){
    //printf("Received command to display splash screen\n");
    showSplashScreen();    
    sendAckMessage(ACK_SPLASH_SCREEN_MSG);
}


void showSplashScreen(void){
    EPD_7IN5_V2_Init();
    Paint_SelectImage(BlackImage, EPD_7IN5_V2_WIDTH, EPD_7IN5_V2_HEIGHT);
    Paint_Clear(WHITE);


    char idString[17];
    getPicoSerialNumber(idString);

    Paint_DrawRectangle(300, 100, 500, 140, BLACK, DOT_PIXEL_3X3, DRAW_FILL_EMPTY);

    Paint_DrawString_EN(315, 110, "Pico Paper", &Font24, BLACK, WHITE);
    Paint_DrawString_EN(310, 160, "(c) 2025 DevOats", &Font16, BLACK, WHITE);

    Paint_DrawLine(295, 240, 295, 370, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(320, 300, 320, 370, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);

    Paint_DrawString_EN(300, 240, "Version:", &Font12, BLACK, WHITE);
    Paint_DrawString_EN(400, 240, ident_version, &Font12, BLACK, WHITE);

    Paint_DrawString_EN(300, 255, "Board:", &Font12, BLACK, WHITE);
    Paint_DrawString_EN(400, 255, ident_board, &Font12, BLACK, WHITE);

    Paint_DrawString_EN(300, 270, "ID:", &Font12, BLACK, WHITE);
    Paint_DrawString_EN(400, 270, idString, &Font12, BLACK, WHITE);

    Paint_DrawString_EN(300, 285, "Display:", &Font12, BLACK, WHITE);

    Paint_DrawString_EN(325, 300, "Type:", &Font12, BLACK, WHITE);
    Paint_DrawString_EN(450, 300, ident_display_type, &Font12, BLACK, WHITE);

    Paint_DrawString_EN(325, 315, "Size:", &Font12, BLACK, WHITE);
    Paint_DrawString_EN(450, 315, ident_display_size, &Font12, BLACK, WHITE);

    Paint_DrawString_EN(325, 330, "Color:", &Font12, BLACK, WHITE);
    Paint_DrawString_EN(450, 330, ident_display_color, &Font12, BLACK, WHITE);

    Paint_DrawString_EN(325, 345, "PixelFormat:", &Font12, BLACK, WHITE);
    Paint_DrawString_EN(450, 345, ident_display_format, &Font12, BLACK, WHITE);

    char resolutionString[12];
    snprintf(resolutionString, 12, "%d x %d", ident_display_width, ident_display_height);

    Paint_DrawString_EN(325, 360, "Resolution:", &Font12, BLACK, WHITE);
    Paint_DrawString_EN(450, 360, resolutionString, &Font12, BLACK, WHITE);

    Paint_DrawString_EN(195, 450, "Waiting for the PicoPaper connection to upload an image...", &Font12, BLACK, WHITE);

    Paint_DrawRectangle(1, 1, EPD_7IN5_V2_WIDTH-1, EPD_7IN5_V2_HEIGHT-1, BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    Paint_DrawRectangle(5, 5, EPD_7IN5_V2_WIDTH-5, EPD_7IN5_V2_HEIGHT-5, BLACK, DOT_PIXEL_3X3, DRAW_FILL_EMPTY);

    EPD_7IN5_V2_Display(BlackImage);
    EPD_7IN5_V2_Sleep();
    DEV_Delay_ms(50);
}


void runDisplayImageCommand(){
    //printf("Received command to display image\n");

    EPD_7IN5_V2_Init();
    EPD_7IN5_V2_Display(BlackImage);
    EPD_7IN5_V2_Sleep();
    DEV_Delay_ms(50);
    sendAckMessage(ACK_DISPLAY_IMG_BUFFER);
}


void initialize(void){
    //printf("EPD_7IN5_V2_test Demo\r\n");
    if(DEV_Module_Init()!=0){
        return;
    }

    ImagesizeInBytes = ((EPD_7IN5_V2_WIDTH % 8 == 0)? (EPD_7IN5_V2_WIDTH / 8 ): (EPD_7IN5_V2_WIDTH / 8 + 1)) * EPD_7IN5_V2_HEIGHT;
    if((BlackImage = (UBYTE *)malloc(ImagesizeInBytes)) == NULL) {
        printf("Failed to apply for black memory...\r\n");
        return;
        }
    Paint_NewImage(BlackImage, EPD_7IN5_V2_WIDTH, EPD_7IN5_V2_HEIGHT, 0, WHITE);     
    messageByteString[2] = 0;
}



