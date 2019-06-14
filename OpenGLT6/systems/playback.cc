#include "playback.h"
#include "editor/gui.h"
#include <ctime>
#include "utils/filesystem.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

extern "C" {
#include "libswscale/swscale.h"
#include "libavcodec/avcodec.h"
#include "libavutil/opt.h"
#include "libavutil/imgutils.h"
#include "libavutil/mathematics.h"
#include "libavutil/samplefmt.h"
#include "libavformat/avformat.h"
}

namespace nabla {
struct VideoStreamer {
	AVCodec* codec = nullptr;
	AVCodecContext* c = nullptr;
	AVFrame* frame;
	AVFrame* yuv_frame;
	// AVPacket* pkt;
	// FILE* file;
	AVFormatContext* outctx;
	AVStream* outst;
	int width;
	int height;
	int cnt = 0;
	uint8_t tail[4] = { 0, 0, 1, 0xb7 };
	SwsContext* sws_rgb2yuv;
	bool Init(const char* file_name, int _width, int _height) {
		width = _width;
		height = _height;
		cnt = 0;

		int ret;
		AVOutputFormat* fmt;
		
		avcodec_register_all();
		
		fmt = av_guess_format(NULL, file_name, NULL);
		// codec = avcodec_find_encoder(AVCodecID::AV_CODEC_ID_H264);
		codec = avcodec_find_encoder_by_name("libx264");
		ret = avformat_alloc_output_context2(&outctx, fmt, NULL, file_name);
		if (ret < 0) {
			return false;
		}

		// outctx->oformat = av_guess_format(NULL, file_name, NULL);
		//if (avio_open2(&outctx->pb, file_name, AVIO_FLAG_WRITE, NULL, NULL) < 0)
			//return false;

		outst = avformat_new_stream(outctx, codec);

		c = avcodec_alloc_context3(codec);
		// outst->codecpar->codec_id = codec->id;
		// outst->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
		// outst->codecpar->width = _width;
		// outst->codecpar->height = _height;
		// outst->codecpar->format = AVPixelFormat::AV_PIX_FMT_RGB24;
		// outst->codecpar->bit_rate = c->bit_rate;
		// c->time_base = AVRational{ 1, 25 };

		c->bit_rate = 4000000;
		c->width = _width;                                        // resolution must be a multiple of two (1280x720),(1900x1080),(720x480)
		c->height = _height;
		c->time_base = AVRational{ 1, 60 };                                  // framerate numerator
		// c->framerate = AVRational{ 25, 1 };
		c->gop_size = 10;                                       // emit one intra frame every ten frames
		c->max_b_frames = 1;                                    // maximum number of b-frames between non b-frames
		c->keyint_min = 1;                                      // minimum GOP size
		c->i_quant_factor = (float)0.71;                        // qscale factor between P and I frames
		//c->b_frame_strategy = 20;                               ///// find out exactly what this does
		// c->qcompress = (float)0.6;                              ///// find out exactly what this does
		// c->qmin = 20;                                           // minimum quantizer
		// c->qmax = 51;                                           // maximum quantizer
		// c->max_qdiff = 4;                                       // maximum quantizer difference between frames
		// c->refs = 4;                                            // number of reference frames
		// c->trellis = 1;                                         // trellis RD Quantization
		// c->pix_fmt = AV_PIX_FMT_YUV420P;                           // universal pixel format for video encoding
		c->pix_fmt = AVPixelFormat::AV_PIX_FMT_YUV420P;
		// c->codec_id = AV_CODEC_ID_MPEG1VIDEO;
		// c->codec_type = AVMEDIA_TYPE_VIDEO;
		/* frames per second */
		
		av_opt_set(c->priv_data, "preset", "ultrafast", 0);
		
		ret = avcodec_open2(c, codec, NULL);

		frame = av_frame_alloc();
		frame->format = AV_PIX_FMT_RGB24;
		frame->width =  c->width;
		frame->height = c->height;
		ret = av_frame_get_buffer(frame, 32);
		// frame->data[0] += frame->linesize[0] * (height - 1);
		// frame->linesize[0] = -frame->linesize[0];

		yuv_frame = av_frame_alloc();
		yuv_frame->format = AV_PIX_FMT_YUV420P;
		yuv_frame->width = c->width;
		yuv_frame->height = c->height;
		ret = av_frame_get_buffer(yuv_frame, 32);
		
		if (outctx->oformat->flags & AVFMT_GLOBALHEADER) {
			c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
		}

		avcodec_parameters_from_context(outst->codecpar, c);
		if (avio_open2(&outctx->pb, file_name, AVIO_FLAG_WRITE, NULL, NULL) < 0) {
			return false;
		}
		// avcodec_copy_context(outst->codec, c);
		if (avformat_write_header(outctx, NULL) < 0) {
			return false;
		}
		
		sws_rgb2yuv = sws_getContext(width, height, AVPixelFormat::AV_PIX_FMT_RGB24, width, height, AV_PIX_FMT_YUV420P, SWS_FAST_BILINEAR, NULL, NULL, NULL);
		// av_dump_format(outctx, 0, file_name, 0);
		// av_dump_format(outctx, 0, VIDEO_TMP_FILE, 1);
		return true;
	}

