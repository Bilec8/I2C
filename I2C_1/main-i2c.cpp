#include <mbed.h>

#include "i2c-lib.h"
#include "si4735-lib.h"

//************************************************************************

// Direction of I2C communication
#define R	0b00000001
#define W	0b00000000

#pragma GCC diagnostic ignored "-Wunused-but-set-variable"

DigitalOut g_led_PTA1( PTA1, 0 );
DigitalOut g_led_PTA2( PTA2, 0 );

DigitalIn g_but_PTC9( PTC9 );
DigitalIn g_but_PTC10( PTC10 );
DigitalIn g_but_PTC11( PTC11 );
DigitalIn g_but_PTC12( PTC12 );


uint8_t i2c_out_in( uint8_t t_adr,
                   uint8_t *t_out_data, uint32_t t_out_len,
                   uint8_t *t_in_data, uint32_t t_in_len )
{
  i2c_start();

  uint8_t l_ack = i2c_output( t_adr | W );

  if ( l_ack == 0 )
  {
    for ( uint32_t i=0; i<t_out_len;i++ )
    {
      l_ack |= i2c_output( t_out_data[i] ); // send all t_out_data
    }
  }

  if ( l_ack != 0 ) // error?
  {
    i2c_stop();
    return l_ack;
  }

  if ( t_in_data != nullptr )
  {
    i2c_start(); // repeated start

    l_ack |= i2c_output( t_adr | R );

    for (uint32_t i = 1; i < t_in_len; i ++)
    {
      i2c_ack();
      t_in_data[i] = i2c_input(); // receive all t_data_in
    }

    i2c_nack();
  }

  i2c_stop();

  return l_ack;
}

class Radio{
public:
uint8_t set_freq( uint16_t t_freq )
{
  uint8_t l_data_out[ 5 ] = { 0x20, 0x00, t_freq >> 8, t_freq & 0xFF, 0 };
  return i2c_out_in( SI4735_ADDRESS, l_data_out, 5, nullptr, 0 );
}

uint8_t get_tune_status( uint8_t *t_data_status, uint32_t t_data_len )
{
  uint8_t l_data_out[ 2 ] = { 0x22, 0 };
  return i2c_out_in( SI4735_ADDRESS, l_data_out, 2, t_data_status, t_data_len );
}

uint8_t signal_quality(uint8_t *t_data_status, uint32_t t_data_len){
	uint8_t l_data_out[2] = {0x23, 0x00};
	return i2c_out_in(SI4735_ADDRESS, l_data_out, 2, t_data_status,t_data_len);
}

uint8_t set_volume(uint16_t volume){
	if(volume > 63){
		volume = 63;
	}
	uint8_t l_data_out[6] = {0x12, 0x00, 0x40, 0x00, 0x00, volume};
	return i2c_out_in(SI4735_ADDRESS, l_data_out,6, nullptr, 0);
	}

uint8_t search_freq(){
	  uint8_t l_data_out[ 2 ] = { 0x21, 0b00000100};
	  return i2c_out_in( SI4735_ADDRESS, l_data_out, 2, nullptr, 0 );
	}
};

class Expander{
public:
	//int address = 73 % 8;
	void bar(uint8_t t_level){
		int leds = 0;
		for(int i=0;i<t_level;i++){
			leds = (leds << 1);
			leds ++;
		}

		uint8_t l_data_out[6] = {leds};
		int l_ack = i2c_out_in(0x42,l_data_out,1,nullptr,0);
	}
};

int main( void )
{
	uint8_t l_ack = 0;
	uint8_t info[8];
	Radio r;

	Expander e;

	i2c_init();

	if ( ( l_ack = si4735_init() ) != 0 )
	{
		printf( "Initialization of SI4735 finish with error (%d)\r\n", l_ack );
		return 0;
	}
	else
		printf( "SI4735 initialized.\r\n" );

	printf( "\nTunig of radio station...\r\n" );

	l_ack = r.set_freq(10140);
	l_ack = r.set_volume(30);

	e.bar(2);



	if ( l_ack != 0 )
		printf( "Communication error!\r\n" );


	return 0;
}

