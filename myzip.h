#ifndef H_AM71113363_ZIP_H
#define H_AM71113363_ZIP_H

#define YES      1
#define NO       0

//note: HEADER files from Wikipedia
typedef struct PACKED ARCHIVE_HEADER
{
     DWORD    sg; //signature 0x04034b50
     USHORT   v;         //version;
     USHORT   bf;        //bit flag;
     USHORT   cm;        //compression method
     USHORT   mt;        //modification time
     USHORT   md;        //modification date
     DWORD    crc;       //CRC-32 value
     DWORD    cz;        //compressed size
     DWORD    uz;        //uncompressed size
     USHORT   nl;        //filename length
     USHORT   ex;        //extra field length(UNUSED)
     UCHAR   *name;      //String of filename,local sprintf  
     DWORD    of;        //offset this DIR
     struct ARCHIVE_HEADER *next;
}ARCH_HDR;

typedef struct PACKED LIST_HEADER
{
     DWORD    sg; //signature 0x02014b50
     USHORT   vm;        //version made;
     USHORT   v;         //version, same as ARCH_HDR.v
     USHORT   bf;        //bit flag;
     USHORT   cm;        //compression method
     USHORT   mt;        //modification time
     USHORT   md;        //modification date
     DWORD    crc;       //CRC-32 value
     DWORD    cz;        //compressed size
     DWORD    uz;        //uncompressed size
     USHORT   nl;        //filename length
     USHORT   ex;        //extra field length(UNUSED)
     USHORT   cl;        //comment length (UNUSED)
     USHORT   fl;        //file STart addr
     USHORT   at;        //file attributes
     DWORD    et;        //external file attributes
     DWORD    of;        //offset this DIR
}LIST_HDR;

typedef struct PACKED END_HEADER
{
     DWORD    sg; //signature 0x06054b50
     USHORT   nd;        //number of DISK (UNUSED)
     USHORT   ad;        //addr of 1st DISK (UNUSED)
     USHORT   num;       //number of records on 1st DISK
     USHORT   nm;        // total number of records
     DWORD    sz;        //size of LIST
     DWORD    of;        //offset of LIST
     USHORT   cl;        //comment length(UNUSED);
}END_HDR;


typedef struct ZIP_
{
    ARCH_HDR *AK;      //header
    ARCH_HDR *pAK;     //pointer to AK
    DWORD number;       //number of records; 
    DWORD ListOffset;   //offset of record LISTs   
    DWORD ListSize;     //size of records LIST
    HANDLE fd;          //file Handle of Zip File
    UCHAR *buffer;      //this holds the DATA of the FILE added to zip
    DWORD  bsize;       //size of the buffer,using realloc
    UCHAR err[64];      //on error ,display this error
    UCHAR flag;         //an error flag used by FileToZip
    USHORT   mt;        //modification time
    USHORT   md;        //modification date
}ZIP;                  


UCHAR BufferToZip(ZIP *Z,UCHAR *name,UCHAR *buffer,DWORD len);

UCHAR FileToZip(ZIP *Z,UCHAR *path,UCHAR *name);

void FreeZip(ZIP *Z);

UCHAR EndZip(ZIP *Z);

UCHAR ZipInit(ZIP *Z,UCHAR *filename);

#endif  //PROTECT

