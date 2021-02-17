#include "main.h"
#include "myzip.h"

char szClassName[ ] = "WindowsAppSkinPack";
void CenterOnScreen();

#define SMS(_x_)  MessageBox(hWnd,_x_,"#Error",MB_OK |MB_ICONERROR)


MYSKINS *myskins=NULL;
MYSKINS *PM;              //pointer to heade list of "myskins"
UINT myskinsLen=0;
UCHAR Status[32];

void FreeList()
{
    MYSKINS *s,*prev; 
    for(s=PM;s!=NULL;)
    {
        if(s->name) free(s->name);
        if(s->path) free(s->path);                       
        
        prev = s;
        s=s->next;
        free(prev);                       
    } 
    myskins=NULL;
}

void AddToList(UCHAR *path, UCHAR *name)
{
    MYSKINS *s=(MYSKINS*)malloc(sizeof(MYSKINS));
    if(s==NULL)
     return;
    s->name=(UCHAR*)malloc(strlen(name)+1);
    if(s->name==NULL)
    {
      free(s);
      return;
    }
    s->path=(UCHAR*)malloc(strlen(path)+1);
    if(s->path==NULL)
    {
      free(s->name);
      free(s);
      return;
    }
    sprintf(s->path,"%s\0",path);
    sprintf(s->name,"%s\0",name);
    s->flag=NO; 
    if(myskins==NULL)
    {
        myskins=s;
        myskins->next=NULL;
        if(myskinsLen==0)
            PM = myskins;
    }
    else
    {           
       s->next=NULL;       
       myskins->next=s;
       myskins = s;
    }
    myskinsLen++;
}

UCHAR AddSkin(UCHAR *filename)
{
    UCHAR *p,*e; UINT i; UCHAR temp[MAX_PATH];
    e=strstr(filename,".png");
    if(!e)
     return NO;
    p=strrchr(filename,'\\');
    if(!p)
     return NO;
    p++; 
    e[0]=0; //delete .png
    sprintf(temp,"%s\0",p);
    e[0]='.'; //restore .png
    AddToList(filename, temp);
    sprintf(Status,"Imported Skins[ %d ]\0",myskinsLen);
    SetWindowText(hWnd,Status);
    if(myskinsLen==1)
     EnableWindow(B_go,1);  //no point of creating a skinPack with only 1 skin 
    return YES;
}

