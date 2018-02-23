/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 5 $
 * $Date: 15/09/02 10:03a $
 * @brief    Display an string on TFT LCD panel via SPI interface.
 *
 * @note
 * Copyright (C) 2013~2015 Nuvoton Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include <stdio.h>
#include "M451Series.h"

#define SPI_LCD_PORT  SPI2

#define ILI9341_RESET   PB15
#define ILI9341_DC      PB11
#define ILI9341_LED     PB5
#define BUZZER(x)       PC14 = (!x)

extern uint8_t Font8x16[];
extern unsigned char acFont[];
    
#define White           0xFFFF
#define Black           0x0000
#define Blue            0x001F
#define Blue2           0x051F
#define Red             0xF800
#define Magenta         0xF81F
#define Green           0x07E0
#define Cyan            0x7FFF
#define Yellow          0xFFE0

uint8_t LCD_ReadReg(uint8_t u8Comm)
{
    SPI_ClearRxFIFO(SPI_LCD_PORT);

    ILI9341_DC = 0;

    SPI_WRITE_TX(SPI_LCD_PORT, u8Comm);
    SPI_WRITE_TX(SPI_LCD_PORT, 0x00);

    // wait tx finish
    while(SPI_IS_BUSY(SPI_LCD_PORT));

    SPI_READ_RX(SPI_LCD_PORT);

    return (SPI_READ_RX(SPI_LCD_PORT));
}

void LCD_WriteCommand(uint8_t u8Comm)
{
    ILI9341_DC = 0;

    SPI_WRITE_TX(SPI_LCD_PORT, u8Comm);

    // wait tx finish
    while(SPI_IS_BUSY(SPI_LCD_PORT));
}

void LCD_WriteData(uint8_t u8Data)
{
    ILI9341_DC = 1;

    SPI_WRITE_TX(SPI_LCD_PORT, u8Data);

    // wait tx finish
    while(SPI_IS_BUSY(SPI_LCD_PORT));
}


void ILI9341_LCD_SetAddress(uint32_t x1, uint32_t x2, uint32_t y1, uint32_t y2)
{
    if(x1 >= 240)
        x1 = 239;
    if(x2 >= 240)
        x2 = 239;
    if(y1 >= 320)
        y1 = 319;
    if(y2 >= 320)
        y2 = 319;

    LCD_WriteCommand(0x2a);
    LCD_WriteData(x1 >> 8);
    LCD_WriteData(x1);
    LCD_WriteData(x2 >> 8);
    LCD_WriteData(x2);

    LCD_WriteCommand(0x2b);
    LCD_WriteData(y1 >> 8);
    LCD_WriteData(y1);
    LCD_WriteData(y2 >> 8);
    LCD_WriteData(y2);
}

void ILI9341_LCD_PutChar8x16(uint16_t x, uint16_t y, uint8_t c, uint32_t fColor, uint32_t bColor)
{
    uint32_t i, j;
    for(i = 0; i < 16; i++)
    {
        uint8_t m = Font8x16[c * 16 + i];
        ILI9341_LCD_SetAddress(x + i, x + i, y, y + 7);
        LCD_WriteCommand(0x2c);

        for(j = 0; j < 8; j++)
        {
            if((m & 0x01) == 0x01)
            {
                LCD_WriteData(fColor >> 8);
                LCD_WriteData(fColor);
            }
            else
            {
                LCD_WriteData(bColor >> 8);
                LCD_WriteData(bColor);
            }
            m >>= 1;
        }
    }
}


void ILI9341_LCD_Fill(uint16_t x, uint16_t y, uint16_t x1, uint16_t y1, uint32_t bColor)
{
    uint32_t i, j;
    
    ILI9341_LCD_SetAddress(x, x1-1, y, y1-1);
    
    //for(i=0;i<x1-x;i++)
    {
        LCD_WriteCommand(0x2c);

        for(j=0;j<(x1-x)*(y1-y);j++)
        {
            LCD_WriteData(bColor >> 8);
            LCD_WriteData(bColor);
        }
    }
}

