#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#define YES         1
#define NO          0

HINSTANCE ins;
HWND hWnd,B_go; 
HWND SKPname;         WNDPROC OldNameEditProc;
HWND SKPdescription;  WNDPROC OldDescEditProc;

typedef struct MYSKINS_
{
    UCHAR *name;    //the Name Of Skin
    UCHAR *path;    //path of the SKIN to import
    UCHAR flag;     //if the SKIN fails to READ   
    struct MYSKINS_ *next;
}MYSKINS;

//arg=   %s(name) %s(name) %s(uiid) %s(new_uuid)
const static char MANIFEST_JSON[]=
"{\n"
"\x20\x20\x20\x20\"format_version\": 1,\n"
"\x20\x20\x20\x20\"header\": {\n"
"\x20\x20\x20\x20\x20\x20\x20\x20\"description\": \"%s SKinPack\",\n"
"\x20\x20\x20\x20\x20\x20\x20\x20\"name\": \"%s\",\n"
"\x20\x20\x20\x20\x20\x20\x20\x20\"uuid\": \"%s\",\n"
"\x20\x20\x20\x20\x20\x20\x20\x20\"version\": [1, 0, 0],\n"
"\x20\x20\x20\x20\x20\x20\x20\x20\"min_engine_version\": [1, 2, 6]\n"
"\x20\x20\x20\x20},\n"
"\x20\x20\x20\x20\"modules\": [\n"
"\x20\x20\x20\x20{\n"
"\x20\x20\x20\x20\x20\x20\x20\x20\"type\": \"skin_pack\",\n"
"\x20\x20\x20\x20\x20\x20\x20\x20\"uuid\": \"%s\",\n"
"\x20\x20\x20\x20\x20\x20\x20\x20\"version\": [1, 0, 0]\n"
"\x20\x20\x20\x20\x20\x20\x20\x20}\n"
"\x20\x20\x20\x20]\n"
"}\0";


//arg=   %d(num) %d(num)
const static char SKINS_JSON_HEADER[]=
"{\n"
"\x20\x20\x20\x20\"skins\":[\n"
"\x20\x20\x20\x20\x20\x20\x20\x20{\n"
"\"localization_name\": \"%dam\",\n"
"\"texture\": \"%dam.png\",\n"
"\"type\": \"free\"\n"
"}\0";

//arg=   %d(num) %d(num)
const static char SKINS_JSON_BODY[]=    
",\n"
"{\n"
"\"localization_name\": \"%dam\",\n"
"\"texture\": \"%dam.png\",\n"
"\"type\": \"free\"\n"
"}\0";

//arg=   %s(name)   %s(name)
const static char SKINS_JSON_END[]=      
"\n\x20\x20\x20\x20],\n"
"\x20\x20\x20\x20\"serialize_name\": \"%s\",\n"
"\x20\x20\x20\x20\"localization_name\": \"%s\"\n"
"}\0";
