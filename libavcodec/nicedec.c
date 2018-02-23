/*
 * BMP image format decoder
 *Edited by JoCee Porter and Jacob Bullard for CS3505
 * Copyright (c) 2005 Mans Rullgard
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

#include <inttypes.h>

#include "avcodec.h"
#include "bytestream.h"
#include "nice.h"
#include "internal.h"
#include "msrledec.h"


static int nice_decode_frame(AVCodecContext *avctx,
                            void *data, int *got_frame,
                            AVPacket *avpkt)
{
    static int printed = 0;
    
    printed != 1 ? printf("\n*** CS 3505: Executing in bmp_decode_frame in bmp.c *** \n*** CS 3505: Modified by JoCee Porter & Jacob Bullard ***\n"), printed = 1 : NULL;

    const uint8_t *buf = avpkt->data;
    int buf_size       = avpkt->size;
    AVFrame *p         = data;
    int hsize = 12;
    int width, height;
    int depth = 1;
    BiCompression comp;
    int i, j, n, linesize, ret;
    uint8_t *ptr;
    int dsize;
    const uint8_t *buf0 = buf;
    GetByteContext gb;
    unsigned int pixel;

    if (buf_size < 14) {
        av_log(avctx, AV_LOG_ERROR, "buf size too small (%d)\n", buf_size);
        return AVERROR_INVALIDDATA;
    }

    if (bytestream_get_byte(&buf) != 'N' ||
        bytestream_get_byte(&buf) != 'I' ||
        bytestream_get_byte(&buf) != 'C' || 
        bytestream_get_byte(&buf) != 'E' ) {
        av_log(avctx, AV_LOG_ERROR, "bad magic number\n");
        return AVERROR_INVALIDDATA;
    }

    
        width  = bytestream_get_le32(&buf);
	printf("width : %u \n", width);
        height = bytestream_get_le32(&buf);
	printf("height : %u \n", height);


	int pad_bytes = bytestream_get_le32(&buf);

    ret = ff_set_dimensions(avctx, width, height > 0 ? height : -(unsigned)height);
    if (ret < 0) {
        av_log(avctx, AV_LOG_ERROR, "Failed to set dimensions %d %d\n", width, height);
        return AVERROR_INVALIDDATA;
    }

    avctx->pix_fmt = AV_PIX_FMT_RGB8;


    if ((ret = ff_get_buffer(avctx, p, 0)) < 0)
        return ret;
    p->pict_type = AV_PICTURE_TYPE_I;
    p->key_frame = 1;

    buf   = buf0 + hsize;
    dsize = buf_size - hsize;

    if (height > 0) {
        ptr      = p->data[0] + (avctx->height - 1) * p->linesize[0];
        linesize = -p->linesize[0];
    } else {
        ptr      = p->data[0];
        linesize = p->linesize[0];
    }

    printf("linesize %u \n", linesize);
    //linesize = width;
    //ptr = p->data[0];

    //TODO:  What is n.
    //TODO:  Why is linesize so big?
    n = ((avctx->width * depth + 31) / 8) & ~3;
    printf("n %u \n", n);
    //n = width;
    int counter = 0;
 for (i = 0; i < height; i++) 
   {
     printf("count %u \n", counter);
        memcpy(ptr, buf, n);
        buf += n;
        ptr += linesize;
	counter = counter +1;
   }

    *got_frame = 1;

    return buf_size;
}

AVCodec ff_nice_decoder = {
    .name           = "nice",
    .long_name      = NULL_IF_CONFIG_SMALL("NICE image (a project for CS 3505)"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_NICE,
    .decode         = nice_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};
