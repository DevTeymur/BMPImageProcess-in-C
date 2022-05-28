#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

// ----------------------------------------------------------------------

char help[]="Usage:\nRedirect the output to the file \nexample::\n$ autoadjust image.bmp > image2.bmp\nOr mention flag “-o” to output the result into an output file \nexample::\n$ autoadjust image.bmp -o image2.bmp\n";

// Union for converting char-integer or viceversa
typedef union unionPix{
    int n;
    char s[4];
}UNIP;

// Properties of BMP header
typedef struct bmp{
    // file format for 2 bytes (SEEK>2);
    unsigned char format[2];
    // 4 bytes
    int fileSize;  // size of file in decimal (pixels?bytes?bites)
    // 4 empty bytes (SEEK>10)
    unsigned char reserve[4];
    // 4 bytes
    int endHeader; // end of header files | start of pixels (in bytes)  // <- Seek to that point after bpp in dib
}BMP;              

// Properties of DIB header
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
    int bipp;     // bits per pixel
    // not important
    unsigned char* arr;

}DIB;

// Combination of headers
typedef struct header{
    DIB dHeader;
    BMP bHeader;
    unsigned char *heads;
}Header;

// RGB and Alpha values of one pixel
typedef struct pixel{   // Struct of pixel values of RGB
    // Values of RGB
    unsigned char alpha;
    unsigned char blue;
    unsigned char green;
    unsigned char red;   
}PIX;

// Struct for collecting all pixels from file and extra needed data
typedef struct pixels{
    int nPixels; // all number of pixels
    PIX *pixArr; // array of pixels
    int pixMin;  // min RGB values of file (red, green, blue can be from different pixels);
    int pixMax;  // max RGB values of file (red, green, blue can be from different pixels);
}Pixels;

// Reading BMP header
BMP fGetBmp(FILE *pFile){
    BMP b;
    UNIP px; 
    
    fseek(pFile,0,SEEK_SET);
    fread(b.format,sizeof(char),2,pFile);
    fread(&(b.fileSize),sizeof(char),4,pFile);
    fread(b.reserve,sizeof(char),4,pFile);
    fread(&(b.endHeader),sizeof(char),4,pFile);
    return b;
} 

// Reading DIB header
DIB fGetDib(FILE *pFile){
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
    d.bipp=px.n;
    d.bypp=d.bipp/8;
    // Writing end of dib into array (not important info)
    int nEmptyBytes=d.sizeDib-16;
    d.arr=(unsigned char*)malloc(sizeof(char)*nEmptyBytes);
    fread(d.arr,sizeof(char),nEmptyBytes,pFile);
    
    return d;
}

// Getting the whole header of file
Header fGetHeader(FILE *pFile){
    Header fHeader;
    fHeader.dHeader=fGetDib(pFile);
    fHeader.bHeader=fGetBmp(pFile);
    int end=fHeader.bHeader.endHeader;
    fHeader.heads=(unsigned char*)malloc(sizeof(unsigned char)*end);

    // Reading the full header data into array to easily write in new file after processing
    fseek(pFile,0,SEEK_SET);
    fread(fHeader.heads,sizeof(unsigned char),end,pFile);
    return fHeader;
}

// Print functions
void fPrintBmp(BMP header){
    puts("--------------- BMP INFO ---------------\n");
    printf("Size of file:: %d\n",header.fileSize);
    printf("Size of headers:: %d\n",header.endHeader);
    puts("----------------------------------------\n");
}

void fPrintDib(DIB header){
    puts("--------------- DIB INFO ---------------\n");
    printf("Size of DIB:: %d\n",header.sizeDib);
    printf("Width of file:: %d\n",header.width);
    printf("Height of file:: %d\n",header.height);
    printf("Color Plane:: %d\n",header.cplane);
    printf("Bytes per pixel:: %d\n",header.bypp);
    printf("Bites per pixel:: %d\n",header.bipp);
    puts("----------------------------------------\n");
}

void fPrintHeader(Header header){
    fPrintBmp(header.bHeader);
    fPrintDib(header.dHeader);
}

