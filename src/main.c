#include "myevic.h"
//#include "atomizer.h"
//#include "display.h"
//#include "battery.h"
//#include "screens.h"
//#include "events.h"
//#include "myrtc.h"
#include "timers.h"
#include "dataflash.h"
//#include "meadc.h"
#include "megpio.h"
#include "meusbd.h"
//#include "miscs.h"
//#include "flappy.h"
//#include "tetris.h"


//=========================================================================
// Globals

volatile gFlags_t gFlags;
//uint8_t BoxModel;

//=========================================================================
//----- (0000652C) --------------------------------------------------------
void InitDevices()
{
	SYS_UnlockReg();

	// Internal 22.1184MHz oscillator
	CLK_EnableXtalRC( CLK_PWRCTL_HIRCEN_Msk );
	CLK_WaitClockReady( CLK_STATUS_HIRCSTB_Msk );
        // HCLK clock source: HIRC, HCLK source divider: 1
	CLK_SetHCLK( CLK_CLKSEL0_HCLKSEL_HIRC, CLK_CLKDIV0_HCLK( 1 ) );
	
        // LIRC clock (internal RC 10kHz)
	//	CLK_EnableXtalRC(CLK_PWRCTL_LIRCEN_Msk);
	//	CLK_WaitClockReady(CLK_STATUS_LIRCSTB_Msk);
        
	// 12.000MHz external crystal
	CLK_EnableXtalRC( CLK_PWRCTL_HXTEN_Msk );
	CLK_WaitClockReady( CLK_STATUS_HXTSTB_Msk );

	// FMC Frequency Optimisation mode <= 72MHz
	FMC_EnableFreqOptimizeMode( FMC_FTCTL_OPTIMIZE_72MHZ );

	// Setup PLL to 144MHz and HCLK source to PLL/2
	CLK_SetCoreClock( CPU_FREQ );

	// UART0 CLK = HXT/1
	//#if (ENABLE_UART)
	//CLK_EnableModuleClock( UART0_MODULE );
	//CLK_SetModuleClock( UART0_MODULE, CLK_CLKSEL1_UARTSEL_HXT, CLK_CLKDIV0_UART( 1 ) );
	//#endif

	// USB CLK = PLL/3 (48MHz)
	CLK_EnableModuleClock( USBD_MODULE );
	CLK_SetModuleClock( USBD_MODULE, 0, CLK_CLKDIV0_USB( 3 ) );
        // Enable USB 3.3V LDO
	SYS->USBPHY = SYS_USBPHY_LDO33EN_Msk;

	// WDT CLK = LIRC/1
	CLK_EnableModuleClock( WDT_MODULE );
	CLK_SetModuleClock( WDT_MODULE, CLK_CLKSEL1_WDTSEL_LIRC, 0 );

	// SPI0 CLK = PCLK0/1
        //CLK_SetModuleClock(SPI0_MODULE, CLK_CLKSEL2_SPI0SEL_PCLK0, 0);
	CLK_EnableModuleClock( SPI0_MODULE );

	// EADC CLK = PCLK1/8 (9MHz)
	CLK_EnableModuleClock( EADC_MODULE );
	CLK_SetModuleClock( EADC_MODULE, 0, CLK_CLKDIV0_EADC( 8 ) );

	// CRC CLK = HCLK/1
	CLK_EnableModuleClock( CRC_MODULE );

	// TIMERS CLOCKS
	CLK_EnableModuleClock( TMR0_MODULE );
	CLK_EnableModuleClock( TMR1_MODULE );
	CLK_EnableModuleClock( TMR2_MODULE );
	CLK_EnableModuleClock( TMR3_MODULE );
	CLK_SetModuleClock( TMR0_MODULE, CLK_CLKSEL1_TMR0SEL_HXT, 0 );
	CLK_SetModuleClock( TMR1_MODULE, CLK_CLKSEL1_TMR1SEL_PCLK0, 0 );
	CLK_SetModuleClock( TMR2_MODULE, CLK_CLKSEL1_TMR2SEL_HIRC, 0 );
	CLK_SetModuleClock( TMR3_MODULE, CLK_CLKSEL1_TMR3SEL_HXT, 0 );

	// Enable battery voltage sampling by ADC
	SYS->IVSCTL |= SYS_IVSCTL_VBATUGEN_Msk;

	// ADC reference voltage
	SYS->VREFCTL = SYS_VREFCTL_VREF_2_56V;

	// Brown-out detector; interrupts under 2.2V
	SYS_EnableBOD( SYS_BODCTL_BOD_RST_EN, SYS_BODCTL_BODVL_2_2V );

	// Update clock data
	SystemCoreClockUpdate();

	WDT_Open( WDT_TIMEOUT_2POW18, WDT_RESET_DELAY_18CLK, TRUE, FALSE );

	SYS_LockReg();
}


