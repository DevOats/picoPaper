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
} rxFunctionStates;

rxByteStates rxByteState = WAITING_FOR_START; 
rxFunctionStates rxFunctionState = RX_FUNCTION_IDLE;

const char BYTE_START_CHAR = ':';
const char UART_RESET_CHAR = '/';

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


char* identString = "{"
"\"device\":\"pi"
"coPaper\","
"\"version\":\"1"
".0.0\","
"\"Display\":{"
""
"\"type\":\""
"ePaper\","
"\"size\":"
"\"7.5\","
"\"resolution"
"\":{"
"\"width"
"\":800,"
"\"height"
"\":480"
"},"
"\"color\":"
"\"BW\","
"\"format\":"
"\"1bpp\""
"}"
"}\0";


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
char* createIdentJson(void);



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


char* createIdentJson(){

    char serialString[17];
    getPicoSerialNumber(serialString);

    sendDebugMessage(serialString);

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
        default:
            // Unsuppported command
            sendErrorMessage("Unsupported command: 0x%2x");
            rxFunctionState = RX_FUNCTION_IDLE;
            break;
    }
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
    
    sendAckMessage(identString);
    createIdentJson();
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

    Paint_DrawString_EN(320, 160, "Pico Paper", &Font24, BLACK, WHITE);
    Paint_DrawString_EN(315, 200, "(c) 2025 DevOats", &Font16, BLACK, WHITE);
    Paint_DrawString_EN(320, 300, "WaveShare 7.5\" e-Paper", &Font12, BLACK, WHITE);
    Paint_DrawString_EN(320, 315, "Black/White", &Font12, BLACK, WHITE);
    Paint_DrawString_EN(320, 330, "800 x 480", &Font12, BLACK, WHITE);

    Paint_DrawString_EN(230, 450, "Use the PicoPaper PC application to display images", &Font12, BLACK, WHITE);

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



