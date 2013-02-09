/*****************************************************************************
 * video_output.h
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

/* This file is available under an ISC license. */

typedef struct
{
    int                enabled;
    int                flags;
    int                input_width;
    int                input_height;
    enum AVPixelFormat input_pixel_format;
    enum AVPixelFormat output_pixel_format;
    struct SwsContext *sws_ctx;
} video_scaler_handler_t;

typedef struct
{
    video_scaler_handler_t   scaler;
    uint32_t                 first_valid_frame_number;
    void                    *first_valid_frame;
    int                      output_linesize;
    uint32_t                 output_frame_size;
    void                    *private_handler;
    void (*free_private_handler)( void *private_handler );
} video_output_handler_t;
