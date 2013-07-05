//
//  VideoPlayer.cpp
//  IbexMac
//
//  Created by Hesham Wahba on 4/30/13.
//  Copyright (c) 2013 Hesham Wahba. All rights reserved.
//

#include "VideoPlayer.h"

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include "../opengl_helpers.h"

#include <iostream>
#include <chrono>
#include <thread>
#include <stdint.h>

#define MAX_VIDEO_QUEUE_SIZE 2
#define MAX_AUDIO_FRAME_QUEUE_SIZE 40
#define NUM_BUFFERS 3
#define BUFFER_SIZE 20480

#ifdef _WIN32

#ifdef _USE_XAUDIO2
#include <xaudio2.h>
#endif

#include<Objbase.h>
#include<Mmreg.h>
#include <MMDeviceAPI.h>
#include <AudioClient.h>
#include <AudioPolicy.h>

#define SAFE_RELEASE(p) if((p) != NULL) { (p)->Release(); (p) = NULL; }

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);

#define MONO SPEAKER_FRONT_CENTER
#define STEREO (SPEAKER_FRONT_LEFT|SPEAKER_FRONT_RIGHT)

class Ibex::WindowsAudioSource {
private:
	Ibex::VideoPlayer *player;
	AVCodecContext *avAudioCodecCtx;
public:
	WindowsAudioSource(Ibex::VideoPlayer *player_, AVCodecContext *avAudioCodecCtx_) : player(player_),avAudioCodecCtx(avAudioCodecCtx_),remainingBytes(-1) {
	}
	HRESULT AdjustFormat(WAVEFORMATEX *waveFormatEx) {
		if(avAudioCodecCtx == 0) return 0;
    
		int channels, bits;
		channels = avAudioCodecCtx->channels;//2;//av_frame_get_channels(avAudioFrame);
		bits = avAudioCodecCtx->bits_per_coded_sample;
		//channels = 1;

		unsigned int frequency = avAudioCodecCtx->sample_rate;

		waveFormatEx->wFormatTag = WAVE_FORMAT_PCM; //WAVE_FORMAT_EXTENSIBLE;
		waveFormatEx->nChannels = channels;//(channels == 2) ? STEREO : MONO;
		waveFormatEx->cbSize = 0;//sizeof(WAVEFORMATEXTENSIBLE)-sizeof(WAVEFORMATEX);//bits;
		waveFormatEx->wBitsPerSample = bits;
		waveFormatEx->nSamplesPerSec = frequency;
		waveFormatEx->nBlockAlign = waveFormatEx->nChannels * (waveFormatEx->wBitsPerSample/8); 
		waveFormatEx->nAvgBytesPerSec = waveFormatEx->nSamplesPerSec * waveFormatEx->nBlockAlign; 
	
		return 0;
	}
	HRESULT SetFormat(WAVEFORMATEX *waveFormatEx) {
		return 0;
	}
	#ifdef _USE_XAUDIO2
	HRESULT LoadData(XAUDIO2_BUFFER &buffer) {
		if(avAudioCodecCtx == 0) return 0;
    
		int channels, bits;
		channels = 2;//av_frame_get_channels(avAudioFrame);
		bits = avAudioCodecCtx->bits_per_coded_sample;
		//channels = 1;

		unsigned int frequency = avAudioCodecCtx->sample_rate;

		unsigned long count = 0;
		int val = 0;
		AudioPacket avAudioFrame;

		//while(!player->done) {

		while(true) {//player->audioQueue.size() < 0 && !player->done) {
			if(player->audioQueue.size() < 0 && !player->done) {
				continue;
			} else {
				break;
			}
		}

		if(player->done) return 0;
			if(player->audioQueue.size() > 0) {
				try {
					player->aMutex1.lock();
					avAudioFrame = player->audioQueue.front();
					player->audioQueue.pop();
					player->aMutex1.unlock();
				} catch(...) {
					buffer.AudioBytes = 0;
				 buffer.pAudioData = 0;
				return 0;
				}
			} else {
				//std::this_thread::yield();
	//            std::this_thread::sleep_for(std::chrono::milliseconds(1));
				//continue;
				
				 buffer.AudioBytes = 0;
				 buffer.pAudioData = 0;
				return 0;
			}

			if(avAudioFrame.size > 0) {
	             //std::cerr << "^^^ PLAYING AUDIO" << count << std::endl;
				//channels = av_frame_get_channels(avAudioFrame.avAudioFrame);
				//bits = avAudioCodecCtx->bits_per_coded_sample;

				//frequency = avAudioCodecCtx->sample_rate;
				//avAudioFrame.audioBuffer, avAudioFrame.size, frequency);
				BYTE * dataBufferBuffer = new BYTE[avAudioFrame.size*channels];
				 memcpy(dataBufferBuffer, avAudioFrame.audioBuffer, avAudioFrame.size);
				 buffer.AudioBytes = avAudioFrame.size;
				 buffer.pAudioData = dataBufferBuffer;
				 //buffer.Flags = XAUDIO2_END_OF_STREAM;
			} else {
				buffer.AudioBytes = 0;
				 buffer.pAudioData = 0;
				return 0;
			}

			aMutex2.lock();
			player->audioBufferQueue.push(avAudioFrame);
			aMutex2.unlock();
		//}
		return 0;
	}
#endif

