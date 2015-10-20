/*
 * Copyright (C) 2009 Niek Linnenbank
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

#include <FreeNOS/System.h>
#include <FreeNOS/Config.h>
#include <Vector.h>
#include <ExecutableFormat.h>
#include <BootImage.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include "BootEntry.h"

Size readBootSymbols(char *prog, char *file,
                     Vector<BootEntry *> *entries)
{
    char line[128];
    Size num = MAX_REGIONS;
    Size totalBytes = 0, totalEntries = 0;
    BootEntry *entry;
    FILE *fp;
    ExecutableFormat *format;
    
    /* Open configuration file. */
    if ((fp = fopen(file, "r")) == NULL)
    {
        fprintf(stderr, "%s: failed to open `%s': %s\n",
                prog, file, strerror(errno));
        exit(EXIT_FAILURE);
    }
    /* Read out lines. */
    while (fgets(line, sizeof(line), fp) != NULL)
    {
        /* Clear newline. */
        line[strlen(line) - 1] = 0;

        /* Allocate new boot entry. */
        entry = new BootEntry;
        strncpy(entry->symbol.name, line, BOOTIMAGE_NAMELEN);

        // Find the file
        struct stat st;
        if (stat(line, &st) == -1)
        {
            fprintf(stderr, "%s: failed to stat `%s': %s\n",
                    prog, line, strerror(errno));
            exit(EXIT_FAILURE);
        }
        // Allocate buffer
        u8 *buffer = new u8[st.st_size];

        // Read the file
        FILE *entry_fd = fopen(line, "r");
        if (!entry_fd)
        {
            fprintf(stderr, "%s: failed to open `%s': %s\n",
                    prog, line, strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (fread(buffer, st.st_size, 1, entry_fd) != 1)
        {
            fprintf(stderr, "%s: failed to fread `%s': %s\n",
                    prog, line, strerror(errno));
            exit(EXIT_FAILURE);
        }
        fclose(entry_fd);

        // Try to parse as BootProgram using libexec.
        if (ExecutableFormat::find(buffer, st.st_size, &format) == ExecutableFormat::Success)
        {
            // Extract memory regions
            if (format->regions(entry->regions, &num) != ExecutableFormat::Success || num <= 0)
            {
                fprintf(stderr, "%s: failed to extract memory regions from `%s': %s\n",
                            prog, line, strerror(errno));
                exit(EXIT_FAILURE);
            }
            entry->numRegions   = num;
            entry->symbol.type  = BootProgram;
            format->entry((Address *)&entry->symbol.entry);
        }
        // BootData
        else
        {
            // Fill BootEntry
            entry->symbol.type = BootData;
            entry->numRegions  = 1;
            entry->regions[0].virt = 0;
            entry->regions[0].access = Memory::User | Memory::Readable | Memory::Writable;
            entry->regions[0].size = st.st_size;
            entry->regions[0].data = buffer;
        }
        // Insert into Array
        entries->insert(entry);
        totalEntries++;
        
        /* Debug out memory sections. */
        for (Size i = 0; i < entry->numRegions; i++)
        {
            printf("%s[%u]: vaddr=%x size=%u\n",
                    line, i, (uint) entry->regions[i].virt,
                    entry->regions[i].size);
            totalBytes += entry->regions[i].size;
        }
    }
    /* All done. */
    printf("%d entries, %d bytes total\n", totalEntries, totalBytes);
    return totalEntries;
}    

int main(int argc, char **argv)
{
    Vector<BootEntry *> input;
    BootImage image;
    BootSymbol *symbols;
    BootSegment *segments;
    FILE *fp;
    Size numSegments = 0, dataOffset = 0;
    uint segCount = 0;
    
    /* Verify command-line arguments. */
    if (argc < 3)
    {
        fprintf(stderr, "usage: %s CONFFILE OUTFILE\n",
                argv[0]);
        return EXIT_FAILURE;
    }
    // Read boot symbols
    readBootSymbols(argv[0], argv[1], &input);

    /* Allocate tables. */
    symbols = new BootSymbol[ input.count() ];

    /* Clear first. */
    memset(&image, 0, sizeof(image));
    memset(symbols, 0, sizeof(BootSymbol) * input.count());

    /* Fill in the boot image header. */
    image.magic[0]       = BOOTIMAGE_MAGIC0;
    image.magic[1]       = BOOTIMAGE_MAGIC1;
    image.layoutRevision = BOOTIMAGE_REVISION;
    image.symbolTableOffset   = sizeof(BootImage);
    image.symbolTableCount    = input.count();
    image.segmentsTableOffset  = image.symbolTableOffset  +
                                (image.symbolTableCount   * sizeof(BootSymbol));

    // Fill in the boot symbols
    for (Size i = 0; i < input.count(); i++)
    {
        strncpy(symbols[i].name, input[i]->symbol.name, BOOTIMAGE_NAMELEN);
        symbols[i].type  = input[i]->symbol.type;
        symbols[i].entry = input[i]->symbol.entry;
        symbols[i].segmentsOffset = numSegments;
        symbols[i].segmentsCount  = input[i]->numRegions;
        symbols[i].segmentsTotalSize = 0;
        numSegments += input[i]->numRegions;
    }
    // Update BootImage
    image.segmentsTableCount = numSegments;
    
    /* Now we allocate and clear the segments table. */
    segments = new BootSegment[numSegments];
    memset(segments, 0, sizeof(BootSegment) * numSegments);
    
    /* Point segment data after the segments table. */
    dataOffset  = image.segmentsTableOffset +
                  image.segmentsTableCount  * sizeof(BootSegment);
    dataOffset += PAGESIZE - (dataOffset % PAGESIZE);
    
    /* Fill the segments table by looping symbols */
    for (Size i = 0; i < input.count(); i++)
    {
        /* Loop the symbol segments. */
        for (Size j = 0; j < input[i]->numRegions; j++)
        {
            /* Fill in the segment. */
            segments[segCount].virtualAddress = input[i]->regions[j].virt;
            segments[segCount].size           = input[i]->regions[j].size;
            segments[segCount].offset         = dataOffset;
            
            // Increment total segments size
            symbols[i].segmentsTotalSize += segments[segCount].size;

            /* Increment data pointer. Align on memory page boundary. */
            dataOffset += segments[i].size;
            dataOffset += PAGESIZE - (dataOffset % PAGESIZE);
            segCount++;
        }
    }
    /* Open boot image for writing. */
    if ((fp = fopen(argv[2], "w")) == NULL)
    {
        fprintf(stderr, "%s: failed to open `%s': %s\n",
                argv[0], argv[2], strerror(errno));
        return EXIT_FAILURE;
    }
    /* Write the final boot image headers. */
    if (fwrite(&image,    sizeof(image), 1, fp) <= 0 ||
        fwrite( symbols,  sizeof(BootSymbol)  * input.count(),  1, fp) <= 0 ||
        fwrite( segments, sizeof(BootSegment) * numSegments, 1, fp) <= 0)
    {
        fprintf(stderr, "%s: failed to write BootImage headers to `%s': %s\n",
                argv[0], argv[2], strerror(errno));
        return EXIT_FAILURE;
    }
    /* Write the contents of the BootSegments. */
    for (Size i = 0; i < input.count(); i++)
    {
        /* Loop regions/segments per boot symbol entry. */
        for (Size j = 0; j < input[i]->numRegions; j++)
        {
            /* Adjust file pointer. */
            if (fseek(fp, segments[symbols[i].segmentsOffset].offset,
                      SEEK_SET) == -1)
            {
                fprintf(stderr, "%s: failed to seek to BootSegment contents in `%s': %s\n",
                        argv[0], argv[2], strerror(errno));
                return EXIT_FAILURE;
            }
            /* Write segment contents. */
            if (fwrite(input[i]->regions[j].data,
                       input[i]->regions[j].size, 1, fp) <= 0)
            {
                fprintf(stderr, "%s: failed to write BootSegment contents to `%s': %s\n",
                        argv[0], argv[2], strerror(errno));
                return EXIT_FAILURE;
            }
        }
    }
    /* Close file. */
    fclose(fp);
    
    /* Exit immediately. */
    return EXIT_SUCCESS;
}
