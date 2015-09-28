/*
 * Copyright (C) 2015 Niek Linnenbank
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __LIBARCH_ARM_TAGS_H
#define __LIBARCH_ARM_TAGS_H

#include <Types.h>
#include <Macros.h>
#include <Memory.h>

struct ATagHeader
{
    u32 size; /**<< length of tag in words including this header */
    u32 tag;  /**<< tag type */
};

struct ATagCore
{
    u32 flags;
    u32 pagesize;
    u32 rootdev;
};

struct ATagMemory
{
    u32 size;
    u32 start;
};

struct ATagVideoText
{
    u8  x;
    u8  y;
    u16 video_page;
    u8  video_mode;
    u8  video_cols;
    u16 video_ega_bx;
    u8  video_lines;
    u8  video_isvga;
    u16 video_points;
};

struct ATagRamdisk
{
    u32 flags;
    u32 size;
    u32 start;
};

struct ATagInitrd2
{
    u32 start;
    u32 size;
};

struct ATagSerialnr
{
    u32 low;
    u32 high;
};

struct ATagRevision
{
    u32 rev;
};

struct ATagVideoBuffer
{
    u16 lfb_width;
    u16 lfb_height;
    u16 lfb_depth;
    u16 lfb_linelength;
    u32 lfb_base;
    u32 lfb_size;
    u8  red_size;
    u8  red_pos;
    u8  green_size;
    u8  green_pos;
    u8  blue_size;
    u8  blue_pos;
    u8  rsvd_size;
    u8  rsvd_pos;
};

struct ATagCommandLine
{
    char cmdline[1];
};

struct ATag
{
    struct ATagHeader hdr;
    union
    {
        struct ATagCore         core;
        struct ATagMemory       mem;
        struct ATagVideoText    videotext;
        struct ATagRamdisk      ramdisk;
        struct ATagInitrd2      initrd2;
        struct ATagSerialnr     serialnr;
        struct ATagRevision     revision;
        struct ATagVideoBuffer  videolfb;
        struct ATagCommandLine  cmdline;
    } u;
};

/**
 * ARM boot tags implementation.
 *
 * @see http://www.simtec.co.uk/products/SWLINUX/files/booting_article.html#appendix_tag_reference
 */
class ARMTags
{
  public:

    /**
     * ARM boot tag types.
     */
    typedef enum Type
    {
        None        = 0x00000000,
        Core        = 0x54410001,
        Memory      = 0x54410002,
        VideoText   = 0x54410003,
        RamDisk     = 0x54410004,
        InitRd2     = 0x54420005,
        Serial      = 0x54410006,
        Revision    = 0x54410007,
        VideoBuffer = 0x54410008,
        CommandLine = 0x54410009
    }
    Type;

    /**
     * Constructor
     *
     * @param base Start address of the ARM tags.
     */
    ARMTags(Address base);

    /**
     * Get the InitRd2 address.
     *
     * @return Memory range of the initrd image.
     */
    Memory::Range getInitRd2();

  private:

    /** Base memory address of the ARM tags */
    Address m_base;
};

#endif /* __LIBARCH_ARM_TAGS_H */
