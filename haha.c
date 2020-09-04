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
    /*
    Note that we must not use the AVCodecContext from the video stream directly! 
    So we have to use avcodec_copy_context() to copy the context to a new location (after allocating memory for it, of course).
    */
    int av_cc_i = avcodec_copy_context(pCodecCtx, pCodecCtxOrig);
    
    if(av_cc_i != 0) {
        fprintf(stderr, "Couldn't copy codec context");
        print_error(av_cc_i);
        return -1; // Error copying codec context
    }

    // AVCodecContext *dest, avcodec_copy_context you are required to call avcodec_open2() before you can use this AVCodecContext to decode/encode video/audio data.
     // Open codec
    if(avcodec_open2(pCodecCtx, pCodec, NULL)<0){
        return -1; // Could not open codec
    }

    // AVFrame中存储的是经过解码后的原始数据。在解码中，AVFrame是解码器的输出；在编码中，AVFrame是编码器的输入。
    AVFrame *pFrame = NULL;

    // Allocate video frame
    pFrame=av_frame_alloc();
    if(pFrame==NULL){
        printf("av_frame_alloc error\n");
        return -1;
    }

    // Since we're planning to output PPM files, which are stored in 24-bit RGB, 
    // we're going to have to convert our frame from its native format to RGB. 
    // ffmpeg will do these conversions for us. For most projects (including ours)
    // we're going to want to convert our initial frame to a specific format. Let's allocate a frame for the converted frame now.
    AVFrame *pFrameRGB = NULL;
    pFrameRGB=av_frame_alloc();

    if(pFrameRGB==NULL){
        printf("av_frame_alloc error\n");
        return -1;
    }

    /*
    Even though we've allocated the frame, we still need a place to put the raw data when we convert it. 
    We use avpicture_get_size to get the size we need, and allocate the space manually:
    */
    uint8_t *buffer = NULL;
    int numBytes;
    // Determine required buffer size and allocate buffer
    // Calculates how many bytes will be required for a picture of the given width, height, and pic format.
    numBytes=avpicture_get_size(PIX_FMT_RGB24, pCodecCtx->width,
                            pCodecCtx->height);

    // Memory allocation of size byte with alignment suitable for all memory accesses (including vectors if available on the CPU). av_malloc(0) must return a non NULL pointer.
    // av_malloc is ffmpeg's malloc that is just a simple wrapper around malloc that makes sure the memory addresses are aligned and such. It will not protect you from memory leaks, double freeing, or other malloc problems.
    buffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));

    // Now we use avpicture_fill to associate the frame with our newly allocated buffer. About the AVPicture cast: the AVPicture struct is a subset of the AVFrame struct - the beginning of the AVFrame struct is identical to the AVPicture struct.

    // Assign appropriate parts of buffer to image planes in pFrameRGB
    // Note that pFrameRGB is an AVFrame, but AVFrame is a superset
    // of AVPicture
    avpicture_fill((AVPicture *)pFrameRGB, buffer, PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);


    // ----------------------------------
    // Reading the Data
    // ----------------------------------
    // struct SwsContext  （software scale） 主要用于视频图像的转换，比如格式转换：
    // struct SwrContext   （software resample） 主要用于音频重采样，比如采样率转换，声道转换
    // What we're going to do is read through the entire video stream by reading in the packet, decoding it into our frame, and once our frame is complete, we will convert and save it.
    
    printf("hello av\n");
}







