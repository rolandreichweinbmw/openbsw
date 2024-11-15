// Copyright 2024 Accenture.

#include "util/stream/StringBufferOutputStream.h"

#include <etl/char_traits.h>
#include <etl/limits.h>
#include <etl/memory.h>
#include <etl/span.h>
#include <etl/string.h>

#include <cstring>

namespace util
{
namespace stream
{
StringBufferOutputStream::StringBufferOutputStream(
    ::etl::span<char> const buf, char const* const endOfString, char const* const ellipsis)
: IOutputStream()
, _buffer(buf)
, _endOfString((endOfString != nullptr) ? endOfString : "")
, _ellipsis((ellipsis != nullptr) ? ellipsis : "")
, _currentIndex(0U)
, _overflow(false)
{}

StringBufferOutputStream::~StringBufferOutputStream() { (void)getString(); }

bool StringBufferOutputStream::isEof() const { return (_currentIndex + 1U) >= _buffer.size(); }

void StringBufferOutputStream::write(uint8_t const data)
{
    if (_currentIndex < _buffer.size())
    {
        _buffer[_currentIndex] = static_cast<char>(data);
        ++_currentIndex;
    }
}

void StringBufferOutputStream::write(::etl::span<uint8_t const> const& buffer)
{
    auto trimmedBuffer = buffer;
    size_t size        = buffer.size();
    if ((_currentIndex + size) > _buffer.size())
    {
        size          = _buffer.size() - _currentIndex;
        _overflow     = true;
        trimmedBuffer = trimmedBuffer.subspan(0, size);
    }
    (void)::etl::mem_copy(
        trimmedBuffer.begin(),
        trimmedBuffer.size(),
        reinterpret_cast<uint8_t*>(_buffer.begin() + _currentIndex));
    _currentIndex += size;
}

::etl::span<char> StringBufferOutputStream::getBuffer()
{
    char const* const tempString = getString();
    size_t const tempSize        = _buffer.size();
    return _buffer.subspan(0, ::etl::strlen(tempString, tempSize) + 1U);
}

void StringBufferOutputStream::reset()
{
    _currentIndex = 0U;
    _overflow     = false;
}

char const* StringBufferOutputStream::getString()
{
    auto const dataBuffer
        = ::etl::span<uint8_t>(reinterpret_cast<uint8_t*>(_buffer.begin()), _buffer.size());
    size_t const eolLen = ::etl::strlen(_endOfString, _buffer.size()) + 1U;
    if (_overflow || ((eolLen + _currentIndex) > _buffer.size()))
    {
        size_t const ellipsisLen = ::etl::strlen(_ellipsis, _buffer.size());
        _currentIndex            = _buffer.size() - (eolLen + ellipsisLen);
        (void)::etl::mem_copy(
            reinterpret_cast<uint8_t const*>(_ellipsis),
            ellipsisLen,
            dataBuffer.begin() + _currentIndex);
        _currentIndex += ellipsisLen;
    }
    (void)::etl::mem_copy(
        reinterpret_cast<uint8_t const*>(_endOfString), eolLen, dataBuffer.begin() + _currentIndex);
    return _buffer.data();
}

} // namespace stream
} // namespace util
