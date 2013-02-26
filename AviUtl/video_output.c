/*****************************************************************************
 * video_output.c
 *****************************************************************************
 * Copyright (C) 2012-2013 L-SMASH Works project
 *
 * Authors: Yusuke Nakamura <muken.the.vfrmaniac@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *****************************************************************************/

/* This file is available under an ISC license.
 * However, when distributing its binary file, it will be under LGPL or GPL.
 * Don't distribute it if its license is GPL. */

#include "lsmashinput.h"

#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>

#include "colorspace.h"
#include "video_output.h"

output_colorspace_index determine_colorspace_conversion
(
    enum AVPixelFormat  input_pixel_format,
    enum AVPixelFormat *output_pixel_format
)
{
    DEBUG_VIDEO_MESSAGE_BOX_DESKTOP( MB_OK, "input_pixel_format = %s", (av_pix_fmt_desc_get( input_pixel_format ))->name );
    avoid_yuv_scale_conversion( &input_pixel_format );
    switch( input_pixel_format )
    {
        case AV_PIX_FMT_YUV444P :
        case AV_PIX_FMT_YUV440P :
        case AV_PIX_FMT_YUV420P9LE :
        case AV_PIX_FMT_YUV420P9BE :
        case AV_PIX_FMT_YUV422P9LE :
        case AV_PIX_FMT_YUV422P9BE :
        case AV_PIX_FMT_YUV444P9LE :
        case AV_PIX_FMT_YUV444P9BE :
        case AV_PIX_FMT_YUV420P10LE :
        case AV_PIX_FMT_YUV420P10BE :
        case AV_PIX_FMT_YUV422P10LE :
        case AV_PIX_FMT_YUV422P10BE :
        case AV_PIX_FMT_YUV444P10LE :
        case AV_PIX_FMT_YUV444P10BE :
        case AV_PIX_FMT_YUV420P16LE :
        case AV_PIX_FMT_YUV420P16BE :
        case AV_PIX_FMT_YUV422P16LE :
        case AV_PIX_FMT_YUV422P16BE :
        case AV_PIX_FMT_YUV444P16LE :
        case AV_PIX_FMT_YUV444P16BE :
        case AV_PIX_FMT_RGB48LE :
        case AV_PIX_FMT_RGB48BE :
        case AV_PIX_FMT_BGR48LE :
        case AV_PIX_FMT_BGR48BE :
        case AV_PIX_FMT_GBRP9LE :
        case AV_PIX_FMT_GBRP9BE :
        case AV_PIX_FMT_GBRP10LE :
        case AV_PIX_FMT_GBRP10BE :
        case AV_PIX_FMT_GBRP16LE :
        case AV_PIX_FMT_GBRP16BE :
            *output_pixel_format = AV_PIX_FMT_YUV444P16LE;  /* planar YUV 4:4:4, 48bpp little-endian -> YC48 */
            return OUTPUT_YC48;
        case AV_PIX_FMT_ARGB :
        case AV_PIX_FMT_RGBA :
        case AV_PIX_FMT_ABGR :
        case AV_PIX_FMT_BGRA :
            *output_pixel_format = AV_PIX_FMT_BGRA;         /* packed BGRA 8:8:8:8, 32bpp, BGRABGRA... */
            return OUTPUT_RGBA;
        case AV_PIX_FMT_RGB24 :
        case AV_PIX_FMT_BGR24 :
        case AV_PIX_FMT_BGR8 :
        case AV_PIX_FMT_BGR4 :
        case AV_PIX_FMT_BGR4_BYTE :
        case AV_PIX_FMT_RGB8 :
        case AV_PIX_FMT_RGB4 :
        case AV_PIX_FMT_RGB4_BYTE :
        case AV_PIX_FMT_RGB565LE :
        case AV_PIX_FMT_RGB565BE :
        case AV_PIX_FMT_RGB555LE :
        case AV_PIX_FMT_RGB555BE :
        case AV_PIX_FMT_BGR565LE :
        case AV_PIX_FMT_BGR565BE :
        case AV_PIX_FMT_BGR555LE :
        case AV_PIX_FMT_BGR555BE :
        case AV_PIX_FMT_RGB444LE :
        case AV_PIX_FMT_RGB444BE :
        case AV_PIX_FMT_BGR444LE :
        case AV_PIX_FMT_BGR444BE :
        case AV_PIX_FMT_GBRP :
        case AV_PIX_FMT_PAL8 :
            *output_pixel_format = AV_PIX_FMT_BGR24;        /* packed RGB 8:8:8, 24bpp, BGRBGR... */
            return OUTPUT_RGB24;
        default :
            *output_pixel_format = AV_PIX_FMT_YUYV422;      /* packed YUV 4:2:2, 16bpp */
            return OUTPUT_YUY2;
    }
}

int convert_colorspace
(
    lw_video_output_handler_t *vohp,
    AVCodecContext            *ctx,
    AVFrame                   *picture,
    uint8_t                   *buf
)
{
    /* Convert color space. We don't change the presentation resolution. */
    au_video_output_handler_t *au_vohp = (au_video_output_handler_t *)vohp->private_handler;
    enum AVPixelFormat *input_pixel_format = (enum AVPixelFormat *)&picture->format;
    int yuv_range = avoid_yuv_scale_conversion( input_pixel_format );
    lw_video_scaler_handler_t *vshp = &vohp->scaler;
    if( !vshp->sws_ctx
     || vshp->input_width        != picture->width
     || vshp->input_height       != picture->height
     || vshp->input_pixel_format != *input_pixel_format
     || vshp->input_colorspace   != ctx->colorspace
     || vshp->input_yuv_range    != yuv_range )
    {
        /* Update scaler. */
        vshp->sws_ctx = update_scaler_configuration( vshp->sws_ctx, vshp->flags,
                                                     picture->width, picture->height,
                                                     *input_pixel_format, vshp->output_pixel_format,
                                                     ctx->colorspace, yuv_range );
        if( !vshp->sws_ctx )
            return 0;
        vshp->input_width        = picture->width;
        vshp->input_height       = picture->height;
        vshp->input_pixel_format = *input_pixel_format;
        vshp->input_colorspace   = ctx->colorspace;
        vshp->input_yuv_range    = yuv_range;
        memcpy( buf, au_vohp->back_ground, vohp->output_frame_size );
    }
    if( au_vohp->convert_colorspace( ctx, vshp->sws_ctx, picture, buf, vohp->output_linesize, vohp->output_height ) < 0 )
        return 0;
    return vohp->output_frame_size;
}

void free_au_video_output_handler( void *private_handler )
{
    au_video_output_handler_t *au_vohp = (au_video_output_handler_t *)private_handler;
    if( !au_vohp )
        return;
    if( au_vohp->back_ground )
        free( au_vohp->back_ground );
    free( au_vohp );
}