// Copyright (c) 2021 AM71113363

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>


DWORD CRC32(DWORD start,UCHAR *buf, DWORD len);

UCHAR *GenerateUUID(UCHAR *buffer,DWORD *crc)
{
    UCHAR *p; UCHAR c;
    UCHAR *b; int i=0,n=0;
    DWORD r=GetTickCount();
    DWORD t=*crc; 
    p=buffer;
    n=sprintf(buffer,"%x%d%X\0",r,r,r);
    if(n<30)
    {
      memset(&buffer[n],'X',31-n);
      n+=(30-n);
    } 
    
    t=CRC32(t,buffer,n);
    i=sprintf(p,"%08x",t);
    p+=i;
    t=CRC32(t,buffer,n);
    i=sprintf(p,"%08x",t);
    p+=i;
    t=CRC32(t,buffer,n);
    i=sprintf(p,"%08x",t);
    p+=i;
    t=CRC32(t,buffer,n);
    i=sprintf(p,"%08x",t);
    
    //remove '0' from buffer
     c=("0123456789abcdef")[t&0xf];
     for(i=0;i<32;i++){ if(buffer[i]=='0') buffer[i]=c; }
     b=buffer+31;
     p=buffer+35;
     buffer[36]=0;
     //copy, reverse mode
     for(i=0;i<12;i++){ *p--=*b--; }
     *p--='-';
     for(i=0;i<3;i++){ *p--=*b--; }
     *p--=("89ab")[*b--&3];
     *p--='-';
     for(i=0;i<3;i++){ *p--=*b--; }
     *p--='4';
     *p--='-';
     for(i=0;i<4;i++){ *p--=*b--; }
     p[0]='-';
     *crc = t;
 return buffer;     
}