	AudioPacket avAudioFrameRemaining;
	long remainingBytes;
	HRESULT LoadData(UINT32 bufferFrameCount, BYTE *dataBuffer, DWORD *flags) {
		if(avAudioCodecCtx == 0) return 0;
    
		int channels, bits;
		channels = 2;//av_frame_get_channels(avAudioFrame);
		bits = avAudioCodecCtx->bits_per_coded_sample;
		//channels = 1;

		unsigned int frequency = avAudioCodecCtx->sample_rate;

		unsigned long count = 0;
		int val = 0;
		AudioPacket avAudioFrame;

		//while(!player->done) {
		if(player->done) return 0;

		int readBytes = 0;
		while(readBytes < bufferFrameCount) {
			if(player->done) return 0;

			if(remainingBytes <= 0) {
				if(!player->audioQueue.empty()) {
					player->aMutex1.lock();
					avAudioFrame = player->audioQueue.front();
					avAudioFrameRemaining = avAudioFrame;
					player->audioQueue.pop();
					player->aMutex1.unlock();
				} else {
					std::this_thread::yield();
		//            std::this_thread::sleep_for(std::chrono::milliseconds(1));
					continue;
					//return 0;
				}
				remainingBytes = avAudioFrame.size;
			} else {
				avAudioFrame = avAudioFrameRemaining;
			}

			if(avAudioFrame.size > 0) {
	             //std::cerr << "^^^ PLAYING AUDIO" << count << std::endl;
				int readSize = (avAudioFrame.size < (bufferFrameCount-readBytes)) ? avAudioFrame.size : (bufferFrameCount-readBytes);
				memcpy(dataBuffer+readBytes, avAudioFrame.audioBuffer+(avAudioFrame.size-remainingBytes), readSize);
				readBytes += readSize;
				remainingBytes -= readSize;
			}
			*flags = 0;

			if(remainingBytes <= 0) {
				player->aMutex2.lock();
				player->audioBufferQueue.push(avAudioFrame);
				player->aMutex2.unlock();
			}
			//if(readBytes >= bufferFrameCount) {
			//	return 0;
			//}
		}
		return 0;
	}
};

#ifdef _USE_XAUDIO2
HRESULT PlayAudioStreamXAUDIO2(Ibex::WindowsAudioSource *WindowsAudioSource)
{
	WAVEFORMATEX waveFormatEx = {0};
	XAUDIO2_BUFFER buffer = {0};

	WindowsAudioSource->AdjustFormat(&waveFormatEx);

	CoInitializeEx( NULL, COINIT_MULTITHREADED );

    IXAudio2* xaudio2 = NULL;
	HRESULT hr;
	if ( FAILED(hr = XAudio2Create( &xaudio2, 0, XAUDIO2_DEFAULT_PROCESSOR ) ) )
		return hr;

	IXAudio2MasteringVoice* masteringVoice = NULL;
	//create the mastering voice
	if( FAILED( hr = xaudio2->CreateMasteringVoice( &masteringVoice ) ) )
	{
		return hr;
	}

	IXAudio2SourceVoice* sourceVoice = NULL;
	if( FAILED(hr = xaudio2->CreateSourceVoice(&sourceVoice, (WAVEFORMATEX*)&waveFormatEx)))	
		return hr;

	while(true) {
		WindowsAudioSource->LoadData(buffer);
		if(buffer.AudioBytes == 0) continue;
		if(FAILED(hr = sourceVoice->SubmitSourceBuffer(&buffer)))
			continue;//return hr;

		if (FAILED(hr = sourceVoice->Start(0)))
			return hr;

		//Sleep((DWORD)(2));//buffer.AudioBytes/waveFormatEx.nBlockAlign*1000/waveFormatEx.nSamplesPerSec));//actualDuration/REFTIMES_PER_MILLISEC/2));
	}
}
#endif

