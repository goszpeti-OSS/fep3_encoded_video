#include "stdafx.h"

using namespace std;

//this opens a font style and sets a size
TTF_Font* Sans = NULL;
double fps=0;

// this is the color in rgb format,
SDL_Color White = { 255, 255, 255 };
uint16_t tick_interval = 20;

uint64_t next_time, time_left;

static AVBufferRef* hw_device_ctx = NULL;
static enum AVPixelFormat hw_pix_fmt;

static int hw_decoder_init(AVCodecContext* ctx, const enum AVHWDeviceType type)
{
	int err = 0;

	if ((err = av_hwdevice_ctx_create(&hw_device_ctx, type,
		NULL, NULL, 0)) < 0) {
		fprintf(stderr, "Failed to create specified HW device.\n");
		return err;
	}
	ctx->hw_device_ctx = av_buffer_ref(hw_device_ctx);

	return err;
}

static enum AVPixelFormat get_hw_format(AVCodecContext* ctx,
	const enum AVPixelFormat* pix_fmts)
{
	const enum AVPixelFormat* p;

	for (p = pix_fmts; *p != -1; p++) {
		//if (*p == hw_pix_fmt)
		return *p;
	}

	fprintf(stderr, "Failed to get HW surface format.\n");
	return AV_PIX_FMT_NONE;
}

 

Player* Player::instance = 0;
Player* Player::get_instance()
{
	if (instance == 0)
		instance = new Player();
	return instance;
}

void Player::run(std::string video_addr, std::string window_name)
{
	this->video_addr = video_addr;
	this->window_name = window_name;

	this->open();
	this->malloc();
	this->create_display();
	this->display_video();
}

void Player::open()
{
	audioStream = -1;

	// open video
	//int res = avformat_open_input(&pFormatCtx, this->video_addr.c_str(), NULL, NULL);

	avdevice_register_all();
	const AVInputFormat* iformat = av_find_input_format("dshow");
	printf("========Device Info=============\n");
	AVDeviceInfoList* device_list = NULL;
	int result = avdevice_list_input_sources(iformat, NULL, NULL, &device_list);

	if (result < 0)
		printf("Error Code:%s\n", ""); //, av_err2str(result));//Returns -40 AVERROR(ENOSYS)
	else printf("Devices count:%d\n", result);

	auto device = device_list->devices[0];
	int res = 0;
    AVDictionary* options = NULL;
    av_dict_set(&options, "rtbufsize", "2000M", 0);
	if ((res = avformat_open_input(&pFormatCtx, (std::string("video=") + device->device_name).c_str(), iformat, &options)) < 0) {
		av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
		return;
	}

	// check video
	if (res != 0)
		Utils::display_ffmpeg_exception(res);

	// get video info
	res = avformat_find_stream_info(pFormatCtx, NULL);
	if (res < 0)
		Utils::display_ffmpeg_exception(res);

	// get video stream
	videoStream = get_video_stream();
	if (videoStream == -1)
		Utils::display_exception("Error opening your video using AVCodecParameters, probably doesnt have codecpar_type type AVMEDIA_TYPE_VIDEO");
	
	auto fr = av_guess_frame_rate(pFormatCtx, pFormatCtx->streams[videoStream], NULL);
	tick_interval = (double (fr.den) / double(fr.num) * double(1000));
	// open
	read_audio_video_codec();
	av_dump_format(pFormatCtx, 0, device->device_description, 0);


}

void Player::clear()
{
	// close context info
	avformat_close_input(&pFormatCtx);
	avcodec_free_context(&pCodecCtx);

	// free buffers
	av_free(buffer);
	av_free(pFrameRGB);

	// Free the YUV frame
	av_free(pFrame);

	// Close the codecs
	avcodec_close(pCodecCtx);

	// Close the video file
	avformat_close_input(&pFormatCtx);
	av_buffer_unref(&hw_device_ctx);

	delete Player::get_instance();
}

/*
Acquires video stream
*/
int Player::get_video_stream(void)
{
	int videoStream = -1;

	for (unsigned int i = 0; i<pFormatCtx->nb_streams; i++){
		if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) videoStream = i;
		if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) audioStream = i;
	}

	if (videoStream == -1)
		Utils::display_exception("Couldnt find stream");

	pCodecParameters = pFormatCtx->streams[videoStream]->codecpar;
	if(audioStream != -1) pCodecAudioParameters = pFormatCtx->streams[audioStream]->codecpar;

	return videoStream;
}