//=========================================================================
//----- (00000C48) --------------------------------------------------------
__myevic__ void InitHardware()
{
/*
	SYS_UnlockReg();

	//  32.768kHz external crystal
	if ( dfStatus.x32off )
	{
		CLK_DisableXtalRC( CLK_PWRCTL_LXTEN_Msk );
	}
	else
	{
		SYS->GPF_MFPL &= ~(SYS_GPF_MFPL_PF0MFP_Msk|SYS_GPF_MFPL_PF1MFP_Msk);
		SYS->GPF_MFPL |=  (SYS_GPF_MFPL_PF0MFP_X32_OUT|SYS_GPF_MFPL_PF1MFP_X32_IN);

		CLK_EnableXtalRC( CLK_PWRCTL_LXTEN_Msk );
		CLK_WaitClockReady( CLK_STATUS_LXTSTB_Msk );
	}

	//SetPWMClock();

	SYS_LockReg();
*/

	//#if (ENABLE_UART)
	//InitUART0();
	//#endif

	InitGPIO();

	InitTimers();
	InitUSB();
}

//=========================================================================
// BSOD

__myevic__ void Plantouille( int xpsr, int* stack )
{
/*
	int i, k;

	k = 0;

	SYS_UnlockReg();
	WDT_Close();
	SYS_LockReg();

	InitDisplay();

	while ( 1 )
	{
		ClearScreenBuffer();

		DrawImage( 0, 0, 'X'+0x27 );
		DrawHexLong( 16, 0, xpsr, 0 );

		DrawHexDigit( 0, 16, k );

		for ( i = 0; i < 14 ; ++i )
		{
			DrawHexLong( 16, 16+i*8, stack[i+k*14], 0 );
		}

		DisplayRefresh();

		while ( !PE0 || !PD2 || !PD3 )
			CLK_SysTickDelay( 10000 );

		while ( PE0 && PD2 && PD3 )
			CLK_SysTickDelay( 10000 );

		if ( !PE0 )
		{
		  SYS_UnlockReg();
		  SYS_ResetChip();
		  while ( 1 )
			;
		}

		if ( !PD2 ) ++k;
		if ( !PD3 ) --k;

		if ( k < 0 ) k = 0;
		else if ( k > 15 ) k = 15;
	}
 */
}


