#include <stdio.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

//gcc -o haha haha.c -Wall -lavformat -lavcodec -lswresample -lswscale -lavutil -lm

void print_argv(int argc, char *argv[]){
    
    int i;
    printf("%d\n",argc);
    for(i=0;i<argc;i++)
    {
        printf("%s ",argv[i]);
    }
    printf("\n");
}


int main(int argc, char *argv[]) {
    //print_argv(argc, argv);
    if(argc < 2){
       return 2;
    }

    printf("----------------------------\n");
    printf("avformat: %d.%d.%d\n", LIBAVFORMAT_VERSION_MAJOR, LIBAVFORMAT_VERSION_MINOR, LIBAVFORMAT_VERSION_MICRO);
    printf("avcodec: %d.%d.%d\n", LIBAVCODEC_VERSION_MAJOR, LIBAVCODEC_VERSION_MINOR, LIBAVCODEC_VERSION_MICRO);
    printf("----------------------------\n");

    av_register_all();
    AVFormatContext *pFormatCtx = NULL;
     // Open video file
    if(avformat_open_input(&pFormatCtx, argv[1], 0, 0)<0){
       printf("invalid file.");
       return 3; // Couldn't open file
    }
    
    //char a[1024];
    //strcpy(a, pFormatCtx->filename);
    //printf("%s", a);
    // Retrieve stream information
    if(avformat_find_stream_info(pFormatCtx, NULL)<0){
        return 4; // Couldn't find stream information
    }
    // Dump information about file onto standard error
    av_dump_format(pFormatCtx, 0, argv[1], 0);
    printf("-----------------------------------------------");
    printf("%d", pFormatCtx->nb_streams);
}
