#include "pyramid_project.h"

// Pyramid constructor.
Pyramid::Pyramid(Pyramid* parent, int pyramid_fd, unsigned char *key) {
  if (key == NULL) return;
  if (parent == NULL) {
    level = 1;
    this->pyramid_fd = pyramid_fd;
    this->key = new unsigned char[KEY_LENGTH];
    for (int i = 0; i < KEY_LENGTH; ++i) {
      this->key[i] = key[i];
    }
  }
  else {
    level = parent->level + 1;
    this->pyramid_fd = parent->get_pyramid_fd();
    this->key = parent->get_key();
  }
  identifiers = NULL;
  next_level = NULL;
  size_t howmanyentries = how_many_ids();
  if (howmanyentries == 0) {
    return;
  }
  identifiers = new long[howmanyentries];
  if (identifiers == NULL) {
    return;
  }
  for (int i = 0; i < howmanyentries; ++i) {
    identifiers[i] = -1L;
  }
  if (parent != NULL) {
    parent->set_next_level(this);
  }
}

// Pyramid destructor.
Pyramid::~Pyramid() {
  if (next_level != NULL) delete next_level;
  if (identifiers != NULL) delete[] identifiers;
  if ((key != NULL) && (next_level == NULL)) delete[] key;
}

/*
 * Function: how_many_ids
 *
 * Description: This function calculates the total number of block identifiers
 *  that may be stored in the input pyramid (or rather, at that level of the
 *  main pyramid).
 *
 * Return value:
 *  0 if p is a NULL pointer.
 *  Otherwise, returns 4 to the power of the level of *p.
 */
size_t Pyramid::how_many_ids() {
  double howmany = pow(4.0, (double) level);
  return (size_t) howmany;
}

/*
 * Function: how_many_levels
 *
 * Description: This function, when given any pyramid level, returns the
 *  number of levels in the pyramid.
 *
 * Return value:
 *  0 if pyra is NULL.
 *  Otherwise, the number of levels in the full pyramid is returned.
 */
unsigned Pyramid::how_many_levels() {
  Pyramid* p = this;
  while (p->get_next_level() != NULL) {
    p = p->get_next_level();
  }
  return p->get_level();
}

/*
 * Function: pyramid_overflow
 *
 * Description: This function is called whenever a level in the pyramid is about
 *              to contain more block identifiers than is allowable, and it
 *              removes all block identifiers from this level and put them into
 *              the next one, recursively calling itself on that next level if
 *              necessary.
 *
 * Return value:
 *  1 upon success.
 *  -3 if a call to add_id_pyramid failed.
 */
int Pyramid::pyramid_overflow() {
  if (next_level == NULL) {
    next_level = new Pyramid(this, pyramid_fd, key);
  }
  
  for (int idx = 0; idx < how_many_ids(); ++idx) {
    if (next_level->add_id_pyramid(identifiers[idx]) == 0) {
      return -3;
    }
    identifiers[idx] = -1L;
  }
  return 1;
}

/*
 * Function: move_id_in_pyramid
 *
 * Description: This function removes a block identifier from this level in the
 *              pyramid, if present, and stores it in the top level of the
 *              pyramid.
 *
 * Inputs:
 *  top_level - A pointer to level 1 of the pyramid.
 *  id - The identifier to move.
 *
 * Return value:
 *  1 if the block identifier was moved successfully.
 *  -1 if either of the arguments is invalid.
 *  -2 if the pyramid *this_level has a NULL identifier list.
 *  -3 if the given block identifier was not found at the specified level of the
 *   pyramid.
 *  -4 if the call to pyramid_overflow failed.
 */
