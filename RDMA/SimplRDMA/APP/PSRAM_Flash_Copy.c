#include "SWM330.h"

extern uint32_t * PSRAM_Buf;
extern const uint16_t FlashData[0x10000];


/* 执行此函数时，由于 CPU 一直持续访问 Flash 导致 RDMA 无法读取 Flash，
   因此只能将此函数放入 RAM 中执行，这样 CPU 从　RAM 中取指，RDMA 才能正常读取 Flash
 */


void PSRAM_Flash_Copy(void)
{
	RDMA_memcpy(PSRAM_Buf, FlashData, RDMA_UNIT_WORD, sizeof(FlashData)/4);
	while(RDMA_Remaining()) __NOP();
	
#define PSRAM_HPtr	((uint16_t *)PSRAM_Buf)
	
	for(int i = 0; i < sizeof(FlashData)/2; i++)
	{
		if(PSRAM_HPtr[i] != FlashData[i])
		{
			printf("PSRAM_Flash_Copy fail: expected=0x%04X, PSRAM=0x%04X\n", FlashData[i], PSRAM_HPtr[i]);
			
			return;
		}
	}
	
	printf("PSRAM_Flash_Copy pass!\n");
}
