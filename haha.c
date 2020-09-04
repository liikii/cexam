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
    numBytes=avpicture_get_size(AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);

    // Memory allocation of size byte with alignment suitable for all memory accesses (including vectors if available on the CPU). av_malloc(0) must return a non NULL pointer.
    // av_malloc is ffmpeg's malloc that is just a simple wrapper around malloc that makes sure the memory addresses are aligned and such. It will not protect you from memory leaks, double freeing, or other malloc problems.
    buffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));

    // Now we use avpicture_fill to associate the frame with our newly allocated buffer. About the AVPicture cast: the AVPicture struct is a subset of the AVFrame struct - the beginning of the AVFrame struct is identical to the AVPicture struct.

    // Assign appropriate parts of buffer to image planes in pFrameRGB
    // Note that pFrameRGB is an AVFrame, but AVFrame is a superset
    // of AVPicture
    // 这个函数的使用本质上是为已经分配的空间的结构体AVPicture挂上一段用于保存数据的空间，这个结构体中有一个指针数组data[4]，挂在这个数组里。一般我们这么使用：
    avpicture_fill((AVPicture *)pFrameRGB, buffer, AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);


    // ----------------------------------
    // Reading the Data
    // ----------------------------------
    // struct SwsContext  （software scale） 主要用于视频图像的转换，比如格式转换：
    // struct SwrContext   （software resample） 主要用于音频重采样，比如采样率转换，声道转换
    // What we're going to do is read through the entire video stream by reading in the packet, decoding it into our frame, and once our frame is complete, we will convert and save it.
    // The struct in which raw packet data is stored. This data should be given to avcodec_decode_audio2 or avcodec_decode_video to to get a frame.
    struct SwsContext *sws_ctx = NULL;
    int frameFinished;
    // 
    /*
    1.AVPacket简介
    AVPacket是FFmpeg中很重要的一个数据结构，它保存了解复用（demuxer)之后，解码（decode）之前的数据（仍然是压缩后的数据）和关于这些数据的一些附加的信息，如显示时间戳（pts），解码时间戳（dts）,数据时长（duration），所在流媒体的索引（stream_index）等等。
    对于视频（Video）来说，AVPacket通常包含一个压缩的Frame；而音频（Audio）则有可能包含多个压缩的Frame。并且，一个packet也有可能是空的，不包含任何压缩数据data，只含有边缘数据side data（side data,容器提供的关于packet的一些附加信息，例如，在编码结束的时候更新一些流的参数,在另外一篇av_read_frame会介绍）
    AVPacket的大小是公共的ABI(Public ABI)一部分，这样的结构体在FFmpeg很少，由此也可见AVPacket的重要性，它可以被分配在栈空间上（可以使用语句AVPacket pkt;在栈空间定义一个Packet），
    3.AVPacket中的内存管理

    AVPacket实际上可看作一个容器，它本身并不包含压缩的流媒体数据，而是通过data指针引用数据的缓存空间。所以将Packet作为参数传递的时候，就要根据具体的需求，对data引用的这部分数据缓存空间进行特殊的处理。当从一个Packet去创建另一个Packet的时候，有两种情况：

    1）两个Packet的data引用的是同一数据缓存空间，这个时候要注意数据缓存空间的释放问题和修改问题（相当于iOS的retain）

    2）两个Packet的data引用不同的数据缓存空间，每个Packet都有数据缓存空间的copy

    AVPacket中的AVBufferRef *buf;就是用来管理这个引用计数的，AVBufferRef有两个函数：av_packet_ref() 和av_packet_unref()增加和减少引用计数的，AVBufferRef的声明如下：
     操作AVPacket的函数大约有30个，主要分为：AVPacket的创建初始化，AVPacket中的data数据管理（clone，free,copy），AVPacket中的side_data数据管理。

        void av_init_packet(AVPacket *pkt);

              初始化packet的值为默认值，该函数不会影响data引用的数据缓存空间和size，需要单独处理。

        int av_new_packet(AVPacket *pkt, int size);

                av_init_packet的增强版，不但会初始化字段，还为data分配了存储空间

        AVPacket *av_packet_alloc(void);

                  创建一个AVPacket，将其字段设为默认值（data为空，没有数据缓存空间）。

        void av_packet_free(AVPacket **pkt);

                   释放使用av_packet_alloc创建的AVPacket，如果该Packet有引用计数（packet->buf不为空），则先调用av_packet_unref。

        AVPacket *av_packet_clone(const AVPacket *src);

                  其功能是av_packet_alloc和av_packet_ref

        int av_copy_packet(AVPacket *dst, const AVPacket *src);

                 复制一个新的packet，包括数据缓存

        int av_copy_packet_side_data(AVPacket *dst, const AVPacket *src);

                初始化一个引用计数的packet，并指定了其数据缓存

        int av_grow_packet(AVPacket *pkt, int grow_by);

                    增大Packet->data指向的数据缓存

        void av_shrink_packet(AVPacket *pkt, int size);

                减小Packet->data指向的数据缓存

        3.1 废弃函数介绍 ------> av_dup_packet和av_free_packet

        int av_dup_packet(AVPacket *pkt);

                复制src->data引用的数据缓存，赋值给dst。也就是创建两个独立packet，这个功能现在可用使用函数av_packet_ref来代替

        void av_free_packet(AVPacket *pkt);

                释放packet，包括其data引用的数据缓存，现在可以使用av_packet_unref代替

        3.2 函数对比 --------->av_free_packet和av_packet_free

        void av_free_packet(AVPacket *pkt);

                    只是清空里边的数据内容，内存地址仍然在。我的版本是3.3已经废弃，所以用av_packet_unref替代。

            如果不清空会发生什么情况呢，举个简单的例子，一个char数组大小为128，里面有100个自己的内容。第二次使用你没有清空第一次的内容，第二次的数据大小为60，那么第一次的最后40个字节的数据仍会保留，造成数据冗余，极大可能对你的处理造成影响（这个跟自己的处理有关系，并不一定）。

        void av_packet_free(AVPacket **pkt);

                    类似于free(p); p = Null;不仅清空内容还清空内存（一般就是如果用了av_packet_alloc后就要调用av_packet_free来释放。但如果有引用计数，在调用av_packet_free前一般先调用av_packet_unref）


    */
    AVPacket packet;
    // initialize SWS context for software scaling
    sws_ctx = sws_getContext(pCodecCtx->width,
        pCodecCtx->height,
        pCodecCtx->pix_fmt,
        pCodecCtx->width,
        pCodecCtx->height,
        AV_PIX_FMT_RGB24,
        SWS_BILINEAR,
        NULL,
        NULL,
        NULL
        );

    printf("hello av\n");
}