/*
//=========================================================================
//----- (00005D24) --------------------------------------------------------
__myevic__ void DevicesOnOff( int off )
{
	if ( off )
	{
		TIMER_DisableInt( TIMER0 );
		TIMER_DisableInt( TIMER1 );
		TIMER_DisableInt( TIMER2 );

		if ( !gFlags.light_sleep )
		{
			TIMER_DisableInt( TIMER3 );
		}

		EADC_Close( EADC );
                
		SetADCState( 1, 0 );
		SetADCState( 2, 0 );
                
                if ( ISRX300 )
                    SetADCState( 17, 0 );
                else
                    SetADCState( 14, 0 ); //ISSINP80

		if ( ISVTCDUAL || ISCUBOID || ISCUBO200 || ISRX200S || ISRX23 || ISRX300 || ISPRIMO1 || 
                        ISPRIMO2 || ISPREDATOR || ISGEN3 || ISINVOKE || ISRX2 || ISSINFJ200 || ISRX217 )
		{

                        SetADCState( 3, 0 );
                        SetADCState( 13, 0 );
                    
			if ( ISCUBO200 || ISRX200S || ISRX23 || ISRX300 )
			{
				SetADCState( 15, 0 );
			}

                        if ( ISSINFJ200 )
                        {
                                SetADCState( 4, 0 );
                        }
                        
			PD7 = 0;                                                // 0x400048DC
			BBC_Configure( BBC_PWMCH_CHARGER, 0 );                  // 5 0
			PD7 = 0;

			if ( ISCUBOID || ISCUBO200 || ISRX200S || ISRX23 )
			{
				PF2 = 0;                                        // 0x40004948
			}
		}

		PC1 = 0;                                                        // 40004884
		PC0 = 0;                                                        // 40004880
		BBC_Configure( BBC_PWMCH_BUCK, 0 );                             // 0 0
                
		if ( !ISVTCDUAL ) 
                    PC3 = 0;                                                    // 4000488C
                
                if ( !ISINVOKE )
                    PC2 = 0;                                                        // 40004888
                
		BBC_Configure( BBC_PWMCH_BOOST, 0 );                            // 2 0
                
		if ( ISCUBO200 || ISRX200S || ISRX23 )
		{
			PF1 = 0;                                                // 40004944
		}
		else if ( ISRX300 )
		{
			PD1 = 0;                                                // 400048C4
		}
                else if ( ISSINFJ200 )
                {
                    	PA1 = 0;                                                // 0x400048DC
			BBC_Configure( 4, 0 );                                  // 4 0
			PA1 = 0;
                        PA2 = 0;
                }                
                else if ( ISRX2 || ISRX217 )
		{
                        PA3 = 0;
                }
		else if ( !ISSINP80 )
		{
			PB7 = 0;                                                // 4000485C
		}

                if ( !ISRX2 && !ISSINP80 && !ISINVOKE && !ISRX217 )
		{
                    GPIO_DisableInt( PD, 0 );
                    PD0 = 0;                                                        // 400048C0
                    GPIO_SetMode( PD, GPIO_PIN_PIN0_Msk, GPIO_MODE_OUTPUT );        // 400040C0 1 1
                }
                    
		if ( ISRX300 || ISPRIMO1 || ISPRIMO2 || ISPREDATOR || ISGEN3 || ISRX2 
                        || ISINVOKE || ISSINFJ200 || ISRX217 )
		{
                    if ( ISRX300 || ISRX2 || ISSINFJ200 || ISRX217 )
                    {
			PF5 = 0;                                                // 40004954
			PF6 = 0;                                                // 40004958
                    }   
                    else if ( ISGEN3 )
                    {
                        PF5 = 0;
                    }
                    
                    if ( !ISRX2 && !ISSINFJ200 && !ISRX217 )
                    {
			PA3 = 0;                                                // 4000480C
			PA2 = 0;                                                // 40004808
                    }
		}
                
		if ( ISVTCDUAL )
		{
			GPIO_DisableInt( PD, 1 );
			PD1 = 0;                                                // 400048C4
			GPIO_SetMode( PD, GPIO_PIN_PIN1_Msk, GPIO_MODE_OUTPUT );
		}
		else if ( !ISCUBOID && !ISCUBO200 && !ISRX200S && !ISRX23 && !ISRX300 
                        && !ISPRIMO1 && !ISPRIMO2 && !ISPREDATOR && !ISGEN3 && !ISRX2 
                        && !ISINVOKE && !ISSINFJ200 && !ISRX217 )
		{
			GPIO_DisableInt( PD, 7 ); //ISSINP80
			PD7 = 0;                                                // 400048DC
			GPIO_SetMode( PD, GPIO_PIN_PIN7_Msk, GPIO_MODE_OUTPUT );
		}

		SYS->GPE_MFPH &= ~(SYS_GPE_MFPH_PE11MFP_Msk|SYS_GPE_MFPH_PE12MFP_Msk|SYS_GPE_MFPH_PE13MFP_Msk);
		SYS->GPE_MFPH |= (SYS_GPE_MFPH_PE11MFP_GPIO|SYS_GPE_MFPH_PE12MFP_GPIO|SYS_GPE_MFPH_PE13MFP_GPIO);
                
		PE11 = 0;                                                       // 4000492C
		GPIO_SetMode( PE, GPIO_PIN_PIN11_Msk, GPIO_MODE_OUTPUT );
		PE12 = 0;                                                       // 40004930
		GPIO_SetMode( PE, GPIO_PIN_PIN12_Msk, GPIO_MODE_OUTPUT );
		PE13 = 0;                                                       // 40004934
		GPIO_SetMode( PE, GPIO_PIN_PIN13_Msk, GPIO_MODE_OUTPUT );
		PE10 = 0;                                                       // 40004928

		GPIO_EnableInt( PE, 0, GPIO_INT_BOTH_EDGE );                    // 4100
		GPIO_EnableInt( PD, 2, GPIO_INT_BOTH_EDGE );                    // 40C0
		GPIO_EnableInt( PD, 3, GPIO_INT_BOTH_EDGE );                    // 40C0

		if ( ISVTCDUAL )
		{
			PA3 = 0;                                                // 480C
			PC3 = 0;                                                // 488C
			PF2 = 0;                                                // 4948
			PA2 = 0;                                                // 4808
		}
		else if ( ISCUBOID || ISCUBO200 || ISRX200S || ISRX23 || ISRX300 )
		{
			PF0 = 0;                                                // 4940
		}
                else if ( ISPRIMO1 || ISPRIMO2 || ISPREDATOR || ISINVOKE )
                {
                        PD1 = 0;                                                // 48C4
                }
                else if ( ISGEN3 )
                {
                        PF0 = 0;                                                // 4940
                        PD1 = 0;                                                // 48C4
                }
                else if ( ISRX2 || ISRX217 )
                {
                        PF2 = 0;                                                // 4948
                }
                
		SYS_UnlockReg();
		SYS->USBPHY &= ~SYS_USBPHY_LDO33EN_Msk;
		SYS->IVSCTL &= ~(SYS_IVSCTL_VBATUGEN_Msk|SYS_IVSCTL_VTEMPEN_Msk);
		SYS_DisableBOD();
		SYS->VREFCTL = 0;
		SYS_LockReg();

		USBD_CLR_INT_FLAG( USBD_INTSTS_WAKEUP|USBD_INTSTS_FLDET|USBD_INTSTS_BUS|USBD_INTSTS_USB );
		USBD_ENABLE_INT( USBD_INT_WAKEUP );
	}
	else //Device On
	{
		USBD_CLR_INT_FLAG( USBD_INTSTS_WAKEUP );

		SYS_UnlockReg();
		SYS->USBPHY |= SYS_USBPHY_LDO33EN_Msk;
		SYS->IVSCTL |= SYS_IVSCTL_VBATUGEN_Msk;
                               
		if ( ISRX300 )
		{
			SYS->IVSCTL |= SYS_IVSCTL_VTEMPEN_Msk;
		}
                
		SYS->VREFCTL = SYS_VREFCTL_VREF_2_56V;
		SYS_EnableBOD( SYS_BODCTL_BOD_RST_EN, SYS_BODCTL_BODVL_2_2V );
		SYS_LockReg();

		GPIO_DisableInt( PE, 0 );                                       // 4100
		GPIO_DisableInt( PD, 2 );                                       // 40C0
		GPIO_DisableInt( PD, 3 );                                       // 40C0

		if ( ISCUBOID || ISCUBO200 || ISRX200S || ISRX23 )
		{
			PF2 = 1;                                                // 4948
		}

		SYS->GPE_MFPH &= ~(SYS_GPE_MFPH_PE11MFP_Msk|SYS_GPE_MFPH_PE12MFP_Msk|SYS_GPE_MFPH_PE13MFP_Msk);
		SYS->GPE_MFPH |= (SYS_GPE_MFPH_PE11MFP_SPI0_MOSI0|SYS_GPE_MFPH_PE12MFP_SPI0_SS|SYS_GPE_MFPH_PE13MFP_SPI0_CLK);

                if ( !ISRX2 && !ISSINP80 && !ISINVOKE && !ISRX217 )
                {
                        GPIO_SetMode( PD, GPIO_PIN_PIN0_Msk, GPIO_MODE_INPUT );         // 40C0 1 0
                        GPIO_EnableInt( PD, 0, GPIO_INT_FALLING );                      // 40C0 0 1
                }
                
		if ( ISVTCDUAL )
		{
			GPIO_SetMode( PD, GPIO_PIN_PIN1_Msk, GPIO_MODE_INPUT ); // 40C0
			GPIO_EnableInt( PD, 1, GPIO_INT_RISING );
			GPIO_ENABLE_DEBOUNCE( PD, GPIO_PIN_PIN1_Msk );
		}
		else if ( !ISCUBOID && !ISCUBO200 && !ISRX200S && !ISRX23 && !ISRX300 
                        && !ISPRIMO1 && !ISPRIMO2 && !ISPREDATOR && !ISGEN3 && !ISRX2 
                        && !ISINVOKE && !ISSINFJ200 && !ISRX217 )
		{
			GPIO_SetMode( PD, GPIO_PIN_PIN7_Msk, GPIO_MODE_INPUT ); // 40C0 0x80 0     ISSINP80
			GPIO_EnableInt( PD, 7, GPIO_INT_RISING );               // 40C0 7 0x10000
			GPIO_ENABLE_DEBOUNCE( PD, GPIO_PIN_PIN7_Msk );
		}

		if ( ISCUBO200 || ISRX200S || ISRX23 )
		{
			PF1 = 1;                                                // 4944
		}
		else if ( ISRX300 )
		{
			PD1 = 1;                                                // 48C4
		}
		else if ( ISRX2 || ISRX217 )
		{
			PA3 = 1;                                                // 48C4
		}
                else if ( ISSINFJ200 )
                {
                        PA2 = 1;
                }
		else if ( !ISSINP80 )
		{
			PB7 = 1;                                                // 485C
		}

		SetADCState( 1, 1 );
		SetADCState( 2, 1 );
                
                if ( ISRX300 )
                    SetADCState( 17, 1 );
                else
                    SetADCState( 14, 1 );

		if ( ISVTCDUAL || ISCUBOID || ISCUBO200 || ISRX200S || ISRX23 || ISRX300 
                        || ISPRIMO1 || ISPRIMO2 || ISPREDATOR || ISGEN3 || ISRX2 || ISINVOKE 
                        || ISSINFJ200 || ISRX217 )
		{
			SetADCState( 3, 1 );
			SetADCState( 13, 1 );

			if ( ISCUBO200 || ISRX200S || ISRX23 || ISRX300 )
			{
				SetADCState( 15, 1 );
			}
                        else if ( ISSINFJ200 )
                        {
                                SetADCState( 4, 1 );
                        }
		}

		TIMER_EnableInt( TIMER0 );
		TIMER_EnableInt( TIMER1 );
		TIMER_EnableInt( TIMER2 );
		TIMER_EnableInt( TIMER3 );
	}
}
*/





