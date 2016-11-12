#include "pyramid_project.h"

/*
 * Function: determine_blocks
 *
 * Description: This function determines which block(s) in the file are to be
 *              accessed.
 *
 * Inputs:
 *  count - The number of bytes requested for reading/writing.
 *  position - The current position of the file.
 *
 * Outputs:
 *  first - The number of the first block of the range encompassing the
 *   requested bytes.
 *  last - The number of the last block of the range encompassing the requested
 *   bytes.
 */
void AccessFile::determine_blocks(size_t count, long *first, long *last, off_t position) {
  *first = (long) (position / (off_t) BLOCK_SIZE);
  *last = (long) ((position + (off_t) (count - 1)) / (off_t) BLOCK_SIZE);
  return;
}


/*
 * Function: pyramid_close
 *
 * Description: This function closes the file descriptor fd, encrypts the
 *              associated pyramid structure, writes the encrypted pyramid to
 *              the file "[filename]_pyramid", closes this file, and frees the
 *              pyramid.
 *
 * Inputs:
 *  fd - The file descriptor of the file you wish to close.
 *  pyra - A pointer to the top level of the pyramid associated with this file
 *   descriptor.
 *
 * Return value:
 *  0 upon success.
 *  -1 if the attempt to close fd failed; check errno.
 *  -2 if the attempt to close the pyramid file failed; check errno.
 *  -3 if pyra is NULL.
 *  -4 if pyra is not the top level of its pyramid.
 *  -5 if a call to sizeof_pyramid failed.
 *  -6 if a call to encrypt_pyramid failed.
 *  -7 if fd is invalid (it's negative, or it's reserved for stdin, stdout, or
 *   stderr).
 *  -8 if a call to snprintf failed.
 *  -9 if a call to open failed; check errno.
 *  -10 if a call to write didn't write as many bytes as was expected.
 *  -11 if a call to sign_pyramid failed.
 *  -12 if a call to ftruncate failed; check errno.
 *  -13 if a call to lseek failed; check errno.
 */
int AccessFile::pyramid_close(int fd, Pyramid* pyra) {

  // If any of the inputs are invalid, return immediately.
  if (fd < 3) return -7;
  if (pyra == NULL) return -3;
  if (pyra->get_level() != 1) return -4;

  // Erase the contents of the current pyramid file.
  if (ftruncate(pyra->get_pyramid_fd(), 0)) {
    return -12;
  }
  if (lseek(pyra->get_pyramid_fd(), 0, SEEK_SET)) {
    return -13;
  }

  // Determine how many identifiers are capable of being stored in the input
  // pyramid, returning immediately in the event of an error in doing so.
  size_t pyramid_size = pyra->sizeof_pyramid();
  if (pyramid_size == 0) return -5;

  // Allocate space for the block IDs to be copied from the pyramid and the
  // encrypted version of the block IDs, based on the size of the pyramid.
  // Also, allocate space for the randomly generated nonce to be used in the
  // encryption of the pyramid.
  long* identifiers = new long[pyramid_size];
  long* encrypted_identifiers = new long[pyramid_size];
  unsigned char* nonce = new unsigned char[KEY_LENGTH];

  // Copy the pyramid's block IDs to 'identifiers'.
  size_t offset = 0L;
  size_t num_ids = 4L;
  Pyramid* p = pyra;
  while (p) {
    bcopy(p->get_identifiers(), identifiers + offset, sizeof(long) * num_ids);
    offset += num_ids;
    num_ids = num_ids << 2;
    p = p->get_next_level();
  }
  
  // Encrypt 'identifiers' and store it in 'encrypted_identifiers', and store
  // the nonce in 'nonce', while checking for errors.
  int enc_return = pyra->encrypt_pyramid(identifiers, encrypted_identifiers, (int) (pyramid_size * sizeof(long)), nonce, (long) rand());
  if (enc_return < 0) {
    delete[] identifiers;
    delete[] encrypted_identifiers;
    delete[] nonce;
    return -6;
  }

  // Append nonce to beginning of encrypted pyramid in to_write.
  unsigned long write_length = KEY_LENGTH + (pyramid_size * sizeof(long)) + SIGLEN;
  char *to_write = new char[write_length];
  bcopy(nonce, to_write + SIGLEN, KEY_LENGTH);
  bcopy(encrypted_identifiers, to_write + SIGLEN + KEY_LENGTH,
        pyramid_size * sizeof(long));

  // Sign pyramid file.
  char *sig = new char[SIGLEN];
  unsigned siglen;
  if (Pyramid::sign_pyramid(to_write + SIGLEN, write_length - SIGLEN, sig, &siglen)) {
    delete[] identifiers;
    delete[] encrypted_identifiers;
    delete[] nonce;
    delete[] to_write;
    delete[] sig;
    return -11;
  }

  // Append signature to beginning of nonce and write buffer to file.
  bcopy(sig, to_write, SIGLEN);
  if (write(pyra->get_pyramid_fd(), to_write, write_length) < write_length) {
    delete[] identifiers;
    delete[] encrypted_identifiers;
    delete[] nonce;
    delete[] to_write;
    delete[] sig;
    return -10;
  }

  int file_close_return = close(fd);
  if (file_close_return) {
    delete[] identifiers;
    delete[] encrypted_identifiers;
    delete[] nonce;
    delete[] to_write;
    delete[] sig;
    return -1;
  }
  int pyramid_close_return = close(pyra->get_pyramid_fd());
  if (pyramid_close_return) {
    delete[] identifiers;
    delete[] encrypted_identifiers;
    delete[] nonce;
    delete[] to_write;
    delete[] sig;
    return -2;
  }

  delete[] identifiers;
  delete[] encrypted_identifiers;
  delete[] nonce;
  delete[] to_write;
  delete[] sig;
  return 0;
}


