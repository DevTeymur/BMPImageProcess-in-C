#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef union PIX{
    int n;
    char s[4];
}UNIP;

typedef struct bmp{
    // file format for 2 bytes (SEEK>2);
    // 4 bytes
    int fileSize;  // size of file in decimal (pixels?bytes?bites)
    // 4 empty bytes (SEEK>10)
    // 4 bytes
    int endHeader; // end of header files | start of pixels (in bytes) 
}BMP;              // ^^ Seek to that point after bpp in dib

typedef struct dib{
    // 4 bytes
    int sizeDib; // bytes for dib header  
    // 4 bytes
    int width;   // image width
    // 4 bytes
    int height;  // image height
    // 2 bytes
    int cplane;  // color plane - 1
    /// 2 bytes
    int bpp;     // bits per pixel
}DIB;

BMP getBmp(FILE *pFile){
    BMP b;
    UNIP px;
    
    fseek(pFile,2,SEEK_SET);
    fread(px.s,sizeof(char),4,pFile);
    b.fileSize=px.n;
    fseek(pFile,10,SEEK_SET);
    fread(px.s,sizeof(char),4,pFile);
    b.endHeader=px.n;

    return b;
} 


DIB getDib(FILE *pFile){
    DIB d;
    UNIP px;

    // Seeking cursor just in case
    fseek(pFile,14,SEEK_SET);
    fread(px.s,sizeof(char),4,pFile);
    d.sizeDib=px.n;
    fread(px.s,sizeof(char),4,pFile);
    d.width=px.n;
    fread(px.s,sizeof(char),4,pFile);
    d.height=px.n;
    fread(px.s,sizeof(char),2,pFile);
    d.cplane=px.n;
    fread(px.s,sizeof(char),2,pFile);
    d.bpp=px.n;
    
    return d;
}


void printBmp(BMP header){
    puts("--------------- BMP INFO ---------------\n");
    printf("Size of file:: %d\n",header.fileSize);
    printf("Size of headers:: %d\n",header.endHeader);
    puts("----------------------------------------\n");
}

void printDib(DIB header){
    puts("--------------- DIB INFO ---------------\n");
    printf("Size of DIB:: %d\n",header.sizeDib);
    printf("Width of file:: %d\n",header.width);
    printf("Height of file:: %d\n",header.height);
    printf("Color Plane:: %d\n",header.cplane);
    printf("Bytes per pixel:: %d\n",header.bpp);
    puts("----------------------------------------\n");
}
int main(){

    FILE *pfile=fopen("examples/example2.bmp","r");
    BMP h1=getBmp(pfile);
    printBmp(h1);
    DIB h2=getDib(pfile);
    printDib(h2);
    fclose(pfile);

    return 0;
}