void ILI9341_LCD_acFont(uint16_t x, uint16_t y, int32_t idx, int32_t scale, uint32_t fColor, uint32_t bColor)
{
    int32_t i, j, k, l;
    uint8_t c;
    
    
    ILI9341_LCD_SetAddress(x, x+scale*24-1, y, y+scale*32-1);
    LCD_WriteCommand(0x2c);
    for(j=0;j<scale*32;j++)
    {
        for(i=0;i<3;i++)
        {
            c = acFont[idx*3*32+(j/scale)*3+i];
            for(k=7;k>=0;k--)
            {
                for(l=0;l<scale;l++)
                {
                    if(((c >> k)&1) == 1)
                    {
                        LCD_WriteData(fColor >> 8);
                        LCD_WriteData(fColor);
                    }
                    else
                    {
                        LCD_WriteData(bColor >> 8);
                        LCD_WriteData(bColor);
                    }
                }
            }
        }
    }
}



void ILI9341_LCD_PutChar16x32(uint16_t x, uint16_t y, uint8_t c, uint32_t fColor, uint32_t bColor)
{
    uint32_t i, j,k;
    int32_t n;
    
    n = 16;
    for(i = 0; i < 16*n; i++)
    {
        uint8_t m = Font8x16[c * 16 + i/n];
        ILI9341_LCD_SetAddress(x + i, x + i, y, y + (8*n-1));
        LCD_WriteCommand(0x2c);

        for(j = 0; j < 8; j++)
        {
            if((m & 0x01) == 0x01)
            {
                for(k=0;k<n;k++)
                {
                    LCD_WriteData(fColor >> 8);
                    LCD_WriteData(fColor);
                }
            }
            else
            {
                for(k=0;k<n;k++)
                {
                    LCD_WriteData(bColor >> 8);
                    LCD_WriteData(bColor);
                }
            }
            m >>= 1;
        }
    }
}


void ILI9341_LCD_PutString(uint16_t x, uint16_t y, uint8_t *s, uint32_t fColor, uint32_t bColor)
{
    uint8_t l = 0;
    while(*s)
    {
        if(*s < 0x80)
        {
            //ILI9341_LCD_PutChar8x16(x, 312 - y - l * 8, *s, fColor, bColor);
            ILI9341_LCD_PutChar16x32(x, 312-120 - y - l * 120, *s, fColor, bColor);
            
            s++;
            l++;
        }
    }
}



void ILI9341_LCD_PutString8x16(uint16_t x, uint16_t y, uint8_t *s, uint32_t fColor, uint32_t bColor)
{
    uint8_t l = 0;
    while(*s)
    {
        if(*s < 0x80)
        {
            ILI9341_LCD_PutChar8x16(x, 312 - y - l * 8, *s, fColor, bColor);
            //ILI9341_LCD_PutChar16x32(x, 312-120 - y - l * 120, *s, fColor, bColor);
            
            s++;
            l++;
        }
    }
}


