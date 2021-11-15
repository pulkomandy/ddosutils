/*
 * linux.c
 * Copyright (C) 2021 pulkomandy <pulkomandy@kitt>
 *
 * Distributed under terms of the MIT license.
 */

#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>

int fd;

const char* FILENAME;

void* malloc_dma_page_aligned_block(int size)
{
	return malloc(size);
}

void free_dma_page_aligned_block(void* ptr)
{
	free(ptr);
}

void bios_reset_drive(int drivenum)
{
	if (fd != 0)
		close(fd);
	fd = open(FILENAME, O_RDONLY);
	// TODO read track and side count
	// TODO guess sector size from filesize?
}

int bios_read_sectors(int drive, int track, int side, int sector, int count, unsigned char* buffer, int secsize)
{
	int offset;
	int i;
	int sizeRead;

	track *= 2;
	track += side;
	offset = secsize * (sector - 1 + 18 * track);

	lseek(fd, offset + 12, SEEK_SET);
	sizeRead = read(fd, buffer, secsize * count);

	return 0;
}

const char* bios_error_description(int error)
{
	return "FAKE ERROR";
}

void hook_disk_base_table()
{
}

void update_disk_base_table()
{
}

void unhook_disk_base_table()
{
}

void strlwr(char* string)
{
	int i;
	for (i = 0; string[i]; i++)
		string[i] = tolower(string[i]);
}
