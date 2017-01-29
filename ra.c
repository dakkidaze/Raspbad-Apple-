/*
g.h存放着3285?的?片（以二?数?的形式）
??需要使用WiringPi?
gcc -o ra ra.c -L lib -l wiringPi
并以root身??行
sudo ./ra
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <wiringPi.h>
#include <iconv.h> 
#include <sys/time.h>
#include "g.h"
#include <time.h>
#define LCD_RS 4 //Display Control
#define LCD_RW 5
#define LCD_EN 1

#define D1 30 //Data
#define D2 21
#define D3 22
#define D4 23
#define D5 24
#define D6 25
#define D7 26
#define D8 27

char u2g_out[255];

//UTF-8→GB3212
int code_convert(char *from_charset,char *to_charset,char *inbuf,int inlen,char *outbuf,int outlen){
iconv_t cd;
int rc;
char **pin = &inbuf;
char **pout = &outbuf;

cd = iconv_open(to_charset,from_charset);
if (cd==0) return -1;
memset(outbuf,0,outlen);
if (iconv(cd,pin,&inlen,pout,&outlen)==-1) return -1;
iconv_close(cd);
return 0;
}

int u2g(char *inbuf,int inlen,char *outbuf,int outlen){ 
return code_convert("utf-8","gb2312",inbuf,inlen,outbuf,outlen); 
} 

//Hex
void bus_write(unsigned char data){
int t[10];
int f=0,i=0,d=data;


for(i=0;i<8;i++){
t[i]=data%2;
data=data/2;
}


digitalWrite(D1,t[0]);
digitalWrite(D2,t[1]);
digitalWrite(D3,t[2]);
digitalWrite(D4,t[3]);
digitalWrite(D5,t[4]);
digitalWrite(D6,t[5]);
digitalWrite(D7,t[6]);
digitalWrite(D8,t[7]);
}
//Check busy
void chk_busy(){
digitalWrite(LCD_RS,0);
digitalWrite(LCD_RW,1);
digitalWrite(LCD_EN,1);
bus_write(0xff);
pinMode(D8, INPUT);
while(digitalRead(D8));
pinMode(D8, OUTPUT);
digitalWrite(LCD_EN,0);
}
//Write
void WriteCmd_LCD12864(unsigned char cmdcode){
chk_busy();
digitalWrite(LCD_RS,0);
digitalWrite(LCD_RW,0);
digitalWrite(LCD_EN,1);
delayMicroseconds(1);
bus_write(cmdcode);
digitalWrite(LCD_EN,0);
delayMicroseconds(1);
}
//Write data
void WriteData_LCD12864(unsigned char Dispdata){
chk_busy();
digitalWrite(LCD_RS,1);
digitalWrite(LCD_RW,0);
digitalWrite(LCD_EN,1);
delayMicroseconds(1);
bus_write(Dispdata);
digitalWrite(LCD_EN,0);
delayMicroseconds(1);
}
//Send text
void WriteWord_LCD12864(unsigned char a,unsigned char *d){
unsigned char *s;
u2g(d,strlen(d),u2g_out,255);
s=u2g_out;
WriteCmd_LCD12864(a);
while(*s>0){
WriteData_LCD12864(*s); 
s++;
}
}

void WriteWord_LCD12864_2(unsigned char *d){
int i=0;
unsigned char *s;
u2g(d,strlen(d),u2g_out,255);
s=u2g_out;
WriteCmd_LCD12864(0x80);
while(*s>0){
WriteData_LCD12864(*s); 
s++;
i++;
if(i==16){
WriteCmd_LCD12864(0x90);
}
if(i==32){
WriteCmd_LCD12864(0x88);
}
if(i==48){
WriteCmd_LCD12864(0x98);
}
}
}
//Initial
void Init_LCD12864(void){ 
pinMode(D1, OUTPUT); 
pinMode(D2, OUTPUT);
pinMode(D3, OUTPUT);
pinMode(D4, OUTPUT);
pinMode(D5, OUTPUT);
pinMode(D6, OUTPUT);
pinMode(D7, OUTPUT);
pinMode(D8, OUTPUT);

pinMode(LCD_RS, OUTPUT);
pinMode(LCD_RW, OUTPUT);
pinMode(LCD_EN, OUTPUT);

WriteCmd_LCD12864(0x38); 
delay(20);
WriteCmd_LCD12864(0x01); 
delay(20);
WriteCmd_LCD12864(0x0c); 
delay(20);
}
//-----display--one-picture--------
void Picture_12864(unsigned char *pic)
{	unsigned char i,j;
	WriteCmd_LCD12864(0x36);
	chk_busy();
	//X address
	//Y address
	//data
	for(i=0;i<32;i++)
	{
		WriteCmd_LCD12864(0x80 + i);
		chk_busy();
		WriteCmd_LCD12864(0x80);
		chk_busy();
		
		for(j=0;j<16;j++)
		{
			WriteData_LCD12864(*pic);
			chk_busy();
			pic ++;
		}
	}
	for(i=0;i<32;i++)
	{
		WriteCmd_LCD12864(0x80 + i);
		chk_busy();
		WriteCmd_LCD12864(0x88);
		chk_busy();
		
		for(j=0;j<16;j++)
		{
			WriteData_LCD12864(*pic);
			chk_busy();
			pic ++;
		}
	}
}

//----------------------------------



int main (int args, char *argv[]){

int frame;

long delta_time;
int current_frame;
struct timeval start, end;

wiringPiSetup();
Init_LCD12864();
WriteCmd_LCD12864(0x01);

gettimeofday( &start, NULL );//start time of the video

for(frame = 0;frame < 3285 ;frame++)
{
	Picture_12864(*(gImage+frame));
	
    gettimeofday( &end, NULL );
    delta_time = 1000000 * ( end.tv_sec - start.tv_sec ) + end.tv_usec - start.tv_usec;
	current_frame = (long)(delta_time/66667);
	
	if(!(((current_frame-1) < frame) && (frame<(current_frame+1)))) //faster or slower
	{
		frame = current_frame;
	}
/*debug
	printf ( "\007The current frame is: %d.\n",current_frame );
	printf ( "\007The delta time is: %d.\n",delta_time );
	printf ( "\007The frame is: %d.\n",frame );
*/
}

}
