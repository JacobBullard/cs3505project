/*
 * NICE image format decoder
 *
 * BMP decoder that was modified by Jacob Bullard and Jocee Porter for CS 3505 Spring 2018
 *
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
    const uint8_t *buf = avpkt->data;
    int buf_size       = avpkt->size;
    AVFrame *p         = data;
    unsigned int hsize;
    int width, height;
    unsigned int depth;
    int i, n, linesize, ret;
    uint8_t *ptr;
    const uint8_t *buf0 = buf;
    int dsize;

    if (bytestream_get_byte(&buf) != 'N' ||
        bytestream_get_byte(&buf) != 'I' ||
        bytestream_get_byte(&buf) != 'C' || 
        bytestream_get_byte(&buf) != 'E' ) 
      {
        av_log(avctx, AV_LOG_ERROR, "bad magic number\n");
        return AVERROR_INVALIDDATA;
      }

    // Set the value of header and depth, load our width and height from header
    hsize = 12;
    width  = bytestream_get_le32(&buf); 
    height = bytestream_get_le32(&buf);
    depth = 8; // Found this by checking where it was inserted into the header from the original bmpenc.c. This value is the same as bit_count from bmpenc.c

    // Error checking by setting dimensions
    ret = ff_set_dimensions(avctx, width, height > 0 ? height : -(unsigned)height);
    if (ret < 0) {
        av_log(avctx, AV_LOG_ERROR, "Failed to set dimensions %d %d\n", width, height);
        return AVERROR_INVALIDDATA;
    }

    // Set our pixel format
    avctx->pix_fmt = AV_PIX_FMT_RGB8;

    if ((ret = ff_get_buffer(avctx, p, 0)) < 0)
        return ret;

    // Code deemed necessary by professor
    p->pict_type = AV_PICTURE_TYPE_I;
    p->key_frame = 1;

    // The following code is needed to make sure that images are not skewed
    buf   = buf0 + hsize;
    dsize = buf_size - hsize;

    /* Line size in file multiple of 4 */
    n = ((avctx->width * depth + 31) / 8) & ~3;

    if (n * avctx->height > dsize) 
        n = (avctx->width * depth + 7) / 8;
   
     if (height > 0) 
       { 
         ptr      = p->data[0] + (avctx->height - 1) * p->linesize[0]; 
         linesize = -p->linesize[0]; 
       } 
     else 
       {
         ptr      = p->data[0]; 
         linesize = p->linesize[0]; 
       } 

     for (i = 0; i < avctx->height; i++) 
       {
                memcpy(ptr, buf, n);
                buf += n;
                ptr += linesize;
       }

    *got_frame = 1;

    return buf_size;
}

AVCodec ff_nice_decoder = 
{
    .name           = "nice",
    .long_name      = NULL_IF_CONFIG_SMALL("NICE image (a project for CS 3505)"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_NICE,
    .decode         = nice_decode_frame,
    .capabilities   = AV_CODEC_CAP_DR1,
};
