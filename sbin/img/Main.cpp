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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <Version.h>
#include <ExecutableFormat.h>
#include <arch/BootImage.h>
#include "BootEntry.h"
#include "Main.h"

int readBootEntries(char *prog, char *file,
		    Vector<BootEntry> *entries)
{
    char line[128];
    int totalBytes = 0, totalEntries = 0, num;
    BootEntry *entry;
    FILE *fp;
    
    /* Open configuration file. */
    if ((fp = fopen(file, "r")) == NULL)
    {
	fprintf(stderr, "%s: failed to open `%s': %s\n",
		prog, file, strerror(errno));
	return EXIT_FAILURE;
    }
    /* Read out lines. */
    while (fgets(line, sizeof(line), fp) != NULL)
    {
	/* Clear newline. */
	line[strlen(line) - 1] = 0;

	/* Allocate new boot entry. */
	entry = new BootEntry;
    
	/* Attempt to parse the executable headers. */
	if (!(entry->format = ExecutableFormat::find(line)))
	{
	    fprintf(stderr, "%s: failed to read `%s': %s\n",
		    prog, line, strerror(errno));
	    return EXIT_FAILURE;
	}
	/* Extract memory regions. */
	else if ((num = entry->format->regions(entry->regions, MAX_REGIONS)) <= 0)
        {
	    fprintf(stderr, "%s: failed to extract memory regions from `%s': %s\n",
			prog, line, strerror(errno));
	    return EXIT_FAILURE;
	}
	/* Insert into Vector. */
	entries->insert(entry);
	totalEntries++;
	
	/* Debug out memory sections. */
	for (int i = 0; i < num; i++)
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
    Vector<BootEntry> entries(128);
    BootImage image;
    BootVariable variables[13];
    FILE *fp;
    int numEntries;
    
    /* Verify command-line arguments. */
    if (argc < 3)
    {
	fprintf(stderr, "usage: %s FILE FILE\n",
		argv[0]);
	return EXIT_FAILURE;
    }
    /* Read executables. */
    numEntries = readBootEntries(argv[0], argv[1], &entries);

    /* Clear first. */
    memset(&image, 0, sizeof(image));
    memset(&variables, 0, sizeof(variables));

    /* Fill in the boot image header. */
    image.magic[0]       = BOOTIMAGE_MAGIC0;
    image.magic[1]       = BOOTIMAGE_MAGIC1;
    image.layoutRevision = BOOTIMAGE_REVISION;
    image.variablesTableOffset = sizeof(BootImage);
    image.variablesTableCount  = 13;

    /* Fill in the boot variables. */
    VARIABLE(variables[0],  RELEASE);
    VARIABLE(variables[1],  COMPILER);
    VARIABLE(variables[2],  DATETIME);
    VARIABLE(variables[3],  ARCH);
    VARIABLE(variables[4],  BUILDUSER);
    VARIABLE(variables[5],  BUILDHOST);
    VARIABLE(variables[6],  BUILDOS);
    VARIABLE(variables[7],  BUILDARCH);
    VARIABLE(variables[8],  BUILDCPU);
    VARIABLE(variables[9],  BUILDPY);
    VARIABLE(variables[10], BUILDER);
    VARIABLE(variables[11], BUILDPATH);
    VARIABLE(variables[12], BUILDURL);

    /* Write the final boot image. */
    if ((fp = fopen(argv[2], "w")) == NULL)
    {
	fprintf(stderr, "%s: failed to open `%s': %s\n",
		argv[0], argv[2], strerror(errno));
	return EXIT_FAILURE;
    }
    /* Output BootImage and BootVariables. */
    if (fwrite(&image,     sizeof(image),     1, fp) <= 0 ||
        fwrite(&variables, sizeof(variables), 1, fp) <= 0)
    {
	fprintf(stderr, "%s: failed to write `%s': %s\n",
		argv[0], argv[2], strerror(errno));
    }
    /* Close file. */
    fclose(fp);
    
    /* Exit immediately. */
    return EXIT_SUCCESS;
}