/*
//=========================================================================
//----- (00005D14) --------------------------------------------------------
__myevic__ void FlushAndSleep()
{
	//#if (ENABLE_UART)
	//	UART_WAIT_TX_EMPTY( UART0 );
	//#endif

	if ( !gFlags.light_sleep )
	{
		CLK_PowerDown();
	}
	else
	{
		LightSleep();
	}
}
*/


//=========================================================================
//----- (00004F0C) --------------------------------------------------------

/*
void GoToSleep()
{
	gFlags.light_sleep = !( gFlags.has_x32 || dfStatus.lsloff || gFlags.noclock );

	ScreenOff();
	LEDOff();
	gFlags.firing = 0;
	BatReadTimer = 50;
	RTCSleep();
	DevicesOnOff( 1 ); //off
	CLK_SysTickDelay( 250 );
	CLK_SysTickDelay( 250 );
	CLK_SysTickDelay( 250 );
	if ( dfStatus.off || PE0 || KeyPressTime == 1100 )
	{
		SYS_UnlockReg();
		WDT_Close();
		FlushAndSleep();
		PreheatDelay = 0;
                CurveDelay = 0;
	}       
	WDT_Open( WDT_TIMEOUT_2POW14, WDT_RESET_DELAY_18CLK, TRUE, FALSE );
	SYS_LockReg();
	gFlags.refresh_battery = 1;
        
        if ( dfStatus.invert ) gFlags.inverse = 1;
                                        
	DevicesOnOff( 0 ); //on
	RTCWakeUp();
	InitDisplay();
       
        //if ( !dfStatus.off ) gFlags.asleep = 1;
}
*/