/*
Reads audio and video codec
*/
int Player::read_audio_video_codec(void) 
{
	pCodec = avcodec_find_decoder(pCodecParameters->codec_id);
	if (pCodecAudioParameters) {
		pAudioCodec = avcodec_find_decoder(pCodecAudioParameters->codec_id);
	}
	if (pCodec == NULL)
		Utils::display_exception("Video decoder not found");

	//if (pAudioCodec == NULL) 
	//	Utils::display_exception("Audio decoder not found");

	pCodecCtx = avcodec_alloc_context3(pCodec);

	if(pCodecCtx == NULL)
		Utils::display_exception("Failed to allocate video context decoder");

	//pCodecAudioCtx = avcodec_alloc_context3(pAudioCodec);

	//if(pCodecAudioCtx == NULL)
	//	Utils::display_exception("Failed to allocate audio context decoder");

	int res = avcodec_parameters_to_context(pCodecCtx, pCodecParameters);

	if(res < 0)
		Utils::display_exception("Failed to transfer video parameters to context");

	//res = avcodec_parameters_to_context(pCodecAudioCtx, pCodecAudioParameters);

	//if (res < 0) 
	//	Utils::display_exception("Failed to transfer audio parameters to context");
	pCodecCtx->get_format = get_hw_format;

	std::string hw_type_str = "d3d11va";
	auto type = av_hwdevice_find_type_by_name(hw_type_str.c_str());
	if (type == AV_HWDEVICE_TYPE_NONE) {
		fprintf(stderr, "Device type %s is not supported.\n", hw_type_str.c_str());
		fprintf(stderr, "Available device types:");
		while ((type = av_hwdevice_iterate_types(type)) != AV_HWDEVICE_TYPE_NONE)
			fprintf(stderr, " %s", av_hwdevice_get_type_name(type));
		fprintf(stderr, "\n");
		return -1;
	}
	if (hw_decoder_init(pCodecCtx, type) < 0)
		return -1;

	res = avcodec_open2(pCodecCtx, pCodec, NULL);

	if(res < 0)
		Utils::display_exception("Failed to open video codec");

	//res = avcodec_open2(pCodecAudioCtx, pAudioCodec, NULL);

	//if (res < 0)
	//	Utils::display_exception("Failed to open auvio codec");

	return 1;
}

/*
Alloc memory for the display
*/
int Player::malloc(void)
{
	// Audio::get_instance()->malloc(pCodecAudioCtx);

	//Audio::get_instance()->open();

	pFrame = av_frame_alloc();
	if (pFrame == NULL)
		Utils::display_exception("Couldnt allocate frame memory");

	pFrameRGB = av_frame_alloc();
	if (pFrameRGB == NULL)
		Utils::display_exception("Couldnt allocate rgb frame memory");

	int numBytes = av_image_get_buffer_size(VIDEO_FORMAT, pCodecCtx->width, pCodecCtx->height,1);

	buffer = (uint8_t *)av_malloc(numBytes*sizeof(uint8_t));

	int res = av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, buffer, VIDEO_FORMAT, pCodecCtx->width, pCodecCtx->height, 1);
	if (res < 0)
		Utils::display_ffmpeg_exception(res);
	return 1;
}


int Player::getAudioPacket(AudioPacket* q, AVPacket* pkt, int block){

	AVPacketList* pktl;
    int ret;

    SDL_LockMutex(q->mutex);

    while (1)
    {
        pktl = q->first;
        if (pktl)
        {
            q->first = pktl->next;
            if (!q->first)
                q->last = NULL;

            q->nb_packets--;
            q->size -= pktl->pkt.size;

            *pkt = pktl->pkt;
            av_free(pktl);
            ret = 1;
            break;
        }
        else if (!block)
        {
            ret = 0;
            break;
        }
        else
        {
            SDL_CondWait(q->cond, q->mutex);
        }
    }

    SDL_UnlockMutex(q->mutex);

    return ret;
}

