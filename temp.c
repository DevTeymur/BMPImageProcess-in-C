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
    unsigned char reserve[4];
    // 4 bytes
    int endHeader; // end of header files | start of pixels (in bytes)  // <- Seek to that point after bpp in dib
}BMP;              

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
    int bypp;     // bytes per pixel
    int bipp;     // bites per pixel
    // not important
    unsigned char* arr;

}DIB;

typedef struct header{
    DIB dHeader;
    BMP bHeader;
}Header;

typedef struct pixel{   // Struct of pixel values of RGB
    // Values of RGB
    unsigned char red;   
    unsigned char green;
    unsigned char blue;
    unsigned char alpha;
}PIX;

typedef struct pixels{
    int nPixels;
    PIX *pixArr;
    PIX pixMin;
    PIX pixMax;
}Pixels;

BMP getBmp(FILE *pFile){
    BMP b;
    UNIP px;
    
    fseek(pFile,2,SEEK_SET);
    fread(px.s,sizeof(char),4,pFile);
    b.fileSize=px.n;
    // fread(b.reserve,sizeof(char),4,pFile);
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
    d.bypp=px.n;
    d.bipp=d.bypp/8;
    // Writing end of dib into array (not important info)
    int nEmptyBytes=d.sizeDib-16;
    d.arr=(unsigned char*)malloc(sizeof(char)*nEmptyBytes);
    // fread(d.arr,sizeof(char),nEmptyBytes,pFile);
    
    return d;
}

Header getHeader(FILE *pFile){
    Header fHeader;
    fHeader.dHeader=getDib(pFile);
    fHeader.bHeader=getBmp(pFile);
    return fHeader;
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
    printf("Bytes per pixel:: %d\n",header.bypp);
    printf("Bites per pixel:: %d\n",header.bipp);
    puts("----------------------------------------\n");
}

void printHeader(Header header){
    printBmp(header.bHeader);
    printDib(header.dHeader);
}

PIX getPix(FILE* pFile,int bpp){
    PIX pTemp;
    fread(&(pTemp.red),sizeof(char),1,pFile);
    fread(&(pTemp.green),sizeof(char),1,pFile);
    fread(&(pTemp.blue),sizeof(char),1,pFile);
    if(bpp==4){
        fread(&(pTemp.alpha),sizeof(char),1,pFile);
    }else{
        pTemp.alpha=0;
    }
    return pTemp;
}

PIX ifMinPix(PIX p1,PIX min){
    if(p1.red<min.red){
        min.red=p1.red;
    }
    if(p1.green<min.green){
        min.green=p1.green;
    }
    if(p1.blue<min.blue){
        min.blue=p1.blue;
    }
    if(p1.alpha<min.alpha){
        min.alpha=p1.alpha;
    }
    return min;
}

PIX ifMaxPix(PIX p1,PIX max){
    if(p1.red>max.red){
        max.red=p1.red;
    }
    if(p1.green>max.green){
        max.green=p1.green;
    }
    if(p1.blue>max.blue){
        max.blue=p1.blue;
    }
    if(p1.alpha>max.alpha){
        max.alpha=p1.alpha;
    }
    return max;
}

Pixels getPixels(FILE* pFile,Header header){
    Pixels pxs;
    int nPixels=header.dHeader.width*header.dHeader.height;
    pxs.nPixels=nPixels;
    pxs.pixArr=(PIX*)malloc(sizeof(PIX)*nPixels);
    int bpp=header.dHeader.bipp; // bites per pixel;

    PIX pTemp;
    // To the end of the header and start of the colors;
    fseek(pFile,header.bHeader.endHeader,SEEK_SET);
    for(int i=0;i<nPixels;i++){
        pTemp=getPix(pFile,bpp);
        pxs.pixArr[i]=pTemp;
        pxs.pixMin=ifMinPix(pTemp,pxs.pixMin);
        pxs.pixMax=ifMaxPix(pTemp,pxs.pixMax);
    }

    return pxs;

}

void printPixel(PIX p){
    printf("( %d %d %d %d )",p.red,p.green,p.blue,p.alpha);
}

void printPixels(Pixels pxs){
    // PIX p;
    // for(int i=0;i<pxs.nPixels;i++){
        // p=pxs.pixArr[i];
    //     printPixel(pxs.pixArr[i]);
    //     if(i%4==0){printf("\n");}
    // }
    puts("----------------------------------------------------");
    printf("Total number of pixels:: %d\n",pxs.nPixels);
    printf("Minimum Pixel:: ");printPixel(pxs.pixMin);
    printf("\nMaximum Pixel:: ");printPixel(pxs.pixMax);
    puts("\n----------------------------------------------------");
}

PIX fEditPixel(PIX px, PIX min, float coefs[]){
    PIX npx;
    npx.red=(px.red-min.red)*coefs[0];
    npx.green=(px.green-min.green)*coefs[1];
    npx.blue=(px.blue-min.blue)*coefs[2];
    npx.alpha=(px.alpha-min.alpha)*coefs[3];
    return npx;
}

float* getCoefs(PIX max, PIX min){
    float* pCoefs=(float*)malloc(sizeof(float)*4);
    if(max.red-min.red!=0){ pCoefs[0]=255/(max.red-min.red); }
    if(max.green-min.green!=0){ pCoefs[1]=255/(max.green-min.green); }
    if(max.blue-min.blue!=0){ pCoefs[2]=255/(max.blue-min.blue); }
    if(max.alpha-min.alpha!=0){ pCoefs[3]=255/(max.alpha-min.alpha); }

    return pCoefs;
} 

Pixels fAutoAdjust(Pixels pxs){
    Pixels new;
    new.nPixels=pxs.nPixels;
    new.pixArr=(PIX*)malloc(sizeof(PIX)*new.nPixels);

    PIX min=pxs.pixMin;
    PIX max=pxs.pixMax;

    float *coefs=getCoefs(max,min);
    
    for(int i=0;i<pxs.nPixels;i++){
        new.pixArr[i]=fEditPixel(pxs.pixArr[i],min,coefs);
    }
    return new;
}
#include <math.h>
int main(){

    printf("%d\n",(int)254.8);

    return 0;
}