void MakeMyPack()
{
	UCHAR RootFolder[100]; //the Pack will use this NAME
	UCHAR ThisDescription[100]; //this is used only in en_US.lang,DISPLAY pack name
	UCHAR Temp[512];
	UCHAR UUID1[36]; UCHAR UUID2[36]; ULONG crc;

	ZIP A; UCHAR Ret; UCHAR *buffer; MYSKINS *s;
	DWORD len; UINT i; 

    EnableWindow(B_go,0);	 
//Get INFO     
    memset(RootFolder,0,100);
    if(GetWindowText(SKPname,RootFolder,99)<1)
    {
        SMS("SkinPack Name : Is Empty!!!!");
        EnableWindow(B_go,1);
        return;
    }
    memset(ThisDescription,0,100);
    if(GetWindowText(SKPdescription,ThisDescription,99)<1)
    { 
        //is empty use RootFolder name
        strcpy(ThisDescription,RootFolder);
        SetWindowText(SKPdescription,ThisDescription);
    }
//calculate buffer size to hold data
    len = (myskinsLen+10) * sizeof(SKINS_JSON_HEADER);
    buffer = (UCHAR*)malloc(len);
    if(buffer==NULL)
    {  
        SMS("Malloc Failed!!!!");
        exit(0);
        return;
    }  	
    
//create Zip File
    sprintf(Temp,"%s.mcpack\0",RootFolder);
    Ret=ZipInit(&A,Temp);
    if(Ret==NO)
    {  
        SMS("Failed To Create SkinPack!!!!");
        exit(0);
        return;
    }  	
//add PNG to ZIP, maybe some files are not for READ
    len=0;
    i=0;
    for(s=PM;s!=NULL;s=s->next)
    {
	    sprintf(Temp,"%dam.png\0",i);	
	    Ret=FileToZip(&A,s->path,Temp);
	    if(Ret==NO)
	    {
           if(A.flag==YES) //cand read file
              continue;
           free(buffer);            
           FreeList();
           FreeZip(&A);
           SMS("Error: Add(PNG to MCPACK)\nDelete the created .mcpack");
           SMS(A.err);
           exit(0);
           return;
        }
        i++;                              
	    s->flag=YES; 
    }
    //just checking
    if(i==0)
    {
        FreeList();    
        FreeZip(&A); 
        free(buffer);     
	    SMS("SkinPack Failed To COPY!!!!\nDelete the created .mcpack");
        exit(0);
	    return;
    }     
 //create LANG file
 	
   	len=sprintf(buffer,"skinpack.%s=%s\n\0",RootFolder,ThisDescription); 
	
	i=0; myskinsLen = 0;
    for(s=PM;s!=NULL;s=s->next)
    {
		if(s->flag!=YES)
				continue;
	     len+=sprintf(&buffer[len],"skin.%s.%dam=%s\n\0",RootFolder,i,s->name);
	     i++;
	     myskinsLen++;
    }
    buffer[len]=0;
 //   Ret = BufferToZip(&A,"texts/",NULL,0);
    Ret = BufferToZip(&A,"texts/en_US.lang",buffer,len);
    if(Ret==NO)
    {
        FreeList();
        FreeZip(&A); 
        free(buffer);     
	    SMS("Adding en_US.lang Failed!!!!\nDelete the created .mcpack");
	    SMS(A.err);
        exit(0);
	    return;
    }	
//create manifest.json 
    crc=0;
    //generate UUIDs
    GenerateUUID(UUID1,&crc);
    GenerateUUID(UUID2,&crc);
    len=sprintf(buffer,MANIFEST_JSON,RootFolder,RootFolder,UUID1,UUID2);
	buffer[len]=0;
	Ret = BufferToZip(&A,"manifest.json",buffer,len);
    if(Ret==NO)
    {
        FreeList();
        FreeZip(&A); 
        free(buffer);     
	    SMS("Adding manifest.json Failed!!!!\nDelete the created .mcpack");
	    SMS(A.err);
        exit(0);
	    return;
    }
//create skins.json        
	len=sprintf(buffer,SKINS_JSON_HEADER,0,0);
    //write other skins
	for(i=1;i<myskinsLen;i++)
	{
        len+=sprintf(&buffer[len],SKINS_JSON_BODY,i,i);
	}		
    //write END
    len+=sprintf(&buffer[len],SKINS_JSON_END,RootFolder,RootFolder);
    buffer[len]=0;
	Ret = BufferToZip(&A,"skins.json",buffer,len);
    if(Ret==NO)
    {
        FreeList();
        FreeZip(&A); 
        free(buffer);     
	    SMS("Adding skins.json Failed!!!!\nDelete the created .mcpack");
	    SMS(A.err);
        exit(0);
	    return;
    } 
//close ZIP, Write Final DATA
	Ret=EndZip(&A);
    if(Ret==NO)
    {
        FreeList();
        FreeZip(&A); 
        free(buffer);     
	    SMS("Adding Zip Extra DATA Failed!!!!\nDelete the created .mcpack");
	    SMS(A.err);
        exit(0);
	    return;
    } 		

    FreeList();
    free(buffer);
	myskinsLen=0;  
   SetWindowText(hWnd,"DONE"); 
   _sleep(2000); 
   exit(0);                    
}



LRESULT CALLBACK NameEditProc(HWND hnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) 
	{
    	case WM_CHAR: 
        {
             if(wParam == VK_BACK)
             break;
			if(!(isalnum((char)wParam))) return 0;
	     }break;		
    }
	return CallWindowProc(OldNameEditProc, hnd, message, wParam, lParam);
}

LRESULT CALLBACK DescEditProc(HWND hnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) 
	{
    	case WM_CHAR: 
        {
             if((wParam == VK_BACK) || (wParam == VK_SPACE) || 
                (wParam == '_') || (wParam == '-') ||
                (wParam == '(') || (wParam == ')'))
             break;
			if(!(isalnum((char)wParam))) return 0;
	     }break;		
    }
	return CallWindowProc(OldDescEditProc, hnd, message, wParam, lParam);
}


LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_CREATE:
        {
             hWnd = hwnd;
             CreateWindow("BUTTON","SkinPack Name",WS_CHILD|WS_VISIBLE|BS_GROUPBOX,2,2,217,50,hwnd,NULL,ins,NULL);
             SKPname= CreateWindowEx(WS_EX_CLIENTEDGE,"edit","",WS_CHILD|WS_VISIBLE,11,22,200,22,hwnd,NULL,ins,NULL);
             OldNameEditProc=(WNDPROC)SetWindowLong(SKPname, GWL_WNDPROC, (LPARAM)NameEditProc);
                 
             CreateWindow("BUTTON","Description",WS_CHILD|WS_VISIBLE|BS_GROUPBOX,2,55,277,50,hwnd,NULL,ins,NULL);
             SKPdescription= CreateWindowEx(WS_EX_CLIENTEDGE,"edit","",WS_CHILD|WS_VISIBLE,11,75,260,22,hwnd,NULL,ins,NULL);
    		 OldDescEditProc=(WNDPROC)SetWindowLong(SKPdescription, GWL_WNDPROC, (LPARAM)DescEditProc);
                     
             B_go = CreateWindow("BUTTON","Start",WS_CHILD|WS_VISIBLE|WS_DISABLED,225,11,54,40,hwnd,(HMENU)1111,ins,NULL);
            
             CenterOnScreen();
             DragAcceptFiles(hwnd,1);
        }
        break;   
        case WM_DROPFILES:
		{
           HDROP hDrop;
		   UINT sa;
           UINT x;    
           UCHAR tmp[MAX_PATH];

           hDrop=(HDROP)wParam;	
           sa=DragQueryFile(hDrop,0xFFFFFFFF,0,0);
           for(x=0;x<sa;x++)
           {
               memset(tmp,0,MAX_PATH);
               DragQueryFile(hDrop,x,tmp,MAX_PATH);
               if((GetFileAttributes(tmp) & FILE_ATTRIBUTE_DIRECTORY)==FILE_ATTRIBUTE_DIRECTORY)
                   continue;
               AddSkin(tmp);
           }
		   DragFinish(hDrop);
     }
     break; 
       case WM_COMMAND:
       {
            switch(LOWORD(wParam))
            {                  
                 case 1111:
                 {
                    EnableWindow(B_go,0);  
                    CreateThread(0,0,(LPTHREAD_START_ROUTINE)MakeMyPack,0,0,0);        
                 }
                 break;                 
            }
            return 0;
       }
       break;
        case WM_DESTROY:
            PostQuitMessage (0);       /* send a WM_QUIT to the message queue */
            break;
        default:                      /* for messages that we don't deal with */
            return DefWindowProc (hwnd, message, wParam, lParam);
    }

    return 0;
}


int WINAPI WinMain (HINSTANCE hThisInstance, HINSTANCE hPrevInstance, LPSTR lpszArgument, int nFunsterStil)
{
            
    MSG messages;    
    WNDCLASSEX wincl;  
    HWND hwnd;       
    ins=hThisInstance;

    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;
    wincl.style = CS_DBLCLKS;  
    wincl.cbSize = sizeof (WNDCLASSEX);
    wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon (ins,MAKEINTRESOURCE(200));
    wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;  
    wincl.cbClsExtra = 0;  
    wincl.cbWndExtra = 0;      
    wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND+1;

    if (!RegisterClassEx (&wincl))
        return 0;

    hwnd = CreateWindowEx(WS_EX_TOPMOST,szClassName,"SkinPack Creator",WS_SYSMENU|WS_MINIMIZEBOX,CW_USEDEFAULT,CW_USEDEFAULT,
    290,140,HWND_DESKTOP,NULL,hThisInstance,NULL );
    
    ShowWindow (hwnd, nFunsterStil);

    while (GetMessage (&messages, NULL, 0, 0))
    {
         TranslateMessage(&messages);
         DispatchMessage(&messages);
    }

     return messages.wParam;
}

void CenterOnScreen()
{
     RECT rcClient, rcDesktop;
	 int nX, nY;
     SystemParametersInfo(SPI_GETWORKAREA, 0, &rcDesktop, 0);
     GetWindowRect(hWnd, &rcClient);
	 nX=((rcDesktop.right - rcDesktop.left) / 2) -((rcClient.right - rcClient.left) / 2);
	 nY=((rcDesktop.bottom - rcDesktop.top) / 2) -((rcClient.bottom - rcClient.top) / 2);
SetWindowPos(hWnd, NULL, nX, nY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

