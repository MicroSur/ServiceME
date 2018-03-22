#ifndef __MYEVIC_H__
#define __MYEVIC_H__

#include "M451Series.h"
#include "myprintf.h"
//#include "dtmacros.h"

//=========================================================================

#define __myevic__ __attribute__ ((section (".myevic")))

//-------------------------------------------------------------------------

#define CPU_FREQ		72000000
#define PLL_FREQ		(CPU_FREQ<<1)

#define BBC_PWM_FREQ	150000

#define FWVERSION	111

//=========================================================================
// Global flags bitfield
//-------------------------------------------------------------------------

typedef struct
{                                               //0x20000064
/* 00000001 */	int	tick_5khz:1;            //lsls 0x1F
/* 00000002 */	int	tick_1khz:1;            //lsls 0x1E
/* 00000004 */	int	tick_us:1;              //lsls 0x1D
/* 00000008 */	int	tick_100hz:1;           //lsls 0x1C
/* 00000010 */	int	tick_10hz:1;            //lsls 0x1B
/* 00000020 */	int	tick_5hz:1;             //lsls 0x1A
/* 00000040 */	int	tick_2hz:1;             //lsls 0x19
/* 00000080 */	int read_battery:1;             //lsls 0x18

/* 00000100 */	int firing:1;                   //lsls 0x17
/* 00000200 */	int low_battery:1;              //lsls 0x16
/* 00000400 */	int usb_attached:1;             //lsls 0x15
/* 00000800 */	int refresh_battery:1;          //lsls 0x14
/* 00001000 */	int battery_charging:1;         //lsls 0x13
/* 00002000 */	int probing_ato:1;              //lsls 0x12
/* 00004000 */	int user_idle:1;                //lsls 0x11
/* 00008000 */	int sample_vbat:1;              //lsls 0x10

/* 00010000 */	int sample_btemp:1;             //lsls 0x0F
/* 00020000 */	int refresh_display:1;          //lsls 0x0E
/* 00040000 */	int draw_edited_item:1;         //lsls 0x0D
/* 00080000 */	int battery_10pc:1;             //lsls 0x0C
/* 00100000 */	int draw_battery:1;             //lsls 0x0B
/* 00200000 */	int draw_battery_charging:1;    //lsls 0x0A
/* 00400000 */	int decrease_voltage:1;         //lsls 9
/* 00800000 */	int check_mode:1;               //lsls 8

/* 01000000 */	int FireNotFlipped:1;           //lsls 7      bit 24
/* 02000000 */	int MainContrast:1;                 //lsls 6
/* 04000000 */	int limit_ato_temp:1;           //lsls 5
/* 08000000 */	int new_rez_ti:1;               //lsls 4
/* 10000000 */	int new_rez_ni:1;               //lsls 3
/* 20000000 */	int limit_power:1;              //lsls 2
/* 40000000 */	int batteries_ooe:1;            //lsls 1
/* 80000000 */	int new_rez_ss:1;               //lsls 0
                                                //0x20000068
/* 00000001 */	int edit_value:1;
/* 00000002 */	int new_rez_tcr:1;              //lsls 0x1E
/* 00000004 */	int nbcr:1;
/* 00000008 */	int bad_cell:1;
/* 00000010 */	int edit_capture_evt:1;
/* 00000020 */	int osc_1hz:1;
/* 00000040 */	int unused38:1;
/* 00000080 */	int batt_unk:1;

/* 00000100 */	int tick_1hz:1;
/* 00000200 */	int playing_fb:1;
/* 00000400 */	int has_x32:1;
/* 00000800 */	int asleep:1;               //not used yet flag when mod sleep
/* 00001000 */	int sample_atemp:1;         //was debug 
/* 00002000 */	int noclock:1;
/* 00004000 */	int read_bir:1;
/* 00008000 */	int monitoring:1;

/* 00010000 */	int screen_on:1; //bit 48
/* 00020000 */	int pwm_pll:1;
/* 00040000 */	int rtcinit:1;
/* 00080000 */	int light_sleep:1;
/* 00100000 */	int wake_up:1;
/* 00200000 */	int fading:1;
/* 00400000 */	int led_on:1;
/* 00800000 */	int apuff:1;       //auto fire used
/* 01000000 */	int playing_tt:1;
/* 02000000 */	int soft_charge:1;
/* 04000000 */	int inverse:1;
/* 08000000 */	int pbank:1;
/* 10000000 */	int vcom:1;
}

gFlags_t;

extern volatile gFlags_t gFlags;

//=========================================================================
// Functions from assembly language part
//-------------------------------------------------------------------------

extern void MemCpy( void*, const void*, const uint32_t );
extern void MemCpy2( const void*, void*, const uint32_t );
extern void MemClear( void*, const uint32_t );
extern void MemClear2( void*, void*, const uint32_t );
extern void MemSet( void*, const char, const uint32_t );

extern void DrawHexDigit( int x, int y, int v );
extern void DrawHexLong( int x, int y, int v, int font );

//-------------------------------------------------------------------------

extern uint32_t sqrtul( uint32_t v );
extern uint32_t GetFirmwareSize();

//extern const uint8_t  MaxBoardTemp;

//=========================================================================
#endif /* __MYEVIC_H__ */
