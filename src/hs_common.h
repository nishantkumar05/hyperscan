/*
 * Copyright (c) 2015, Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of Intel Corporation nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef HS_COMMON_H_
#define HS_COMMON_H_

#include <stdlib.h>

/**
 * @file
 * @brief The Hyperscan common API definition.
 *
 * Hyperscan is a high speed regular expression engine.
 *
 * This header contains functions available to both the Hyperscan compiler and
 * runtime.
 */

#ifdef __cplusplus
extern "C"
{
#endif

struct hs_database;

/**
 * A Hyperscan pattern database.
 *
 * Generated by one of the Hyperscan compiler functions:
 *  - @ref hs_compile()
 *  - @ref hs_compile_multi()
 *  - @ref hs_compile_ext_multi()
 */
typedef struct hs_database hs_database_t;

/**
 * A type for errors returned by Hyperscan functions.
 */
typedef int hs_error_t;

/**
 * Free a compiled pattern database.
 *
 * The free callback set by @ref hs_set_database_allocator() (or @ref
 * hs_set_allocator()) will be used by this function.
 *
 * @param db
 *      A compiled pattern database. NULL may also be safely provided, in which
 *      case the function does nothing.
 *
 * @return
 *      @ref HS_SUCCESS on success, other values on failure.
 */
hs_error_t hs_free_database(hs_database_t *db);

/**
 * Serialize a pattern database to a stream of bytes.
 *
 * The allocator callback set by @ref hs_set_misc_allocator() (or @ref
 * hs_set_allocator()) will be used by this function.
 *
 * @param db
 *      A compiled pattern database.
 *
 * @param bytes
 *      On success, a pointer to an array of bytes will be returned here.
 *      These bytes can be subsequently relocated or written to disk. The
 *      caller is responsible for freeing this block.
 *
 * @param length
 *      On success, the number of bytes in the generated byte array will be
 *      returned here.
 *
 * @return
 *      @ref HS_SUCCESS on success, @ref HS_NOMEM if the byte array cannot be
 *      allocated, other values may be returned if errors are detected.
 */
hs_error_t hs_serialize_database(const hs_database_t *db, char **bytes,
                                 size_t *length);

/**
 * Reconstruct a pattern database from a stream of bytes previously generated
 * by @ref hs_serialize_database().
 *
 * This function will allocate sufficient space for the database using the
 * allocator set with @ref hs_set_database_allocator() (or @ref
 * hs_set_allocator()); to use a pre-allocated region of memory, use the @ref
 * hs_deserialize_database_at() function.
 *
 * @param bytes
 *      A byte array generated by @ref hs_serialize_database() representing a
 *      compiled pattern database.
 *
 * @param length
 *      The length of the byte array generated by @ref hs_serialize_database().
 *      This should be the same value as that returned by @ref
 *      hs_serialize_database().
 *
 * @param db
 *      On success, a pointer to a newly allocated @ref hs_database_t will be
 *      returned here. This database can then be used for scanning, and
 *      eventually freed by the caller using @ref hs_free_database().
 *
 * @return
 *      @ref HS_SUCCESS on success, other values on failure.
 */
hs_error_t hs_deserialize_database(const char *bytes, const size_t length,
                                   hs_database_t **db);

/**
 * Reconstruct a pattern database from a stream of bytes previously generated
 * by @ref hs_serialize_database() at a given memory location.
 *
 * This function (unlike @ref hs_deserialize_database()) will write the
 * reconstructed database to the memory location given in the @a db parameter.
 * The amount of space required at this location can be determined with the
 * @ref hs_serialized_database_size() function.
 *
 * @param bytes
 *      A byte array generated by @ref hs_serialize_database() representing a
 *      compiled pattern database.
 *
 * @param length
 *      The length of the byte array generated by @ref hs_serialize_database().
 *      This should be the same value as that returned by @ref
 *      hs_serialize_database().
 *
 * @param db
 *      Pointer to an 8-byte aligned block of memory of sufficient size to hold
 *      the deserialized database. On success, the reconstructed database will
 *      be written to this location. This database can then be used for pattern
 *      matching. The user is responsible for freeing this memory; the @ref
 *      hs_free_database() call should not be used.
 *
 * @return
 *      @ref HS_SUCCESS on success, other values on failure.
 */
hs_error_t hs_deserialize_database_at(const char *bytes, const size_t length,
                                      hs_database_t *db);

/**
 * Provides the size of the stream state allocated by a single stream opened
 * against the given database.
 *
 * @param database
 *      Pointer to a compiled (streaming mode) pattern database.
 *
 * @param stream_size
 *      On success, the size in bytes of an individual stream opened against the
 *      given database is placed in this parameter.
 *
 * @return
 *      @ref HS_SUCCESS on success, other values on failure.
 */
hs_error_t hs_stream_size(const hs_database_t *database, size_t *stream_size);

/**
 * Provides the size of the given database in bytes.
 *
 * @param database
 *      Pointer to compiled pattern database.
 *
 * @param database_size
 *      On success, the size of the compiled database in bytes is placed in this
 *      parameter.
 *
 * @return
 *      @ref HS_SUCCESS on success, other values on failure.
 */
hs_error_t hs_database_size(const hs_database_t *database,
                            size_t *database_size);

/**
 * Utility function for reporting the size that would be required by a
 * database if it were deserialized.
 *
 * This can be used to allocate a shared memory region or other "special"
 * allocation prior to deserializing with the @ref hs_deserialize_database_at()
 * function.
 *
 * @param bytes
 *      Pointer to a byte array generated by @ref hs_serialize_database()
 *      representing a compiled pattern database.
 *
 * @param length
 *      The length of the byte array generated by @ref hs_serialize_database().
 *      This should be the same value as that returned by @ref
 *      hs_serialize_database().
 *
 * @param deserialized_size
 *      On success, the size of the compiled database that would be generated
 *      by @ref hs_deserialize_database_at() is returned here.
 *
 * @return
 *      @ref HS_SUCCESS on success, other values on failure.
 */
hs_error_t hs_serialized_database_size(const char *bytes, const size_t length,
                                       size_t *deserialized_size);

/**
 * Utility function providing information about a database.
 *
 * @param database
 *      Pointer to a compiled database.
 *
 * @param info
 *      On success, a string containing the version and platform information for
 *      the supplied database is placed in the parameter. The string is
 *      allocated using the allocator supplied in @ref hs_set_misc_allocator()
 *      (or malloc() if no allocator was set) and should be freed by the caller.
 *
 * @return
 *      @ref HS_SUCCESS on success, other values on failure.
 */
hs_error_t hs_database_info(const hs_database_t *database, char **info);

/**
 * Utility function providing information about a serialized database.
 *
 * @param bytes
 *      Pointer to a serialized database.
 *
 * @param length
 *      Length in bytes of the serialized database.
 *
 * @param info
 *      On success, a string containing the version and platform information
 *      for the supplied serialized database is placed in the parameter. The
 *      string is allocated using the allocator supplied in @ref
 *      hs_set_misc_allocator() (or malloc() if no allocator was set) and
 *      should be freed by the caller.
 *
 * @return
 *      @ref HS_SUCCESS on success, other values on failure.
 */
hs_error_t hs_serialized_database_info(const char *bytes, size_t length,
                                       char **info);

/**
 * The type of the callback function that will be used by Hyperscan to allocate
 * more memory at runtime as required, for example in @ref hs_open_stream() to
 * allocate stream state.
 *
 * If Hyperscan is to be used in a multi-threaded, or similarly concurrent
 * environment, the allocation function will need to be re-entrant, or
 * similarly safe for concurrent use.
 *
 * @param size
 *      The number of bytes to allocate.
 * @return
 *      A pointer to the region of memory allocated, or NULL on error.
 */
typedef void *(*hs_alloc_t)(size_t size);

/**
 * The type of the callback function that will be used by Hyperscan to free
 * memory regions previously allocated using the @ref hs_alloc_t function.
 *
 * @param ptr
 *      The region of memory to be freed.
 */
typedef void (*hs_free_t)(void *ptr);

/**
 * Set the allocate and free functions used by Hyperscan for allocating
 * memory at runtime for stream state, scratch space, database bytecode,
 * and various other data structure returned by the Hyperscan API.
 *
 * The function is equivalent to calling @ref hs_set_stream_allocator(),
 * @ref hs_set_scratch_allocator(), @ref hs_set_database_allocator() and
 * @ref hs_set_misc_allocator() with the provided parameters.
 *
 * This call will override any previous allocators that have been set.
 *
 * Note: there is no way to change the allocator used for temporary objects
 * created during the various compile calls (@ref hs_compile(), @ref
 * hs_compile_multi(), @ref hs_compile_ext_multi()).
 *
 * @param alloc_func
 *      A callback function pointer that allocates memory. This function must
 *      return memory suitably aligned for the largest representable data type
 *      on this platform.
 *
 * @param free_func
 *      A callback function pointer that frees allocated memory.
 *
 * @return
 *      @ref HS_SUCCESS on success, other values on failure.
 */
hs_error_t hs_set_allocator(hs_alloc_t alloc_func, hs_free_t free_func);

/**
 * Set the allocate and free functions used by Hyperscan for allocating memory
 * for database bytecode produced by the compile calls (@ref hs_compile(), @ref
 * hs_compile_multi(), @ref hs_compile_ext_multi()) and by database
 * deserialization (@ref hs_deserialize_database()).
 *
 * If no database allocation functions are set, or if NULL is used in place of
 * both parameters, then memory allocation will default to standard methods
 * (such as the system malloc() and free() calls).
 *
 * This call will override any previous database allocators that have been set.
 *
 * Note: the database allocator may also be set by calling @ref
 * hs_set_allocator().
 *
 * Note: there is no way to change how temporary objects created during the
 * various compile calls (@ref hs_compile(), @ref hs_compile_multi(), @ref
 * hs_compile_ext_multi()) are allocated.
 *
 * @param alloc_func
 *      A callback function pointer that allocates memory. This function must
 *      return memory suitably aligned for the largest representable data type
 *      on this platform.
 *
 * @param free_func
 *      A callback function pointer that frees allocated memory.
 *
 * @return
 *      @ref HS_SUCCESS on success, other values on failure.
 */
hs_error_t hs_set_database_allocator(hs_alloc_t alloc_func,
                                     hs_free_t free_func);

/**
 * Set the allocate and free functions used by Hyperscan for allocating memory
 * for items returned by the Hyperscan API such as @ref hs_compile_error_t, @ref
 * hs_expr_info_t and serialized databases.
 *
 * If no misc allocation functions are set, or if NULL is used in place of both
 * parameters, then memory allocation will default to standard methods (such as
 * the system malloc() and free() calls).
 *
 * This call will override any previous misc allocators that have been set.
 *
 * Note: the misc allocator may also be set by calling @ref hs_set_allocator().
 *
 * @param alloc_func
 *      A callback function pointer that allocates memory. This function must
 *      return memory suitably aligned for the largest representable data type
 *      on this platform.
 *
 * @param free_func
 *      A callback function pointer that frees allocated memory.
 *
 * @return
 *      @ref HS_SUCCESS on success, other values on failure.
 */
hs_error_t hs_set_misc_allocator(hs_alloc_t alloc_func, hs_free_t free_func);

/**
 * Set the allocate and free functions used by Hyperscan for allocating memory
 * for scratch space by @ref hs_alloc_scratch() and @ref hs_clone_scratch().
 *
 * If no scratch allocation functions are set, or if NULL is used in place of
 * both parameters, then memory allocation will default to standard methods
 * (such as the system malloc() and free() calls).
 *
 * This call will override any previous scratch allocators that have been set.
 *
 * Note: the scratch allocator may also be set by calling @ref
 * hs_set_allocator().
 *
 * @param alloc_func
 *      A callback function pointer that allocates memory. This function must
 *      return memory suitably aligned for the largest representable data type
 *      on this platform.
 *
 * @param free_func
 *      A callback function pointer that frees allocated memory.
 *
 * @return
 *      @ref HS_SUCCESS on success, other values on failure.
 */
hs_error_t hs_set_scratch_allocator(hs_alloc_t alloc_func, hs_free_t free_func);

/**
 * Set the allocate and free functions used by Hyperscan for allocating memory
 * for stream state by @ref hs_open_stream().
 *
 * If no stream allocation functions are set, or if NULL is used in place of
 * both parameters, then memory allocation will default to standard methods
 * (such as the system malloc() and free() calls).
 *
 * This call will override any previous stream allocators that have been set.
 *
 * Note: the stream allocator may also be set by calling @ref
 * hs_set_allocator().
 *
 * @param alloc_func
 *      A callback function pointer that allocates memory. This function must
 *      return memory suitably aligned for the largest representable data type
 *      on this platform.
 *
 * @param free_func
 *      A callback function pointer that frees allocated memory.
 *
 * @return
 *      @ref HS_SUCCESS on success, other values on failure.
 */
hs_error_t hs_set_stream_allocator(hs_alloc_t alloc_func, hs_free_t free_func);

/**
 * Utility function for identifying this release version.
 *
 * @return
 *      A string containing the version number of this release build and the
 *      date of the build. It is allocated statically, so it does not need to
 *      be freed by the caller.
 */
const char *hs_version(void);

/**
 * @defgroup HS_ERROR hs_error_t values
 *
 * @{
 */

/**
 * The engine completed normally.
 */
#define HS_SUCCESS              0

/**
 * A parameter passed to this function was invalid.
 */
#define HS_INVALID              (-1)

/**
 * A memory allocation failed.
 */
#define HS_NOMEM                (-2)

/**
 * The engine was terminated by callback.
 *
 * This return value indicates that the target buffer was partially scanned,
 * but that the callback function requested that scanning cease after a match
 * was located.
 */
#define HS_SCAN_TERMINATED      (-3)

/**
 * The pattern compiler failed, and the @ref hs_compile_error_t should be
 * inspected for more detail.
 */
#define HS_COMPILER_ERROR       (-4)

/**
 * The given database was built for a different version of Hyperscan.
 */
#define HS_DB_VERSION_ERROR     (-5)

/**
 * The given database was built for a different platform (i.e., CPU type).
 */
#define HS_DB_PLATFORM_ERROR    (-6)

/**
 * The given database was built for a different mode of operation. This error
 * is returned when streaming calls are used with a block or vectored database
 * and vice versa.
 */
#define HS_DB_MODE_ERROR        (-7)

/**
 * A parameter passed to this function was not correctly aligned.
 */
#define HS_BAD_ALIGN            (-8)

/**
 * The memory allocator (either malloc() or the allocator set with @ref
 * hs_set_allocator()) did not correctly return memory suitably aligned for the
 * largest representable data type on this platform.
 */
#define HS_BAD_ALLOC            (-9)

/** @} */

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* HS_COMMON_H_ */