void ILI9341_LCD_Init(void)
{
    /* Configure DC/RESET/LED pins */
    ILI9341_DC = 0;
    ILI9341_RESET = 0;
    ILI9341_LED = 0;

    GPIO_SetMode(PB, BIT5, GPIO_MODE_OUTPUT);
    GPIO_SetMode(PB, BIT11, GPIO_MODE_OUTPUT);
    GPIO_SetMode(PB, BIT15, GPIO_MODE_OUTPUT);

    /* Configure LCD */
    ILI9341_DC = 1;

    ILI9341_RESET = 0;
    TIMER_Delay(TIMER0, 20000);

    ILI9341_RESET = 1;
    TIMER_Delay(TIMER0, 40000);

    LCD_WriteCommand(0xCB);
    LCD_WriteData(0x39);
    LCD_WriteData(0x2C);
    LCD_WriteData(0x00);
    LCD_WriteData(0x34);
    LCD_WriteData(0x02);

    LCD_WriteCommand(0xCF);
    LCD_WriteData(0x00);
    LCD_WriteData(0xC1);
    LCD_WriteData(0x30);

    LCD_WriteCommand(0xE8);
    LCD_WriteData(0x85);
    LCD_WriteData(0x00);
    LCD_WriteData(0x78);

    LCD_WriteCommand(0xEA);
    LCD_WriteData(0x00);
    LCD_WriteData(0x00);

    LCD_WriteCommand(0xED);
    LCD_WriteData(0x64);
    LCD_WriteData(0x03);
    LCD_WriteData(0x12);
    LCD_WriteData(0x81);

    LCD_WriteCommand(0xF7);
    LCD_WriteData(0x20);

    LCD_WriteCommand(0xC0);
    LCD_WriteData(0x23);

    LCD_WriteCommand(0xC1);
    LCD_WriteData(0x10);

    LCD_WriteCommand(0xC5);
    LCD_WriteData(0x3e);
    LCD_WriteData(0x28);

    LCD_WriteCommand(0xC7);
    LCD_WriteData(0x86);

    LCD_WriteCommand(0x36);
    LCD_WriteData(0x48);

    LCD_WriteCommand(0x3A);
    LCD_WriteData(0x55);

    LCD_WriteCommand(0xB1);
    LCD_WriteData(0x00);
    LCD_WriteData(0x18);

    LCD_WriteCommand(0xB6);
    LCD_WriteData(0x08);
    LCD_WriteData(0x82);
    LCD_WriteData(0x27);

    LCD_WriteCommand(0xF2);
    LCD_WriteData(0x00);

    LCD_WriteCommand(0x26);
    LCD_WriteData(0x01);

    LCD_WriteCommand(0xE0);
    LCD_WriteData(0x0F);
    LCD_WriteData(0x31);
    LCD_WriteData(0x2B);
    LCD_WriteData(0x0C);
    LCD_WriteData(0x0E);
    LCD_WriteData(0x08);
    LCD_WriteData(0x4E);
    LCD_WriteData(0xF1);
    LCD_WriteData(0x37);
    LCD_WriteData(0x07);
    LCD_WriteData(0x10);
    LCD_WriteData(0x03);
    LCD_WriteData(0x0E);
    LCD_WriteData(0x09);
    LCD_WriteData(0x00);

    LCD_WriteCommand(0xE1);
    LCD_WriteData(0x00);
    LCD_WriteData(0x0E);
    LCD_WriteData(0x14);
    LCD_WriteData(0x03);
    LCD_WriteData(0x11);
    LCD_WriteData(0x07);
    LCD_WriteData(0x31);
    LCD_WriteData(0xC1);
    LCD_WriteData(0x48);
    LCD_WriteData(0x08);
    LCD_WriteData(0x0F);
    LCD_WriteData(0x0C);
    LCD_WriteData(0x31);
    LCD_WriteData(0x36);
    LCD_WriteData(0x0F);

    LCD_WriteCommand(0x11);
    TIMER_Delay(TIMER0, 60000);

    LCD_WriteCommand(0x29);    //Display on

    ILI9341_LED = 1;
}

void SYS_Init(void)
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Unlock protected registers */
    SYS_UnlockReg();

    /* Enable External XTAL (4~24 MHz) */
    CLK_EnableXtalRC(CLK_PWRCTL_HXTEN_Msk);

    /* Waiting for 12MHz clock ready */
    CLK_WaitClockReady(CLK_STATUS_HXTSTB_Msk);

    /* Switch HCLK clock source to HXT */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_HXT, CLK_CLKDIV0_HCLK(1));

    /* Set PLL to power down mode and PLL_STB bit in CLKSTATUS register will be cleared by hardware.*/
    CLK->PLLCTL |= CLK_PLLCTL_PD_Msk;

    /* Set PLL frequency */
    CLK->PLLCTL = CLK_PLLCTL_72MHz_HXT;

    /* Waiting for clock ready */
    CLK_WaitClockReady(CLK_STATUS_PLLSTB_Msk);

    /* Switch HCLK clock source to PLL */
    CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_PLL, CLK_CLKDIV0_HCLK(1));

    /* Enable IP clock */
    CLK_EnableModuleClock(UART0_MODULE);
    CLK_EnableModuleClock(TMR0_MODULE);
    CLK_EnableModuleClock(SPI2_MODULE);

    /* Select IP clock source */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UARTSEL_HXT, CLK_CLKDIV0_UART(1));
    CLK_SetModuleClock(TMR0_MODULE, CLK_CLKSEL1_TMR0SEL_HXT, 0);
    CLK_SetModuleClock(SPI2_MODULE, CLK_CLKSEL2_SPI2SEL_PLL, 0);


    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate SystemCoreClock. */
    SystemCoreClockUpdate();

    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
     /* Set PD multi-function pins for UART0 RXD(PD.6) and TXD(PD.1) */
    SYS->GPD_MFPL &= ~(SYS_GPD_MFPL_PD6MFP_Msk | SYS_GPD_MFPL_PD1MFP_Msk);
    SYS->GPD_MFPL |= (SYS_GPD_MFPL_PD6MFP_UART0_RXD | SYS_GPD_MFPL_PD1MFP_UART0_TXD);

		/* SPI2: GPD12=SS, GPD15=CLK, GPD14=MISO, GPD13=MOSI */
    SYS->GPD_MFPH &= ~(SYS_GPD_MFPH_PD12MFP_Msk | SYS_GPD_MFPH_PD13MFP_Msk | SYS_GPD_MFPH_PD14MFP_Msk | SYS_GPD_MFPH_PD15MFP_Msk);
    SYS->GPD_MFPH |= (SYS_GPD_MFPH_PD12MFP_SPI2_SS | SYS_GPD_MFPH_PD13MFP_SPI2_MOSI | SYS_GPD_MFPH_PD14MFP_SPI2_MISO | SYS_GPD_MFPH_PD15MFP_SPI2_CLK);

    /* Lock protected registers */
    //SYS_LockReg();
}

