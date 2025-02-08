// Copyright 2024 Accenture.

#ifndef GUARD_9051A456_75DC_4F9B_9229_733D2CA61E80
#define GUARD_9051A456_75DC_4F9B_9229_733D2CA61E80

#include <etl/span.h>

namespace io
{
/**
 * Interface for reading slices of bytes with variable size from a data channel.
 */
class IReader
{
public:
    virtual ~IReader() = default;

    IReader& operator=(IReader const&) = delete;

    // [PUBLICAPI_START]
    /**
     * Returns the maximum number of bytes that may be read in one peek call
     */
    virtual size_t maxSize() const = 0;

    /**
     * Returns a slice to the next piece of available data.
     * \return - Slice of bytes if underlying queue was not empty
     *         - Empty slice otherwise
     */
    virtual ::etl::span<uint8_t> peek() const = 0;

    /**
     * Releases the memory that the last peek() call returned.
     *
     * Calling release() makes a new call to peek() mandatory! The previously peeked data must not
     * be used anymore.
     */
    virtual void release() = 0;
    // [PUBLICAPI_END]
};

/**
 * RAAI guard class that will automatically release the data of a given IReader
 * when the guard goes out of scope.
 *
 * \note
 * When going out of scope, release() will be called even if peek() of this class was not used.
 * This is done because peek() can also be called on the protected IReader directly
 * and there's no way how this guard could know. To assure a consistent behavior, the contract is
 * that data will be released unconditionally.
 */
class ReleaseGuard
{
    IReader& _reader;

public:
    /**
     * Creates a guard protecting a given reader.
     */
    explicit ReleaseGuard(IReader& reader) : _reader(reader) {}

    ~ReleaseGuard() { _reader.release(); }

    /**
     * Alias forwarding peek() to underlying reader.
     */
    ::etl::span<uint8_t> peek() const { return _reader.peek(); }
};

} // namespace io

#endif // GUARD_9051A456_75DC_4F9B_9229_733D2CA61E80