/*
Read frames and display
*/
int Player::display_video(void) {

	AVPacket packet;

	//video context
	sws_ctx = sws_getContext(pCodecCtx->width,
		pCodecCtx->height,
		pCodecCtx->pix_fmt,
		pCodecCtx->width,
		pCodecCtx->height,
		VIDEO_FORMAT,
		SWS_BILINEAR,
		NULL,
		NULL,
		NULL
		);
	SDL_Event evt;
	auto start_time = SDL_GetTicks();
	uint64_t frame_count = 0;
	char fps_chars[5];
	SDL_SetYUVConversionMode(SDL_YUV_CONVERSION_JPEG);

	while (av_read_frame(pFormatCtx, &packet) >= 0) {
		start_time = SDL_GetTicks();
		frame_count++;
		// do stuff

		if (packet.stream_index == audioStream) {
			//Audio::get_instance()->put_audio_packet(&packet);
			continue;
		}

		if (packet.stream_index == videoStream)
		{
			int res = avcodec_send_packet(pCodecCtx, &packet);
			if (res < 0)
				Utils::display_ffmpeg_exception(res);

			res = avcodec_receive_frame(pCodecCtx, pFrame);
			// hw decode
			AVFrame* sw_frame = NULL;
			AVFrame* tmp_frame = NULL;
			int ret = 0;

			sw_frame = av_frame_alloc();

			if (pFrame->format == hw_pix_fmt) {
				/* retrieve data from GPU to CPU */
				if ((ret = av_hwframe_transfer_data(sw_frame, pFrame, 0)) < 0) {
					fprintf(stderr, "Error transferring the data to system memory\n");
				}
				tmp_frame = sw_frame;
			}
			else
				tmp_frame = pFrame;


			 //if (pFrame->linesize[0] > 0 && pFrame->linesize[1] > 0 && pFrame->linesize[2] > 0) {
				 SDL_UpdateYUVTexture(bmp, NULL,
					 tmp_frame->data[0], tmp_frame->linesize[0],
					 tmp_frame->data[1], tmp_frame->linesize[2],
					 tmp_frame->data[2], tmp_frame->linesize[2]);
			 //}
			 //else if (pFrame->linesize[0] < 0 && pFrame->linesize[1] < 0 && pFrame->linesize[2] < 0) {
				// SDL_UpdateYUVTexture(bmp, NULL, pFrame->data[0] + pFrame->linesize[0] * (pFrame->height - 1), -pFrame->linesize[0],
				//	 pFrame->data[1] + pFrame->linesize[1] * (AV_CEIL_RSHIFT(pFrame->height, 1) - 1), -pFrame->linesize[1],
				//	 pFrame->data[2] + pFrame->linesize[2] * (AV_CEIL_RSHIFT(pFrame->height, 1) - 1), -pFrame->linesize[2]);
			 // }
			  
			 SDL_RenderCopy(renderer, bmp, NULL, NULL);

			if (frame_count % 4 == 0) { // only every 5th frame
				sprintf(fps_chars, "%4.1f", fps);
			}
			// as TTF_RenderText_Solid could only be used on
			// SDL_Surface then you have to create the surface first
			SDL_Surface* surfaceMessage = TTF_RenderText_Solid(Sans, fps_chars, White);

			// now you can convert it into a texture
			SDL_Texture* Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);

			SDL_Rect Message_rect; //create a rect
			Message_rect.x = 0;  //controls the rect's x coordinate 
			Message_rect.y = 0; // controls the rect's y coordinte
			Message_rect.w = 100; // controls the width of the rect
			Message_rect.h = 100; // controls the height of the rect

			SDL_RenderCopy(renderer, Message, NULL, &Message_rect);

			// // Don't forget to free your surface and texture
			SDL_FreeSurface(surfaceMessage);
			SDL_DestroyTexture(Message);


			SDL_RenderPresent(renderer);
			SDL_UpdateWindowSurface(screen);
			auto now = SDL_GetTicks();
			if (next_time <= now) {
				time_left = 0;
			}
			else {
				time_left = next_time - now;
			}
			SDL_Delay(time_left);
			next_time += tick_interval;
		}

		SDL_PollEvent(&evt);

		av_packet_unref(&packet);

		auto frame_time = SDL_GetTicks() - start_time;
		fps = (frame_time > 0) ? 1000.0f / frame_time : 0.0f;
		std::cout << fps << std::endl;

        if (SDL_QUIT == evt.type)
        {
            exit(0);
        }
	}

	return 1;

}

/*
Create the display for the received video
*/
int Player::create_display(void) 
{
	screen = SDL_CreateWindow(window_name.c_str(),
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			800, 600,
			//pCodecCtx->width, pCodecCtx->height,
			SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	
	if (!screen)
		Utils::display_exception("Couldn't show display window");

	renderer = SDL_CreateRenderer(screen, -1, 0);
	
 // SDL_PIXELFORMAT_YV12 SDL_TEXTUREACCESS_STATIC AV_PIX_FMT_YUVJ422P
	bmp = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_YV12, SDL_TEXTUREACCESS_STREAMING, pCodecCtx->width, pCodecCtx->height);

	TTF_Init();
	Sans = TTF_OpenFont("C:/Repos/fep3/fep3_encoded_video/build/src/ffmpeg-playground/player/RelWithDebInfo/Franzo-E4GA.ttf", 24);


	return 1;
}