int Pyramid::move_id_in_pyramid(Pyramid* top_level, long id) {
  if ((top_level == NULL) || (id < 0)) return -1;
  long* current_ids = identifiers;
  if (current_ids == NULL) return -2;
  size_t howmanyids = how_many_ids();
  int i;
  bool foundit = false;
  for (i = 0; i < howmanyids; ++i) {
    bool thisisit = (current_ids[i] == id);
    foundit = foundit || thisisit;
    if (thisisit) current_ids[i] = -1L;
  }
  if (!foundit) return -3;
  for (int i = 0; i < top_level->how_many_ids(); ++i) {
    if (top_level->identifiers[i] < 0L) {
      top_level->identifiers[i] = id;
      break;
    }
  }
  if (i >= top_level->how_many_ids()) {
    if (top_level->pyramid_overflow() < 0) {
      return -4;
    }
    top_level->add_id_pyramid(id);
  }
  return 1;
}

/*
 * Function: randomly_select_id_pyramid
 *
 * Description: This function, when passed a pointer to a level of the pyramid,
 *              randomly selects one of the block identifiers stored there.
 *
 * Return value:
 *  -1 if a call to how_many_ids failed.
 *  -2 if the block identifier list at this level of the pyramid was empty.
 *  -3 if the list of block identifiers is NULL.
 *  Otherwise, the randomly selected block identifier is returned.
 */
long Pyramid::randomly_select_id_pyramid() {
  if (identifiers == NULL) return -3L;
  size_t howmanyids = how_many_ids();
  if (howmanyids == 0) return -1L;
  int r = rand() % howmanyids;
  int i = r;
  do {
    if (identifiers[i] >= 0) {
      return identifiers[i];
    }
    i = (i + 1) % howmanyids;
  } while (i != r);
  return -2L;
}

/*
 * Function: add_id_pyramid
 *
 * Description: This function adds a given block identifier to the specified
 *              level in the pyramid, dumping the contents of that level onto
 *              the next if there isn't enough space for it.
 *
 * Inputs:
 *  id - The number of the block to be added.
 *
 * Return value:
 *  0 if id is invalid, or a block with identifier id was already
 *   found at that level in the pyramid.
 *  1 if the block was successfully added.
 */
int Pyramid::add_id_pyramid(long id) {
  if (id < 0) return 0;
  int empty = -1;
  
  for (int i = 0; i < how_many_ids(); ++i) {
    if (identifiers[i] < 0) {
      empty = i;
    }
    else if (identifiers[i] == id) {
      return 0;
    }
  }
  if (empty != -1) {
    identifiers[empty] = id;
    return 1;
  }
  pyramid_overflow();
  return add_id_pyramid(id);
}

/*
 * Function: encrypt_pyramid
 *
 * Description: This function encrypts a buffer containing a pyramid using a
 *              randomly generated IV and an input key.
 *
 * Inputs:
 *  from - A buffer containing a copy of the pyramid block identifiers to be
 *         encrypted.
 *  pyra - A pointer to the pyramid struct.  Is assumed to contain the
 *         appropriate encryption key.
 *  pyramid_size - The number of bytes in from.
 *  seed - A seed to feed to the random number generator for the IV.
 *
 * Outputs:
 *  to - This buffer will contain the encrypted pyramid.  Is assumed to be at
 *       least as big as from.
 *  nonce - The randomly generated IV will be stored here.
 *
 * Return value:
 *  -1 if EVP_EncryptInit failed.
 *  -2 if EVP_EncryptUpdate failed.
 *  -3 if EVP_EncryptFinal failed.
 *  Otherwise, the number of bytes produced by the encryption is returned.
 */