HRESULT PlayAudioStreamWASAPI(Ibex::WindowsAudioSource *WindowsAudioSource)
{
	const long referenceTimeUnitsPerSecond = 10000000;
    HRESULT hr;
    IMMDeviceEnumerator *pEnumerator = NULL;
    IMMDevice *wasapiDevice = NULL;
    IAudioClient *wasapiAudioClient = NULL;
    IAudioRenderClient *wasapiAudioRenderClient = NULL;
    WAVEFORMATEX *waveFormatEx = NULL;
    UINT32 bufferFrameCount;
    UINT32 numAudioFramesAvailable;
    UINT32 numFramesPadding;
    BYTE *dataBuffer;
    DWORD flags = 0;
	REFERENCE_TIME requestedDuration = referenceTimeUnitsPerSecond;
    REFERENCE_TIME actualDuration;
	
	hr = CoInitialize(NULL);
	if (FAILED(hr)) { goto CleanupFunction; }

    hr = CoCreateInstance(
           CLSID_MMDeviceEnumerator, NULL,
           CLSCTX_ALL, IID_IMMDeviceEnumerator,
           (void**)&pEnumerator);
    if (FAILED(hr)) { goto CleanupFunction; }

    hr = pEnumerator->GetDefaultAudioEndpoint(
                        eRender, eConsole, &wasapiDevice);
    if (FAILED(hr)) { goto CleanupFunction; }

    hr = wasapiDevice->Activate(
                    IID_IAudioClient, CLSCTX_ALL,
                    NULL, (void**)&wasapiAudioClient);
    if (FAILED(hr)) { goto CleanupFunction; }

    hr = wasapiAudioClient->GetMixFormat(&waveFormatEx);
    if (FAILED(hr)) { goto CleanupFunction; }

	hr = WindowsAudioSource->AdjustFormat(waveFormatEx);
	if (FAILED(hr)) { goto CleanupFunction; }

    hr = wasapiAudioClient->Initialize(
						AUDCLNT_SHAREMODE_EXCLUSIVE,
						//AUDCLNT_SHAREMODE_SHARED,
                         0,
                         requestedDuration,
                         0,
                         waveFormatEx,
                         NULL);
    if (FAILED(hr)) { goto CleanupFunction; }

    hr = WindowsAudioSource->SetFormat(waveFormatEx);
    if (FAILED(hr)) { goto CleanupFunction; }

    hr = wasapiAudioClient->GetBufferSize(&bufferFrameCount);
    if (FAILED(hr)) { goto CleanupFunction; }

    hr = wasapiAudioClient->GetService(
                         IID_IAudioRenderClient,
                         (void**)&wasapiAudioRenderClient);
    if (FAILED(hr)) { goto CleanupFunction; }

    hr = wasapiAudioRenderClient->GetBuffer(bufferFrameCount, &dataBuffer);
	if (FAILED(hr)) { goto CleanupFunction; }

	hr = WindowsAudioSource->LoadData(bufferFrameCount*waveFormatEx->nBlockAlign, dataBuffer, &flags);
    if (FAILED(hr)) { goto CleanupFunction; }

    hr = wasapiAudioRenderClient->ReleaseBuffer(bufferFrameCount, flags);
    if (FAILED(hr)) { goto CleanupFunction; }

    actualDuration = (double)referenceTimeUnitsPerSecond * bufferFrameCount / waveFormatEx->nSamplesPerSec;

    hr = wasapiAudioClient->Start();  // Start playing.
    if (FAILED(hr)) { goto CleanupFunction; }

    while (flags != AUDCLNT_BUFFERFLAGS_SILENT)
    {
        Sleep((DWORD)(actualDuration/referenceTimeUnitsPerSecond/1000/2));

        hr = wasapiAudioClient->GetCurrentPadding(&numFramesPadding);
        if (FAILED(hr)) { goto CleanupFunction; }

        numAudioFramesAvailable = bufferFrameCount - numFramesPadding;

        hr = wasapiAudioRenderClient->GetBuffer(numAudioFramesAvailable, &dataBuffer);
        if (FAILED(hr)) { goto CleanupFunction; }

		hr = WindowsAudioSource->LoadData(numAudioFramesAvailable*waveFormatEx->nBlockAlign, dataBuffer, &flags);//, &duration);
        if (FAILED(hr)) { goto CleanupFunction; }

        hr = wasapiAudioRenderClient->ReleaseBuffer(numAudioFramesAvailable, flags);
        if (FAILED(hr)) { goto CleanupFunction; }
    }

    Sleep((DWORD)(actualDuration/referenceTimeUnitsPerSecond/1000/2));

    hr = wasapiAudioClient->Stop();  // Stop playing.
    if (FAILED(hr)) { goto CleanupFunction; }

CleanupFunction:
	CoTaskMemFree(waveFormatEx);

    SAFE_RELEASE(pEnumerator)
    SAFE_RELEASE(wasapiDevice)
    SAFE_RELEASE(wasapiAudioClient)
    SAFE_RELEASE(wasapiAudioRenderClient)

    return hr;
}

#endif

uint64_t global_video_pkt_pts = AV_NOPTS_VALUE;

// storge global_pts of the first packet of each video frame
int ibex_get_buffer(struct AVCodecContext *c, AVFrame *pic) {
    int ret = avcodec_default_get_buffer(c, pic);
    uint64_t *pts = (uint64_t*)av_malloc(sizeof(uint64_t));
    *pts = global_video_pkt_pts;
    pic->opaque = pts;
    return ret;
}
// release the buffer and special "opaque" data that we attached to each frame
void ibex_release_buffer(struct AVCodecContext *c, AVFrame *pic) {
    if(pic) av_freep(&pic->opaque);
    avcodec_default_release_buffer(c, pic);
}

Ibex::VideoPlayer::VideoPlayer() :  videoTexture(new unsigned int[2]),
                                    width(0),
                                    height(0),
                                    done(true),
                                    videoDone(true),
                                    audioDone(true),
                                    videoClock(0),
                                    gotCompletePictureFrame(0),
                                    gotCompleteAudioFrame(0),
                                    videoSyncMode(SyncExternal),
                                    avFormatCtx(NULL),
                                    avAudioCodecCtx(NULL),
                                    avAudioCodec(NULL),
									openCVInited(false),
									captureVideo(false),
									cvCapture(0) {
	videoTexture[0] = videoTexture[1] = 0;

    avcodec_register_all();
    av_register_all();
    avfilter_register_all();
	//avdevice_register_all();
}

void Ibex::VideoPlayer::savePPMFrame(const AVFrame *avFrame, int width, int height, int iFrame) const {
    FILE *pFile;
    char szFilename[32];
    int  y;
    
    // Open file
    sprintf(szFilename, "/Users/hesh/frame%d.ppm", iFrame);
    pFile=fopen(szFilename, "wb");
    if(pFile==NULL)
        return;
    
    // Write header
    fprintf(pFile, "P6\n%d %d\n255\n", width, height);
    
    // Write pixel data
    for(y=0; y<height; y++)
        fwrite(avFrame->data[0]+y*avFrame->linesize[0], 1, width*3, pFile);
    
    // Close file
    fclose(pFile);
}

void Ibex::VideoPlayer::addAudioFrame(AudioPacket avAudioFrame) {
	aMutex1.lock();
	audioQueue.push(avAudioFrame);
	aMutex1.unlock();
}
void Ibex::VideoPlayer::addVideoFrame(AudioPacket avVideoFrame) {
    while(videoQueue.size() > MAX_VIDEO_QUEUE_SIZE && !done) {
//        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        std::this_thread::yield();
    }
    
    videoQueue.push(avVideoFrame);
}