volatile uint32_t g_u32Ticks = 0;
void SysTick_Handler(void)
{
    static cnt = 0;

    g_u32Ticks++;
    if(cnt >= 100)
        cnt = 0;
    else if(cnt > 50)
    {
        ILI9341_LED = 1;
    }
    else
    {
        ILI9341_LED = 0;
    }
    cnt++;

}

void PWM_Init(void)
{
    /* Set PC9~PC11 multi-function pins for PWM1 Channel0~2  */
    SYS->GPC_MFPH &= ~(SYS_GPC_MFPH_PC9MFP_Msk | SYS_GPC_MFPH_PC10MFP_Msk | SYS_GPC_MFPH_PC11MFP_Msk);
    SYS->GPC_MFPH |= SYS_GPC_MFPH_PC9MFP_PWM1_CH0 /*| SYS_GPC_MFPH_PC10MFP_PWM1_CH1*/ | SYS_GPC_MFPH_PC11MFP_PWM1_CH2;
    SYS->GPE_MFPL &= ~(SYS_GPE_MFPL_PE2MFP_Msk);
    SYS->GPE_MFPL |= SYS_GPE_MFPL_PE2MFP_PWM1_CH1;

    //SYS->GPC_MFPH &= ~(SYS_GPC_MFPH_PC14MFP_PWM1_CH5);
    //SYS->GPC_MFPH |= SYS_GPC_MFPH_PC14MFP_PWM1_CH5;
    
    /* Enable PWM module clock */
    CLK_EnableModuleClock(PWM1_MODULE);

    /* Select PWM module clock source */
    CLK_SetModuleClock(PWM1_MODULE, CLK_CLKSEL2_PWM1SEL_PCLK1, 0);

    /* Reset PWM1 channel 0~5 */
    SYS_ResetModule(PWM1_RST);
    
    
    // set PWM to down count type(edge aligned)
    PWM1->CTL1 &= ~(PWM_CTL1_CNTTYPE0_Msk | PWM_CTL1_CNTTYPE1_Msk);
    PWM1->CTL1 |= 0x5;

    /*Set PWM Timer clock prescaler*/
    PWM_SET_PRESCALER(PWM1, 0, 72-1); 
    PWM_SET_PRESCALER(PWM1, 1, 72-1); 

    /*Set PWM Timer duty*/
    PWM_SET_CMR(PWM1, 0, 40-1); // 40 ms high (For ADC sampling)
    PWM_SET_CMR(PWM1, 1, 320-1); // 320 ms low  

    /*
            +----+
    ________|40us|______
            ^__Trigger to start EADC here

       +---------+
    ___|    320us|______
    

    */



    /*Set PWM Timer period*/
    PWM_SET_CNR(PWM1, 0, 10000 - 1);
    PWM_SET_CNR(PWM1, 1, 10000 - 1);

    /* Enable PWM_CH0 period point trigger EADC */
    PWM1->EADCTS0 |= (PWM_EADCTS0_TRGEN0_Msk | (PWM_TRIGGER_ADC_EVEN_COMPARE_DOWN_COUNT_POINT << PWM_EADCTS0_TRGSEL0_Pos)) ;

    /* Set waveform generation */
    PWM1->WGCTL0 = 0x50000;
    PWM1->WGCTL1 = 0xa0000;

    // Enable output of PWM1 channel 1
    //PWM1->POEN |= PWM_POEN_POEN1_Msk | PWM_POEN_POEN0_Msk;
    PWM1->POEN |= PWM_POEN_POEN1_Msk;

    // Enable PWM0 channel 0 period interrupt, use channel 0 to measure time.
    //PWM1->INTEN0 = (PWM1->INTEN0 & ~PWM_INTEN0_PIEN0_Msk) | PWM_INTEN0_PIEN0_Msk;
    //NVIC_EnableIRQ(PWM0P1_IRQn);

    // Start
    PWM1->CNTEN |= PWM_CNTEN_CNTEN0_Msk | PWM_CNTEN_CNTEN1_Msk;

}


