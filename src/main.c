/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  *
  * Copyright (c) 2016 Mori
  *
  * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
  * associated documentation files (the "Software"), to deal in the Software without restriction,
  * including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
  * and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
  * subject to the following conditions:
  *
  * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  *
  * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
  * LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
  * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
  * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
  *
  ******************************************************************************
*/

#include "stm32f4xx.h"
#include "stm32f4_generic.h"
			
#include "usbh_core.h"
#include "usbh_hid.h"
#include "usbh_hub.h"

#include "log.h"
#include "microshell.h"
#include "msconf.h"
#include "mscmd.h"

USBH_HandleTypeDef hUSBHost[5];

static void USBH_UserProcess (USBH_HandleTypeDef *pHost, uint8_t vId);
static void hub_process();
void SystemClock_Config(void);
static void on_mainloop_tick();

typedef struct {
} USER_OBJECT;

static MSCMD_USER_RESULT usrcmd_system(MSOPT *msopt, MSCMD_USER_OBJECT usrobj);
static MSCMD_USER_RESULT usrcmd_config(MSOPT *msopt, MSCMD_USER_OBJECT usrobj);
static MSCMD_USER_RESULT usrcmd_help(MSOPT *msopt, MSCMD_USER_OBJECT usrobj);

static MSCMD_COMMAND_TABLE table[] = {
    {   "system",   usrcmd_system   },
    {   "config",   usrcmd_config   },
    {   "help",     usrcmd_help     },
    {   "?",        usrcmd_help     },
};

#define RECV_BUF_COUNT (8)
static uint8_t recv_buf[RECV_BUF_COUNT];
static uint8_t recv_has_data = 0;
static uint8_t recv_write_index = 0;

static void uart_recv_char()
{
	HAL_StatusTypeDef status = HAL_UART_Receive_IT(&hUartHandle, &recv_buf[recv_write_index], 1);
	if (recv_write_index+1 < RECV_BUF_COUNT) {
		recv_write_index += 1;
	}
	if (status) {
		// LOG("recv status=%d", status);
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USARTx) {
		recv_has_data = 1;
		uart_recv_char();
	}
}

static char ms_getchar()
{	
	while (1) {
		if (recv_write_index && recv_has_data) {
			// HAL_NVIC_DisableIRQ(USART2_IRQn); // can not send uart during this time
			char c = recv_buf[recv_write_index-1];
			recv_write_index--;
			if (!recv_write_index) { // no data to read
				recv_has_data = 0;
				// HAL_NVIC_EnableIRQ(USART2_IRQn);
			}
			// LOG1(" 0x%02x", c);
			return c;
		}
		on_mainloop_tick();
	}	
}

static void action_hook(MSCORE_ACTION action)
{

}

static int i = 0;
static int j = 0;
static void on_mainloop_tick()
{
	if (i++ > 150000) {
		i = 0;
		j++;
		// LOG("test %d", j);
	}
			

	if(i > 0 && i <= 150000/2)
		BSP_LED_On(LED1);
	else
		BSP_LED_Off(LED1);

	hub_process();
}

int main(void)
{
	uint32_t i = 0;

	HAL_Init();
	SystemClock_Config();

	BSP_LED_Init(LED1);
	
	while(1)
	{
		if (i++ > 150000)
			// i = 0;
			break;

		if(i > 0 && i <= 150000/2)
			BSP_LED_On(LED1);
		else
			BSP_LED_Off(LED1);
	}

	LOG_INIT(USARTx, 115200);

	// LOG("\033[2J\033[H");
	LOG(" ");
	LOG("APP RUNNING...");
	LOG("MCU-ID %08X", DBGMCU->IDCODE);

	memset(&hUSBHost[0], 0, sizeof(USBH_HandleTypeDef));

	hUSBHost[0].valid   = 1;
	hUSBHost[0].address = USBH_DEVICE_ADDRESS;
	hUSBHost[0].Pipes   = USBH_malloc(sizeof(uint32_t) * USBH_MAX_PIPES_NBR);

	USBH_Init(&hUSBHost[0], USBH_UserProcess, ID_USB_HOST_FS);
	USBH_RegisterClass(&hUSBHost[0], USBH_HID_CLASS);
	USBH_RegisterClass(&hUSBHost[0], USBH_HUB_CLASS);

	USBH_Start(&hUSBHost[0]);

	uart_recv_char();
	char ms_buf[MSCONF_MAX_INPUT_LENGTH];
	MICROSHELL ms;
	MSCMD mscmd;
	USER_OBJECT usrobj = {
    };
	LOG("Type 'help' for list commands.");
	microshell_init(&ms, log_write_char, ms_getchar, action_hook);
	mscmd_init(&mscmd, table, sizeof(table) / sizeof(table[0]), &usrobj);

	while(1) {
		MSCMD_USER_RESULT r;
		LOG1("> ");
		microshell_getline(&ms, ms_buf, sizeof(ms_buf));
		mscmd_execute(&mscmd, ms_buf, &r);
	}
}

