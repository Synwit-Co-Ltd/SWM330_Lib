/*******************************************************************************************************************************
* @brief	PSRAM driver
*
*
********************************************************************************************************************************
* @attention
*
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS WITH CODING INFORMATION 
* REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME. AS A RESULT, SYNWIT SHALL NOT BE HELD LIABLE 
* FOR ANY DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE CONTENT 
* OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING INFORMATION CONTAINED HEREIN IN CONN-
* -ECTION WITH THEIR PRODUCTS.
*
* COPYRIGHT 2012 Synwit Technology 
*******************************************************************************************************************************/
#include "SWM330.h"
#include "SWM330_psram.h"


/*******************************************************************************************************************************
* @brief	PSRAM init
* @param	initStruct is data used to init the PSRAM
* @return
*******************************************************************************************************************************/
void PSRAM_Init(PSRAM_InitStructure * initStruct)
{
	SYS->CLKEN0 |= SYS_CLKEN0_PSRAM_Msk;
	
	uint16_t ns_per_cycle = 1000 / CyclesPerUs;
	
	PSRAMC->TR1US = CyclesPerUs;
	
	PSRAMC->TR = (initStruct->tCSM				  << PSRAMC_TR_CSM_Pos) |
				 (initStruct->tRWR / ns_per_cycle << PSRAMC_TR_RWR_Pos) |
				 (initStruct->tACC / ns_per_cycle << PSRAMC_TR_ACC_Pos);
	
	/* W956A8MB		tVCS	tRP		tRH		tRPH	tHSIN	tCSHS	tEXTHS	tDPDIN	tCSDPD	tEXTDPD
	 * MIN					200ns	200ns	400ns			60ns					200ns
	 * MAX			150us							3us		3000ns	100us	3us		3000ns	150us
	 */
	PSRAMC->INITTR = 300;
	
	PSRAMC->RSTTR  = (1000 / ns_per_cycle << PSRAMC_RSTTR_RP_Pos) |
					 (1000 / ns_per_cycle << PSRAMC_RSTTR_RH_Pos) |
					 (2000 / ns_per_cycle << PSRAMC_RSTTR_RPH_Pos);
	
	PSRAMC->SLPTR  = (10				  << PSRAMC_SLPTR_HSIN_Pos) |
					 (3000 / ns_per_cycle << PSRAMC_SLPTR_CSHS_Pos) |
					 (200				  << PSRAMC_SLPTR_EXTHS_Pos);
	
	PSRAMC->PWDNTR = (10				  << PSRAMC_PWDNTR_DPDIN_Pos) |
					 (3000 / ns_per_cycle << PSRAMC_PWDNTR_CSDPD_Pos) |
					 (200				  << PSRAMC_PWDNTR_EXTDPD_Pos);
	
	PSRAMC->CR0 = (3	<< PSRAMC_CR0_BurstLen_Pos)    |
				  (1	<< PSRAMC_CR0_HybridBurst_Pos) |
				  (0	<< PSRAMC_CR0_FixLatency_Pos)  |
				  (0xF	<< PSRAMC_CR0_InitLatency_Pos) |	// 4 Clock Latency @ 100MHz Max Frequency
				  (1	<< PSRAMC_CR0_MustBe1_Pos);
	
	PSRAMC->CR1 = (1	<< PSRAMC_CR1_RefInterval_Pos) |
				  (0	<< PSRAMC_CR1_PartialRef_Pos)  |
				  (1	<< PSRAMC_CR1_ClockType_Pos)   |
				  PSRAMC_CR1_MustAllBe1_Msk;
	
	PSRAMC->CSR = (initStruct->RowSize << PSRAMC_CSR_ROWSZ_Pos) |
				  (0				   << PSRAMC_CSR_BUS16b_Pos) |
				  (0				   << PSRAMC_CSR_PREFEN_Pos);
	
	while((PSRAMC->CSR & PSRAMC_CSR_INITDONE_Msk) == 0) __NOP();
}


/*******************************************************************************************************************************
* @brief	PSRAM sleep
* @param
* @return
*******************************************************************************************************************************/
void PSRAM_Sleep(void)
{
	PSRAMC->CR1 |= PSRAMC_CR1_Sleep_Msk;
}

/*******************************************************************************************************************************
* @brief	PSRAM sleep state query
* @param
* @return	true: in sleep, false: not in sleep
*******************************************************************************************************************************/
bool PSRAM_IsSleep(void)
{
	return PSRAMC->CSR & PSRAMC_CSR_ISSLEEP_Msk;
}

/*******************************************************************************************************************************
* @brief	PSRAM power-down
* @param
* @return
*******************************************************************************************************************************/
void PSRAM_PowerDown(void)
{
	PSRAMC->CR0 |= PSRAMC_CR0_PowerDown_Msk;
}

/*******************************************************************************************************************************
* @brief	PSRAM power-down state query
* @param
* @return	true: in power-down, false: not in power-down
*******************************************************************************************************************************/
bool PSRAM_IsPowerDown(void)
{
	return PSRAMC->CSR & PSRAMC_CSR_ISPWDN_Msk;
}