/*
 * Function: pyramid_open
 *
 * Description: This function attempts to open the file referenced by filename
 *              and the associated pyramid file [filename]_pyramid, if it
 *              exists, creating it if it does not exist.  It returns a pointer
 *              to a newly allocated and initialized pyramid object upon
 *              success.
 *
 * Inputs:
 *  pathname - The pathname of the file to be passed to open().
 *  flags - The flags to be passed to open(); see manual entry.
 *  key - An array of size KEY_LENGTH containing the key needed to unlock the
 *   files.
 *
 * Outputs:
 *  file_fd - A pointer into which the return value of opening the file will be
 *   stored.
 *  pyramid_fd - A pointer into which the return value of opening the pyramid
 *   file will be stored.
 *  err: An error value, indicating one of the following:
 *   0 if there were no errors.
 *   -1 if the attempt to open the file failed; check errno.
 *   -2 if the attempt to open the pyramid file failed; check errno.
 *   -4 if fstat failed; check errno.
 *   -5 if any attempt to add a block ID into the pyramid struct failed.
 *   -6 if read returned -1; check errno.
 *   -7 if read didn't return an error, but did read fewer bytes than expected.
 *   -8 if decrypt_pyramid failed.
 *   -9 if verify_pyramid indicates that the pyramid file's integrity has been
 *    compromised.
 *   -10 if verify_pyramid failed for some other reason.
 *   -11 if fchmod failed; check errno.
 *   
 * Return value:
 *  NULL upon failure; check err.
 *  Otherwise, returns a pointer to the pyramid struct associated with the file
 *   referenced by pathname.
 */
