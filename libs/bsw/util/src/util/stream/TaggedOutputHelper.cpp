// Copyright 2024 Accenture.

#include "util/stream/TaggedOutputHelper.h"

#include <etl/string_view.h>

namespace util
{
namespace stream
{
TaggedOutputHelper::TaggedOutputHelper(char const* const prefix, char const* const suffix)
: _prefix((prefix != nullptr) ? prefix : "")
, _suffix((suffix != nullptr) ? suffix : "")
, _lineStart(true)
{}

void TaggedOutputHelper::reset() { _lineStart = true; }

bool TaggedOutputHelper::isLineStart() const { return _lineStart; }

void TaggedOutputHelper::writeBytes(IOutputStream& strm, ::etl::span<uint8_t const> const& buffer)
{
    for (auto&& data : buffer)
    {
        if (_lineStart)
        {
            _lineStart = false;
            ::etl::string_view view(_prefix);
            strm.write(::etl::span<uint8_t const>(
                reinterpret_cast<uint8_t const*>(view.begin()), view.size()));
        }
        if (data == static_cast<uint8_t>('\n'))
        {
            ::etl::string_view view(_suffix);
            strm.write(::etl::span<uint8_t const>(
                reinterpret_cast<uint8_t const*>(view.begin()), view.size()));
            _lineStart = true;
        }
        else
        {
            strm.write(data);
        }
    }
}

void TaggedOutputHelper::writeBytes(IOutputStream& strm, ::etl::string_view const& view)
{
    ::etl::span<uint8_t const> buffer{reinterpret_cast<uint8_t const*>(view.begin()), view.size()};
    return writeBytes(strm, buffer);
}

void TaggedOutputHelper::endLine(IOutputStream& strm)
{
    if (!_lineStart)
    {
        ::etl::string_view view(_suffix);
        strm.write(::etl::span<uint8_t const>(
            reinterpret_cast<uint8_t const*>(view.begin()), view.size()));
        _lineStart = true;
    }
}

} // namespace stream
} // namespace util
