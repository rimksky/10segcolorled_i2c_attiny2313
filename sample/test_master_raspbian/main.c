#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <sys/ioctl.h>


int i2c_write( int fd, unsigned char *buf, unsigned int size );
void set_color_temp( int temp, unsigned char *buf );
int check_cpu_temp( void );

int main( int argc, char **argv)
{
  char *i2cdevName = "/dev/i2c-1";
  const unsigned char i2cAddress = 0x55;
  int fd;
  unsigned char buf[20];
  int temp;

  if( ( fd = open( i2cdevName, O_RDWR ) ) < 0 ){
    fprintf( stderr, "fail to open i2c port\n" );
    exit( 1 );
  }
  if( ioctl( fd, I2C_SLAVE, i2cAddress ) < 0 ){
    fprintf( stderr, "fail to open i2c device\n" );
    exit( 1 );
  }

  while(1){
    temp = check_cpu_temp();
    set_color_temp( temp, buf );
    i2c_write( fd, buf, 20 );
    sleep(1);
  }

  exit( 0 );
}

int i2c_write( int fd, unsigned char *buf, unsigned int size ){
  if( write( fd, buf, size ) != size ){
    fprintf( stderr, "fail to write i2c device\n" );
    return 0;
  }
  return 1;
}

void set_color_temp( int temp, unsigned char *buf ){
  int n;
  unsigned char l, h;

  n = temp % 10;
  if( n == 0 ){
    n = 10;
  }
  if( temp <= 10 ){
    l = 0; // black
    h = 0; // black
  }
  else if( 10 < temp && temp <= 20 ){
    l = 0; // black
    h = 3; // lightblue
  }
  else if( 20 < temp && temp <= 30 ){
    l = 3; // lightblue
    h = 1; // blue
  }
  else if( 30 < temp && temp <= 40 ){
    l = 1; // blue
    h = 2; // green
  }
  else if( 40 < temp && temp <= 50 ){
    l = 2; // green
    h = 6; // yellow
  }
  else if( 50 < temp && temp <= 60 ){
    l = 6; // yellow
    h = 7; // white
  }
  else if( 60 < temp && temp <= 70 ){
    l = 7; // white
    h = 5; // magenta
  }
  else if( 70 < temp && temp <= 80 ){
    l = 5; // magenta
    h = 4; // red
  }
  else if( 80 < temp && temp <= 90 ){
    l = 4; // red
    h = 0; // black
  }
  else{
    l = 0; // black
    h = 0; // black
  }

  for( int i = 0; i < 10; i++ ){
    buf[i*2] = i;
    if( i < n ){
      buf[i*2+1] = h;
    }
    else{
      buf[i*2+1] = l;
    }
  }
}

int check_cpu_temp( void ){
  FILE *fp;
  char path[] = "/sys/class/thermal/thermal_zone0/temp";
  char buf[32] = { '\0' }; 
  int temp;
  temp = 0;

  if( ( fp = fopen( path, "r" ) ) ){
    fgets( buf, sizeof( buf ), fp );        
    temp = (int) ( atoi(buf) / 1000 );
  }
  fclose( fp );
  printf( "temp: %d C\n", temp );
  return temp; 
}