int Pyramid::encrypt_pyramid(long *from, long *to, int pyramid_size, unsigned char *nonce, long seed) {
  const void *seed_buf = &seed;
  RAND_seed(seed_buf, sizeof(long));
  BIGNUM *random = BN_new();
  BN_rand(random, 128, -1, 0);
  if (BN_num_bytes(random) < KEY_LENGTH) {
    int i, randint;
    for (i = 0; i < KEY_LENGTH / sizeof(int); ++i) {
      randint = rand();
      bcopy(&randint, nonce, sizeof(int));
    }
  }
  else
    bcopy(random, nonce, KEY_LENGTH);

  EVP_CIPHER_CTX *context = EVP_CIPHER_CTX_new();
  EVP_CIPHER_CTX_init(context);

  if (!EVP_EncryptInit(context, EVP_aes_128_ofb(), key, nonce)) {
    cerr << "EncryptInit: ENCRYPTION FAILED\n";
    EVP_CIPHER_CTX_cleanup(context);
    return -1;
  }
  int update_encrypted_bytes = -1;
  if (!EVP_EncryptUpdate(context, (unsigned char*) to, &update_encrypted_bytes, (unsigned char*) from, pyramid_size)) {
    cerr << "EncryptUpdate: ENCRYPTION FAILED\n";
    EVP_CIPHER_CTX_cleanup(context);
    return -2;
  }
  int final_encrypted_bytes = -1;
  if (!EVP_EncryptFinal(context, (unsigned char*) to, &final_encrypted_bytes)) {
    cerr << "EncryptFinal: ENCRYPTION FAILED\n";
    EVP_CIPHER_CTX_cleanup(context);
    return -3;
  }
  EVP_CIPHER_CTX_cleanup(context);
  return update_encrypted_bytes + final_encrypted_bytes;
}

/*
 * Function: decrypt_pyramid
 *
 * Description: This function decrypts a pyramid file's identifiers.
 *
 * Inputs:
 *  key - The encryption key.
 *  nonce - The nonce used for the encryption of this file.
 *  encrypted - A buffer containing the encrypted block IDs from the file.
 *  bytes - The number of bytes going into the encryption.
 *
 * Outputs:
 *  decrypted - A buffer into which to store the result of the decryption.
 *
 * Return value:
 *  -1 if EVP_DecryptInit failed.
 *  -2 if EVP_DecryptUpdate failed.
 *  -3 if EVP_DecryptFinal failed.
 *  Otherwise, returns the number of bytes produced by the decryption.
 */
int Pyramid::decrypt_pyramid(unsigned char *key, unsigned char *nonce, unsigned char *encrypted, long *decrypted, ssize_t bytes) {
  
  EVP_CIPHER_CTX *context = EVP_CIPHER_CTX_new();
  EVP_CIPHER_CTX_init(context);

  if (!EVP_DecryptInit(context, EVP_aes_128_ofb(), key, nonce)) {
    cerr << "DecryptInit: DECRYPTION FAILED\n";
    EVP_CIPHER_CTX_cleanup(context);
    return -1;
  }
  int update_plaintext_bytes = -1;
  if (!EVP_DecryptUpdate(context, (unsigned char*) decrypted, &update_plaintext_bytes, encrypted, bytes)) {
    cerr << "DecryptUpdate: DECRYPTION FAILED\n";
    EVP_CIPHER_CTX_cleanup(context);
    return -2;
  }
  int final_plaintext_bytes = -1;
  if (!EVP_DecryptFinal(context, (unsigned char*) decrypted, &final_plaintext_bytes)) {
    cerr << "DecryptFinal: DECRYPTION FAILED\n";
    EVP_CIPHER_CTX_cleanup(context);
    return -3;
  }
  EVP_CIPHER_CTX_cleanup(context);
  return update_plaintext_bytes + final_plaintext_bytes;
}

/*
 * Function: verify_pyramid
 *
 * Description: This function verifies the signature against the remainder of
 *              the pyramid file.
 *
 * Inputs:
 *  signature - A buffer containing the signature of the pyramid file.
 *  payload - A buffer containing the remainder of the pyramid file: the
 *            payload for the verification.
 *  payload_size - The length, in bytes, of the payload.
 *
 * Outputs:
 *  signature - After a call to verify_pyramid that returns either 0 or 1, the
 *              message digest produced by the verification will be stored here.
 *
 * Return value:
 *  -3 if EVP_VerifyInit failed.
 *  -2 if EVP_VerifyUpdate failed.
 *  -1 if EVP_DigestFinal_ex returned a value not specified by the OpenSSL
 *   documentation.
 *  0 if the verification failed: the block does not match its signature.
 *  1 if the verification succeeded: the block has not been corrupted.
 */
