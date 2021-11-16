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
static int headersize;
static int sides;

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

	unsigned char byte[2];

	// Get header size (needed to know where the first secor data starts
	lseek(fd, 2, SEEK_SET);
	read(fd, byte, 2);

	headersize = byte[0] | (byte[1] << 8);

	// Get number of sides (needed to convert track and side number to position in file)
	lseek(fd, 9, SEEK_SET);
	read(fd, byte, 1);
	sides = byte[0];

	// All other fields in the VDK header are currently not used. Assume all disks have 18 sectors
	// of 256 bytes per track.
}

int bios_read_sectors(int drive, int track, int side, int sector, int count, unsigned char* buffer, int secsize)
{
	int offset;
	int i;
	int sizeRead;

	offset = secsize * (sector - 1 + 18 * (track * sides + side));

	lseek(fd, offset + headersize, SEEK_SET);
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
