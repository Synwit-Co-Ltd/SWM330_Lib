#ifndef __SWM330_PSRAM_H__
#define __SWM330_PSRAM_H__


typedef struct {
	uint8_t  RowSize;	// PSRAM_ROWSIZE_1KB, PSRAM_ROWSIZE_2KB, PSRAM_ROWSIZE_4KB, PSRAM_ROWSIZE_8KB
	
	uint8_t  tRWR;		// HyperRAM Read-Write Recovery Time in ns
	uint8_t  tACC;		// HyperRAM Read Initial Access Time in ns
	uint8_t  tCSM;		// Chip Select Maximum Low Time in us
} PSRAM_InitStructure;


#define PSRAM_RowSize_1KB	0
#define PSRAM_RowSize_2KB	1
#define PSRAM_RowSize_4KB	2
#define PSRAM_RowSize_8KB	3


void PSRAM_Init(PSRAM_InitStructure * initStruct);

void PSRAM_Sleep(void);
bool PSRAM_IsSleep(void);

void PSRAM_PowerDown(void);
bool PSRAM_IsPowerDown(void);


#endif