//=========================================================================
//----- (0000782C) --------------------------------------------------------
/*
__myevic__ void SleepIfIdle()
{
	if ( !gFlags.firing && !NoEventTimer )
	{
		if ( ( Screen == 0 ) && ( SleepTimer == 0 ) && ( gFlags.user_idle ) )
		{                  
			GoToSleep();

			Set_NewRez_dfRez = 2;
			AtoProbeCount = 0;
			AtoRezMilli = 0;
                        PuffsOffCount = 0;
                        NextPreheatTimer = 0;
                        AutoPuffTimer = 0;
                        gFlags.apuff = 0;
			gFlags.sample_vbat = 1;
			ReadBatteryVoltage();
                        
			if ( dfDimOffMode == 1 || ( ( BatteryVoltage <= BatteryCutOff + 20 ) && !gFlags.usb_attached ) )
			{
				dfStatus.off = 1;
				//Screen = 0;
                                //LEDOff(); ?
                                
			}
                        else if ( dfDimOffMode == 2 && !dfStatus.off )
                        {
                                dfStatus.keylock = 1;
                        }
                        
			gFlags.sample_btemp = 1;
                        
                        if ( ISSINFJ200 )
                            gFlags.sample_atemp = 1;
		}
		NoEventTimer = 200;
	}
}
*/


