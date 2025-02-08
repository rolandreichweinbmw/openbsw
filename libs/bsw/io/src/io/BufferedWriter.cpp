// Copyright 2024 Accenture.

#include "io/BufferedWriter.h"

namespace io
{

::etl::span<uint8_t> BufferedWriter::allocate(size_t const size)
{
    size_t const MAXIMUM_BUFFER_SIZE = _destination.maxSize();
    if (size > MAXIMUM_BUFFER_SIZE)
    {
        _size = 0;
        return {};
    }
    bool const isEnoughSpaceAvailable = _current.size() >= size;
    if (!isEnoughSpaceAvailable)
    {
        flush();
        _current = _destination.allocate(MAXIMUM_BUFFER_SIZE);
        if (_current.size() < MAXIMUM_BUFFER_SIZE)
        {
            _size = 0;
            return {};
        }
    }
    _size = size;
    return _current.subspan(0, _size);
}

void BufferedWriter::commit()
{
    _current = _current.subspan(_size);
    _size    = 0;
}

void BufferedWriter::flush()
{
    if (_current.size() > 0)
    {
        // Trim the buffer before committing.
        (void)_destination.allocate(_destination.maxSize() - _current.size());
    }
    _destination.commit();
    _current = {};
}

} // namespace io
