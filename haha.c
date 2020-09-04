#include <stdio.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

// gcc -o haha haha.c -Wall -lavformat -lavcodec -lswresample -lswscale -lavutil -lm
// 

void print_argv(int argc, char *argv[]){
    
    int i;
    printf("%d\n",argc);
    for(i=0;i<argc;i++)
    {
        printf("%s ",argv[i]);
    }
    printf("\n");
}

void print_error(int err_code){
    char errbuf[100];
    av_strerror(err_code, errbuf, sizeof(errbuf));
    printf("error: %s \n", errbuf);
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
    
    printf("-----------------------------------------------\n");

    int i;

    // Find the first video stream
    int videoStream=-1;
    for(i=0; i<pFormatCtx->nb_streams; i++){
        if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) {
            videoStream=i;
            break;
        }
    }
        
    if(videoStream==-1){
        printf("Didn't find a video stream\n");
        return -1; // Didn't find a video stream
    }

    printf("video stream index:  %d\n", i);

    /*
    The stream's information about the codec is in what we call the "codec context." This contains all the information about the codec that the stream is using, and now we have a pointer to it. But we still have to find the actual codec and open it:
    */
    AVCodecContext *pCodecCtxOrig = NULL;
    AVCodecContext *pCodecCtx = NULL;
    // Get a pointer to the codec context for the video stream
    pCodecCtxOrig=pFormatCtx->streams[videoStream]->codec;

    AVCodec *pCodec = NULL;
    // Find the decoder for the video stream
    // Find the decoder with the CodecID id. Returns NULL on failure. 
    // This should be called after getting the desired AVCodecContext from a stream in AVFormatContext, using codecCtx->codec_id.

    pCodec=avcodec_find_decoder(pCodecCtxOrig->codec_id);
    if(pCodec==NULL) {
      fprintf(stderr, "Unsupported codec!\n");
      return -1; // Codec not found
    }

    // //  // 配置解码器
    // Copy context
    pCodecCtx = avcodec_alloc_context3(pCodec);
    if (!pCodecCtx) {
        fprintf(stderr, "Could not allocate video codec context\n");
        exit(1);
    }

    // if(avcodec_copy_context(pCodecCtxOrig, pCodecCtx) != 0) {
    //     fprintf(stderr, "Couldn't copy codec context");
    //     return -1; // Error copying codec context
    // }
    /*
    Copy the settings of the source AVCodecContext into the destination AVCodecContext. 
    The resulting destination codec context will be unopened, i.e. 
    you are required to call avcodec_open2() before you can use this AVCodecContext 
    to decode/encode video/audio data.
    dest should be initialized with avcodec_alloc_context3(NULL), but otherwise uninitialized.
    dest should be initialized with avcodec_alloc_context3(NULL), but otherwise uninitialized.
    */
    int av_cc_i = avcodec_copy_context(pCodecCtx, pCodecCtxOrig);
    
    if(av_cc_i != 0) {
        fprintf(stderr, "Couldn't copy codec context");
        print_error(av_cc_i);
        return -1; // Error copying codec context
    }


    printf("hello av\n");
}







