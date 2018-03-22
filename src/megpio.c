#include "myevic.h"
#include "dataflash.h"
//#include "events.h"
//#include "atomizer.h"
//#include "battery.h"


//=========================================================================
//----- (000022EC) --------------------------------------------------------
__myevic__ void GPD_IRQHandler()
{
	if ( GPIO_GET_INT_FLAG( PD, GPIO_PIN_PIN7_Msk ) )
	{
		GPIO_CLR_INT_FLAG( PD, GPIO_PIN_PIN7_Msk );
// usb detach?
	}
	else if ( GPIO_GET_INT_FLAG( PD, GPIO_PIN_PIN1_Msk ) )
	{
		GPIO_CLR_INT_FLAG( PD, GPIO_PIN_PIN1_Msk );

	}
	else if ( GPIO_GET_INT_FLAG( PD, GPIO_PIN_PIN0_Msk ) )
	{
		GPIO_CLR_INT_FLAG( PD, GPIO_PIN_PIN0_Msk );

	}
	else if ( GPIO_GET_INT_FLAG( PD, GPIO_PIN_PIN2_Msk|GPIO_PIN_PIN3_Msk ) )
	{
		GPIO_CLR_INT_FLAG( PD, GPIO_PIN_PIN2_Msk|GPIO_PIN_PIN3_Msk );
// +- buttons press
//gFlags.wake_up = 1;
	}
	else
	{
		PD->INTSRC = PD->INTSRC;
	}
}

//----- (00002334) --------------------------------------------------------
__myevic__ void GPE_IRQHandler()
{
	if ( GPIO_GET_INT_FLAG( PE, GPIO_PIN_PIN0_Msk ) )
	{
		GPIO_CLR_INT_FLAG( PE, GPIO_PIN_PIN0_Msk );
//fire press
		//gFlags.wake_up = 1;
	}
	else
	{
		PE->INTSRC = PE->INTSRC;
	}
}

//----- (00002342) --------------------------------------------------------
__myevic__ void GPF_IRQHandler()
{
	PF->INTSRC = PF->INTSRC;
}


//=========================================================================
//----- (00002384) --------------------------------------------------------
__myevic__ void InitGPIO()
{
       
	// BUTTONS
	GPIO_SetMode( PE, GPIO_PIN_PIN0_Msk, GPIO_MODE_INPUT );     // 0x40004100 1 0
	GPIO_SetMode( PD, GPIO_PIN_PIN2_Msk, GPIO_MODE_INPUT );     // 0x400040C0 4 0
	GPIO_SetMode( PD, GPIO_PIN_PIN3_Msk, GPIO_MODE_INPUT );     // 0x400040C0 8 0


	// BUCK/BOOST CONVERTER CONTROL LINES
        PC1 = 0;                                                                // 0x40004884
//        GPIO_SetMode( PC, GPIO_PIN_PIN1_Msk, GPIO_MODE_OUTPUT );                // 0x40004080 2 1

	// SSD RESET/VDD/VCC
	PA0 = 0;                                                                // 0x40004800
	GPIO_SetMode( PA, GPIO_PIN_PIN0_Msk, GPIO_MODE_OUTPUT );                // 0x40004000 1 1
        
	PC4 = 0;                                                                // 0x40004890
	GPIO_SetMode( PC, GPIO_PIN_PIN4_Msk, GPIO_MODE_OUTPUT );                // 0x40004080 0x10 1

	// SPI0 (Display control)
	PE10 = 0;                                                               // 0x40004928
//	GPIO_SetMode( PE, GPIO_PIN_PIN10_Msk, GPIO_MODE_OUTPUT );               // 0x40004100 (0x400 = 0x40004928 ASR 0x14) 1       
//	GPIO_SetMode( PE, GPIO_PIN_PIN12_Msk, GPIO_MODE_OUTPUT );               // 0x40004100 (0x1000 = 0x40004930 ASR 0x12) 1

	NVIC_EnableIRQ( GPD_IRQn );                                         // 0x13
	NVIC_EnableIRQ( GPE_IRQn );                                         // 0x14
	NVIC_EnableIRQ( GPF_IRQn );                                         // 0x15

	// Debounce time = 100ms
	GPIO_SET_DEBOUNCE_TIME( GPIO_DBCTL_DBCLKSRC_LIRC, GPIO_DBCTL_DBCLKSEL_1024 );       // 0x3A 0x40004440
}

