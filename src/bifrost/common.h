/*
 * Copyright (c) 2016, The Bifrost Authors. All rights reserved.
 * Copyright (c) 2016, NVIDIA CORPORATION. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * * Neither the name of The Bifrost Authors nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*! \file common.h
 *  \brief Common definitions used throughout the library
 */

#ifndef BF_COMMON_H_INCLUDE_GUARD_
#define BF_COMMON_H_INCLUDE_GUARD_
#define BF_MAX_DIM 3

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int                BFstatus;
typedef int                BFbool;
typedef int                BFenum;
typedef float              BFcomplex[2];
typedef float              BFreal;
typedef uint64_t           BFsize; // TODO: Check this on TK1 (32 bit)
//typedef unsigned long      BFsize;
//typedef size_t             BFsize;
//typedef unsigned long long BFoffset;
typedef uint64_t BFoffset;
//typedef unsigned char      BFoffset; // HACK TESTING correct offset wrapping
typedef   signed long long BFdelta;
enum {
	BF_SPACE_AUTO         = 0,
	BF_SPACE_SYSTEM       = 1, // aligned_alloc
	BF_SPACE_CUDA         = 2, // cudaMalloc
	BF_SPACE_CUDA_HOST    = 3, // cudaHostAlloc
	BF_SPACE_CUDA_MANAGED = 4  // cudaMallocManaged
};

typedef BFenum BFspace;
/// Defines a single atom of data to be passed to a function.
typedef struct BFarray_ {
    /*! The data pointer can point towards any type of data, 
     *  so long as there is a corresponding definition in dtype. 
     *  This data should be an ndim array, which every element of
     *  type dtype.
     */
    void* data;
    /*! Where this data is located in memory.
     *  Used to ensure that operations called are localized within
     *  that space, such as a CUDA funciton operating on device
     *  memory.
     */
    BFspace space;
    unsigned dtype;
    int ndim;
    BFsize shape[BF_MAX_DIM];
    BFsize strides[BF_MAX_DIM];
} BFarray;

enum {
	BF_STATUS_SUCCESS            = 0,
	BF_STATUS_END_OF_DATA        = 1,
	BF_STATUS_INVALID_POINTER    = 2,
	BF_STATUS_INVALID_HANDLE     = 3,
	BF_STATUS_INVALID_ARGUMENT   = 4,
	BF_STATUS_INVALID_STATE      = 5,
	BF_STATUS_MEM_ALLOC_FAILED   = 6,
	BF_STATUS_MEM_OP_FAILED      = 7,
	BF_STATUS_UNSUPPORTED        = 8,
	BF_STATUS_FAILED_TO_CONVERGE = 9,
	BF_STATUS_INTERNAL_ERROR     = 10
};

// Utility
const char* bfGetStatusString(BFstatus status);
BFbool      bfGetDebugEnabled();
BFbool      bfGetCudaEnabled();

#ifdef __cplusplus
} // extern "C"
#endif

#endif // BF_COMMON_H_INCLUDE_GUARD_