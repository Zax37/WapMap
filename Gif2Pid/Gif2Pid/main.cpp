#include <iostream>
#include <cstdio>
#include <conio.h>
#include "../../shared/cPID.h"
#include <gd.h>

using namespace std;

char * GetFile(char * filepath)
{
    char * dir;
    char * pch;
    pch=strchr(filepath,'/');
    int location = 0;
    while (pch!=NULL)
    {
     location = pch-filepath;
     pch=strchr(pch+1,'/');
    }
    pch=strchr(filepath,'\\');
    int location2 = 0;
    while (pch!=NULL)
    {
     location2 = (pch-filepath+1);
     pch=strchr(pch+1,'\\');
    }
    int len = 0;
    if( location > location2 )
     len = location;
    if( location2 > location )
     len = location2;
    dir = new char[strlen(filepath)-len+1];
    for(int i=len;i<strlen(filepath);i++)
     dir[i-len] = filepath[i];
    dir[strlen(filepath)-len] = '\0';
    return dir;
}

char * GetFileWithoutExt(char * file)
{
    char * pch;

    pch=strchr(file,'.');
    if( pch == NULL ){
     char * ret = new char[strlen(file)+1];
     strcpy(ret, file);
     return ret;
    }

    int last = 0;

    while( pch!=NULL ){
     last = pch-file+1;
     pch=strchr(pch+1,'.');
    }

    if( strlen(file)-last <= 0 ){
     char * ret = new char[strlen(file)+1];
     strcpy(ret, file);
     return ret;
    }
    last;
    char * ret = new char[last];
    for(int i=0;i<last;i++){
     ret[i] = file[i];
     if( ret[i] == '.' )
      ret[i] = '\0';

     if( ret[i] == '\0' )
      return ret;
    }
}

