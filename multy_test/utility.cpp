/* Copyright 2018 by Multy.io
 * Licensed under Multy.io license
 *
 * See LICENSE for details
 */

#include "multy_test/utility.h"

#include "multy_core/account.h"
#include "multy_core/common.h"
#include "multy_core/src/api/big_int_impl.h"
#include "multy_core/src/api/key_impl.h"

#include "multy_test/value_printers.h"

#include "wally_core.h"

#include <exception>

#include <memory.h>
#include <string.h>

namespace
{
using namespace multy_core::internal;

size_t dummy_fill_entropy(void*, size_t size, void* dest)
{
    static const size_t entropy_max_size = 1024;
    unsigned char silly_entropy[entropy_max_size];

    if (size > entropy_max_size)
    {
        return 0;
    }

    memcpy(dest, silly_entropy, size);
    return size;
}

} // namespace

namespace test_utility
{

const uint64_t SATOSHIS_IN_BTC = 100L*1000*1000;
const uint64_t WEIS_IN_ETH = 1000L*1000*1000*1000*1000*1000;
const uint64_t WEIS_IN_GWEI = 1000L*1000*1000;


// Returns Satoshi corresponding to fractional amount of BTC.
BigInt operator"" _BTC(const long double btc)
{
    const double satoshi = btc * SATOSHIS_IN_BTC;
    if (satoshi < 0)
    {
        THROW_EXCEPTION("Value is too low.");
    }
    if(satoshi > std::numeric_limits<uint64_t>::max())
    {
        THROW_EXCEPTION("Value is too high.");
    }

    return BigInt(static_cast<uint64_t>(satoshi));
}

BigInt operator"" _SATOSHI(const unsigned long long int satoshi)
{
    return BigInt(static_cast<uint64_t>(satoshi));
}

BigInt operator "" _ETH(const long double eth)
{
    const double wei = eth * WEIS_IN_ETH;
    if (wei < 0)
    {
        THROW_EXCEPTION("Value is too low.");
    }
    if(wei > std::numeric_limits<uint64_t>::max())
    {
        THROW_EXCEPTION("Value is too high.");
    }

    return BigInt(static_cast<uint64_t>(wei));
}

BigInt operator "" _GWEI(const long double gwei)
{
    const double wei = gwei * WEIS_IN_GWEI;
    if (wei < 0)
    {
        THROW_EXCEPTION("Value is too low.");
    }
    if(wei > std::numeric_limits<uint64_t>::max())
    {
        THROW_EXCEPTION("Value is too high.");
    }

    return BigInt(static_cast<uint64_t>(wei));
}

BigInt operator "" _WEI(const unsigned long long int wei)
{
    return BigInt(static_cast<uint64_t>(wei));
}

bytes from_hex(const char* hex_str)
{
    const size_t expected_size = strlen(hex_str) / 2;
    bytes result(expected_size);
    size_t bytes_written = 0;

    E(wally_hex_to_bytes(
            hex_str, result.data(), result.size(), &bytes_written));
    result.resize(bytes_written);

    return result;
}

std::string to_hex(const bytes& bytes)
{
    CharPtr hex_str;
    E(wally_hex_from_bytes(bytes.data(), bytes.size(), reset_sp(hex_str)));
    return std::string(hex_str.get());
}

std::string to_hex(const BinaryData& data)
{
    if (!data.data)
    {
        throw std::runtime_error("BinaryData::data is null");
    }

    CharPtr hex_str;
    E(wally_hex_from_bytes(data.data, data.len, reset_sp(hex_str)));
    return std::string(hex_str.get());
}

ExtendedKey make_dummy_extended_key()
{
    ExtendedKey result;
    memset(&result.key, 0, sizeof(result.key));
    return result;
}

ExtendedKeyPtr make_dummy_extended_key_ptr()
{
    return ExtendedKeyPtr(new ExtendedKey(make_dummy_extended_key()));
}

EntropySource make_dummy_entropy_source()
{
    return EntropySource{nullptr, dummy_fill_entropy};
}

void throw_exception(const char* message)
{
    throw std::runtime_error(message);
}

void throw_exception_if_error(Error* error)
{
    multy_core::internal::throw_if_error(error);
}

bool blockchain_can_derive_address_from_private_key(Blockchain blockchain)
{
    switch(blockchain)
    {
        case BLOCKCHAIN_BITCOIN:
        case BLOCKCHAIN_ETHEREUM:
            return true;
#if MULTY_WITH_GOLOS
        case BLOCKCHAIN_GOLOS:
            return false;
#endif // MULTY_WITH_GOLOS
#if MULTY_WITH_EOS
        case BLOCKCHAIN_EOS:
            return false;
#endif // MULTY_WITH_EOS
        default:
            assert(false && "Unsupported blockchain type");
    }
    return false;
}

std::string minify_json(const std::string& input_json)
{
    std::string result;
    result.reserve(input_json.length());
    bool quoted = false;
    bool last_backslash = false;
    for (const auto& c : input_json)
    {
        if (!isspace(c) || quoted)
        {
            result += c;
        }
        if (c == '"' && !last_backslash)
        {
            quoted = !quoted;
        }
        if (c == '\\' && !last_backslash)
        {
            last_backslash = true;
        } else
        {
            last_backslash = false;
        }
     }
    return result;
}

} // namespace test_utility

bool operator==(const PublicKey& lhs, const PublicKey& rhs)
{
    return lhs.get_content() == rhs.get_content();
}

bool operator==(const PrivateKey& lhs, const PrivateKey& rhs)
{
    return lhs.to_string() == rhs.to_string();
}