void fPrintPixel(PIX p){
    printf("( %d %d %d %d )",p.red,p.green,p.blue,p.alpha);
}

void fPrintPixelsInfo(Pixels pxs){
    puts("----------------------------------------------------");
    printf("Total number of pixels:: %d\n",pxs.nPixels);
    printf("Minimum Pixel:: %d\n",pxs.pixMin);
    printf("\nMaximum Pixel:: %d\n",pxs.pixMax);
    puts("\n----------------------------------------------------");
}

// Reading one pixel from file
PIX fGetPix(FILE* pFile,int bpp){
    PIX pTemp;
    if(bpp==4){ // Condition if there alpha value or not
        fread(&(pTemp.alpha),sizeof(char),1,pFile);
    }else{
        pTemp.alpha=0;
    }
    fread(&(pTemp.blue),sizeof(char),1,pFile);
    fread(&(pTemp.green),sizeof(char),1,pFile);
    fread(&(pTemp.red),sizeof(char),1,pFile);
    
    return pTemp;
}

// Checking if the values current pixel are less than minimum or not
int ifMinPix(PIX p1,int min){
    if(p1.red<min){
        min=p1.red;
    }
    if(p1.green<min){
        min=p1.green;
    }
    if(p1.blue<min){
        min=p1.blue;
    }
    return min;
}

// Checking if the values current pixel are more than maiximum or not
int ifMaxPix(PIX p1,int max){
    if(p1.red>max){
        max=p1.red;
    }
    if(p1.green>max){
        max=p1.green;
    }
    if(p1.blue>max){
        max=p1.blue;
    }
    return max;
}

// Dynamically initializing structure Pixels (nPixels, pixArr)
Pixels fInitPixels(Header header){
    Pixels pxs;
    int nPixels=header.dHeader.width*header.dHeader.height;
    pxs.nPixels=nPixels;
    pxs.pixArr=(PIX*)malloc(sizeof(PIX)*nPixels);
    return pxs;

}

// Reading all pixels from file, detecting min and max values
Pixels fGetPixels(FILE* pFile,Header header){
    Pixels pxs=fInitPixels(header);
    PIX pTemp;
    int nPixels=pxs.nPixels;
    int bpp=header.dHeader.bypp; // bites per pixel;
    // To the end of the header and start of the colors;
    fseek(pFile,header.bHeader.endHeader,SEEK_SET);
    for(int i=0;i<nPixels;i++){
        pTemp=fGetPix(pFile,bpp);
        if(i==0){
            // Initialize min and max
            pxs.pixMin=pTemp.red;
            pxs.pixMax=pTemp.red;
        }else{
            pxs.pixMin=ifMinPix(pTemp,pxs.pixMin);
            pxs.pixMax=ifMaxPix(pTemp,pxs.pixMax);
        }
        pxs.pixArr[i]=pTemp;
    }
    return pxs;
}

// Finding maximizing coefficients for RGBA
float* fGetCoefs(int max, int min){
    float* pCoefs=(float*)malloc(sizeof(float)*4);

    if(max-min!=0){  pCoefs[0]=(255.0/(max-min)); }
    if(max-min!=0){ pCoefs[1]=255.0/(max-min); }
    if(max-min!=0){ pCoefs[2]=255.0/(max-min); }
    return pCoefs;
} 

// Modifying pixel respectively to minimum and maximum pixel values
PIX fEditPixel(PIX px, int min, float coefs[]){
    PIX npx;
    npx.red=(int)round((px.red-min)*coefs[0]);
    npx.green=(int)round((px.green-min)*coefs[1]);
    npx.blue=(int)round((px.blue-min)*coefs[2]);
    npx.alpha=px.alpha;
    return npx;
}

