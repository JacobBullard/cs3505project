/*
 * NICE image format encoder
 *
 * BMP encoder modified by Jacob Bullard and Jocee Porter for CS 3505 FFMPEG project 2018 Spring
 *
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

static av_cold int nice_encode_init(AVCodecContext *avctx)
{
    // Because our only supported Pixel Format is AV_PIX_FMT_RGB8,
    // we can set our bits_per_coded_sample directly to 8
    avctx->bits_per_coded_sample = 8;

    return 0;
}

static int nice_encode_frame(AVCodecContext *avctx, AVPacket *pkt,
                            const AVFrame *pict, int *got_packet)
{
    // Variables needed for encoding
    const AVFrame * const p = pict;
    int n_bytes_per_row, header_size, ret, n_bytes_image, n_bytes;
    int bit_count = avctx->bits_per_coded_sample;
    uint8_t *ptr, *buf;

    // Code that was deemed necessary by professor
#if FF_API_CODED_FRAME
FF_DISABLE_DEPRECATION_WARNINGS
    avctx->coded_frame->pict_type = AV_PICTURE_TYPE_I;
    avctx->coded_frame->key_frame = 1;
FF_ENABLE_DEPRECATION_WARNINGS
#endif

    // Set our header size and number of bytes per row
    n_bytes_per_row = ((int64_t)avctx->width * (int64_t)bit_count + 7LL) >> 3LL;
    header_size = 12;

    // The following code is used for error checking and allocating memory
    n_bytes_image = avctx->height * n_bytes_per_row;
    n_bytes = n_bytes_image + header_size;
    if ((ret = ff_alloc_packet2(avctx, pkt, n_bytes, 0)) < 0)
        return ret;

    // Set our buffer to be the size of the data in our packet and 
    // then fill our header will all the information that we will 
    // need in the decoder
    buf = pkt->data;
    bytestream_put_byte(&buf, 'N');                   
    bytestream_put_byte(&buf, 'I');                   
    bytestream_put_byte(&buf, 'C');
    bytestream_put_byte(&buf, 'E');
    bytestream_put_le32(&buf, avctx->width);
    bytestream_put_le32(&buf, avctx->height);

    // Prepare the picture data buffer and the buffer
    // used to write to file
    ptr = p->data[0] + (avctx->height - 1) * p->linesize[0];
    buf = pkt->data + header_size;
    
    // While we have rows to traverse, 
    // traverse them
    for(int i = 0; i < avctx->height; i++) 
    {
        // Perform a byte-for-byte copy from
        // our picture data buffer into the 
        // buffer used as storage for our file
        memcpy(buf, ptr, n_bytes_per_row);

	// Increment our buffer index address
	buf += n_bytes_per_row;

	// Decrement our picture data buffer index address
        ptr -= p->linesize[0];  
    }

    pkt->flags |= AV_PKT_FLAG_KEY;
    *got_packet = 1;
    return 0;
}

AVCodec ff_nice_encoder = 
{
    .name           = "nice",
    .long_name      = NULL_IF_CONFIG_SMALL("NICE image (a project for CS 3505)"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_NICE,
    .init           = nice_encode_init,
    .encode2        = nice_encode_frame,
    .pix_fmts       = (const enum AVPixelFormat[])
    {
         AV_PIX_FMT_RGB8,
        AV_PIX_FMT_NONE
    },
};