int Ibex::VideoPlayer::playAudio(AVCodecContext *avAudioCodecCtx) {
    if(avAudioCodecCtx == 0) return 0;
    
	int channels, bits;
    channels = 2;//av_frame_get_channels(avAudioFrame);
    bits = avAudioCodecCtx->bits_per_coded_sample;
    channels = 1;

	unsigned int frequency = avAudioCodecCtx->sample_rate;
#ifdef OPENAL
    ////// OpenAL ////////
    ALCdevice *dev;
    ALCcontext *ctx;
    
    dev = alcOpenDevice(NULL);
    if(!dev)
    {
        fprintf(stderr, "Oops\n");
        return 1;
    }
    ctx = alcCreateContext(dev, NULL);
    alcMakeContextCurrent(ctx);
    if(!ctx)
    {
        fprintf(stderr, "Oops2\n");
        return 1;
    }
    
    ALuint source, buffers[NUM_BUFFERS];
    ALenum format;
    
    alGenBuffers(NUM_BUFFERS, buffers);
    alGenSources(1, &source);
    if(alGetError() != AL_NO_ERROR)
    {
        fprintf(stderr, "Error generating :(\n");
        return 1;
    }
    
	format = 0;
    if(bits == 8)
    {
        if(channels == 1)
            format = AL_FORMAT_MONO8;
        else if(channels == 2)
            format = AL_FORMAT_STEREO8;
    }
    else if(bits == 16)
    {
        if(channels == 1)
            format = AL_FORMAT_MONO16;
        else if(channels == 2)
            format = AL_FORMAT_STEREO16;
    }
    if(!format)
    {
        fprintf(stderr, "Incompatible format (%d, %d) :(\n", channels, bits);
//        fclose(f);
        return 1;
    }
    /////
#endif

    unsigned long count = 0;
    int val = 0;
    AudioPacket avAudioFrame;
    while(!done) {
        if(audioQueue.size() > 0) {
			aMutex1.lock();
            avAudioFrame = audioQueue.front();
            audioQueue.pop();
			aMutex1.unlock();
        } else {
            std::this_thread::yield();
//            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }
        
//        int data_size = av_samples_get_buffer_size(NULL, avAudioCodecCtx->channels,
//                                                   avAudioFrame.avAudioFrame->nb_samples,
//                                                   avAudioCodecCtx->sample_fmt, 1);
//        fprintf(stderr, "data_size: %d\n", avAudioFrame.size);//data_size);
        if(avAudioFrame.size > 0) {//data_size > 0) {
//            std::cerr << "^^^ PLAYING AUDIO" << count << std::endl;
            channels = av_frame_get_channels(avAudioFrame.avAudioFrame);
            bits = avAudioCodecCtx->bits_per_coded_sample;

#ifdef OPENAL
            format = 0;
//            channels = 2;
//            channels = 1;
            if(bits == 8)
            {
                if(channels == 1)
                    format = AL_FORMAT_MONO8;
                else if(channels == 2)
                    format = AL_FORMAT_STEREO8;
            }
            else if(bits == 16)
            {
                if(channels == 1)
                    format = AL_FORMAT_MONO16;
                else if(channels == 2)
                    format = AL_FORMAT_STEREO16;
            }
#endif
            frequency = avAudioCodecCtx->sample_rate;
//            frequency = 44100;
            
            if(count >= NUM_BUFFERS) {
                do {
                    if(done) return 0;
#ifdef OPENAL
                    alGetSourcei(source, AL_BUFFERS_PROCESSED, &val);
#else
					val = 1;
#endif
                    std::this_thread::yield();
//                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                } while(val <= 0 && !done);
                if(done) break;
                //                    fprintf(stderr, "Unqueued buffer\n");

#ifdef OPENAL
                ALuint buffer;
                alSourceUnqueueBuffers(source, 1, &buffer);
                
    //            memset(avAudioFrame->data[0],0,data_size);
                
                alBufferData(buffer, format, avAudioFrame.audioBuffer, avAudioFrame.size, frequency);
                alSourceQueueBuffers(source, 1, &buffer);
                
                if(alGetError() != AL_NO_ERROR)
                {
                    fprintf(stderr, "Error loading 2 :(\n");
                    exit(1);
                    return 1;
                }
#endif
            } else {
#ifdef OPENAL
                alBufferData(buffers[count], format, avAudioFrame.audioBuffer, avAudioFrame.size, frequency);
                alSourceQueueBuffers(source, 1, &(buffers[count]));
                
                if(alGetError() != AL_NO_ERROR)
                {
                    fprintf(stderr, "Error loading buffers[%d] :(\n", count);
                    exit(1);
                    return 1;
                }
#endif
            }
            ++count;
        }
        
#ifdef OPENAL
        // Make sure the source is still playing, and restart it if needed.
        alGetSourcei(source, AL_SOURCE_STATE, &val);
        if(val != AL_PLAYING)
            alSourcePlay(source);
        if(alGetError() != AL_NO_ERROR)
        {
            fprintf(stderr, "Error starting :(\n");
            exit(1);
            return 1;
        }
#endif
		aMutex2.lock();
        audioBufferQueue.push(avAudioFrame);
		aMutex2.unlock();
    }
    
    return 0;
}

int Ibex::VideoPlayer::initVideo(const char *fileName, bool isStereo) {
    done = false;
    videoDone = false;
    audioDone = false;
    
//    av_register_all();
    
    avFormatCtx = NULL;
    
	//AVFormatContext *formatC = avformat_alloc_context();
	//AVDictionary* options = NULL;
	//av_dict_set(&options,"list_devices","true",0);
	//AVInputFormat *iformat = av_find_input_format("dshow");

	//std::cerr << iformat << std::endl;
 //   if(avformat_open_input(&avFormatCtx, "video=Roxio Video Capture USB", NULL, NULL)!=0) {
	if(avformat_open_input(&avFormatCtx, fileName, NULL, NULL)!=0) {
        videoDone = audioDone = done = true;
        return -1;
    }
    
    if(avformat_find_stream_info(avFormatCtx, NULL)<0) {
        videoDone = audioDone = done = true;
        return -1;
    }
    
    av_dump_format(avFormatCtx, 0, fileName, 0);
    
    // Find the first video stream
    videoStream=-1;
    audioStream=-1;
    for(int i=0; i<avFormatCtx->nb_streams&&(videoStream<0||audioStream<0); i++) {
        if(videoStream < 0 && avFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) {
            videoStream=i;
            avVideoStream = avFormatCtx->streams[videoStream];
        }
        if(audioStream < 0 && avFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO) {
            audioStream=i;
            avAudioStream = avFormatCtx->streams[audioStream];
        }
    }
    
    if(videoStream == -1) {
        videoDone = audioDone = done = true;
        return -1; // don't play if no video stream, don't want to do the same for audio
    }
    
    // Get video stream codec context
    avCodecCtx = avFormatCtx->streams[videoStream]->codec;
    avCodec=avcodec_find_decoder(avCodecCtx->codec_id);
    if(avCodec==NULL) {
        fprintf(stderr, "Couldn't find decoder for '%s'!\n", fileName);
        return -1;
    }
    if(avcodec_open2(avCodecCtx, avCodec,NULL)<0) {
        fprintf(stderr, "Couldn't load the codec for '%s'!\n", fileName);
        return -1;
    }
    
    avCodecCtx->get_buffer = ibex_get_buffer;
    avCodecCtx->release_buffer = ibex_release_buffer;
    
    width = avCodecCtx->width;
    height = avCodecCtx->height;
    
    
    // Get audio stream codec context
    if(audioStream != -1) {
        avAudioCodecCtx = avFormatCtx->streams[audioStream]->codec;
        avAudioCodec=avcodec_find_decoder(avAudioCodecCtx->codec_id);
        if(avAudioCodec==NULL) {
            fprintf(stderr, "Couldn't find decoder for '%s'!\n", fileName);
            return -1;
        }
        avAudioCodecCtx->request_sample_fmt = AV_SAMPLE_FMT_S16;
        if(avcodec_open2(avAudioCodecCtx, avAudioCodec,NULL)<0) {
            fprintf(stderr, "Couldn't load the codec for '%s'!\n", fileName);
            return -1;
        }
    }
    
    for(int i = 0; i <= MAX_VIDEO_QUEUE_SIZE+1; ++i) {
        videoFrameQueue.push(avcodec_alloc_frame());
    }
    for(int i = 0; i <= MAX_AUDIO_FRAME_QUEUE_SIZE+1; ++i) {
        AudioPacket a;
        a.audioBuffer = new uint8_t[BUFFER_SIZE+FF_INPUT_BUFFER_PADDING_SIZE];
        a.avAudioFrame = avcodec_alloc_frame();
        avcodec_get_frame_defaults(a.avAudioFrame);
		aMutex2.lock();
        audioBufferQueue.push(a);
		aMutex2.unlock();
    }
    // prepare audio,video frame
    avAudioFrame = avcodec_alloc_frame();
    avcodec_get_frame_defaults(avAudioFrame);
    
    // create RGB framebuffer
    avFrameRGB = avcodec_alloc_frame();
    if(avFrameRGB==NULL)
        return -1;
    
    // create RGB framebuffer backing store
    numBytes = avpicture_get_size(PIX_FMT_RGB24, width, height);
    buffer = (uint8_t*)av_malloc(sizeof(uint8_t)*numBytes);
    memset(buffer, 0, sizeof(uint8_t)*numBytes);
    
    // assign buffer to avFrameRGB
    avpicture_fill((AVPicture *)avFrameRGB, buffer, PIX_FMT_RGB24, width, height);
    
    return 0;
}
double Ibex::VideoPlayer::synchronize_video(AVFrame *src_frame, double pts) {
    double frame_delay;
    
    if(pts != 0) {
        /* if we have pts, set video clock to it */
        videoClock = pts;
    } else {
        /* if we aren't given a pts, set it to the clock */
        pts = videoClock;
    }
    /* update the video clock */
    frame_delay = av_q2d(avVideoStream->codec->time_base);
    /* if we are repeating a frame, adjust clock accordingly */
    frame_delay += src_frame->repeat_pict * (frame_delay * 0.5);
    videoClock += frame_delay;
    return pts;
}

int Ibex::VideoPlayer::loadSyncAudioVideo(const char *fileName_, bool isStereo) {
    if(avAudioCodecCtx == 0) audioDone = true;
    
    SwrContext * swrContext = 0;
    if(avAudioCodecCtx != 0) {
        swrContext = swr_alloc();
        av_opt_set_int(swrContext, "in_channel_layout",  avAudioCodecCtx->channel_layout, 0);
        av_opt_set_int(swrContext, "out_channel_layout", avAudioCodecCtx->channel_layout,  0);
        av_opt_set_int(swrContext, "in_sample_rate",     avAudioCodecCtx->sample_rate, 0);
        av_opt_set_int(swrContext, "out_sample_rate",    avAudioCodecCtx->sample_rate, 0);
        av_opt_set_sample_fmt(swrContext, "in_sample_fmt",  avAudioCodecCtx->sample_fmt, 0);
		av_opt_set_sample_fmt(swrContext, "out_sample_fmt", AV_SAMPLE_FMT_S16 /* P */,  0);
        swr_init(swrContext);
    }
    
    int data_size = 0;
    AVRational rational = avCodecCtx->time_base;
    double timeBase = (double)av_q2d(rational);
    int msec = 0;
    
    // start playing video
    //    int frame = 0;
    int len = 0;
    int bufferIndex = 0;
    uint8_t *audioBuffer = new uint8_t[BUFFER_SIZE+FF_INPUT_BUFFER_PADDING_SIZE];//size];
    memset(audioBuffer, 0, BUFFER_SIZE+FF_INPUT_BUFFER_PADDING_SIZE);//size);
    
#ifdef _WIN32
#ifdef _USE_XAUDIO2
	audioThread = std::thread(PlayAudioStreamXAUDIO2, new Ibex::WindowsAudioSource(this, avAudioCodecCtx));
#else
	audioThread = std::thread(PlayAudioStreamWASAPI, new Ibex::WindowsAudioSource(this, avAudioCodecCtx));
#endif
#else
    audioThread = std::thread(&Ibex::VideoPlayer::playAudio,this, avAudioCodecCtx);
#endif
    
    avFrame = videoFrameQueue.front();
    videoFrameQueue.pop();
    
	aMutex2.lock();
    AudioPacket p = audioBufferQueue.front();
    audioBufferQueue.pop();
	aMutex2.unlock();

    avAudioFrame = p.avAudioFrame;
    audioBuffer = p.audioBuffer;
    
    int64_t globalPts;
    unsigned long count = 0;
    while(av_read_frame(avFormatCtx, &avPacket)>=0 && !done) {
        globalPts = avPacket.pts;
        
        ++count;
        msec = 1000*(avPacket.pts * timeBase * avCodecCtx->ticks_per_frame);
        // play audio stream
        if(avPacket.stream_index == audioStream && avAudioCodecCtx) {
            int size = avPacket.size;
//            std::cerr << "packet.pts: " << avPacket.pts << std::endl;
            
            if(size > 0) {
                int len2 = avcodec_decode_audio4(avAudioCodecCtx, avAudioFrame, &gotCompleteAudioFrame, &avPacket);
                len += len2;
                if(gotCompleteAudioFrame) {
                    bufferIndex = 0;
                    data_size = av_samples_get_buffer_size(NULL, avAudioCodecCtx->channels,
                                                           avAudioFrame->nb_samples,
                                                           avAudioCodecCtx->sample_fmt, 1);
                    
//                    int channels = av_frame_get_channels(avAudioFrame);
                    int out_samples = data_size/2;
                    swr_convert(swrContext, &audioBuffer, out_samples, (const uint8_t **)avAudioFrame->extended_data,
                                avAudioFrame->nb_samples);
                    // look into libavfilter 'aformat'

                    memcpy(audioBuffer+out_samples, audioBuffer, out_samples);
                    bufferIndex += len2;//data_size;
                    
                    //AV_SAMPLE_FMT_FLTP
                    data_size = out_samples;//*4;
                    len = data_size;
                    if(true && len >= data_size) {
                        AudioPacket a;
                        a.audioBuffer = audioBuffer;
                        a.size = data_size;//len;
                        a.avAudioFrame = avAudioFrame;
                        addAudioFrame(a);
                        
                        while(audioBufferQueue.empty() && !done) {
                            std::this_thread::yield();
//                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                            continue;
                        }
                        if(done) continue;
						aMutex2.lock();
                        p = audioBufferQueue.front();
                        audioBufferQueue.pop();
						aMutex2.unlock();
                        avAudioFrame = p.avAudioFrame;
                        audioBuffer = p.audioBuffer;
                        
                        len = 0;
                        bufferIndex = 0;
                    }
                }
            }
            
            // don't leak, free packet before reading next
            av_free_packet(&avPacket);
        } else if(avPacket.stream_index == videoStream) { // play video stream
//            if(avPacket.pts != avPacket.dts) {
//                std::cerr << "* BFrame - avPacket.dts: " << avPacket.dts << ", avPacket.pts: " << avPacket.pts << std::endl;
//            } else {
//                std::cerr << "+ IFrame - avPacket.dts: " << avPacket.dts << ", avPacket.pts: " << avPacket.pts << std::endl;
//            }
            
            if(avFrame == 0) {
                while(videoFrameQueue.size() <= 0 && !done) {
                    std::this_thread::yield();
//                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    continue;
                }
                if(done) continue;
                
                avFrame = videoFrameQueue.front();
                videoFrameQueue.pop();
            }
            
            // PTS of the Frame
            global_video_pkt_pts = avPacket.pts;
            
            // decode frame
            avcodec_decode_video2(avCodecCtx, avFrame, &gotCompletePictureFrame, &avPacket);
            
//            if(avFrame->opaque) {
//                std::cerr << "^ avFrame->pts: " << *(uint64_t*)avFrame->opaque << std::endl;
//            }
            
//            if(avFrame->repeat_pict) {
//                std::cerr << "^^^^^^^ repeat: " << avFrame->repeat_pict << std::endl;
//            }
            
            double pts = 0;
            if(gotCompletePictureFrame) {
//                std::cerr << "complete" << std::endl;
                if(avPacket.dts == AV_NOPTS_VALUE
                   && avFrame->opaque && *(uint64_t*)avFrame->opaque != AV_NOPTS_VALUE) {
                    pts = *(uint64_t *)avFrame->opaque;
                } else if(avPacket.dts != AV_NOPTS_VALUE) {
                    pts = avPacket.dts;
                } else {
                    pts = 0;
                }
                pts *= av_q2d(avVideoStream->time_base);
                
                AudioPacket a;
                a.avAudioFrame = avFrame;
                a.dts = avPacket.dts;
                a.pts = synchronize_video(avFrame, pts);
//                std::cerr << "^^^^ pts: " << a.pts << ", localPTS: " << pts << std::endl;
                addVideoFrame(a);
                
                avFrame = 0;
            }
            
            // don't leak, free packet before reading next
            av_free_packet(&avPacket);
        } else {
            // don't leak, free packet before reading next
            av_free_packet(&avPacket);
        }
    }
//    videoDone = true;
    
    audioThread.join();
    audioDone = true;
    
    for(int i = 0; i < audioBufferQueue.size(); ++i) {
		aMutex2.lock();
        AudioPacket avAudioFrame = audioBufferQueue.front();
        audioBufferQueue.pop();
		aMutex2.unlock();
        av_free(avAudioFrame.avAudioFrame);
        delete [] avAudioFrame.audioBuffer;
    }

    // free original frame
    av_free(avAudioFrame);

    // close codec
    avcodec_close(avCodecCtx);

    // close video
    avformat_close_input(&avFormatCtx);
    
//    done = true;

    return 0;
}

double Ibex::VideoPlayer::getGlobalVideoPTS(AVFrame *src_frame, double pts) {
    
    double frame_delay;
    
    if(pts != 0) {
        /* if we have pts, set video clock to it */
        videoClock = pts;
    } else {
        /* if we aren't given a pts, set it to the clock */
        pts = videoClock;
    }
    /* update the video clock */
    frame_delay = av_q2d(avVideoStream->codec->time_base);
    /* if we are repeating a frame, adjust clock accordingly */
    frame_delay += src_frame->repeat_pict * (frame_delay * 0.5);
    videoClock += frame_delay;
    return pts;
}

int Ibex::VideoPlayer::playVideo(const char *fileName, bool isStereo)
{
    if(!done) {
        done = true;
        while(!videoDone || !audioDone);
    }

    width = 0;
    height = 0;
    done = true;
    videoDone = true;
    audioDone = true;
    videoClock = 0;
    gotCompletePictureFrame = 0;
    gotCompleteAudioFrame = 0;
    videoSyncMode = SyncExternal;
    avFormatCtx = NULL;
    avAudioCodecCtx = NULL;
    avAudioCodec = NULL;
    videoStream = -1;
    audioStream = -1;
    
    if(initVideo(fileName, isStereo)) {
        videoDone = audioDone = done = true;
        return -1;
    }
    videoDone = audioDone = done = false;

    
    syncThread = std::thread(&Ibex::VideoPlayer::loadSyncAudioVideo, this, fileName, isStereo);
    
    AVFrame *avFrameRGB;
    int             numBytes;
    uint8_t         *buffer;
    
    // create RGB framebuffer
    avFrameRGB = avcodec_alloc_frame();
    if(avFrameRGB==NULL) {
        videoDone = audioDone = done = true;
        return -1;
    }
    
    // create RGB framebuffer backing store
    numBytes = avpicture_get_size(PIX_FMT_RGB24, width, height);
    buffer = (uint8_t*)av_malloc(sizeof(uint8_t)*numBytes);
    memset(buffer, 0, sizeof(uint8_t)*numBytes);
    
    // assign buffer to avFrameRGB
    avpicture_fill((AVPicture *)avFrameRGB, buffer, PIX_FMT_RGB24, width, height);
    
    
    // image conversion context
    struct  SwsContext *img_convert_ctx = NULL;
    
    createVideoTextures(isStereo, width, height);
    ////////////////
    bool first = true;
    AudioPacket videoFrame;
    double lastPts = -1;
    double timeOfLastFrame = 0;
    long count = 0;
    while(!done) {
        if(videoQueue.size() > 0) {
            videoFrame = videoQueue.front();
            videoQueue.pop();
            if(lastPts == -1) {
                lastPts = videoFrame.pts;
            }

            if(++count%100 == 0) {
                std::cerr << "video fps: " << 1000.0/((videoFrame.pts-lastPts)*1000) << std::endl;
            }
            
            double frameDiff = (videoFrame.pts-lastPts);
            double timeDiff = getSyncClock()-timeOfLastFrame;
            if(timeDiff<frameDiff) {
                if(done) continue;
//                std::cerr << frameDiff << ", " << timeDiff << std::endl;
//                std::cerr << "sleep for: " << (int)((frameDiff-timeDiff)*1000.0) << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds((int)((frameDiff-timeDiff)*1000.0)));
            }
        } else {
            std::this_thread::yield();
//                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }
        
        timeOfLastFrame = getSyncClock();
        
        AVFrame *avFrame = videoFrame.avAudioFrame;
//        std::cerr << "video PTS: " << videoFrame.pts << std::endl;
        lastPts = videoFrame.pts;
        
        if(img_convert_ctx == NULL)
        {
            img_convert_ctx = sws_getContext(width,height, avFormatCtx->streams[videoStream]->codec->pix_fmt , width, height, PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);
            
            if(img_convert_ctx == NULL)
            {
                fprintf(stderr, "ERROR creating sws context\n");
                exit(1);
            }
        }
        sws_scale(img_convert_ctx, avFrame->data, avFrame->linesize, 0, avFormatCtx->streams[videoStream]->codec->height, avFrameRGB->data,avFrameRGB->linesize);
        videoFrameQueue.push(avFrame);
        
        
        if(isStereo) {
            glBindTexture(GL_TEXTURE_2D, videoTexture[1]);
            int stride = width*2;
            glPixelStorei(GL_UNPACK_ROW_LENGTH,stride);
            if(first) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height/2, 0,
                             GL_RGB, GL_UNSIGNED_BYTE, avFrameRGB->data[0]);
                glBindTexture(GL_TEXTURE_2D, videoTexture[0]);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height/2, 0,
                             GL_RGB, GL_UNSIGNED_BYTE, avFrameRGB->data[0]+(width*3));
            } else {
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height/2,
                             GL_RGB, GL_UNSIGNED_BYTE, avFrameRGB->data[0]);
                glBindTexture(GL_TEXTURE_2D, videoTexture[0]);
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height/2,
                             GL_RGB, GL_UNSIGNED_BYTE, avFrameRGB->data[0]+(width*3));
            }
        } else {
            glBindTexture(GL_TEXTURE_2D, videoTexture[0]);
            if(first) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0,
                             GL_RGB, GL_UNSIGNED_BYTE, avFrameRGB->data[0]);
            } else {
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height,
                             GL_RGB, GL_UNSIGNED_BYTE, avFrameRGB->data[0]);
            }
        }
        glFlush();
        