// Autoadjusting image brightness and contrast with previously mentioned method
// Applying fEditPixel to all pixels of file
Pixels fAutoAdjust(Pixels pxs){
    Pixels new;
    new.nPixels=pxs.nPixels;
    new.pixArr=(PIX*)malloc(sizeof(PIX)*new.nPixels);

    int min=pxs.pixMin;
    int max=pxs.pixMax;
    float *coefs=fGetCoefs(max,min);

    for(int i=0;i<pxs.nPixels;i++){
        new.pixArr[i]=fEditPixel(pxs.pixArr[i],min,coefs);
    }
    return new;
}

// Writing ehader info into new file, or to stdout
void fWriteHeader(FILE* pfile,Header header){
    BMP b=header.bHeader;
    DIB d=header.dHeader;
    if(pfile!=NULL){
        fseek(pfile,0,SEEK_SET);
        fwrite(header.heads,sizeof(unsigned char),b.endHeader,pfile);
    }else{
        fwrite(header.heads,sizeof(unsigned char),b.endHeader,stdout);
    }

}

// Writing one pixel to new file (flag for aplha)
void fWritePix(FILE *pfile,PIX px,int flag){
    if(pfile==NULL){pfile=stdout;}

    if(flag){fwrite(&(px.alpha),sizeof(unsigned char),1,pfile);}
    fwrite(&(px.blue),sizeof(unsigned char),1,pfile);
    fwrite(&(px.green),sizeof(unsigned char),1,pfile);    
    fwrite(&(px.red),sizeof(unsigned char),1,pfile);
}

// Writing all pixels to new file
int fWritePixels(FILE *pfile,Pixels pxs, Header header){
    
    fWriteHeader(pfile,header);
    int flag=header.dHeader.bypp-3;
    for(int i=0;i<pxs.nPixels;i++){
        fWritePix(pfile,pxs.pixArr[i],flag);
    }
    if(pfile!=NULL){fclose(pfile);}
    return 1;
}

// Check if the file is in .bmp format or asked for help message
int fCheckFileInput(char* file){
    char *dot=strrchr(file, '.');
    if(dot!=NULL && strcmp(dot,".bmp")==0){return 1;}
    else{return -1;}
}

// Filtering and checking the inputs
int fGetFiles(char **filename1, char **filename2,int argc, char **argv){
    if(argc==2){
        if(strcmp(argv[1],"--help")==0){
            printf("%s",help);
            return -1;
        }
        if(fCheckFileInput(argv[1])!=-1){
            *filename1=(char*)malloc(sizeof(char)*strlen(argv[1]));
            strcpy(*filename1,argv[1]); 
        }
    }
    else if(argc>2){
        int opt;
        while ((opt = getopt(argc, argv, "o:")) != -1) {
            switch (opt) {
                case 'o':
                    if(fCheckFileInput(optarg)!=-1 && fCheckFileInput(argv[1])!=-1){
                        *filename1=(char*)malloc(sizeof(char)*strlen(argv[1]));
                        strcpy(*filename1,argv[1]); 
                        *filename2=(char*)malloc(sizeof(char)*strlen(optarg));
                        strcpy(*filename2,optarg);  
                    }
                    break;
                default:{
                    puts("invalid flag");
                }
            }
        }
    }

    if(*filename1!=NULL){
        if(*filename2!=NULL){
            return 1;
        }
        return 0;
    }
    else{
        puts("ERROR: you must provide a .bmp file name");
        return -1;
    }
}

int main(int argc, char** argv){
    char *filename1=NULL;
    char *filename2=NULL;
    FILE *pfile1;
    FILE *pfile2=NULL;
    int flag=fGetFiles(&filename1,&filename2,argc,argv);
    if(flag==-1){
        return 0;
    }
    if((pfile1=fopen(filename1,"r"))==NULL){perror("File doesn't exist!"); return 0;}

    Header hheader=fGetHeader(pfile1);
    Pixels pxs=fGetPixels(pfile1,hheader);
    Pixels new=fAutoAdjust(pxs);

    if(flag==1){
        if((pfile2=fopen(filename2,"w"))==NULL){perror("File doesn't exist!"); return 0;};
    }
    
    fWritePixels(pfile2,new,hheader);

    fclose(pfile1);

    return 0;
}