	bool EncodePass(AVFrame* _frame) {
		AVPacket packet;
		av_init_packet(&packet);
		packet.data = NULL;
		packet.size = 0;
		int ret = avcodec_send_frame(c, _frame);
		if (ret < 0) {
			return false;
		}
		// int got_packet;
		// ret = avcodec_encode_video2(c, &packet, _frame, &got_packet);
		// if (got_packet) {
		// 	ret = av_interleaved_write_frame(outctx, &packet);
		// }
		// else {
		// 	ret = 0;
		// }
		// return !ret;

		while (ret >= 0) {
			ret = avcodec_receive_packet(c, &packet);
			if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
				return true;
			if (ret < 0) {
				return false;
			}
			
			av_packet_rescale_ts(&packet, AVRational { 1, 25 }, outst->time_base); // We set the packet PTS and DTS taking in the account our FPS (second argument) and the time base that our selected format uses (third argument).
			packet.stream_index = outst->index;
			// ret == 0
			av_interleaved_write_frame(outctx, &packet);
			// fwrite(pkt->data, 1, pkt->size, file);
			av_packet_unref(&packet);
		}
		return true;
	}
	
	bool Encode(const Vector<renderer::SolidPixel>& color) {
		int ret = av_frame_make_writable(frame);
		if (ret < 0) {
			return false;
		}
		auto itr = color.begin();
		
		frame->data[0] += frame->linesize[0] * (height - 1);
		frame->linesize[0] = -frame->linesize[0];

		for (int y = 0; y < c->height; y++) {
			for (int x = 0; x < c->width; x++) {
				frame->data[0][y * frame->linesize[0] + 3 * x] = itr->r;
				frame->data[0][y * frame->linesize[0] + 3 * x + 1] = itr->g;
				frame->data[0][y * frame->linesize[0] + 3 * x + 2] = itr->b;
				++itr;
			}
		}

		frame->linesize[0] = -frame->linesize[0];
		frame->data[0] -= frame->linesize[0] * (frame->height - 1);

		yuv_frame->pts = frame->pts = cnt++;
		sws_scale(sws_rgb2yuv, frame->data, frame->linesize, 0, height, yuv_frame->data, yuv_frame->linesize);
		return EncodePass(yuv_frame);

	}