//=========================================================================
// Monitoring
//-------------------------------------------------------------------------
/*
__myevic__ void Monitor()
{
	if ( gFlags.firing )
	{
		myprintf( "FIRING "
					"RESM=%d BATT=%d VOUT=%d CUR=%d",
					AtoRezMilli,
					RTBattVolts,
					AtoVolts,
					AtoCurrent
				);

		if ( ISMODETC(dfMode) )
		{
			myprintf( " SPWR=%d RPWR=%d CELS=%d STEMP=%d RTEMP=%d\n",
						dfTCPower,
						AtoPower( AtoVolts ),
						dfIsCelsius ? 1 : 0,
						dfTemp,
						dfIsCelsius ? FarenheitToC( AtoTemp ) : AtoTemp
					);
		}
		else if ( ISMODEBY(dfMode) )
		{
			myprintf( " RPWR=%d\n",
						AtoPower( AtoVolts )
					);
		}
		else
		{
			myprintf( " SPWR=%d RPWR=%d\n",
						dfPower,
						AtoPower( AtoVolts )
					);
		}
	}
	else
	{
		myprintf( "STANDBY "
					"BATT=%d CHG=%d BRD=%d ATO=%d "
					"RES=%d RESM=%d MODE=%d",
					BatteryVoltage,
					gFlags.battery_charging ? 1 : 0,
					dfIsCelsius ? BoardTemp : CelsiusToF( BoardTemp ),
					AtoStatus,
					AtoRez,
					AtoRezMilli,
					dfMode
				);

		if ( ISMODETC(dfMode) )
		{
			ReadAtoTemp();
			myprintf( " SPWR=%d CELS=%d STEMP=%d RTEMP=%d\n",
						dfTCPower,
						dfIsCelsius ? 1 : 0,
						dfTemp,
						dfIsCelsius ? FarenheitToC( AtoTemp ) : AtoTemp
					);
		}
		else if ( ISMODEBY(dfMode) )
		{
			myprintf( "\n" );
		}
		else
		{
			myprintf( " SPWR=%d\n",
						dfPower
					);
		}
	}
}
*/

//=========================================================================
//----- (00000148) --------------------------------------------------------
__myevic__ void Main()
{
        //Init
	InitDevices();

	InitDataFlash();
        
        dfStatus.vcom = 1; //before init usb
	InitHardware();

	myprintf( "\n\nJoyetech APROM\n" ); // need for identify FW file
	myprintf( "CPU @ %dHz(PLL@ %dHz)\n", SystemCoreClock, PllClock );

	if ( !PD3 )
	{
		//DrawScreen();
		while ( !PD3 )
			;
	}
    
	while ( 1 )
	{            

		if ( gFlags.tick_1khz )
		{
			// 1000Hz
			gFlags.tick_1khz = 0;

			if ( dfStatus.vcom )
			{
				VCOM_Poll();
			}
		}

		if ( gFlags.tick_100hz )
		{
			// 100Hz
			gFlags.tick_100hz = 0;
            
			ResetWatchDog();

			//GetUserInput();
		}

		if ( gFlags.tick_10hz )
		{
			// 10Hz
			gFlags.tick_10hz = 0;


		}

		if ( gFlags.tick_5hz )
		{
			// 5Hz
			gFlags.tick_5hz = 0;

               
		}

		if ( gFlags.tick_2hz )
		{
			// 2Hz
			gFlags.tick_2hz = 0;
                        
			gFlags.osc_1hz ^= 1;

		}
                
		if ( gFlags.tick_1hz )
		{
			// 1Hz
			gFlags.tick_1hz = 0;
                        

		}

		//EventHandler();

	}
}

