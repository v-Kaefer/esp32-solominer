#include <string.h>
#include "unity.h"
#include "mbedtls/md.h"

// Forward declarations of functions from main.c that we want to test
// These should ideally be in a header file
extern void double_sha256(mbedtls_md_context_t* ctx, const uint8_t* data, size_t len, uint8_t* hash);
extern uint32_t count_leading_zeros(const uint8_t* hash);
extern void init_block_header(void);

// Test fixtures
static uint8_t test_hash[32];
static uint8_t test_data[80];
static mbedtls_md_context_t sha_ctx;

void setUp(void)
{
    memset(test_hash, 0, sizeof(test_hash));
    memset(test_data, 0, sizeof(test_data));
    
    // Initialize SHA256 context for tests
    mbedtls_md_init(&sha_ctx);
    mbedtls_md_setup(&sha_ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 0);
}

void tearDown(void)
{
    // Cleanup after each test
    mbedtls_md_free(&sha_ctx);
}

// Test double_sha256 with known input
void test_double_sha256_basic(void)
{
    // Simple test data
    const char* test_input = "hello";
    uint8_t hash[32];
    
    double_sha256(&sha_ctx, (const uint8_t*)test_input, strlen(test_input), hash);
    
    // Verify we got a hash (not all zeros)
    bool all_zeros = true;
    for (int i = 0; i < 32; i++) {
        if (hash[i] != 0) {
            all_zeros = false;
            break;
        }
    }
    TEST_ASSERT_FALSE(all_zeros);
}

// Test double_sha256 with empty input
void test_double_sha256_empty(void)
{
    uint8_t hash[32];
    uint8_t empty_data[1] = {0};
    
    double_sha256(&sha_ctx, empty_data, 0, hash);
    
    // Verify we got a hash
    TEST_ASSERT_NOT_NULL(hash);
}

// Test double_sha256 determinism (same input produces same output)
void test_double_sha256_deterministic(void)
{
    const char* test_input = "test data";
    uint8_t hash1[32];
    uint8_t hash2[32];
    
    double_sha256(&sha_ctx, (const uint8_t*)test_input, strlen(test_input), hash1);
    double_sha256(&sha_ctx, (const uint8_t*)test_input, strlen(test_input), hash2);
    
    // Both hashes should be identical
    TEST_ASSERT_EQUAL_MEMORY(hash1, hash2, 32);
}

// Test count_leading_zeros with all zeros
void test_count_leading_zeros_all_zeros(void)
{
    uint8_t hash[32];
    memset(hash, 0, 32);
    
    uint32_t zeros = count_leading_zeros(hash);
    
    TEST_ASSERT_EQUAL_UINT32(256, zeros);
}

// Test count_leading_zeros with no leading zeros
void test_count_leading_zeros_none(void)
{
    uint8_t hash[32];
    memset(hash, 0xFF, 32);
    
    uint32_t zeros = count_leading_zeros(hash);
    
    TEST_ASSERT_EQUAL_UINT32(0, zeros);
}

// Test count_leading_zeros with one byte of zeros
void test_count_leading_zeros_one_byte(void)
{
    uint8_t hash[32];
    memset(hash, 0, 32);
    hash[31] = 0;   // Last byte (most significant in Bitcoin) is zero
    hash[30] = 0x80; // Second-to-last byte starts with 1
    
    uint32_t zeros = count_leading_zeros(hash);
    
    TEST_ASSERT_EQUAL_UINT32(8, zeros);
}

// Test count_leading_zeros with partial byte
void test_count_leading_zeros_partial_byte(void)
{
    uint8_t hash[32];
    memset(hash, 0, 32);
    hash[31] = 0x0F; // 0000 1111 - 4 leading zeros
    
    uint32_t zeros = count_leading_zeros(hash);
    
    TEST_ASSERT_EQUAL_UINT32(4, zeros);
}

// Test count_leading_zeros with multiple bytes
void test_count_leading_zeros_multiple_bytes(void)
{
    uint8_t hash[32];
    memset(hash, 0, 32);
    hash[31] = 0x00;
    hash[30] = 0x00;
    hash[29] = 0x01; // 16 zeros + 7 zeros = 23 leading zeros
    
    uint32_t zeros = count_leading_zeros(hash);
    
    TEST_ASSERT_EQUAL_UINT32(23, zeros);
}

// Register tests with Unity
void test_mining_functions(void)
{
    RUN_TEST(test_double_sha256_basic);
    RUN_TEST(test_double_sha256_empty);
    RUN_TEST(test_double_sha256_deterministic);
    RUN_TEST(test_count_leading_zeros_all_zeros);
    RUN_TEST(test_count_leading_zeros_none);
    RUN_TEST(test_count_leading_zeros_one_byte);
    RUN_TEST(test_count_leading_zeros_partial_byte);
    RUN_TEST(test_count_leading_zeros_multiple_bytes);
}
