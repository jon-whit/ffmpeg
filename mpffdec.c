/*
 * MPFF image format decoder
 * Copyright (c) 2015 Jonathan Whitaker, Christopher Hartley
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
#include <stdio.h>

#include "avcodec.h"
#include "bytestream.h"
#include "internal.h"
#include "msrledec.h"


static int mpff_decode_frame(AVCodecContext *avctx,
                             void *data, int *got_frame,
                             AVPacket *avpkt)
{
    static int printed = 0;
    char c[] = "*** CS 3505:  Executing in libavcodec/mpffdec.c: In function 'mpff_decode_frame' ***\n";
    if (!printed) {
        printf("%s", c);
        printed = 1;
    }
    const uint8_t *buf = avpkt->data;
    int buf_size       = avpkt->size;
    AVFrame *p         = data;
    int image_width, image_height, depth, ret, n;
    int i, linesize;
    uint8_t *ptr;

    
    // if the first 4 bytes don't match our file header, return an error.
    if (bytestream_get_byte(&buf) != 'M' &&
        bytestream_get_byte(&buf) != 'P' &&
        bytestream_get_byte(&buf) != 'F' &&
        bytestream_get_byte(&buf) != 'F')
    {
        av_log(avctx, AV_LOG_ERROR, "bad magic number\n");
        return AVERROR_INVALIDDATA;
    }

    // get image width and height in bytes.
    //image_width  = bytestream_get_le32(&buf);
    image_width = 420;
    //image_height = bytestream_get_le32(&buf);
    image_height = 280;
    depth = 8;
   
    printf("setting image width and image height\n");
    avctx->width  = image_width > 0 ? image_width : -image_width;
    printf("width set to %d\n", avctx->width);
    avctx->height = image_height > 0 ? image_height : -image_height;
    printf("height set to %d\n", avctx->height);
    
    // set the pixel format.
    avctx->pix_fmt = AV_PIX_FMT_RGB8;

    // get the buffer for the frame
    if ((ret = ff_get_buffer(avctx, p, 0)) < 0)
        return ret;
    p->pict_type = AV_PICTURE_TYPE_I;
    p->key_frame = 1;
    
    printf("Setting linesize..\n");
    if (image_height < 0)
      linesize = -p->linesize[0];
    else
      linesize = p->linesize[0];
    printf("Linesize set to %d\n", linesize);

    printf("Starting at first pixel\n");
    // start at the first pixel of data and get the line size of the image
    ptr = p->data[0];

    n = (avctx->width * depth) / 8;

    printf("Starting to decode data\n");
    // decoding pixel data
    for (i = 0; i < avctx->height; i++) {
      memcpy(ptr, buf, n);
      buf += n;
      ptr += linesize;
    }

    *got_frame = 1;
    
    return buf_size;
}

AVCodec ff_mpff_decoder = {
    .name           = "mpff",
    .long_name      = NULL_IF_CONFIG_SMALL("MPFF image (a CS 3505 project)"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_MPFF,
    .decode         = mpff_decode_frame,
    .capabilities   = CODEC_CAP_DR1,
};
