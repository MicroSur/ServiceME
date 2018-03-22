#include "myevic.h"
//#include "myrtc.h"
//#include "screens.h"
//#include "atomizer.h"
//#include "display.h"
//#include "events.h"
//#include "battery.h"
//#include "atomizer.h"
//#include "miscs.h"

#include "dataflash.h"

//=========================================================================
// DataFlash parameters global structure

dfStruct_t DataFlash;

uint8_t ParamsBackup[DATAFLASH_PARAMS_SIZE];


//-------------------------------------------------------------------------
// Global variables

uint8_t		UpdateDFTimer;
//uint8_t		UpdatePTTimer;
uint8_t		DFMagicNumber;
//uint8_t		X32Off;
//uint8_t		ScrFlip;
//char            *BoxName;


//-------------------------------------------------------------------------
// Internal variables

uint16_t	fmcCntrsIndex;

//-------------------------------------------------------------------------

#define __PIDATTR__ \
	__attribute__((aligned(4))) \
	__attribute__((section (".productid")))

//=========================================================================
// Reset device to LDROM
//-------------------------------------------------------------------------
__myevic__ void ResetToLDROM()
{
	dfBootFlag = 1;
	UpdateDataFlash();

	SYS_UnlockReg();

	FMC_SELECT_NEXT_BOOT( 1 );
	SCB->AIRCR = 0x05FA0004;

	while ( 1 )
		;
}


//=========================================================================
//----- (000018D0) --------------------------------------------------------
__myevic__ int FMCCheckConfig( unsigned long cfg[] )
{
	if ( cfg[0] & 1 || cfg[1] != DATAFLASH_PARAMS_BASE )
	{
		FMC_EnableConfigUpdate();
		FMC_Erase( FMC_CONFIG_BASE );

		cfg[0] &= ~1;
		cfg[1] = DATAFLASH_PARAMS_BASE;

		if ( FMC_WriteConfig( cfg, 2 ) < 0 )
			return 0;

		FMC_ReadConfig( cfg, 2 );

		if ( cfg[0] & 1 || cfg[1] != DATAFLASH_PARAMS_BASE )
			return 0;

		SYS_ResetChip();
	}
	return 1;
}


//=========================================================================
//----- (00001926) --------------------------------------------------------
__myevic__ void FMCRead256( uint32_t u32Addr, uint32_t *pu32Buf )
{
	for ( uint32_t offset = 0 ; offset < 0x100 ; offset += 4 )
	{
		*pu32Buf = FMC_Read( u32Addr + offset );
		++pu32Buf;
	}
}


//=========================================================================
//----- (00001CEC) --------------------------------------------------------
__myevic__ uint32_t ReadDataFlash( uint32_t u32Addr, uint32_t *pu32Buf )
{
	uint32_t offset;

	for ( offset = 0 ; offset < DATAFLASH_PARAMS_SPACE ; offset += DATAFLASH_PARAMS_SIZE )
	{
		if ( FMC_Read( u32Addr + offset ) == ~0 && FMC_Read( u32Addr + offset + 4 ) == ~0 )
		{
			break;
		}
		offset += DATAFLASH_PARAMS_SIZE;
	}

	if ( offset )
	{
		u32Addr += offset - DATAFLASH_PARAMS_SIZE;
	}

	for ( offset = 0 ; offset < DATAFLASH_PARAMS_SIZE ; offset += 0x100 )
	{
		FMCRead256( u32Addr + offset, pu32Buf + offset / 4 );
	}

	return u32Addr;
}


//=========================================================================
//----- (0000119C) --------------------------------------------------------
__myevic__ uint32_t CalcPageCRC( uint32_t *pu32Addr )
{
	uint32_t idx;
	uint16_t *addr;
	uint32_t crc;

	CRC_Open( CRC_CCITT, 0, 0xFFFF, CRC_CPU_WDATA_16 );

	idx = 0;
	addr = (uint16_t*)(pu32Addr+1);

	do
	{
		CRC_WRITE_DATA( addr[idx] );
	}
	while ( ++idx < ( DATAFLASH_PARAMS_SIZE - 4 ) / 2 );

	crc = CRC_GetChecksum();

	CRC->CTL &= ~1;

	return crc;
}


//=========================================================================
//----- (00001FD0) --------------------------------------------------------
// Writes 256 bytes from address from pu32Data to first free page
// in DF after u32Addr
__myevic__ void WriteDataFlash( uint32_t u32Addr, const uint32_t *pu32Data )
{
	uint32_t offset;

	for ( offset = 0 ; offset < DATAFLASH_PARAMS_SPACE ; offset += DATAFLASH_PARAMS_SIZE )
	{
		if ( FMC_Read( u32Addr + offset ) == ~0 && FMC_Read( u32Addr + offset + 4 ) == ~0 )
		{
			break;
		}
	}

	if ( offset >= DATAFLASH_PARAMS_SPACE )
	{
		offset = 0;
		FMC_Erase( u32Addr );
	}
	else if ( offset < DATAFLASH_PARAMS_SPACE - FMC_FLASH_PAGE_SIZE )
	{
		if ( offset % FMC_FLASH_PAGE_SIZE == DATAFLASH_PARAMS_SIZE )
		{
			FMC_Erase( u32Addr + offset - DATAFLASH_PARAMS_SIZE + FMC_FLASH_PAGE_SIZE );
		}
	}

	u32Addr += offset;

	for ( offset = 0 ; offset < DATAFLASH_PARAMS_SIZE ; offset += 4 )
	{
		FMC_Write( u32Addr + offset, pu32Data[ offset / 4 ] );
	}
}