#define ADC_RESULT()    ((EADC->DAT[0] & EADC_DAT_RESULT_Msk) >> EADC_DAT_RESULT_Pos)
#define ADC_RESULT1()    ((EADC->DAT[1] & EADC_DAT_RESULT_Msk) >> EADC_DAT_RESULT_Pos)

void EADC_Init(void)
{
    /* Enable EADC module clock */
    CLK->APBCLK0 |= CLK_APBCLK0_EADCCKEN_Msk;

    /* EADC clock source is 72MHz, set divider to 8, ADC clock is 72/8 MHz */
    CLK->CLKDIV0  = (CLK->CLKDIV0 & ~CLK_CLKDIV0_EADCDIV_Msk) | (((8) - 1) << CLK_CLKDIV0_EADCDIV_Pos);

    /* Configure the GPB0 - GPB3 ADC analog input pins.  */
    SYS->GPB_MFPL &= ~(SYS_GPB_MFPL_PB0MFP_Msk | SYS_GPB_MFPL_PB1MFP_Msk |
                       SYS_GPB_MFPL_PB2MFP_Msk | SYS_GPB_MFPL_PB3MFP_Msk);
    SYS->GPB_MFPL |= (SYS_GPB_MFPL_PB0MFP_EADC_CH0 | SYS_GPB_MFPL_PB1MFP_EADC_CH1 |
                      SYS_GPB_MFPL_PB2MFP_EADC_CH2 | SYS_GPB_MFPL_PB3MFP_EADC_CH3);

    /* Disable the GPB0 - GPB3 digital input path to avoid the leakage current. */
    GPIO_DISABLE_DIGITAL_PATH(PB, 0xF);

    /* Set the ADC internal sampling time, input mode as single-end and enable the A/D converter */
    EADC->CTL = (EADC_CTL_SMPTSEL5 | EADC_CTL_DIFFEN_SINGLE_END | EADC_CTL_ADCEN_Msk);
    /* Configure the sample module 0 for analog input channel 3 and software trigger source.*/
    EADC->SCTL[0] = EADC_PWM1TG0_TRIGGER | EADC_SCTL_CHSEL(3);
    EADC->SCTL[1] = EADC_SOFTWARE_TRIGGER | EADC_SCTL_CHSEL(0);
    //EADC->SCTL[0] = EADC_SOFTWARE_TRIGGER | EADC_SCTL_CHSEL(3);
    
    
    /* Clear the A/D ADINT0 interrupt flag for safe */
    EADC->STATUS2 = EADC_STATUS2_ADIF0_Msk | EADC_STATUS2_ADIF1_Msk;

    /* Enable the sample module 0 interrupt.  */
    //EADC->CTL |= EADC_CTL_ADCIEN0_Msk | EADC_CTL_ADCIEN1_Msk;  //Enable sample module A/D ADINT0 Interrupt.
    //EADC->INTSRC[0] = 0x3;              //Enable sample module 0/1 interrupt.
//    NVIC_EnableIRQ(ADC00_IRQn);

    /* Reset the ADC interrupt indicator and trigger sample module 1 to start A/D conversion */
    EADC->SWTRG |= (0x2 << EADC_SWTRG_SWTRG_Pos);

}