Pyramid* AccessFile::pyramid_open(char *pathname, int flags, int* file_fd, int* pyramid_fd, int *err, unsigned char *key) {
  *err = 0;

  // Open the file itself.
  *file_fd = open(pathname, flags);
  if (*file_fd < 0) {
    *err = -1;
    return NULL;
  }
  
  // Open the pyramid file.
  size_t pathname_len = strlen(pathname);
  char *pyramid_filename = new char[pathname_len + 10];
  pyramid_filename[pathname_len + 9] = 0;
  snprintf(pyramid_filename, pathname_len + 10, "%s_pyramid", pathname);
  *pyramid_fd = open(pyramid_filename, O_EXCL|O_CREAT|O_RDWR);
  bool new_pyramid = true;
  if ((*pyramid_fd < 0) && (errno == EEXIST)) {
    *pyramid_fd = open(pyramid_filename, O_RDWR);
    new_pyramid = false;
  }
  if (*pyramid_fd < 0) {
    *err = -2;
    delete[] pyramid_filename;
    return NULL;
  }

  // Allocate and initialize pyramid.  If no pyramid file existed, fill in the
  // pyramid with the block IDs in arbitrary order.
  Pyramid *p = new Pyramid(NULL, *pyramid_fd, key);
  if (new_pyramid) {
    if (fchmod(*pyramid_fd, S_IRUSR|S_IWUSR)) {
      delete p;
      delete[] pyramid_filename;
      *err = -11;
      return NULL;
    }
    struct stat filestat;
    if (fstat(*file_fd, &filestat)) {
      delete p;
      delete[] pyramid_filename;
      *err = -4;
      return NULL;
    }
    ssize_t filesize = filestat.st_size;
    long num_blocks = filesize / BLOCK_SIZE;
    if (num_blocks * BLOCK_SIZE != filesize) num_blocks++;
    for (long i = 0L; i < num_blocks; ++i) {
      if (p->add_id_pyramid(i) == 0) {
        delete p;
	delete[] pyramid_filename;
        *err = -5;
        return NULL;
      }
    }
    return p;
  }

  // Define signature, verification payload, nonce, and decryption payload of
  // pyramid file.
  unsigned char* signature = new unsigned char[SIGLEN];
  struct stat pyramid_filestat;
  if (fstat(*pyramid_fd, &pyramid_filestat)) {
    delete p;
    delete[] pyramid_filename;
    delete[] signature;
    *err = -4;
    return NULL;
  }
  ssize_t pyramid_filesize = pyramid_filestat.st_size - SIGLEN - KEY_LENGTH;
  unsigned char *verification_payload = new unsigned char[pyramid_filesize + KEY_LENGTH];
  unsigned char *nonce = verification_payload;
  unsigned char *decryption_payload = verification_payload + KEY_LENGTH;

  // Get signature and payload from pyramid file and verify pyramid file.
  ssize_t readreturn = read(*pyramid_fd, signature, SIGLEN);
  if (readreturn != SIGLEN) {
    delete p;
    delete[] pyramid_filename;
    delete[] signature;
    delete[] verification_payload;
    if (readreturn < 0) *err = -6;
    else *err = -7;
    return NULL;
  }
  readreturn = read(*pyramid_fd, verification_payload, pyramid_filesize + KEY_LENGTH);
  if (readreturn != pyramid_filesize + KEY_LENGTH) {
    delete p;
    delete[] pyramid_filename;
    delete[] signature;
    delete[] verification_payload;
    if (readreturn < 0) *err = -6;
    else *err = -7;
    return NULL;
  }
  int verifyreturn = Pyramid::verify_pyramid(signature, verification_payload, pyramid_filesize + KEY_LENGTH);
  if (verifyreturn < 0) {
    delete p;
    delete[] pyramid_filename;
    delete[] signature;
    delete[] verification_payload;
    if (!verifyreturn) *err = -9;
    else *err = -10;
    return NULL;
  }

  // Decrypt pyramid file into decrypted_identifiers.
  long *decrypted_identifiers = new long[pyramid_filesize / sizeof(long)];
  if (Pyramid::decrypt_pyramid(key, nonce, decryption_payload, decrypted_identifiers, pyramid_filesize) < 0) {
    delete p;
    delete[] pyramid_filename;
    delete[] signature;
    delete[] verification_payload;
    delete[] decrypted_identifiers;
    *err = -8;
    return NULL;
  }

  // Count the number of layers of the pyramid.
  int counter = 0;
  long num_ids = 4L;
  while (num_ids < pyramid_filesize / sizeof(long)) {
    counter++;
    num_ids *= 4L;
  }

  // Copy the decrypted identifiers into a pyramid structure.
  int offset = 0;
  Pyramid* this_pyramid = p;
  bcopy(decrypted_identifiers + offset, this_pyramid->get_identifiers(), this_pyramid->how_many_ids() * sizeof(long));
  offset += this_pyramid->how_many_ids();
  for (int i = 1; i < counter; ++i) {
    Pyramid *next_pyramid = new Pyramid(this_pyramid, *pyramid_fd, key);
    this_pyramid = next_pyramid;
    bcopy(decrypted_identifiers + offset, this_pyramid->get_identifiers(), this_pyramid->how_many_ids() * sizeof(long));
    offset += this_pyramid->how_many_ids();
  }
  
  return p;
}


