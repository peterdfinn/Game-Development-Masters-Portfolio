#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <strings.h>
#include <sys/stat.h>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <iostream>

#include <openssl/evp.h>
#include <openssl/bn.h>
#include <openssl/err.h>
#include <openssl/rand.h>

#define KEY_LENGTH 16
#define SIGLEN (384 / 8)
#define BLOCK_SIZE (1 << 12)

using namespace std;

/*
 * The Pyramid class represents one level of a pyramid structure and contains
 * functions necessary for managing the pyramid.
 */
class Pyramid {
 private:
  unsigned level; // the level in a file's pyramid that this object represents
  int pyramid_fd; // the FD of the pyramid file (not the read/write file itself)
  long* identifiers; // an array of block identifiers
  Pyramid* next_level;
  unsigned char* key; // key used for encryption and decryption of the file

 public:
  // Descriptions can be found in Pyramid.cpp.
  Pyramid(Pyramid*, int, unsigned char*);
  ~Pyramid();
  size_t how_many_ids();
  unsigned how_many_levels();
  int pyramid_overflow();
  int move_id_in_pyramid(Pyramid*, long);
  long randomly_select_id_pyramid();
  int add_id_pyramid(long);
  int encrypt_pyramid(long*, long*, int, unsigned char*, long);
  static int decrypt_pyramid(unsigned char*, unsigned char*, unsigned char*, long*, ssize_t);
  static int verify_pyramid(unsigned char*, unsigned char*, ssize_t);
  static int sign_pyramid(const void*, size_t, char*, unsigned*);
  size_t sizeof_pyramid();
  unsigned get_level();
  int get_pyramid_fd();
  unsigned char* get_key();
  void set_next_level(Pyramid*);
  Pyramid* get_next_level();
  long* get_identifiers();
};

/*
 * The AccessFile class contains static functions that are used for accessing a
 * file.  For example, the pyramid_read, pyramid_write, pyramid_open, and
 * pyramid_close functions, which were designed to replace the read, write,
 * open, and close system calls respectively, are all found in this class.
 */
class AccessFile {
 public:
  // Descriptions can be found in AccessFile.cpp.
  static void determine_blocks(size_t, long*, long*, off_t);
  static int pyramid_close(int, Pyramid*);
  static Pyramid* pyramid_open(char*, int, int*, int*, int*, unsigned char*);
  static ssize_t pyramid_read(int, unsigned char*, size_t, Pyramid*);
  static ssize_t pyramid_write(int, unsigned char*, size_t, Pyramid*);
  static ssize_t read_block (int, long, Pyramid*, long*, unsigned char*);
  static void write_block_to_big_buffer(unsigned char*, long, long, long, unsigned char*, size_t, size_t, off_t);
};