#define KEY_EVENT ((PD11==0)||(PD2==0)||(PD3==0)||(PC0==0)||(PA8==0)||(PF2==0))

void GPA_IRQHandler(void)
{
    PA->INTSRC = PA->INTSRC;
}
void GPB_IRQHandler(void)
{
    PB->INTSRC = PB->INTSRC;
}
void GPC_IRQHandler(void)
{
    PC->INTSRC = PC->INTSRC;
}
void GPD_IRQHandler(void)
{
    PD->INTSRC = PD->INTSRC;
}
void GPE_IRQHandler(void)
{
    PE->INTSRC = PE->INTSRC;
}
void GPF_IRQHandler(void)
{
    PF->INTSRC = PF->INTSRC;
}

void ShowNum(uint32_t n)
{
    
    uint32_t color;
    
    color = Cyan;
    
    if(n >=100000)
        n = 99999;
    ILI9341_LCD_acFont(0, 0, n/10000, 1, color, Black);

    ILI9341_LCD_acFont(24, 0, 11, 1,color, Black);

    n = n % 10000;
    ILI9341_LCD_acFont(48, 0, n/1000, 1,color, Black);
    n = n % 1000;
    ILI9341_LCD_acFont(72, 0, n/100, 1,color, Black);
    n = n % 100;
    ILI9341_LCD_acFont(96, 0, n/10, 1,color, Black);
    n = n % 10;
    ILI9341_LCD_acFont(120, 0, n, 1,color, Black);
    
    ILI9341_LCD_acFont(144, 0, 12, 1,color, Black);
    ILI9341_LCD_acFont(168, 0, 13, 1,color, Black);
    
}



void ShowNum2(uint32_t n)
{
    
    uint32_t color;
    
    color = Red;
    
    if(n >=100000)
        n = 99999;
    ILI9341_LCD_acFont(0, 48, n/10000, 1, color, Black);

    ILI9341_LCD_acFont(24, 48, 11, 1,color, Black);

    n = n % 10000;
    ILI9341_LCD_acFont(48, 48, n/1000, 1,color, Black);
    n = n % 1000;
    ILI9341_LCD_acFont(72, 48, n/100, 1,color, Black);
    n = n % 100;
    ILI9341_LCD_acFont(96, 48, n/10, 1,color, Black);
    n = n % 10;
    ILI9341_LCD_acFont(120, 48, n, 1,color, Black);
    
    ILI9341_LCD_acFont(144, 48, 12, 1,color, Black);
    ILI9341_LCD_acFont(168, 48, 13, 1,color, Black);
    
}


void ShowPm25(float f)
{
    int32_t n;
    uint32_t color;
    
    color = Cyan;
    if(f < 0)
        f = 0;
    
    n = (int32_t)(f*1000);
    
    if(n >=10000)
        n = 9999;
    ILI9341_LCD_acFont(0, 0, n/1000, 1,color, Black);
    n = n % 1000;
    ILI9341_LCD_acFont(24, 0, n/100, 1,color, Black);
    n = n % 100;
    ILI9341_LCD_acFont(48, 0, n/10, 1,color, Black);
    n = n % 10;
    ILI9341_LCD_acFont(72, 0, n, 1,color, Black);
    
    ILI9341_LCD_acFont(96, 0, 12, 1,color, Black);
    ILI9341_LCD_acFont(120, 0, 13, 1,color, Black);
    
}


void ShowTemp(float ftemp)
{
    int32_t n;
    uint32_t color;
    
    color = Green;

    n = (int)(ftemp * 10 + 0.5);
    
    if(n >=1000)
        n = 999;
    ILI9341_LCD_acFont(0, 48, n/100, 1, color, Black);
    n = n % 100;
    ILI9341_LCD_acFont(24, 48, n/10, 1,color, Black);

    ILI9341_LCD_acFont(48, 48, 11, 1,color, Black);

    n = n % 10;
    ILI9341_LCD_acFont(72, 48, n, 1,color, Black);
    
    ILI9341_LCD_acFont(96, 48, 14, 1,color, Black);
}