// -----

static MSCMD_USER_RESULT usrcmd_system(MSOPT *msopt, MSCMD_USER_OBJECT usrobj)
{
    char buf[MSCONF_MAX_INPUT_LENGTH];
    int argc;
    int i;
    LOG1("[SYSTEM]\r\n");
    msopt_get_argc(msopt, &argc);
    for (i = 0; i < argc; i++) {
        msopt_get_argv(msopt, i, buf, sizeof(buf));
        LOG1(" '");
        LOG1(buf);
        LOG("'\r\n");
    }
    return 0;
}

static MSCMD_USER_RESULT usrcmd_config(MSOPT *msopt, MSCMD_USER_OBJECT usrobj)
{
    char buf[MSCONF_MAX_INPUT_LENGTH];
    int argc;
    int i;
    LOG1("[CONFIG]");
    msopt_get_argc(msopt, &argc);
    for (i = 0; i < argc; i++) {
        msopt_get_argv(msopt, i, buf, sizeof(buf));
        LOG1(" '");
        LOG1(buf);
        LOG("'");
    }
    return 0;
}

static MSCMD_USER_RESULT usrcmd_help(MSOPT *msopt, MSCMD_USER_OBJECT usrobj)
{
    USER_OBJECT *uo = (USER_OBJECT *)usrobj;
    LOG1(
            "system : system command\r\n"
            "config : config command\r\n"
            "help   : help command\r\n"
            );
    return 0;
}

// -----

void hub_process()
{
	// LOG("hub_process");
	static uint8_t current_loop = -1;
	static USBH_HandleTypeDef *_phost = 0;

	if(_phost != NULL && _phost->valid == 1)
	{
		USBH_Process(_phost);

		if(_phost->busy)
			return;
	}

	while(1)
	{
		// LOG("hub loop %d", current_loop);
		current_loop++;

		if(current_loop > MAX_HUB_PORTS)
			current_loop = 0;

		if(hUSBHost[current_loop].valid)
		{
			_phost = &hUSBHost[current_loop];
			USBH_LL_SetupEP0(_phost);

			if(_phost->valid == 3)
			{
LOG("PROCESSING ATTACH %d", _phost->address);
				_phost->valid = 1;
				_phost->busy  = 1;
			}

			break;
		}
	}

	if(_phost != NULL && _phost->valid)
	{
		HID_MOUSE_Info_TypeDef *minfo;
		minfo = USBH_HID_GetMouseInfo(_phost);
		if(minfo != NULL)
		{
LOG("BUTTON %d", minfo->buttons[0]);
		}
		else
		{
			HID_KEYBD_Info_TypeDef *kinfo;
			kinfo = USBH_HID_GetKeybdInfo(_phost);
			if(kinfo != NULL)
			{
LOG("KEYB %d", kinfo->keys[0]);
			}
		}
	}

}

void USBH_UserProcess (USBH_HandleTypeDef *pHost, uint8_t vId)
{
	switch (vId)
	{
		case HOST_USER_SELECT_CONFIGURATION:
			break;

		case HOST_USER_CLASS_SELECTED:
			break;

		case HOST_USER_CLASS_ACTIVE:
			break;

		case HOST_USER_CONNECTION:
			break;

		case HOST_USER_DISCONNECTION:
			break;

		case HOST_USER_UNRECOVERED_ERROR:
			USBH_ErrLog("HOST_USER_UNRECOVERED_ERROR %d", hUSBHost[0].RequestState);
			NVIC_SystemReset();
			break;

		default:
			break;
	}
}

void Error_Handler(void)
{
	
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 15;
  RCC_OscInitStruct.PLL.PLLN = 144;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 5;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSE;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}
