// Copyright 2024 Accenture.

#ifndef GUARD_D5DE0BBD_C445_4CA1_8C3E_18B55106EF3D
#define GUARD_D5DE0BBD_C445_4CA1_8C3E_18B55106EF3D

#include <etl/span.h>
#include <etl/string_view.h>

#include <cstdint>

namespace util
{
namespace stream
{
class IOutputStream
{
public:
    IOutputStream() = default;

    IOutputStream(IOutputStream const&)            = delete;
    IOutputStream& operator=(IOutputStream const&) = delete;

    /**
     * Indicates whether or not the stream ends i.e. bytes can be written.
     *
     * \return
     * - true if the end of the stream has been reached
     * - false otherwise
     */

    virtual bool isEof() const = 0;
    /**
     * Writes data to the stream.
     *
     * \param data data to write to the buffer.
     */

    virtual void write(uint8_t data) = 0;

    /**
     * Writes the bytes of a buffer to the stream.
     *
     * \param buffer block of bytes to write to the stream.
     */
    virtual void write(::etl::span<uint8_t const> const& buffer) = 0;

    /**
     * Writes the bytes of a string_view to the stream.
     *
     * Convenience function
     *
     * \param view block of bytes to write to the stream.
     */
    inline void write_string_view(::etl::string_view const& view)
    {
        ::etl::span<uint8_t const> buffer{
            reinterpret_cast<uint8_t const*>(view.begin()), view.size()};
        write(buffer);
    }
};

} // namespace stream
} // namespace util

#endif /* GUARD_D5DE0BBD_C445_4CA1_8C3E_18B55106EF3D */