/*
 * Function: pyramid_read
 * 
 * Description: This function attempts to read count bytes from the file with
 *  file descriptor fd.  Upon success, the file position is incremented by
 *  the number of bytes read.  Upon failure, the file position is unchanged.
 *
 * Inputs:
 *  fd - The file descriptor of the file to read from.
 *  count - The number of bytes to be read.
 *  pyra - The pyramid struct associated with the file to read from.
 *
 * Outputs:
 *  buf - A buffer into which to store the bytes read from the file.  IT IS
 *   ASSUMED THAT BUF IS LARGE ENOUGH TO STORE THE BYTES.
 *
 * Return value:
 *  The number of bytes read if the function succeeds.
 *  -1 if any of the function's inputs is invalid.
 *  -2 if a call to read_block failed.
 *  -3 if a call to lseek failed; check errno.
 */
ssize_t AccessFile::pyramid_read(int fd, unsigned char *buf, size_t count, Pyramid *pyra) {
  if ((pyra == NULL) || (fd < 0) || (buf == NULL)) return -1L;

  long first_block_id, last_block_id, curr_block_id;
  unsigned num_levels = pyra->how_many_levels();
  long *block_ids = new long[num_levels];
  ssize_t read_block_return;
  off_t original_position = lseek(fd, (off_t) 0, SEEK_CUR);
  size_t buf_offset = 0L;
  if (original_position < 0) {
    delete[] block_ids;
    return -3L;
  }
  
  unsigned char *big_buffer = new unsigned char[num_levels * BLOCK_SIZE];
  
  determine_blocks(count, &first_block_id, &last_block_id, original_position);
  for (curr_block_id = first_block_id; curr_block_id <= last_block_id; ++curr_block_id) {
    read_block_return = read_block(fd, curr_block_id, pyra, block_ids, big_buffer);
    
    if (read_block_return < 0L) {
      delete[] big_buffer;
      delete[] block_ids;
      lseek(fd, original_position, SEEK_SET);
      return -2L;
    }
    
    if (first_block_id == last_block_id) {
      bcopy(big_buffer + read_block_return + (original_position % BLOCK_SIZE), buf, count);
      buf_offset += count;
    }
    else if (curr_block_id == first_block_id) {
      size_t this_size = BLOCK_SIZE - (original_position % BLOCK_SIZE);
      bcopy(big_buffer + read_block_return + (original_position % BLOCK_SIZE), buf, this_size);
      buf_offset += this_size;
    }
    else if (curr_block_id == last_block_id) {
      size_t this_size = count - buf_offset;
      bcopy(big_buffer + read_block_return, buf + buf_offset, this_size);
      buf_offset += this_size;
    }
    else {
      bcopy(big_buffer + read_block_return, buf + buf_offset, BLOCK_SIZE);
      buf_offset += BLOCK_SIZE;
    }
  }

  delete[] big_buffer;
  delete[] block_ids;
  if (lseek(fd, original_position + buf_offset, SEEK_SET) < 0) return -3L;
  return (ssize_t) buf_offset;
}


/*
 * Function: pyramid_write
 *
 * Description: This function attempts to write count bytes from buf to the
 *              file associated with fd.  Whenever a block is written, its ID is
 *              moved to the top level of pyra.  If the file is not currently
 *              large enough to support the write given its current position and
 *              the value of count, the file is first resized with ftruncate to
 *              make it large enough.  If pyramid_write succeeds, the position
 *              of the file is set to its current position + count.  If it
 *              fails, the position of the file is set to its current position.
 *
 * Inputs:
 *  fd - The file descriptor of the file.
 *  buf - A buffer containing the data to be written to the file.  IS ASSUMED
 *   TO BE AT LEAST COUNT BYTES IN LENGTH.
 *  count - The number of bytes to write to the file.
 *  pyra - A pointer to the pyramid struct associated with this file.
 *
 * Return value:
 *  0 upon success.
 *  -1 if any of pyramid_write's inputs is invalid.
 *  -2 if fstat failed; check errno.
 *  -3 if add_id_pyramid failed.
 *  -4 if ftruncate failed; check errno.
 *  -6 if read_block failed.
 *  -7 if lseek failed; check errno.
 *  -8 if write failed; check errno.
 */
