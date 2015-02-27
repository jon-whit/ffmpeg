/*
 * MPFF file format encoder
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
#include <stdio.h>
#include "libavutil/imgutils.h"
#include "libavutil/avassert.h"
#include "avcodec.h"
#include "bytestream.h"
#include "internal.h"

static av_cold int mpff_encode_init(AVCodecContext *avctx){
    avctx->bits_per_coded_sample = 8;

    avctx->coded_frame = av_frame_alloc();
    if (!avctx->coded_frame)
        return AVERROR(ENOMEM);

    return 0;
}

static int mpff_encode_frame(AVCodecContext *avctx, AVPacket *pkt,
                            const AVFrame *pict, int *got_packet)
{
    static int printed = 0;
    char c[] = "*** CS 3505:  Executing in libavcodec/mpffenc.c: In function 'mpff_encode_frame' ***\n";
    if (!printed) {
        printf("%s", c);
        printed = 1;
    }

    const AVFrame * const p = pict;
    int n_bytes_image, n_bytes_per_row, n_bytes, i, header_size, ret;
    const uint32_t *pal = NULL;
    uint32_t palette256[256];
    int pal_entries = 0;
    int bit_count = avctx->bits_per_coded_sample;
    uint8_t *ptr, *buf;

    avctx->coded_frame->pict_type = AV_PICTURE_TYPE_I;
    avctx->coded_frame->key_frame = 1;

    // set the palette for 8 bit color
    av_assert1(bit_count == 8);
    avpriv_set_systematic_pal2(palette256, avctx->pix_fmt);
    pal = palette256;

    pal_entries = 1 << bit_count;
    n_bytes_per_row = ((int64_t)avctx->width * (int64_t)bit_count + 7LL) >> 3LL;
    n_bytes_image = avctx->height * (n_bytes_per_row);
    header_size = (pal_entries << 2);
    n_bytes = n_bytes_image + header_size;

    // try to allocate a packet
    if ((ret = ff_alloc_packet2(avctx, pkt, n_bytes)) < 0)
        return ret;

    // encode the file header info
    buf = pkt->data;
    bytestream_put_byte(&buf, 'M');                   
    bytestream_put_byte(&buf, 'P');                   
    bytestream_put_byte(&buf, 'F');
    bytestream_put_byte(&buf, 'F');
    bytestream_put_le32(&buf, avctx->width);         
    bytestream_put_le32(&buf, avctx->height);
    
    for (i = 0; i < pal_entries; i++)
        bytestream_put_le32(&buf, pal[i] & 0xFFFFFF);

    // encode MPFF from top to bottom.
    ptr = p->data[0];
    buf = pkt->data + header_size;
    for(i = 0; i < avctx->height; i++) {
        memcpy(buf, ptr, n_bytes_per_row);
  
        buf += n_bytes_per_row;
        ptr += p->linesize[0]; 
    }

    pkt->flags |= AV_PKT_FLAG_KEY;
    *got_packet = 1;
    return 0;
}

static av_cold int mpff_encode_close(AVCodecContext *avctx)
{
    av_frame_free(&avctx->coded_frame);
    return 0;
}

AVCodec ff_mpff_encoder = {
    .name           = "mpff",
    .long_name = NULL_IF_CONFIG_SMALL("MPFF image (a CS 3505 project)"),
    .type           = AVMEDIA_TYPE_VIDEO,
    .id             = AV_CODEC_ID_MPFF,
    .init           = mpff_encode_init,
    .encode2        = mpff_encode_frame,
    .close          = mpff_encode_close,
    .pix_fmts       = (const enum AVPixelFormat[]){
        AV_PIX_FMT_RGB8, AV_PIX_FMT_NONE
    },
};
