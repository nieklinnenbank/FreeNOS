/*
 * Copyright (C) 2009 Niek Linnenbank, 2015 Dan Rulos, 2016 Alvaro Stagg [alvarostagg@openmailbox.org]
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
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#define VERSION "v0.5.8"

static int hflag, vflag, ret = 0;
static int flags[8];

static void usage(void);
static void version(void);
static void cat(char *argv[], int argc, int flags[8]);
static void read(const char *fileName, int flags[8]);
static const char *fileName;

int get_file_size(const char *fileName);

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Uso: %s [OPCIÓN]... [FICHERO]...\n", argv[0]);
        return 1;
    }

    for (int i = 0; i <= 8; i++)
    	flags[i] = 0;

    for (int i = 1; i < argc; i++)
    {
    	if (strcmp(argv[i], "--help") == 0)
    	{
    		hflag = 1;
    		usage();
		break;
    	}
    	else if ((strcmp(argv[i], "--version") == 0))
    	{
    		vflag = 1;
    		version();
    		break;
    	}
    	else if ((strcmp(argv[i], "-n") == 0) || (strcmp(argv[i], "--number") == 0))
		flags[0] = 1;
	else if ((strcmp(argv[i], "-E") == 0) || (strcmp(argv[i], "--show-ends") == 0) || (strcmp(argv[i], "-e") == 0))
		flags[1] = 1;
	else if ((strcmp(argv[i], "-t") == 0) || (strcmp(argv[i], "-T") == 0) || (strcmp(argv[i], "--show-tabs") == 0))
		flags[2] = 1;
	else if ((strcmp(argv[i], "-b") == 0) || (strcmp(argv[i], "--number-nonblank") == 0))
	{
		flags[0] = 0;
		flags[3] = 1;
	}
    }

    if (hflag || vflag)
    	return ret;
    else
    	cat(argv, argc, flags);

    return ret;
}

static void usage(void)
{
	printf("Modo de empleo: cat [OPCIÓN]... [FICHERO]...\n\n");

	printf("  -b, --number-nonblank    No enumera las lineas en blanco, remplaza -n.\n");
	printf("  -e                       Lo mismo que -E.\n");
	printf("  -E, --show-ends          Muestra un $ al final de cada linea.\n");
	printf("  -n, --number             Enumera cada linea.\n");
	printf("  -t                       Lo mismo que -T.\n");
	printf("  -T, --show-tabs          Muestra los caracteres de tabulación como ^I\n");
	printf("      --help               Muestra esta ayuda y finaliza.\n");
	printf("      --version            Informa de la versión y finaliza.\n\n");

	printf("AmayaOS Coreutils %s (C) 2016 AmayaOS Team & Others\n", VERSION);
	printf("Licencia GNU GPL v3 <http://www.gnu.org/licenses/>.\n");
	printf("Reportar errores a traves de http://bugs.amayaos.com o amaya@amayaos.com\n");
}

static void version(void)
{
	printf("cat (AmayaOS CoreUtils) %s\n", VERSION);
	printf("Copyright © 2016 AmayaOS, Inc.\n");
	printf("Licencia GPLv3+: GPL de GNU versión 3 o posterior\n");
	printf("<http://gnu.org/licenses/gpl.html>.\n");
	printf("Esto es software libre: usted es libre de cambiarlo y redistribuirlo.\n");
	printf("No hay NINGUNA GARANTÍA, hasta donde permite la ley.\n\n");

	printf("Escrito por Dan Rulos y Alvaro Stagg.\n");
}

static void cat(char *argv[], int argc, int flags[8])
{
	int fd;

	for (int i = 1; i < argc; i++)
	{
		if ((fd = open(argv[i], O_RDONLY)) < 0)
			continue;
		else
		{
			fileName = argv[i];
			break;
		}
	}

	if (fd < 0)
	{
		printf("%s: No existe el fichero o el directorio.\n", argv[0]);
		ret = 1;
	}
	else
	{
		read(fileName, flags);
		close(fd);
	}
}

static void read(const char *fileName, int flags[8])
{
	FILE *fp = fopen(fileName, "r");
	int lines = 0;

	if (fp == NULL)
	{
		printf("cat: %s: No existe el fichero o el directorio.\n", fileName);
		ret = 1;
	}
	else
	{
		char cnt[get_file_size(fileName)];
		int n_bytes = fread(cnt, 1, sizeof(cnt), fp);

		if (flags[0] == 1 || (flags[3] == 1 && cnt[0] != '\n'))
		{
			lines++;
			printf("     %d  ", lines);
		}

		unsigned int i = 0;

		for (i = 0; i < n_bytes - 1; i++)
		{
			printf("%c", cnt[i]);

			if (flags[0] == 1)
			{
				if (cnt[i] == '\n')
				{
					lines++;
					printf("     %d  ", lines);
				}
			}
			
			if (flags[1] == 1)
			{
				if (cnt[i + 1] == '\n')
					printf("$");
			}

			if (flags[2] == 1)
			{
				if (cnt[i] == '\t')
				{
					printf("^I");
					continue;
				}
			}

			if (flags[3] == 1)
			{
				if (cnt[i] == '\n')
				{
					if (cnt[i + 1] == '\n')
						continue;
					else
					{
						lines++;
						printf("     %d  ", lines);
					}
				}
			}
		}

		/* Imprime el último caracter perdido */
		printf("%c\n", cnt[i]);
	}
}

int get_file_size(const char *fileName)
{
	struct stat st;

	if (stat(fileName, &st) < 0)
		return -1;
	else
		return st.st_size;
}
