// Copyright 2024 Accenture.

#ifndef GUARD_4F4D1CB1_4346_427E_B849_C1AD7AA2FA53
#define GUARD_4F4D1CB1_4346_427E_B849_C1AD7AA2FA53

#include "util/stream/IOutputStream.h"

#include <etl/span.h>
#include <etl/string_view.h>

#include <cstdint>

namespace util
{
namespace stream
{
/**
 * The IOutputStream class that implements specific output logics.
 *
 */
class NormalizeLfOutputStream : public IOutputStream
{
public:
    /**
     * The class c-tor
     * \param stm The output stream
     * \param crlf (curret return line feed): the CR/LF symbols specific for new line.
     */
    explicit NormalizeLfOutputStream(IOutputStream& strm, char const* crlf = nullptr);

    bool isEof() const override;

    void write(uint8_t data) override;
    void write(::etl::span<uint8_t const> const& buffer) override;

private:
    IOutputStream& _stream;
    char const* _crlf;
};

} // namespace stream
} // namespace util

#endif /* GUARD_4F4D1CB1_4346_427E_B849_C1AD7AA2FA53 */