//        first = false;
    }
    
    // free buffer and framebuffer
    av_free(buffer);
    av_free(avFrameRGB);
    
    syncThread.join();
    
    for(int i = 0; i < videoFrameQueue.size(); ++i) {
        AVFrame *avFrame = videoFrameQueue.front();
        videoFrameQueue.pop();
        if(avFrame != NULL) {
            av_free(avFrame);
        }
    }
    
    if(img_convert_ctx != NULL) {
        sws_freeContext(img_convert_ctx);
    }
    
    videoDone = true;
    
    return 0;
}

double Ibex::VideoPlayer::getSyncClock() {
    return av_gettime() / 1000000.0; // get global external clock time
}

std::vector<int> Ibex::VideoPlayer::listCameras() {
    std::vector<int> cameras;
    
    CvCapture *cam;
    int i = 0;
    do
    {
        cam = cvCreateCameraCapture(i++);
        cameras.push_back(i-1);
        if(cam == NULL) break;
        cvReleaseCapture(&cam);
    } while(true && i < 5);
    if(cam) cvReleaseCapture(&cam);
    
    return cameras;
}

void Ibex::VideoPlayer::stopCapturing() {
    captureVideo = false;
    if(cvCapture) {
        cvReleaseCapture(&cvCapture);
        cvCapture = NULL;
        openCVInited = false;
    }
}
void Ibex::VideoPlayer::initOpenCV(bool isStereo, int cameraId) {
	if(!openCVInited) {
        captureVideo = true;
		cvCapture = cvCaptureFromCAM(cameraId);//cvCreateCameraCapture(cameraId);
        width = cvGetCaptureProperty(cvCapture, CV_CAP_PROP_FRAME_WIDTH);
        height = cvGetCaptureProperty(cvCapture, CV_CAP_PROP_FRAME_HEIGHT);
		createVideoTextures(isStereo, width, height);
		openCVInited = true;
	}
}

