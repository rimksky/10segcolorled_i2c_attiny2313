#include <Wire.h>

unsigned char buf[20];
unsigned char address;

void setup() {
  address = 0x55;
  Wire.begin();
}

void loop() {
  for( int i=10; i < 90; i++ ){
    set_color_temp(i, buf);
    Wire.beginTransmission(address);
    for( int j=0; j < 20; j++ ){
      Wire.write(buf[j]);
    }    
    Wire.endTransmission();
    delay(10);
  }
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