	void End() {
		EncodePass(nullptr);
		av_write_trailer(outctx);
		avio_close(outctx->pb);
		avcodec_free_context(&c);
		av_frame_free(&frame);
		// av_packet_free(&pkt);
		avformat_free_context(outctx);
	}
};

void PlaybackSystem::Initialize(SystemContext&)
{
	if (!fs::exists("playback")) {
		fs::create_directories("playback");
	}
}

void PlaybackSystem::OnGui(const Vector<Entity>& actives)
{
	// ImGui::SameLine();
	ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(1.0f / 7.0f, 0.6f, 0.6f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(1.0f / 7.0f, 0.7f, 0.7f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(1.0f / 7.0f, 0.8f, 0.8f));
	screenshot_button_ = ImGui::Button("Screenshot");
	ImGui::PopStyleColor(3);

	ImGui::SameLine();
	ImGui::PushID("playback.record");
	if (!IsRecording()) {
		ImGui::PushStyleColor(ImGuiCol_Button,        (ImVec4)ImColor::HSV(140.0f / 360.0f, 0.4f, 0.45f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(140.0f / 360.0f, 0.5f, 0.55f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive,  (ImVec4)ImColor::HSV(140.0f / 360.0f, 0.6f, 0.65f));
		record_button_ = ImGui::Button("Record");
	}
	else {
		ImGui::PushStyleColor(ImGuiCol_Button,        (ImVec4)ImColor::HSV(354.0f / 360.0f, 0.75f, 0.45f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(354.0f / 360.0f, 0.85f, 0.55f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive,  (ImVec4)ImColor::HSV(354.0f / 360.0f, 0.95f, 0.65f));
		record_button_ = ImGui::Button("Stop");
	}
	ImGui::PopStyleColor(3);
	ImGui::PopID();
	ImGui::SameLine();
	ImGui::Checkbox("Gui", &show_gui_);
}

std::string GetTimeString() {
	typedef std::chrono::system_clock SteadyClock;
	std::time_t now_c = SteadyClock::to_time_t(SteadyClock::now());
	struct tm* t = std::localtime(&now_c);
	std::stringstream ss;
	ss << t->tm_year << "-" << t->tm_mon << "-" << t->tm_mday
		<< "_" << t->tm_hour << "-" << t->tm_min << "-" << t->tm_sec;
	return ss.str();
}

std::string PopulateFileName(std::string prefix, std::string basic_filename, std::string postfix) {
	std::string file_name = prefix + basic_filename;
	int cnt = 0;
	while (fs::exists(file_name)) {
		file_name = basic_filename + std::to_string(cnt++);
	}
	return file_name + postfix;
}

void PlaybackSystem::Update(Clock& clock)
{
	std::tie(width_, height_) = renderer::GetWindowSize();
	if (screenshot_buffer_.size() < static_cast<size_t>(width_) * height_) {
		screenshot_buffer_.resize(static_cast<size_t>(width_) * height_);
	}

	{
		renderer::ScopedState scope(show_gui_ ? renderer::RenderPass::kSkybox : renderer::RenderPass::kPostProc);
		renderer::ReadFromDefaultGBufferAttachment(-1, [this] {

			if (!screenshot_button_ && !record_button_ && !IsRecording()) {
				return;
			}
			renderer::ScreenShot(width_, height_, screenshot_buffer_.begin());

			if (screenshot_button_) {
				std::string file_name = PopulateFileName("playback/screenshot_", GetTimeString(), ".png");

				stbi_flip_vertically_on_write(true);
				if (write_screenshot_promise_.valid()) {
					write_screenshot_promise_.wait();
				}
				write_screenshot_promise_ = std::async([this, file_name] {
					stbi_write_png(file_name.c_str(), width_, height_, 3, screenshot_buffer_.begin(), sizeof(renderer::SolidPixel) * width_);
				});
			}

			if (IsRecording()) {
				VideoStreamer* vs = reinterpret_cast<VideoStreamer*>(hidden_video_streamer_);
				vs->Encode(screenshot_buffer_);
			}

			if (record_button_) {

				if (!IsRecording()) {
					std::string file_name = PopulateFileName("playback/record_", GetTimeString(), ".mp4");
					VideoStreamer* vs = new VideoStreamer();
					hidden_video_streamer_ = vs;

					vs->Init(file_name.c_str(), width_, height_);
				}
				else
				{
					VideoStreamer* vs = reinterpret_cast<VideoStreamer*>(hidden_video_streamer_);
					vs->End();
					delete vs;
					hidden_video_streamer_ = nullptr;
				}

			}
		});
	}



}



}