int Ibex::VideoPlayer::openCamera(bool isStereo, int cameraId) {
	if(!openCVInited) {
		initOpenCV(isStereo, cameraId);
	}
    
	bool first = true;
	while(captureVideo && cvCapture) {
		IplImage* cameraCapture = cvQueryFrame(cvCapture);
        
		if( cameraCapture && (cameraCapture->width > 0) && (cameraCapture->height > 0)) {
            //width = cameraCapture->width;
            //height = cameraCapture->height;
            const int numBytes = (cameraCapture->widthStep/cameraCapture->width == 4 || cameraCapture->nChannels == 4) ? 4 : 3;
            const GLenum formatIn = (numBytes == 4) ? GL_BGRA : GL_BGR;
			if(isStereo) {
				glBindTexture(GL_TEXTURE_2D, videoTexture[1]);
				const int stride = width*2;
				glPixelStorei(GL_UNPACK_ROW_LENGTH,stride);
				if(first) {
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height/2, 0,
								 formatIn, GL_UNSIGNED_BYTE, cameraCapture->imageData);
					glBindTexture(GL_TEXTURE_2D, videoTexture[0]);
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height/2, 0,
								 formatIn, GL_UNSIGNED_BYTE, cameraCapture->imageData+(width*numBytes));
				} else {
					glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height/2,
								 formatIn, GL_UNSIGNED_BYTE, avFrameRGB->data[0]);
					glBindTexture(GL_TEXTURE_2D, videoTexture[0]);
					glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height/2,
								 formatIn, GL_UNSIGNED_BYTE, cameraCapture->imageData+(width*numBytes));
				}
			} else {
				glBindTexture(GL_TEXTURE_2D, videoTexture[0]);
                const int stride = width;
				glPixelStorei(GL_UNPACK_ROW_LENGTH,stride);
                glPixelStorei(GL_UNPACK_ALIGNMENT,1);
				if(first) {
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0,
								 formatIn, GL_UNSIGNED_BYTE, cameraCapture->imageData);
				} else {
					glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height,
								 formatIn, GL_UNSIGNED_BYTE, cameraCapture->imageData);
				}
			}
			glFlush();
		}
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
	}
    stopCapturing();
    
	return 0;
}