//=========================================================================
//----- (00001D30) --------------------------------------------------------
__myevic__ void UpdateDataFlash()
{
	uint8_t *df;
	uint32_t idx;

	//UpdateDFTimer = 0;

	df = (uint8_t*)&DataFlash.params;

	for ( idx = 0 ; idx < DATAFLASH_PARAMS_SIZE ; ++idx )
	{
		if ( df[idx] != ParamsBackup[idx] )
			break;
	}

	if ( idx != DATAFLASH_PARAMS_SIZE )
	{
		dfCRC = CalcPageCRC( DataFlash.params );
		MemCpy( ParamsBackup, DataFlash.params, DATAFLASH_PARAMS_SIZE );
		SYS_UnlockReg();
		FMC_ENABLE_ISP();
		WriteDataFlash( DATAFLASH_PARAMS_BASE, DataFlash.params );
		FMC_DISABLE_ISP();
		SYS_LockReg();
	}
}


//=========================================================================
//----- (00001940) --------------------------------------------------------
__myevic__ void InitDataFlash()
{
	int i;
	unsigned long cfg[2];
	unsigned long hwv;
	unsigned long addr;

	SYS_UnlockReg();
	FMC_ENABLE_ISP();

	MemClear( &DataFlash, sizeof( DataFlash ) );

	dffmcCID = FMC_ReadCID();

	FMC->ISPCMD = FMC_ISPCMD_READ_DID;
	FMC->ISPADDR = 0;
	FMC->ISPTRG = FMC_ISPTRG_ISPGO_Msk;
	while( FMC->ISPTRG & FMC_ISPTRG_ISPGO_Msk )
		;
	dffmcDID = FMC->ISPDAT;

	dffmcPID = FMC_ReadPID();

	for ( i = 0 ; i < 3 ; ++i )
	{
		dffmcUID[i] = FMC_ReadUID( i );
	}

	for ( i = 0 ; i < 4 ; ++i )
	{
		dffmcUCID[i] = FMC_ReadUCID( i );
	}

	FMC_ReadConfig( cfg, 2 );

	addr = DATAFLASH_PARAMS_BASE;

	if ( FMCCheckConfig( cfg ) )
	{
		addr = ReadDataFlash( addr, DataFlash.params );
	}

	if ( CalcPageCRC( DataFlash.params ) != dfCRC )
	{
		if ( addr == DATAFLASH_PARAMS_BASE )
		{
			addr = DATAFLASH_PARAMS_END - DATAFLASH_PARAMS_SIZE;
		}
		else
		{
			addr -= DATAFLASH_PARAMS_SIZE;
		}

		hwv = dfHWVersion;

		for ( i = 0 ; i < DATAFLASH_PARAMS_SIZE ; i += 0x100 )
		{
			FMCRead256( addr + i, &DataFlash.params[ i / 4 ] );
		}

		if ( CalcPageCRC( DataFlash.params ) == dfCRC )
		{
			UpdateDataFlash();
		}
		else
		{
			dfHWVersion = hwv;
		}
	}

	FMC_DISABLE_ISP();
	SYS_LockReg();

        DFMagicNumber = 0xDA; //SME
        dfProductID = 0x56524553;
        
        //dfBuild = __BUILD3;
        dfFWVersion	= FWVERSION;


/*
	myprintf( "  APROM Version ......................... [%d.%d%d]\n",
				FWVERSION / 100,
				FWVERSION / 10 % 10,
				FWVERSION % 10 );
	myprintf( "  Hardware Version ...................... [%d.%d%d]\n",
				dfHWVersion / 100,
				dfHWVersion / 10 % 10,
				dfHWVersion % 10 );
*/

        
/*
	if ( ( dfMagic == DFMagicNumber ) && ( CalcPageCRC( DataFlash.params ) == dfCRC ) )
	{
                myprintf( "Data Flash Check Values Validity\n" );
		//DFCheckValuesValidity();
	}
	else
	{
		myprintf( "Data Flash Re-Initialization\n" );
		//ResetDataFlash();
	}
*/


	MemCpy( ParamsBackup, DataFlash.params, DATAFLASH_PARAMS_SIZE );

	if ( dfBootFlag )
	{
		dfBootFlag = 0;
                UpdateDataFlash();
	}
}


//=========================================================================
//----- (0000169C) --------------------------------------------------------
// Writes 2kB from RAM R1 to DF R0
__myevic__ void FMCWritePage( uint32_t u32Addr, uint32_t *pu32Data )
{
	for ( uint32_t idx = 0 ; idx < FMC_FLASH_PAGE_SIZE / 4 ; ++idx )
	{
		FMC_Write( u32Addr + 4 * idx, pu32Data[ idx ] );
	}
}

//=========================================================================
//----- (000016D0) --------------------------------------------------------
// Compares 2kB (0x800) DF @R0 with RAM @R1
__myevic__ uint32_t FMCVerifyPage( uint32_t u32Addr, uint32_t *pu32Data )
{
	for ( uint32_t idx = 0 ; idx < FMC_FLASH_PAGE_SIZE / 4 ; ++idx )
	{
		if ( FMC_Read( u32Addr + 4 * idx ) != pu32Data[ idx ] )
		{
			return idx + 1;
		}
	}
	return 0;
}


//=========================================================================
//----- (0000170C) --------------------------------------------------------
// Erase & writes 2kB from RAM R1 to DF R0
__myevic__ int FMCEraseWritePage( uint32_t u32Addr, uint32_t *src )
{
	if ( FMC_Erase( u32Addr ) == -1 )
	{
		return 1;
	}
	else
	{
		FMCWritePage( u32Addr, src );
		return 0;
	}
}

