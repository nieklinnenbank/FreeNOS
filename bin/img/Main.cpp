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

#include <FreeNOS/BootImage.h>
#include <Arch/Memory.h>
#include <FreeNOS/Config.h>
#include <Array.h>
#include <ExecutableFormat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "BootEntry.h"
#include "Main.h"

Size readBootEntries(char *prog, char *file,
                    Array<BootEntry> *entries)
{
    char line[128];
    int num = 0;
    Size totalBytes = 0, totalEntries = 0;
    BootEntry *entry;
    FILE *fp;
    
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
    
        /* Attempt to parse the executable headers. */
        if (!(entry->format = ExecutableFormat::find(strdup(line))))
        {
            fprintf(stderr, "%s: failed to read `%s': %s\n",
                    prog, line, strerror(errno));
            exit(EXIT_FAILURE);
        }
        /* Extract memory regions. */
        else if ((num = entry->format->regions(entry->regions, MAX_REGIONS)) <= 0)
        {
            fprintf(stderr, "%s: failed to extract memory regions from `%s': %s\n",
                        prog, line, strerror(errno));
            exit(EXIT_FAILURE);
        }
        else
            entry->numRegions = num;

        /* Insert into Array. */
        entries->insert(entry);
        totalEntries++;
        
        /* Debug out memory sections. */
        for (Size i = 0; i < entry->numRegions; i++)
        {
            printf("%s[%u]: vaddr=%x size=%u\n",
                    line, i, (uint) entry->regions[i].virtualAddress,
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
    Array<BootEntry> entries(128);
    BootImage image;
    BootVariable variables[12];
    BootProgram *programs;
    BootSegment *segments;
    FILE *fp;
    Size numEntries, numSegments = 0, dataOffset = 0;
    
    /* Verify command-line arguments. */
    if (argc < 3)
    {
        fprintf(stderr, "usage: %s CONFFILE OUTFILE\n",
                argv[0]);
        return EXIT_FAILURE;
    }
    /* Read executables. */
    numEntries = readBootEntries(argv[0], argv[1], &entries);

    /* Allocate tables. */
    programs = new BootProgram[numEntries];

    /* Clear first. */
    memset(&image, 0, sizeof(image));
    memset(&variables, 0, sizeof(variables));
    memset(programs, 0, sizeof(BootProgram) * numEntries);

    /* Fill in the boot image header. */
    image.magic[0]       = BOOTIMAGE_MAGIC0;
    image.magic[1]       = BOOTIMAGE_MAGIC1;
    image.layoutRevision = BOOTIMAGE_REVISION;
    image.variablesTableOffset = sizeof(BootImage);
    image.variablesTableCount  = 12;
    image.programsTableOffset  = image.variablesTableOffset + 
                                 (image.variablesTableCount  * sizeof(BootVariable));
    image.programsTableCount   = numEntries;
    image.segmentsTableOffset  = image.programsTableOffset  +
                                 (image.programsTableCount   * sizeof(BootProgram));

    /* Fill in the boot variables. */
    VARIABLE(variables[0],  RELEASE);
    VARIABLE(variables[1],  COMPILER);
    VARIABLE(variables[2],  DATETIME);
    VARIABLE(variables[3],  ARCH);
    VARIABLE(variables[4],  BUILDUSER);
    VARIABLE(variables[5],  BUILDHOST);
    VARIABLE(variables[6],  BUILDOS);
    VARIABLE(variables[7],  BUILDARCH);
    VARIABLE(variables[8],  BUILDPY);
    VARIABLE(variables[9],  BUILDER);
    VARIABLE(variables[10], BUILDPATH);
    VARIABLE(variables[11], BUILDURL);

    /* Fill in the boot programs. */
    for (Size i = 0; i < numEntries; i++)
    {
        strncpy(programs[i].path, entries[i]->format->getPath(), BOOTIMAGE_PATH);
        programs[i].entry = (u32) entries[i]->format->entry();
        programs[i].segmentsOffset = numSegments;
        programs[i].segmentsCount  = entries[i]->numRegions;
        numSegments += entries[i]->numRegions;
    }
    /* Update BootImage. */
    image.segmentsTableCount = numSegments;
    
    /* Now we allocate and clear the segments table. */
    segments = new BootSegment[numSegments];
    memset(segments, 0, sizeof(BootSegment) * numSegments);
    
    /* Point segment data after the segments table. */
    dataOffset  = image.segmentsTableOffset +
                  image.segmentsTableCount  * sizeof(BootSegment);
    dataOffset += PAGESIZE - (dataOffset % PAGESIZE);
    
    /* Fill the segments table by looping programs. */
    for (Size i = 0; i < numEntries; i++)
    {
        /* Loop this program's segments. */
        for (Size j = 0; j < entries[i]->numRegions; j++)
        {
            /* Fill in the segment. */
            segments[i].virtualAddress = entries[i]->regions[j].virtualAddress;
            segments[i].size           = entries[i]->regions[j].size;
            segments[i].offset         = dataOffset;
            
            /* Increment data pointer. Align on memory page boundary. */
            dataOffset += segments[i].size;
            dataOffset += PAGESIZE - (dataOffset % PAGESIZE);
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
    if (fwrite(&image,     sizeof(image),     1, fp) <= 0 ||
        fwrite(&variables, sizeof(variables), 1, fp) <= 0 ||
        fwrite( programs,  sizeof(BootProgram) * numEntries,  1, fp) <= 0 ||
        fwrite( segments,  sizeof(BootSegment) * numSegments, 1, fp) <= 0)
    {
        fprintf(stderr, "%s: failed to write BootImage headers to `%s': %s\n",
                argv[0], argv[2], strerror(errno));
        return EXIT_FAILURE;
    }
    /* Write the contents of the BootSegments. */
    for (Size i = 0; i < numEntries; i++)
    {
        /* Loop regions/segments per BootProgram entry. */
        for (Size j = 0; j < entries[i]->numRegions; j++)
        {
            /* Adjust file pointer. */
            if (fseek(fp, segments[programs[i].segmentsOffset].offset,
                      SEEK_SET) == -1)
            {
                fprintf(stderr, "%s: failed to seek to BootSegment contents in `%s': %s\n",
                        argv[0], argv[2], strerror(errno));
                return EXIT_FAILURE;
            }
            /* Write segment contents. */
            if (fwrite(entries[i]->regions[j].data,
                       entries[i]->regions[j].size, 1, fp) <= 0)
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
