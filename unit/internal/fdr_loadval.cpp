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

#include "config.h"
#include "gtest/gtest.h"

#include "fdr/fdr_loadval.h"
#include "util/alloc.h"

using namespace std;
using namespace testing;
using namespace ue2;

// Normal (unaligned) load.
template <typename T> T lv(const u8 *ptr, const u8 *lo, const u8 *hi);

// Aligned load.
template <typename T> T lv_a(const u8 *ptr, const u8 *lo, const u8 *hi);

// Cautious forward load.
template <typename T> T lv_cf(const u8 *ptr, const u8 *lo, const u8 *hi);

// Cautious backward load.
template <typename T> T lv_cb(const u8 *ptr, const u8 *lo, const u8 *hi);

// Cautious everywhere load.
template <typename T> T lv_ce(const u8 *ptr, const u8 *lo, const u8 *hi);

// Special case: there is no specific _a "aligned load" func for u8. We proxy
// that to the normal load.
static u8 lv_u8_a(const u8 *ptr, const u8 *lo, const u8 *hi) {
    return lv_u8(ptr, lo, hi);
}

#define BUILD_LOADVALS(vtype)                                                  \
    template <> vtype lv<vtype>(const u8 *ptr, const u8 *lo, const u8 *hi) {   \
        return lv_##vtype(ptr, lo, hi);                                        \
    }                                                                          \
    template <> vtype lv_a<vtype>(const u8 *ptr, const u8 *lo, const u8 *hi) { \
        return lv_##vtype##_a(ptr, lo, hi);                                    \
    }                                                                          \
    template <>                                                                \
    vtype lv_cf<vtype>(const u8 *ptr, const u8 *lo, const u8 *hi) {            \
        return lv_##vtype##_cf(ptr, lo, hi);                                   \
    }                                                                          \
    template <>                                                                \
    vtype lv_cb<vtype>(const u8 *ptr, const u8 *lo, const u8 *hi) {            \
        return lv_##vtype##_cb(ptr, lo, hi);                                   \
    }                                                                          \
    template <>                                                                \
    vtype lv_ce<vtype>(const u8 *ptr, const u8 *lo, const u8 *hi) {            \
        return lv_##vtype##_ce(ptr, lo, hi);                                   \
    }

BUILD_LOADVALS(u8)
BUILD_LOADVALS(u16)
BUILD_LOADVALS(u32)
BUILD_LOADVALS(u64a)
BUILD_LOADVALS(m128)

template <typename T> class FDR_Loadval : public testing::Test {
    // empty
};

typedef ::testing::Types<u8, u16, u32, u64a, m128> LoadvalTypes;

TYPED_TEST_CASE(FDR_Loadval, LoadvalTypes);

static void fillWithBytes(u8 *ptr, size_t len) {
    for (size_t i = 0; i < len; i++) {
        ptr[i] = (u8)(i % 254 + 1);
    }
}

TYPED_TEST(FDR_Loadval, Normal) {
    // We should be able to do a normal load at any alignment.
    const size_t len = sizeof(TypeParam);
    aligned_unique_ptr<u8> mem_p = aligned_zmalloc_unique<u8>(len + 15);
    u8 * mem = mem_p.get();
    ASSERT_TRUE(ISALIGNED_16(mem));
    fillWithBytes(mem, len + 15);

    // Test all alignments.
    for (size_t i = 0; i < 16; i++) {
        const u8 *src = mem + i;
        TypeParam val = lv<TypeParam>(src, src, src + len);
        // Should be identical to 'src' in byte order.
        ASSERT_EQ(0, memcmp(&val, src, len));
    }
}

TYPED_TEST(FDR_Loadval, Aligned) {
    const size_t len = sizeof(TypeParam);
    aligned_unique_ptr<u8> mem_p = aligned_zmalloc_unique<u8>(len); // 16 aligned
    u8 * mem = mem_p.get();
    ASSERT_TRUE(ISALIGNED_16(mem));
    fillWithBytes(mem, len);

    TypeParam val = lv_a<TypeParam>(mem, mem, mem + len);

    // Should be identical to 'mem' in byte order.
    ASSERT_EQ(0, memcmp(&val, mem, len));
}

TYPED_TEST(FDR_Loadval, CautiousForward) {
    // For a cautious forward load, we will get zeroes for all bytes after the
    // 'hi' ptr.
    const size_t len = sizeof(TypeParam);

    aligned_unique_ptr<u8> mem_p = aligned_zmalloc_unique<u8>(len + 1);
    u8 *mem = mem_p.get() + 1; // force unaligned
    fillWithBytes(mem, len);

    for (size_t i = 1; i <= len; i++) {
        const u8 *ptr = mem;
        const u8 *lo = ptr;
        const u8 *hi = ptr + i;
        union {
            TypeParam val;
            u8 bytes[sizeof(TypeParam)];
        } x;

        x.val = lv_cf<TypeParam>(ptr, lo, hi);

        // Low bytes will be correct, bytes >= hi will be zero.
        for (size_t j = 0; j < len; j++) {
            ASSERT_EQ(j < i ? mem[j] : 0, x.bytes[j]);
        }
    }
}

TYPED_TEST(FDR_Loadval, CautiousBackward) {
    // For a cautious backwards load, we will get zeroes for all bytes before
    // the 'lo' ptr.
    const size_t len = sizeof(TypeParam);

    aligned_unique_ptr<u8> mem_p = aligned_zmalloc_unique<u8>(len + 1);
    u8 *mem = mem_p.get() + 1; // force unaligned
    fillWithBytes(mem, len);

    for (size_t i = 1; i <= len; i++) {
        const u8 *ptr = mem;
        const u8 *lo = ptr + sizeof(TypeParam) - i;
        const u8 *hi = ptr + sizeof(TypeParam);
        union {
            TypeParam val;
            u8 bytes[sizeof(TypeParam)];
        } x;

        x.val = lv_cb<TypeParam>(ptr, lo, hi);

        // Low bytes will be zero, bytes >= lo will be correct.
        for (size_t j = 0; j < len; j++) {
            ASSERT_EQ(j < sizeof(TypeParam) - i ? 0 : mem[j], x.bytes[j]);
        }
    }
}

TYPED_TEST(FDR_Loadval, CautiousEverywhere) {
    // For a cautious backwards load, we will get zeroes for all bytes before
    // the 'lo' ptr or after the 'hi' ptr.
    const size_t len = sizeof(TypeParam);

    aligned_unique_ptr<u8> mem_p = aligned_zmalloc_unique<u8>(len + 1);
    u8 *mem = mem_p.get() + 1; // force unaligned
    fillWithBytes(mem, len);

    for (size_t i = 0; i <= len; i++) {
        for (size_t j = 0; j <= len; j++) {
            const u8 *ptr = mem;
            const u8 *lo = ptr + i;
            const u8 *hi = ptr + j;
            union {
                TypeParam val;
                u8 bytes[sizeof(TypeParam)];
            } x;

            x.val = lv_ce<TypeParam>(ptr, lo, hi);

            // Bytes outside [lo,hi) will be zero.
            for (size_t k = 0; k < len; k++) {
                ASSERT_EQ((k >= i && k < j) ? mem[k] : 0, x.bytes[k]);
            }
        }
    }
}
