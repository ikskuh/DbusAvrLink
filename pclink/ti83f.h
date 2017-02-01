#pragma once

#include <stdio.h>
#include <stdint.h>

#define TI83F_AUTORELEASE (1<<0)
#define TI83F_ARCHIVED (1<<1)

struct ti83f_entry
{
	/**
	 * Type id of the entry.
	 */
	uint8_t type;
	
	/**
	 * The name of the entry.
	 * @remarks The name is stored in tokenized form.
	 */
	uint8_t name[8];
	
	/**
	 * Pointer to the contents of the entry.
	 */
	void * data;
	
	/**
	 * Length of the memory block pointed by @ref data.
	 */
	uint32_t size;

	/** 
   * Flags that modify the behaviour of saving/loading the file.
	 * - If `TI83F_AUTORELEASE` is set, the memory will be released by ti83f_release.
	 * - If `TI83F_ARCHIVED` is set, the entry will be stored in archive.
	 */
	uint32_t flags;
};

struct ti83f_file
{
	/**
	 * An ASCII comment on the file itself.
	 */
	char comment[43];
	
	/**
	 * Number of entries in the @ref entries array.
	 */
	uint32_t length;
	
	/**
	 * An array of all entries stored in the file.
	 */
	struct ti83f_entry * entries;
};

/**
 * Loads a TI83F file from the given file stream.
 * @param stream The stream where the file should be loaded from.
 * @returns      A pointer to the loaded file or NULL on error.
 * @remarks      The returned pointer should be released by @ref ti83f_release.
 */
struct ti83f_file * ti83f_load(FILE * stream);

/**
 * Stores a TI83F file into the given file stream.
 * @param stream The stream where the file should be stored in.
 * @param file   A pointer to the file structure.
 */
void ti83f_store(FILE * stream, struct ti83f_file const * file);

/**
 * Allocates storage for a new file.
 * @param length The number of entries stored in the file.
 * @returns      A pointer to the created file or NULL on error.
 * @remarks      The returned pointer should be released by @ref ti83f_release.
 */
struct ti83f_file * ti83f_new(uint32_t length);

/**
 * Frees a previously loaded file.
 */
void ti83f_release(struct ti83f_file * file);