int main(int argc, char * argv[])
{
    if (argc == 2) {
        PID::Image image(argv[1]);
        image.Save("dupa.pid");
        return 0;
    }

    printf("gif2pid v1.1 by kijanek6 & Zax37\n");
    if( argc < 3 ){
     printf("Usage:\ngif2pid.exe input [output] [-idA] [-offxA] [-offyA] [-u1vA] [-u2vA] [-t] [-v] [-s] [-m] [-i] [-c] [-l] [-o]\n");
     printf("input: PID file to convert\n");
     printf("output: BMP file to save as\n");
     printf("-idA: sets PID id to A (example: -id30 sets to 30)\n");
     printf("-offxA: sets X offset to A (example: -offx10 sets to 10)\n");
     printf("-offyA: sets Y offset to A (example: -offy15 sets to 15)\n");
     printf("-u1vA: sets user value #1 to A (example: -u1v50 sets to 50)\n");
     printf("-u2vA: sets user value #2 to A (example: -u2v60 sets to 60)\n");
     printf("-t: sets 'transparency' flag\n");
     printf("-v: sets 'video memory' flag\n");
     printf("-s: sets 'system memory' flag\n");
     printf("-m: sets 'mirror' flag\n");
     printf("-i: sets 'invert' flag\n");
     printf("-c: sets 'compression' flag\n");
     printf("-l: sets 'lights' flag\n");
     printf("-o: add palette to PID\n");
     return 1;
    }
    printf("----------------------\n");
    char * szFileIn = argv[1];
    char * szFileOut = NULL;

    if( argc > 2 ){
     szFileOut = new char[strlen(argv[2])+1];
     strcpy(szFileOut, argv[2]);
    }else{
     char * tmp = GetFile(szFileIn);
     char * tmp2 = GetFileWithoutExt(tmp);
     szFileOut = new char[strlen(tmp2)+5];
     sprintf(szFileOut, "%s.pid", tmp2);
     delete [] tmp2;
     delete [] tmp;
    }
    FILE * ftest = fopen(szFileIn, "r");
    if( !ftest ){
     printf("Failed to open %s.\n", szFileIn);
     return 1;
    }
    fclose(ftest);
    printf("File in: %s\n", szFileIn);
    printf("File out: %s\n", szFileOut);

    char flags = 0;
    int id = 0;
    int offx = 0, offy = 0;
    int user[2] = {0,0};

    for(int i=3;i<argc;i++){
     if( !strcmp(argv[i], "-v") ){
      flags += PID::Flag_VideoMemory;
     }else if( !strcmp(argv[i], "-t") ){
      flags += PID::Flag_Transparency;
     }else if( !strcmp(argv[i], "-s") ){
      flags += PID::Flag_SystemMemory;
     }else if( !strcmp(argv[i], "-m") ){
      flags += PID::Flag_Mirror;
     }else if( !strcmp(argv[i], "-i") ){
      flags += PID::Flag_Invert;
     }else if( !strcmp(argv[i], "-c") ){
      flags += PID::Flag_Compression;
     }else if( !strcmp(argv[i], "-l") ){
      flags += PID::Flag_Lights;
     }else if( !strcmp(argv[i], "-o") ){
      flags += PID::Flag_OwnPalette;
     }else if( !strncmp(argv[i], "-id", 3) ){
      sscanf(argv[i], "-id%d", &id);
     }else if( !strncmp(argv[i], "-offx", 5) ){
      sscanf(argv[i], "-offx%d", &offx);
     }else if( !strncmp(argv[i], "-offy", 5) ){
      sscanf(argv[i], "-offy%d", &offy);
     }else if( !strncmp(argv[i], "-u1v", 4) ){
      sscanf(argv[i], "-u1v%d", &user[0]);
     }else if( !strncmp(argv[i], "-u2v", 4) ){
      sscanf(argv[i], "-u2v%d", &user[1]);
     }
    }

    FILE * fin = fopen(szFileIn, "rb");
    gdImagePtr im = gdImageCreateFromGif(fin);
    fclose(fin);

    PID::Palette hPal;
    for(int i=0;i<256;i++){
     hPal.SetColorRGB(i, gdImageRed(im, i), gdImageGreen(im, i), gdImageBlue(im, i));
    }

    PID::Image pid(im->sx, im->sy);
    pid.SetID(id);
    pid.SetOffset(offx, offy);
    for(int i=0;i<2;i++)
     pid.SetUserValue(i+2, user[i]);
    pid.SetFlags((PID::FLAGS)flags);


    printf("---\nPID:\n");
    printf(" id: %d\n", pid.GetID());
    printf(" size: %d,%d\n", pid.GetWidth(), pid.GetHeight());
    printf(" user values: %d,%d,%d,%d\n", pid.GetUserValue(0), pid.GetUserValue(1), pid.GetUserValue(2), pid.GetUserValue(3));
    printf(" offset: %d,%d\n", pid.GetOffsetX(), pid.GetOffsetY());

    printf(" flags:\n");
    if( pid.GetFlags() & PID::Flag_Transparency ) printf("  transparency  [00000001]\n");
    if( pid.GetFlags() & PID::Flag_VideoMemory )  printf("  video memory  [00000010]\n");
    if( pid.GetFlags() & PID::Flag_SystemMemory ) printf("  system memory [00000100]\n");
    if( pid.GetFlags() & PID::Flag_Mirror )       printf("  mirror        [00001000]\n");
    if( pid.GetFlags() & PID::Flag_Invert )       printf("  invert        [00010000]\n");
    if( pid.GetFlags() & PID::Flag_Compression )  printf("  compression   [00100000]\n");
    if( pid.GetFlags() & PID::Flag_Lights )       printf("  lights        [01000000]\n");
    if( pid.GetFlags() & PID::Flag_OwnPalette )   printf("  own palettte  [10000000]\n");
    printf("---\n");

    for(int y=0;y<pid.GetHeight();y++){
     for(int x=0;x<pid.GetWidth();x++){
      pid.SetColorIdAt(x, y, gdImageGetPixel(im, x, y));
     }
    }

    pid.Save(szFileOut);

    gdImageDestroy(im);
    printf("File converted to %s.\n", szFileOut);
    delete [] szFileOut;
    return 0;
}
