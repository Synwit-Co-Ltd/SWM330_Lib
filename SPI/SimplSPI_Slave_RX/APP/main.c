#include "SWM330.h"
#include <string.h>


uint8_t  TXBuffer[32] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 
						 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F};
uint16_t RXBuffer[32] = {0};
uint32_t RXIndex = 0;


void SerialInit(void);
void SPIMstInit(void);
void SPISlvInit(void);
void SPIMstSend(uint8_t buff[], uint32_t count);

int main(void)
{
	SystemInit();
	
	SerialInit();
	
	SPIMstInit();
	
	SPISlvInit();
	
	while(1==1)
	{
		SPIMstSend(TXBuffer, 32);
		
		for(int i = 0; i < SystemCoreClock/4; i++) __NOP();
	}
}


void SPIMstInit(void)
{
	QSPI_InitStructure QSPI_initStruct;
	
	PORT_Init(PORTB, PIN5, PORTB_PIN5_QSPI0_CK, 0);
	PORT_Init(PORTB, PIN4, PORTB_PIN4_QSPI0_CS, 0);
	PORT_Init(PORTB, PIN3, PORTB_PIN3_QSPI0_D0, 1);
	PORT_Init(PORTB, PIN2, PORTB_PIN2_QSPI0_D1, 1);
	PORT_Init(PORTB, PIN1, PORTB_PIN1_QSPI0_D2, 1);
	PORT_Init(PORTB, PIN0, PORTB_PIN0_QSPI0_D3, 1);
	
	QSPI_initStruct.Size = QSPI_Size_16MB;
	QSPI_initStruct.ClkDiv = 4;
	QSPI_initStruct.ClkMode = QSPI_ClkMode_3;
	QSPI_initStruct.SampleShift = QSPI_SampleShift_NONE;
	QSPI_initStruct.IntEn = 0;
	QSPI_Init(QSPI0, &QSPI_initStruct);
	QSPI_Open(QSPI0);
}

void SPIMstSend(uint8_t buff[], uint32_t count)
{
	QSPI_CmdStructure cmdStruct;
	QSPI_CmdStructClear(&cmdStruct);
	
	cmdStruct.InstructionMode 	 = QSPI_PhaseMode_None;
	cmdStruct.AddressMode 		 = QSPI_PhaseMode_None;
	cmdStruct.AlternateBytesMode = QSPI_PhaseMode_None;
	cmdStruct.DummyCycles 		 = 0;
	cmdStruct.DataMode 			 = QSPI_PhaseMode_1bit;
	cmdStruct.DataCount 		 = count;
	
	QSPI_Command(QSPI0, QSPI_Mode_IndirectWrite, &cmdStruct);
	
	while(count--)
	{
		while(QSPI_FIFOSpace(QSPI0) < 1) __NOP();
		
		QSPI0->DRB = buff[count - 1];
	}
	
	while(QSPI_Busy(QSPI0)) __NOP();
}


void SPISlvInit(void)
{
	SPI_InitStructure SPI_initStruct;
	
	PORT_Init(PORTA, PIN12, FUNMUX0_SPI0_SCLK, 1);
	PORT_Init(PORTA, PIN13, FUNMUX1_SPI0_MISO, 0);
	PORT_Init(PORTA, PIN14, FUNMUX0_SPI0_MOSI, 1);
	PORT_Init(PORTA, PIN15, FUNMUX1_SPI0_SSEL, 1);
	
	SPI_initStruct.clkDiv = 1;
	SPI_initStruct.FrameFormat = SPI_FORMAT_SPI;
	SPI_initStruct.SampleEdge = SPI_SECOND_EDGE;
	SPI_initStruct.IdleLevel = SPI_HIGH_LEVEL;
	SPI_initStruct.WordSize = 8;
	SPI_initStruct.Master = 0;
	SPI_initStruct.RXThreshold = 4;
	SPI_initStruct.RXThresholdIEn = 1;
	SPI_initStruct.TXThreshold = 0;
	SPI_initStruct.TXThresholdIEn = 0;
	SPI_initStruct.TXCompleteIEn = 0;
	SPI_Init(SPI0, &SPI_initStruct);
	
	SPI_INTEn(SPI0, SPI_IT_RX_OVF);			// The SPI no longer receives data after overflow, so the RX FIFO needs to be cleared in the overflow ISR
	
	SPI0->IE |= (1 << SPI_IE_SSRISE_Pos);	// enable SPI_SSEL rising edge interrupt
	
	SPI_Open(SPI0);
}

void SPI0_Handler(void)
{
	if(SPI_INTStat(SPI0, SPI_IT_RX_OVF))
	{
		SPI0->CTRL |= SPI_CTRL_RFCLR_Msk;
		__NOP();__NOP();__NOP();__NOP();
		SPI0->CTRL &= ~SPI_CTRL_RFCLR_Msk;

		SPI_INTClr(SPI0, SPI_IT_RX_OVF);
	}
	
	if(SPI0->IF & SPI_IF_RFTHR_Msk)
	{
		while(SPI0->STAT & SPI_STAT_RFNE_Msk)
		{
			RXBuffer[RXIndex++] = SPI0->DATA;
		}
		
		SPI0->IF = (1 << SPI_IF_RFTHR_Pos);
	}
	
	if(SPI0->IF & SPI_IF_SSRISE_Msk)
	{
		while(SPI0->STAT & SPI_STAT_RFNE_Msk)
		{
			RXBuffer[RXIndex++] = SPI0->DATA;
		}
		
		SPI0->IF = (1 << SPI_IF_SSRISE_Pos);
		
		/* detect rising edge on SPI_SSEL pin, so one frame is received */
		for(uint32_t i = 0; i < RXIndex; i++)
			printf("%d, ", RXBuffer[i]);
		printf("\r\n\r\n");
		
		RXIndex = 0;
		memset(RXBuffer, 0, sizeof(RXBuffer));
	}
}


void SerialInit(void)
{
	UART_InitStructure UART_initStruct;
	
	PORT_Init(PORTA, PIN6, FUNMUX0_UART0_TXD, 0);
	PORT_Init(PORTA, PIN7, FUNMUX1_UART0_RXD, 1);
 	
 	UART_initStruct.Baudrate = 57600;
	UART_initStruct.DataBits = UART_DATA_8BIT;
	UART_initStruct.Parity = UART_PARITY_NONE;
	UART_initStruct.StopBits = UART_STOP_1BIT;
	UART_initStruct.RXThreshold = 3;
	UART_initStruct.RXThresholdIEn = 0;
	UART_initStruct.TXThreshold = 3;
	UART_initStruct.TXThresholdIEn = 0;
	UART_initStruct.TimeoutTime = 10;
	UART_initStruct.TimeoutIEn = 0;
 	UART_Init(UART0, &UART_initStruct);
	UART_Open(UART0);
}

int fputc(int ch, FILE *f)
{
	UART_WriteByte(UART0, ch);
	
	while(UART_IsTXBusy(UART0));
 	
	return ch;
}