ssize_t AccessFile::pyramid_write(int fd, unsigned char *buf, size_t count, Pyramid *pyra) {
  if ((fd < 0) || (buf == NULL) || (pyra == NULL)) return -1L;
  if (count == 0L) return 0L;
  
  struct stat filestat;
  if (fstat(fd, &filestat)) {
    return -2L;
  }

  long final_block;
  if (filestat.st_size == 0) {
    final_block = -1;
  }
  else {
    if ((filestat.st_size % (off_t) BLOCK_SIZE) == 0) {
      final_block = (long) ((filestat.st_size / (off_t) BLOCK_SIZE) - (off_t) 1);
    }
    else {
      final_block = (long) (filestat.st_size / (off_t) BLOCK_SIZE);
    }
  }

  bool would_create_blocks;
  long new_final_block;
  off_t original_position, new_size;
  original_position = lseek(fd, 0, SEEK_CUR);
  new_size = original_position + (off_t) count;
  if ((new_size % (off_t) BLOCK_SIZE) == (off_t) 0) {
    new_final_block = (long) ((new_size / (off_t) BLOCK_SIZE) - (off_t) 1);
  }
  else {
    new_final_block = (long) (new_size / (off_t) BLOCK_SIZE);
  }
  would_create_blocks = (new_final_block > final_block);
  
  if (would_create_blocks) {
    for (long i = final_block + 1; i <= new_final_block; ++i) {
      if (pyra->add_id_pyramid(i) == 0) {
        return -3L;
      }
    }
    if (ftruncate(fd, (off_t) ((new_final_block + 1) * BLOCK_SIZE))) {
      return -4L;
    }
  }

  long first_block_id, last_block_id;
  determine_blocks(count, &first_block_id, &last_block_id, original_position);

  unsigned num_levels;
  num_levels = pyra->how_many_levels();
  long *block_ids = new long[num_levels];
  unsigned char *big_buffer = new unsigned char[BLOCK_SIZE * num_levels];

  long curr_block_id;
  ssize_t read_block_return;
  for (curr_block_id = first_block_id; curr_block_id <= last_block_id; ++curr_block_id) {
    read_block_return = read_block(fd, curr_block_id, pyra, block_ids, big_buffer);
    if (read_block_return < 0) {
      delete[] big_buffer;
      delete[] block_ids;
      lseek(fd, original_position, SEEK_SET);
      return -6L;
    }

    write_block_to_big_buffer(buf, first_block_id, last_block_id, curr_block_id, big_buffer, count, read_block_return, original_position);

    size_t big_buffer_offset;
    for (long i = 0, big_buffer_offset = 0; i < num_levels; ++i, big_buffer_offset += BLOCK_SIZE) {
      if (lseek(fd, block_ids[i] * BLOCK_SIZE, SEEK_SET) < 0) {
        delete[] big_buffer;
	delete[] block_ids;
        lseek(fd, original_position, SEEK_SET);
        return -7L;
      }
      if (write(fd, big_buffer + big_buffer_offset, BLOCK_SIZE) < 0) {
        delete[] big_buffer;
	delete[] block_ids;
        lseek(fd, original_position, SEEK_SET);
        return -8L;
      }
    }
  }

  delete[] big_buffer;
  delete[] block_ids;
  lseek(fd, original_position, SEEK_SET);
  return 0L;
}

/*
 * Function: read_block
 *
 * Description: This function will attempt a "noisy" read of the desired block
 *              by going through each level of pyra and either finding the
 *              desired block and reading it, or failing to find it and reading
 *              a randomly selected block.  This function, when it finds the
 *              desired block ID in pyra, moves it to the top level of pyra.
 *
 * Inputs:
 *  file_fd - The file descriptor of the file being accessed.
 *  block - The identifier of the block to be accessed.
 *  pyra - A pointer to the top level of the pyramid for the file being
 *   accessed.
 *
 * Outputs:
 *  block_ids - If the function succeeds, this array will store the block IDs
 *   associated with the blocks stored in big_buffer.  The block with ID
 *   block_ids[i] is stored at big_buffer + BLOCK_SIZE * i.  THIS ARRAY IS
 *   ASSUMED TO BE LARGE ENOUGH TO STORE AS MANY LONGS AS THERE ARE LAYERS IN
 *   PYRA.
 *  big_buffer - If the function succeeds, this buffer will be used to store
 *   copies of each block accessed, one for each non-empty layer of pyra.  THIS
 *   BUFFER IS ASSUMED TO BE LARGE ENOUGH TO STORE AS MANY BLOCKS AS THERE ARE
 *   LAYERS IN PYRA.
 *
 * Return value:
 *  Upon success, this function returns a nonnegative number that is the offset
 *   (in bytes) from the beginning of big_buffer at which the requested block
 *   begins.
 *  -1 if move_id_in_pyramid fails.
 *  -2 if randomly_select_id_pyramid fails.
 *  -3 if any of the function inputs is invalid.
 *  -4 if a call to read caused an error; check errno.
 *  -5 if a call to read caused fewer bytes to be read than expected.
 *  -6 if a call to read caused more bytes to be read than expected.
 *  -7 if a call to fstat failed; check errno.
 *  -8 if the id "block" was not found in pyra.
 *  -9 if a call to lseek failed; check errno.
 */