void Ibex::VideoPlayer::createVideoTextures(bool isStereo, int width, int height) {
	// load OpenGL textures for video/stereo-video if necessary
    if(videoTexture[0]) {
        if(videoTexture[1] == videoTexture[0]) {
            glDeleteTextures(1, videoTexture);
        } else {
            glDeleteTextures(2, videoTexture);
        }
        videoTexture[0] = videoTexture[1] = 0;
    }
    glGenTextures((isStereo) ? 2 : 1, videoTexture);
    for(int i = 0; i < 2; ++i) {
        if(i == 1 && !isStereo) {
            videoTexture[i] = videoTexture[0];
            break;
        }
        
        glBindTexture(GL_TEXTURE_2D, videoTexture[i]);
        if (!checkForErrors()) {
            fprintf(stderr,"Stage 0a - Problem generating videoTexture FBO");
            exit(EXIT_FAILURE);
        }
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        if (!checkForErrors()) {
            fprintf(stderr,"Stage 0b - Problem generating videoTexture FBO");
            exit(EXIT_FAILURE);
        }
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        if (!checkForErrors()) {
            fprintf(stderr,"Stage 0c - Problem generating videoTexture FBO");
            exit(EXIT_FAILURE);
        }
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0,
                     GL_RGB, GL_UNSIGNED_BYTE, 0);
        if (!checkForErrors()) {
            fprintf(stderr,"Stage 0d - Problem generating videoTexture FBO");
            exit(EXIT_FAILURE);
        }
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    glFlush();
}
