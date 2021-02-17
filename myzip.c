// Copyright (c) 2021 AM71113363

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "myzip.h"


DWORD CRC32(DWORD start,UCHAR *buf, DWORD len);


UCHAR GrowPK(ZIP *Z)
{
    ARCH_HDR *s; 
    s=(ARCH_HDR*)malloc(sizeof(ARCH_HDR));
    if(s==NULL)
     return NO;

    if(Z->AK==NULL)
    {
        Z->AK=s;
        Z->AK->next=NULL;
        if(Z->number==0)
           Z->pAK = Z->AK;
    }
    else
    {           
       s->next=NULL;
       Z->AK->next=s;
       Z->AK = s;
    }
    Z->number++;
return YES;
}

//---------
UCHAR BufferToZip(ZIP *Z,UCHAR *name,UCHAR *buffer,DWORD len)
{
    DWORD crc=0;
    DWORD size=0;
    DWORD bytes;
    USHORT nameLen = (USHORT)strlen(name);       
   
    //realloc the struct
    if(GrowPK(Z) == NO)
      return NO;
    

    memset(Z->AK,0,sizeof(ARCH_HDR));
    
    if((buffer!=NULL) && (len > 0) ) //maybe is a folder 
        crc=CRC32(0,buffer,len);

    Z->AK->sg = 0x04034b50;
    Z->AK->v  = 0x0a;
    Z->AK->mt = Z->mt;
    Z->AK->md = Z->md;
    Z->AK->crc= crc;
    if(buffer!=NULL)
    {
      Z->AK->cz = len;
      Z->AK->uz = len;
    }
    Z->AK->nl = nameLen;
    Z->AK->of = Z->ListOffset;
    Z->AK->name = (UCHAR*)malloc(nameLen+1);
    if(Z->AK->name==NULL)
    {
       strcpy(Z->err,"malloc(name) FAILED!!!\0");             
       free(Z->AK);
       Z->AK=NULL;
       return NO;
    }
    sprintf(Z->AK->name,"%s\0",name);
    if(WriteFile(Z->fd,Z->AK,30,&bytes,NULL) == 0)
    {
        strcpy(Z->err,"WriteFile FAILED!!!\0");                                  
        free(Z->AK->name);
        free(Z->AK);
        Z->AK=NULL;       
        return NO; //smth wrong
    }
    size+=bytes;
    if(WriteFile(Z->fd,Z->AK->name,Z->AK->nl,&bytes,NULL) == 0)
    {
        strcpy(Z->err,"WriteFile FAILED!!!\0");                                  
        free(Z->AK->name);
        free(Z->AK);
        Z->AK=NULL; 
        return NO; //smth wrong
    }
    size+=bytes;

    if((buffer!=NULL) && (len>0) ) //maybe is a folder
    {
       if(WriteFile(Z->fd,buffer,len,&bytes,NULL) == 0)
       {
           strcpy(Z->err,"WriteFile FAILED!!!\0");                                  
           free(Z->AK->name);
           free(Z->AK);
           Z->AK=NULL; 
           return NO; //smth wrong
       }
       size+=bytes;                   
   }
   Z->ListOffset+=size;
   return YES;
}


//----------------

UCHAR FileToZip(ZIP *Z,UCHAR *path,UCHAR *name)
{
    DWORD dwFileSize; DWORD dwRead;
    HANDLE hFile;
    hFile = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
    if(hFile == INVALID_HANDLE_VALUE)
    {
        strcpy(Z->err,"CreateFile FAILED!!!\0");  
        Z->flag=YES;                                
        return NO; //smth wrong
    }
    dwFileSize = GetFileSize(hFile, NULL);
    if(dwFileSize == 0xFFFFFFFF)
    {
        strcpy(Z->err,"GetFileSize FAILED!!!\0");                                  
        CloseHandle(hFile);
        Z->flag=YES;
        return NO;
    }            
    if(Z->bsize<dwFileSize)
    {
         Z->bsize=dwFileSize;
         //realloc = malloc(new size), copy the old to new, free the old
         //Z->buffer=(UCHAR*)realloc(Z->buffer,dwFileSize+1);
         if(Z->buffer) free(Z->buffer);
         Z->buffer=(UCHAR*)malloc(dwFileSize+1);
    }
    if(Z->buffer==NULL)
    {
        strcpy(Z->err,"Realloc FAILED!!!\0");                                  
        CloseHandle(hFile);
        return NO;
    }                          
                           
    if(ReadFile(hFile, Z->buffer, dwFileSize, &dwRead, NULL) == 0)
    {
        strcpy(Z->err,"ReadFile FAILED!!!\0");                                  
        CloseHandle(hFile);
        Z->flag=YES;
        return NO;
    }
    Z->buffer[dwFileSize] = 0; // Null terminator
    CloseHandle(hFile);
    Z->flag=NO;
  return BufferToZip(Z,name,Z->buffer,dwFileSize);
}

