/*******************************************************************************************************************************
* @brief	DCMI driver
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
#include "SWM330_dcmi.h"


/*******************************************************************************************************************************
* @brief	DCMI init
* @param	DCMIx is the DCMI to init
* @param	initStruct is data used to init the DCMI
* @return
*******************************************************************************************************************************/
void DCMI_Init(DCMI_TypeDef * DCMIx, DCMI_InitStructure * initStruct)
{
	SYS->CLKEN0 |= (0x01 << SYS_CLKEN0_DVP_Pos);
	
	DCMI->CR = (1        				  << DCMI_CR_CAPMODE_Pos)  |
			   ((initStruct->Format != 1) << DCMI_CR_CROPEN_Pos)   |		//  crop cannot be used in the JPEG format
			   ((initStruct->Format == 1) << DCMI_CR_JPEG_Pos)     |
			   (0                         << DCMI_CR_EMBSYNC_Pos)  |		// hardware sync (DCMI_VSYNC & DCMI_HSYNC)
			   (initStruct->SampleEdge	  << DCMI_CR_PCKPOL_Pos)   |
			   (1						  << DCMI_CR_HSPOL_Pos)    |
			   (1						  << DCMI_CR_VSPOL_Pos)    |
			   ((initStruct->Format >> 8) << DCMI_CR_BUSWIDTH_Pos) |
			   (initStruct->DownSample	  << DCMI_CR_BYTESEL_Pos);
	
	int pixel_clock_count = initStruct->PixelCount;
	if((initStruct->Format == DCMI_FMT_RGB565) || (initStruct->Format == DCMI_FMT_YUV422))
		pixel_clock_count *= 2;		// each RGB565/YUV422 pixel requires two pixel clocks
	
	DCMIx->CWPOS = ((initStruct->StartLine  - 1) << DCMI_CWPOS_LINE_Pos) |
				   ((initStruct->StartPixel - 1) << DCMI_CWPOS_PIXEL_Pos);
	
	DCMIx->CWSIZ = ((initStruct->LineCount  - 1) << DCMI_CWSIZ_LINE_Pos) |
				   ((pixel_clock_count      - 1) << DCMI_CWSIZ_PIXEL_Pos);
	
	DCMIx->CR |= DCMI_CR_ENA_Msk;
	
	DCMI_INTClr(DCMI, initStruct->IntEn);
	DCMI_INTEn(DCMI, initStruct->IntEn);
	
	if(initStruct->IntEn)
		NVIC_EnableIRQ(DVP_IRQn);
}

/*******************************************************************************************************************************
* @brief	DCMI capture start
* @param	DCMIx is the DCMI to start
* @return
*******************************************************************************************************************************/
void DCMI_Start(DCMI_TypeDef * DCMIx)
{
	DCMIx->CR |= DCMI_CR_CAPON_Msk;
}

/*******************************************************************************************************************************
* @brief	DCMI capture stop, actually stop after current frame captured done
* @param	DCMIx is the DCMI to stop
* @return
*******************************************************************************************************************************/
void DCMI_Stop(DCMI_TypeDef * DCMIx)
{
	DCMIx->CR &= ~DCMI_CR_CAPON_Msk;
}

/*******************************************************************************************************************************
* @brief	DCMI busy query
* @param	DCMIx is the DCMI to query
* @return	true: DCMI is busy on image capture, false: capture done
*******************************************************************************************************************************/
bool DCMI_Busy(DCMI_TypeDef * DCMIx)
{
	return DCMIx->CR & DCMI_CR_CAPON_Msk;
}

/*******************************************************************************************************************************
* @brief	switch to use embedded sync, and set embedded sync code
* @param	DCMIx is the DCMI to set
* @param	frame_start is used to set Frame start delimiter code
* @param	line_start is used to set Line start delimiter code
* @param	line_end is used to set Line end delimiter code
* @param	frame_end is used to set Frame end delimiter code
* @return
* @note		applicable to 8-bit data interface only
*******************************************************************************************************************************/
void DCMI_EmbeddedSync(DCMI_TypeDef * DCMIx, uint8_t frame_start, uint8_t line_start, uint8_t line_end, uint8_t frame_end)
{
	DCMIx->CR &= ~DCMI_CR_ENA_Msk;
	
	DCMIx->ESC = (frame_start << DCMI_ESC_FRMSTA_Pos) |
				 (line_start  << DCMI_ESC_LINSTA_Pos) |
				 (line_end    << DCMI_ESC_LINEND_Pos) |
				 (frame_end   << DCMI_ESC_FRMEND_Pos);
	
	DCMIx->ESM = (0xFF        << DCMI_ESM_FRMSTA_Pos) |
				 (0xFF		  << DCMI_ESM_LINSTA_Pos) |
				 (0xFF		  << DCMI_ESM_LINEND_Pos) |
				 (0xFF		  << DCMI_ESM_FRMEND_Pos);
	
	DCMIx->CR |= DCMI_CR_EMBSYNC_Msk;
	
	DCMIx->CR |= DCMI_CR_ENA_Msk;
}

/*******************************************************************************************************************************
* @brief	DCMI interrupt enable
* @param	DCMIx is the DCMI to set
* @param	it is interrupt type, can be DCMI_IT_FRAME, DCMI_IT_FIFOOV, ..., DCMI_IT_HSYNC and their '|' operation
* @return
*******************************************************************************************************************************/
void DCMI_INTEn(DCMI_TypeDef * DCMIx, uint32_t it)
{
	DCMI->IER |=  it;
}

/*******************************************************************************************************************************
* @brief	DCMI interrupt disable
* @param	DCMIx is the DCMI to set
* @param	it is interrupt type, can be DCMI_IT_FRAME, DCMI_IT_FIFOOV, ..., DCMI_IT_HSYNC and their '|' operation
* @return
*******************************************************************************************************************************/
void DCMI_INTDis(DCMI_TypeDef * DCMIx, uint32_t it)
{
	DCMI->IER &= ~it;
}

/*******************************************************************************************************************************
* @brief	DCMI interrupt flag clear
* @param	DCMIx is the DCMI to set
* @param	it is interrupt type, can be DCMI_IT_FRAME, DCMI_IT_FIFOOV, ..., DCMI_IT_HSYNC and their '|' operation
* @return
*******************************************************************************************************************************/
void DCMI_INTClr(DCMI_TypeDef * DCMIx, uint32_t it)
{
	DCMI->ICR = it;
}

/*******************************************************************************************************************************
* @brief	DCMI interrupt state query
* @param	DCMIx is the DCMI to query
* @param	it is interrupt type, can be DCMI_IT_FRAME, DCMI_IT_FIFOOV, ..., DCMI_IT_HSYNC and their '|' operation
* @return	true: interrupt happened, false: interrupt not happen
*******************************************************************************************************************************/
bool DCMI_INTStat(DCMI_TypeDef * DCMIx, uint32_t it)
{
	return DCMI->RIS & it;
}
