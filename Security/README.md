This directory contains all files necessary for a library I created that was intended to provide a more secure means of opening, reading, writing, and closing files than the standard UNIX system calls.

The project was based on a idea for a system called ORAM (Oblivious RAM) that would theoretically prevent an attacker snooping on a memory bus from determining a user’s block access patterns.  This would be done by storing disk blocks in a pyramidal structure, and every time a block was requested by the user, a number of fake blocks would be requested to accompany the real block.  For each level in the pyramid, the real block would be requested if it resided there; if not, a randomly selected block from that level would be requested instead.  Data blocks would be stored in encrypted form on the disk, and then decrypted when requested by the user.  When writing a block to the disk after a user has requested it, it would be re-encrypted with a difference nonce and stored in a different address (that is, a different location in the pyramid) - specifically, it would be placed in the top level of the pyramid.  This would ensure that more frequently desired blocks would be higher up in the pyramid, and thus, found more quickly in scanning each level of the pyramid looking for it.

If you are an attacker, and you want to determine the block access patterns employed by a user, you have two ways of identifying blocks: by their address and by their value.  The encryption and decryption would thwart the latter, and the requesting of fake blocks, along with the storing of blocks at new addresses, would thwart the former.

My idea was to alter this system and see if it would be feasible to use its techniques to access information within a file, rather than from a storage device in general.  This, unfortunately, turned out not to be the case.  File blocks aren’t actually being stored at different memory addresses in my system; they’re just having references to them stored in a pyramidal structure.  The blocks themselves aren’t being written to unique locations in memory.  Furthermore, since file access is often done sequentially, it could be very easy to identify a sequence of blocks being accessed, even if they were being partially obscured by requests for random blocks.  Suppose, for example, that a user wants to access blocks 2, 3, and 4 of a file, and suppose that an attacker sees the following collections of requests: (26, 2, 1), (24, 12, 3), (31, 4, 9).  A smart attacker who sees the sequence 2->3->4 in the collections would be able to deduce that the user was probably requesting blocks 2, 3, and 4.

For that reason, this library should NOT be used in place of existing system calls.  It was an idea that did not bear fruit.

~~~~~~~~~~~~~~~~~

In order to get this library’s object files to compile, one must download the OpenSSL library, which can be found at https://github.com/openssl/openssl, and ensure that the appropriate directory on your machine is added to your compiler’s include path.

~~~~~~~~~~~~~~~~~

This directory contains 4 files.  They are:
 - AccessFile.cpp: A C++ source file that contains all functions related to accessing a file.
 - pyramid_project.h: A header file that is included by AccessFile.cpp and Pyramid.cpp and contains class declarations, #include statements, and #define statements.
 - Pyramid.cpp: A C++ source file that contains all functions needed for pyramid structure management.
 - README.md: This file.