ssize_t AccessFile::read_block(int file_fd, long block, Pyramid *pyra, long *block_ids,
                    unsigned char *big_buffer) {
  
  if ((block < 0L) || (pyra == NULL) || (block_ids == NULL) || (file_fd < 0) || (big_buffer == NULL))
    return (ssize_t) -3;
  int movereturn, notfoundyet, i, j;
  Pyramid *this_level;
  ssize_t offset, readreturn, returnoffset;
  struct stat filestat;
  long final_block, this_block;

  notfoundyet = 1;
  returnoffset = -8;
  for (this_level = pyra, offset = 0, i = 0; this_level != NULL;
       this_level = this_level->get_next_level(), offset += BLOCK_SIZE, ++i) {
    if (notfoundyet && ((movereturn = this_level->move_id_in_pyramid(pyra, block)) == 1)) {
      notfoundyet = 0;
      this_block = block;
    }
    else {
      if ((movereturn != -3) && notfoundyet) {
        return (ssize_t) -1;
      }
      this_block = this_level->randomly_select_id_pyramid();
      if (this_block < 0L) {
        return (ssize_t) -2;
      }
    }
    
    if (lseek(file_fd, this_block * BLOCK_SIZE, SEEK_SET) < 0) {
      return (ssize_t) -9;
    }
    readreturn = read(file_fd, big_buffer + offset, BLOCK_SIZE);
    if (readreturn < 0) return (ssize_t) -4;
    if (readreturn > BLOCK_SIZE) return (ssize_t) -6;
    if (fstat(file_fd, &filestat)) return (ssize_t) -7;
    if (!filestat.st_size) final_block = 0;
    else final_block = (long) ((filestat.st_size - (off_t) 1) / (off_t) BLOCK_SIZE);
    if ((this_block == final_block) && (filestat.st_size % BLOCK_SIZE)) {
      for (j = offset + (filestat.st_size % BLOCK_SIZE); j < offset + BLOCK_SIZE; ++j) big_buffer[j] = (unsigned char) 0;
      if (filestat.st_size % (off_t) BLOCK_SIZE != (off_t) (readreturn % BLOCK_SIZE)) {
        return (ssize_t) -5;
      }
    }
    else {
      if (readreturn != BLOCK_SIZE) {
        return (ssize_t) -5;
      }
    }
    if (this_block == block) {
      returnoffset = offset;
    }
    block_ids[i] = this_block;
  }
  
  return returnoffset;
}

// Workhorse helper function for pyramid_write.  Writes a block from a file to a
// buffer, varying precise effects depending on whether stuff to be written
// constitutes the entire block, a first portion of the block, an ending portion
// of the block, or a middle portion of the block.
void AccessFile::write_block_to_big_buffer(unsigned char *writebuf, long first_block, long last_block, long requested_block, unsigned char *big_buffer, size_t count, size_t big_buffer_offset, off_t original_position) {
  size_t size_of_first = BLOCK_SIZE - (original_position % BLOCK_SIZE);
  unsigned num_blocks = last_block - first_block + 1;
  size_t size_of_last = count - (size_of_first + ((num_blocks - 2) * BLOCK_SIZE));

  if (first_block == last_block) {
    bcopy(writebuf, big_buffer + big_buffer_offset + (original_position % BLOCK_SIZE), count);
  }
  else if (requested_block == last_block) {
    bcopy(writebuf + (count - size_of_last), big_buffer + big_buffer_offset, size_of_last);
  }
  else if (requested_block == first_block) {
    bcopy(writebuf, big_buffer + big_buffer_offset + (original_position % BLOCK_SIZE), size_of_first);
  }
  else {
    bcopy(writebuf + size_of_first + (requested_block - first_block - 1) * BLOCK_SIZE, big_buffer + big_buffer_offset, BLOCK_SIZE);
  }

  return;
}