void ShowFloat(float f)
{
    
    uint32_t color;
    
    color = Cyan;
    
    if(f >=10)
        f = 9.99;
    ILI9341_LCD_acFont(0, 0, (int32_t)f, 1, color, Black);
    f = (f - (int)f) * 10;
    ILI9341_LCD_acFont(24, 0, (int32_t)f, 1,color, Black);
    f = (f - (int)f) * 10;
    ILI9341_LCD_acFont(48, 0, (int32_t)f, 1,color, Black);
    f = (f - (int)f) * 10;
    ILI9341_LCD_acFont(72, 0, (int32_t)f, 1,color, Black);
}



#define FILTER_N        128
float g_aVolt[FILTER_N] = {0};
int32_t g_Idx = 0;


int main(void)
{
    char sbuf[16] = {0};
    int32_t i;
    int32_t hr, min, sec;
    int32_t hr1, min1, sec1;
    float fvolt, fpm25, fvo, fsum,ftemp;
    float fmax;
    uint32_t u32Reg;
    uint32_t u32Temperature, u32pm25;

    /*
        This is a sample code to down count 1 hr.
        Current count will be shown on TFT LCD.
        Press any key button will reset the counter.
        If down count to zero, it will enable buzzer to generate alarm.
    */
    
    
    /* Init System, IP clock and multi-function I/O */
    SYS_Init();
    /* Init UART to 115200-8n1 for print message */
    UART_Open(UART0, 115200);

    /* Configure SPI3 as a master, MSB first, 8-bit transaction, SPI Mode-0 timing, clock is 4MHz */
    SPI_Open(SPI_LCD_PORT, SPI_MASTER, SPI_MODE_0, 8, 24000000);

    /* Configure SPI1 as a low level active device. */
    SPI_EnableAutoSS(SPI_LCD_PORT, SPI_SS, SPI_SS_ACTIVE_LOW);

    /* Start SPI */
    SPI_ENABLE(SPI_LCD_PORT);

    /* Init LCD */
    ILI9341_LCD_Init();

    /* Brightness */
    SysTick_Config(SystemCoreClock / 100000);

    PC->MODE = 1 << 14*2;
    BUZZER(0);

    //PWM_Init();
    /* set PWMB channel 0 output configuration */
    //PWM_ConfigOutputChannel(PWM1, 5, 25000, 50);
    // Start PWM COUNT
    //PWM_Start(PWM1, 1 << 5);
    /* Diable PWM Output path for PWMB channel 0 */
    //PWM_EnableOutput(PWM1, 1 << 5);


    // Set PD2, PD3 as wakeup source
    

    /* Show the String on the screen */
    PB->MODE |= (3 << 6*2);
    PB6 = 1; // SD_PWR

    // Inital wakeup
    PD->INTEN = (1 << 2) | (1 << 3) | (1 << 11) | (1 << 18) | (1 << 19) | (1 << 17);
    PC->INTEN = (1 << 0);
    PA->INTEN = (1 << 8);
    PF->INTEN = (1 << 2);
    
    NVIC_EnableIRQ(GPD_IRQn);
    NVIC_EnableIRQ(GPA_IRQn);
    NVIC_EnableIRQ(GPC_IRQn);
    NVIC_EnableIRQ(GPF_IRQn);

#if 0
    g_u32Ticks = 0;
    while(g_u32Ticks < 300000);
    ILI9341_LED = 0;
    CLK_PowerDown();
    ILI9341_LED = 1;
    while(1);
#endif

    ILI9341_LCD_Fill(0,0, 240, 320, Black);

//    ILI9341_LCD_acFont(0*24, 0, 1, White, Black);
//    ILI9341_LCD_acFont(1*24, 0, 10, White, Black);
//    ILI9341_LCD_acFont(2*24, 0, 7, White, Black);
    
    PWM_Init();
    EADC_Init();
    
    PF->MODE = 0x1;
    
    fmax = -1;
    fvo = 999;
    i = 0;
    while(1)
    {
        /* Waiting for EADC convert */
        PF0 = 0;
        //while((EADC->STATUS2 & EADC_STATUS2_ADIF0_Msk) == 0);
        do{
            u32Reg = EADC->DAT[0];
        }while((u32Reg&EADC_DAT_VALID_Msk) == 0);
        u32pm25 = u32Reg & EADC_DAT_RESULT_Msk;
        
        PF0 = 1;
        /* Clear flag */
        //EADC->STATUS2 = EADC_STATUS2_ADIF0_Msk;
        
        if(g_Idx == 0)
        {
            ShowPm25(fpm25);
            ShowTemp(ftemp);
        }
        
        /* Re-start EADC Sample Module 1 */
        u32Reg = EADC->DAT[1];
        if((u32Reg&EADC_DAT_VALID_Msk))
        {
            u32Temperature = u32Temperature * 28800 + (u32Reg & EADC_DAT_RESULT_Msk)*(32767-28800) >> 15;
            ftemp = 21.0 - ((int)u32Temperature-2070)*31.0/760.0;
            EADC->SWTRG |= (0x2 << EADC_SWTRG_SWTRG_Pos);
        }
        
        fvolt = fvolt *0.98 + (5.0 * u32pm25 / 4096.0) * 0.02;
        if(fvolt < fvo)
        {
            fvo = fvolt;
            if(fvo < 0.03)
                fvo = 0.03;
        }
        
        fpm25 = 0.17 * (fvolt - fvo);
        if(fmax < fpm25)
            fmax = fpm25;
        printf("offset=%f, volt=%f, pm2.5=%f, max=%f, t=%f, \r",fvo, fvolt, fpm25, fmax, ftemp);
        
    }
    
    
    hr = 0;
    min = 0;
    sec = 5;
    hr1 = -1;
    min1 = -1;
    sec1 = -1;
    i = 0;
    while(1)
    {
        if(KEY_EVENT)
        {
            hr = 1;
            min = 0;
            sec = 0;
            
            g_u32Ticks = 100000;
        }

        if(g_u32Ticks >= 100000)
        {
            g_u32Ticks = 0;
            
            //sprintf(sbuf, "%02d", sec);
            //ILI9341_LCD_PutString(0, 0, sbuf, Red, Black);
            //sprintf(sbuf, "%02d:%02d", hr, min);
            //ILI9341_LCD_PutString8x16(0, 0, sbuf, White, Black);
            if(hr1 != hr)
            {
                ILI9341_LCD_acFont(0, 0, hr/10, 1, White, Black);
                ILI9341_LCD_acFont(24, 0, hr%10, 1,White, Black);
                hr1 = hr;
            }
            
            ILI9341_LCD_acFont(48, 0, 10, 1,White, Black);
            
            if(min1 != min)
            {
                ILI9341_LCD_acFont(72, 0, min/10, 1,White, Black);
                ILI9341_LCD_acFont(96, 0, min%10, 1,White, Black);
                min1 = min;
            }
            ILI9341_LCD_acFont(120, 0, 10, 1,White, Black);
            
            if(sec1 != sec)
            {
                ILI9341_LCD_acFont(144, 0, sec/10, 1,White, Black);
                ILI9341_LCD_acFont(168, 0, sec%10, 1,White, Black);
                
                ILI9341_LCD_acFont(0, 96, sec/10, 5,Red, Black);
                ILI9341_LCD_acFont(120, 96, sec%10, 5,Red, Black);
                
                
                sec1 = sec;
            }
            
            sec--;
            if(sec < 0)
            {
                min--;
                if(min < 0)
                {
                    hr--;
                    if(hr < 0)
                    {
                        i = 0;
                        while(1)
                        {
                            //PWM_EnableOutput(PWM1, 1 << 5);
                            BUZZER(1);
                            g_u32Ticks = 0;
                            while(g_u32Ticks < 100* 200)if(KEY_EVENT) break;
                            //PWM_DisableOutput(PWM1, 1 << 5);
                            BUZZER(0);
                            g_u32Ticks = 0;
                            while(g_u32Ticks < 100* 400)if(KEY_EVENT) break;
                            if(KEY_EVENT) break;
                            
                            if(i++> 360)
                            {   
                                /* System Off */
                                ILI9341_LED = 0;
                                CLK_PowerDown();
                                ILI9341_LED = 1;
                            }
                            
                        }
                        
                        hr = 0;
                        min = 59;
                        sec = 59;
                    }
                    else
                    {
                       sec+=60;
                       min+=60;
                    }
                }
                else
                    sec+=60;
            }
        }
    }
    while(1);
}
