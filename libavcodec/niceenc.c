/*
 * BMP image format encoder
 * Copyright (c) 2006, 2007 Michel Bardiaux
 * Copyright (c) 2009 Daniel Verkamp <daniel at drv.nu>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "libavutil/imgutils.h"
#include "libavutil/avassert.h"
#include "avcodec.h"
#include "bytestream.h"
#include "nice.h"
#include "internal.h"

static const uint32_t monoblack_pal[] = { 0x000000, 0xFFFFFF };
static const uint32_t rgb565_masks[]  = { 0xF800, 0x07E0, 0x001F };
static const uint32_t rgb444_masks[]  = { 0x0F00, 0x00F0, 0x000F };

static av_cold int nice_encode_init(AVCodecContext *avctx){
  if(avctx->pix_fmt == AV_PIX_FMT_RGB8)
    avctx->bits_per_coded_sample = 8;
  else {
        av_log(avctx, AV_LOG_INFO, "unsupported pixel format\n");
        return AVERROR(EINVAL);
   }

    return 0;
}

static int nice_encode_frame(AVCodecContext *avctx, AVPacket *pkt,
                            const AVFrame *pict, int *got_packet)
{
    const AVFrame * const p = pict;
    int n_bytes_image, n_bytes_per_row, n_bytes, i, n, header_size, ret;
    const uint32_t *pal = NULL;
    uint32_t palette256[256];
    int pad_bytes_per_row, pal_entries = 0;
    int bit_count = avctx->bits_per_coded_sample;
    uint8_t *ptr, *buf;

#if FF_API_CODED_FRAME
FF_DISABLE_DEPRECATION_WARNINGS
  avctx->coded_frame->pict_type = AV_PICTURE_TYPE_I;
    avctx->coded_frame->key_frame = 1;
FF_ENABLE_DEPRECATION_WARNINGS
#endif
    avctx->pix_fmt = AV_PIX_FMT_RGB8;


    if (pal && !pal_entries) pal_entries = 1 << bit_count;
    n_bytes_per_row = ((int64_t)avctx->width * (int64_t)bit_count + 7LL) >> 3LL;
    pad_bytes_per_row = (4 - n_bytes_per_row) & 3;
    n_bytes_image = avctx->height * (n_bytes_per_row + pad_bytes_per_row);

    //#define SIZE_BITMAPFILEHEADER 14
    //#define SIZE_BITMAPINFOHEADER 40

    header_size = 12;

    printf("after adding 2: %d\n", header_size);
    n_bytes = n_bytes_image + header_size;
    if ((ret = ff_alloc_packet2(avctx, pkt, n_bytes, 0)) < 0)
       return ret;
    buf = pkt->data;
    bytestream_put_byte(&buf, 'N');                   // BITMAPFILEHEADER.bfType
    bytestream_put_byte(&buf, 'I');                   // do.
    bytestream_put_byte(&buf, 'C');
    bytestream_put_byte(&buf, 'E');

    bytestream_put_le32(&buf, avctx->width);          // BITMAPINFOHEADER.biWidth
    bytestream_put_le32(&buf, avctx->height);         // BITMAPINFOHEADER.biHeight
    bytestream_put_le32(&buf, pad_bytes_per_row);     //pad bytes per row


    buf = pkt->data + header_size;
    ptr = p->data[0] + (avctx->height - 1) * p->linesize[0];
    //JoCee Messing with things.  Gives a segmentation fault.
    /* int counter = 0; */
    /* //ptr = p->data[0] + (avctx->height - 1) * p->linesize[0]; */
    /* ptr = p->data[0]; */
    /* //buf = pkt->data + header_size; */
    /* for(i = 0; i<avctx->height; i++) */
    /*   { */
    /* 	for(int j = 0; j<avctx->width; j++) */
    /* 	  { */
    /* 	    bytestream_put_byte(&buf, ptr); */
    /* 	    printf("no segmentation fault %u \n", counter); */
    /* 	    buf += 1; */
    /* 	    ptr -=1; */
    /* 	    counter ++; */
    /* 	  }	 */
	
    /*   } */
    int counter = 0;
    for(i = 0; i < avctx->height; i++) {
      printf("no segmentation fault %u \n", counter);
            memcpy(buf, ptr, n_bytes_per_row);
    	    buf += n_bytes_per_row;
    	    memset(buf, 0, pad_bytes_per_row);
    	    buf += pad_bytes_per_row;
    	    ptr -= p->linesize[0]; // ... and go back
    	    counter = counter +1;
    }

    pkt->flags |= AV_PKT_FLAG_KEY;
    *got_packet = 1;

    return 0;
}

AVCodec ff_nice_encoder = {
    .name           = "nice",
    .long_name      = NULL_IF_CONFIG_SMALL("NICE image (a project for CS 3505)"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_NICE,
    .init           = nice_encode_init,
    .encode2        = nice_encode_frame,
    .pix_fmts       = (const enum AVPixelFormat[]){
         AV_PIX_FMT_RGB8,
        AV_PIX_FMT_NONE
    },
};
