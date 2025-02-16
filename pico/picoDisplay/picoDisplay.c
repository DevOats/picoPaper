#include "EPD_7in5_V2.h"
#include "pico/time.h"
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
const char* ACK_MESSAGE = "~ACK#\0";
const char* ERROR_MESSAGE_START = "~ERR#\0";
const char* ERROR_MESSAGE_END = "^\0";

const UBYTE CMD_DEVICE_IDENT = 0x01;
const UBYTE CMD_IMG_RX = 0x02;
const UBYTE CMD_IMG_DISPLAY = 0x03;
const UBYTE CMD_CLEAR_DISPLAY = 0x04;
const UBYTE CMD_DISPLAY_SPLASH = 0x05;


char* identString = "{\r\n"
"    \"device\": \"pi"
"coPaper\",\r\n"
"    \"version\": \"1"
".0.0\",\r\n"
"    \"Display\":{\r"
"\n"
"        \"type\": \""
"ePaper\",\r\n"
"        \"size\": \""
"7.5\",\r\n"
"        \"resolution"
"\": {\r\n"
"            \"width"
"\": \"800\",\r\n"
"            \"height"
"\": \"480\"\r\n"
"        },\r\n"
"        \"color\": "
"\"BW\",\r\n"
"        \"format\": "
"\"1bpp\"\r\n"
"    }\r\n"
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



void picoDisplay_run(void)
{
    initialize();
    runDisplaySplashScreenCommand();
    listenOnUart();
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
                    printf(ERROR_MESSAGE_END);
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
        printf("Unexpected Character: %c\n", character);
        printf(ERROR_MESSAGE_END);
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
                printf(ERROR_MESSAGE_END);
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
            printf(ERROR_MESSAGE_END);
            break;
    }
}


void selectNewRxFuntion(UBYTE msg){
    switch(msg){
        case CMD_DEVICE_IDENT:
            printf(identString);
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
            printf(ERROR_MESSAGE_START);
            printf("Unsupported command: 0x%2x\n", msg);
            printf(ERROR_MESSAGE_END);
            rxFunctionState = RX_FUNCTION_IDLE;
            break;
    }
}


void receiveNextImageByte(UBYTE msg){
    
    //printf("BlackImage[%d] = 0x%2x\n", imageRxIndex, msg);

    BlackImage[imageRxIndex] = msg;
    imageRxIndex++;
    
    if(imageRxIndex >= ImagesizeInBytes){
        //printf("Image received\n");
        printf(ACK_MESSAGE);
        rxFunctionState = RX_FUNCTION_IDLE;
    }
}


void runClearDisplayCommand(void){
    //printf("Received command to clear display\n");

    EPD_7IN5_V2_Init();
    EPD_7IN5_V2_Clear();
    EPD_7IN5_V2_Sleep();
    DEV_Delay_ms(200);
    printf(ACK_MESSAGE);
}


void runDisplaySplashScreenCommand(){
    //printf("Received command to display splash screen\n");

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
    DEV_Delay_ms(200);
    printf(ACK_MESSAGE);

}


void runDisplayImageCommand(){
    //printf("Received command to display image\n");

    EPD_7IN5_V2_Init();
    EPD_7IN5_V2_Display(BlackImage);
    EPD_7IN5_V2_Sleep();
    DEV_Delay_ms(200);
    printf(ACK_MESSAGE);
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