//----------------
UCHAR GenerateList(ZIP *Z)
{
    DWORD size=0,bytes;
    LIST_HDR LS;
    ARCH_HDR *p;
    Z->number=0;
    for(p=Z->pAK;p!=NULL;p=p->next)
    {
        memset(&LS,0,sizeof(LIST_HDR));
    
        LS.sg = 0x02014b50;
        LS.vm = 0x3f;
        LS.v  = p->v;
        LS.bf = p->bf;
        LS.mt = p->mt;
        LS.md = p->md;
        LS.crc= p->crc;
        LS.cz = p->cz;
        LS.uz = p->uz;
        LS.nl = p->nl;
        LS.et = 0x80;
        LS.of = p->of;
        
        if(WriteFile(Z->fd,&LS,sizeof(LS),&bytes,NULL) == 0)
        {
           strcpy(Z->err,"WriteFile FAILED!!!\0");                                  
           return NO;
        }
        size+=bytes;
        
        if(WriteFile(Z->fd,p->name,p->nl,&bytes,NULL) == 0)
        {
           strcpy(Z->err,"WriteFile FAILED!!!\0");                                  
           return NO;
        }
        size+=bytes;
        Z->number++;
    } 
    Z->ListSize = size;
    return YES;
}
//---------

UCHAR AddEnd(ZIP *Z)
{
    END_HDR ED;
    DWORD bytes;
    memset(&ED,0,sizeof(END_HDR));
    ED.sg  = 0x06054b50;
    ED.num = Z->number;
    ED.nm  = Z->number;
    ED.sz  = Z->ListSize;
    ED.of  = Z->ListOffset;

    if(WriteFile(Z->fd,&ED,sizeof(ED),&bytes,NULL) == 0) //fwrite(&ED,1,sizeof(ED),f);
    {
        strcpy(Z->err,"WriteFile FAILED!!!\0");                                  
        return NO;
    }
         
return YES;
}

void FreeZip(ZIP *Z)
{
    DWORD i; 	
    if(Z->number > 0)
    {
      ARCH_HDR *p,*prev;
      for(p=Z->pAK;p!=NULL;)
      {           
            if(p->nl > 0)
               free(p->name);
            prev=p;
            p=p->next;
            free(prev);
      }
      Z->AK=NULL;
    }                      
    if(Z->bsize > 0)
       free(Z->buffer);
    Z->number = 0;   
   CloseHandle(Z->fd); 
}  
  
UCHAR EndZip(ZIP *Z)
{
   UCHAR Ret=NO;
   if(Z->fd)
   {
       Ret = GenerateList(Z);
       if(Ret==YES)
          Ret = AddEnd(Z);
   FreeZip(Z);
   }else{ strcpy(Z->err,"Zip Handle Invalid!!!\0"); }
   return Ret; 
}
//---------
UCHAR ZipInit(ZIP *Z,UCHAR *filename)
{
   memset(Z,0,sizeof(ZIP));
   Z->fd = CreateFile(filename, GENERIC_WRITE, 0, NULL,OPEN_ALWAYS, 0, 0);
   if(Z->fd == INVALID_HANDLE_VALUE)
	return NO;
   Z->buffer=NULL; //I know,memset did it
   Z->bsize=0;     //I know,memset did it
   Z->flag=NO;     //I know,memset did it
   Z->AK==NULL;    //I know,memset did it

#ifdef _TIME_H_
    time_t tim;
    struct tm * ti;
    time(&tim);
    ti = localtime(&tim);
    Z->mt = (USHORT)(((ti->tm_hour) << 11) + ((ti->tm_min) << 5) + ((ti->tm_sec) >> 1));
    Z->md = (USHORT)(((ti->tm_year + 1900 - 1980) << 9) + ((ti->tm_mon + 1) << 5) + ti->tm_mday);
#else
    Z->mt = 0x48b2; //nothing special about this Date:Time, 
    Z->md = 0x5248; //Its just the Actual (DATE/TIME) when I started creating this
#endif

return YES;
}
   