int Pyramid::verify_pyramid(unsigned char *signature, unsigned char *payload, ssize_t payload_size) {
  EVP_MD_CTX* ctx = EVP_MD_CTX_create();
  if (!EVP_VerifyInit(ctx, EVP_sha384())) {
    EVP_MD_CTX_destroy(ctx);
    return -3;
  }
  if (!EVP_VerifyUpdate(ctx, payload, payload_size)) {
    EVP_MD_CTX_destroy(ctx);
    return -2;
  }
  int return_value = EVP_DigestFinal_ex(ctx, signature, NULL);
  if ((return_value == 1) || (return_value == 0)) {
    EVP_MD_CTX_destroy(ctx);
    return return_value;
  }
  EVP_MD_CTX_destroy(ctx);
  return -1;
}

/*
 * Function: sign_pyramid
 *
 * Description: This function signs an encrypted pyramid file using SHA384.
 *
 * Inputs:
 *  buf - A copy of the nonce-and-pyramid to be signed.
 *  pyramid_size - The size of the nonce-and-pyramid to be signed.
 *
 * Outputs:
 *  sig - The signature produced by the signing is stored here.
 *  siglen - This is a pointer to the length of the signature stored in sig.
 *
 * Return value:
 *  0 if the signing succeeded.
 *  -2 if the signature produced is of an unexpected length.
 *  -3 if EVP_SignInit failed.
 *  -4 if EVP_SignUpdate failed.
 *  -5 if EVP_DigestFinal failed.
 */
int Pyramid::sign_pyramid(const void *buf, size_t pyramid_size, char *sig, unsigned *siglen) {
  EVP_MD_CTX* ctx = EVP_MD_CTX_create();
  if (EVP_SignInit(ctx, EVP_sha384()) != 1) {
    EVP_MD_CTX_destroy(ctx);
    return -3;
  }
  if (EVP_SignUpdate(ctx, buf, pyramid_size) != 1) {
    EVP_MD_CTX_destroy(ctx);
    return -4; 
  }
  if (EVP_DigestFinal(ctx, sig, siglen) != 1) {
    EVP_MD_CTX_destroy(ctx);
    return -5;
  }
  if (*siglen != SIGLEN) return -2;
  EVP_MD_CTX_destroy(ctx);
  return 0;
}

/*
 * Function: sizeof_pyramid
 *
 * Description: This function returns the number of identifiers in the full
 *  pyramid (not just this level) for the purposes of writing the full
 *  pyramid to a file.
 *
 * Inputs:
 *  pyra: A pointer to the top level of the pyramid.
 *
 * Return value:
 *  0 in the case of an error (i.e., this object isn't the top level of its
 *   pyramid).
 *  Otherwise, this function returns the number of block identifiers that may
 *   be stored in the entire pyramid.
 */
size_t Pyramid::sizeof_pyramid() {
  if (level != 1) return 0;
  
  Pyramid* p = this;
  size_t total = 0;
  while (p->get_next_level() != NULL) {
    total += p->how_many_ids();
    p = p->get_next_level();
  }
  total += p->how_many_ids();

  return total;
}

unsigned Pyramid::get_level() {return level;}
int Pyramid::get_pyramid_fd() {return pyramid_fd;}
unsigned char* Pyramid::get_key() {return key;}
void Pyramid::set_next_level(Pyramid* next) {next_level = next;}
Pyramid* Pyramid::get_next_level() {return next_level;}
long* Pyramid::get_identifiers() {return identifiers;}
