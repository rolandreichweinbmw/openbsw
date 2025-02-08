// Copyright 2024 Accenture.

#ifndef GUARD_F7ED78E6_29F6_4D5B_89DE_C8EB8DCC3F1E
#define GUARD_F7ED78E6_29F6_4D5B_89DE_C8EB8DCC3F1E

#include "io/IWriter.h"

#include <etl/memory.h>
#include <etl/span.h>
#include <util/estd/assert.h>

#include <array>
#include <limits>

namespace io
{
/**
 * Class for writing at once to multiple queues
 *
 * This "splits" the stream of data into multiple copies, by saving the data to the first queue and
 * then writing it to all of them at commit time.
 *
 * [TPARAMS_BEGIN]
 * \tparam N Number of IWriter this SplitWriter wraps.
 * [TPARAMS_END]
 */
template<size_t N>
class SplitWriter : public IWriter
{
private:
    static constexpr size_t INVALID_BUF = std::numeric_limits<size_t>::max();

public:
    // [PUBLIC_API_BEGIN]
    /**
     * Constructs a SplitWriter from an array of IWriter destinations.
     *
     * \assert destinations[0..N-1] != nullptr
     * \assert destinations[0]->maxSize() == destinations[1]->maxSize() ... ==
     * destinations[N-1]->maxSize()
     */
    explicit SplitWriter(::etl::span<IWriter*, N> destinations);

    /**
     * \see ::io::IWriter::maxSize()
     */
    size_t maxSize() const override;

    /**
     * Allocates a slice of bytes of a given size from the first queue that has free space
     *
     * \param size  Number of bytes to allocate
     * \return  - Empty slice, if requested size was greater as MAX_ELEMENT_SIZE or no memory
     *            is available
     *          - Slice of bytes otherwise.
     */
    ::etl::span<uint8_t> allocate(size_t size) override;

    /**
     * Commits the previously allocated data to all destinations. If a destination is not
     * capable of receiving the data, the corresponding drop counter is increased. If the
     * data has been committed successfully, the corresponding sent counter is increased.
     */
    void commit() override;

    /**
     * Empty function, nothing special to do for flushing to the other end.
     */
    void flush() override;
    // [PUBLIC_API_END]

    /**
     * Statistics about dropped messages for each writer
     */
    std::array<size_t, N> drops;

    /**
     * Statistics about messages sent to each writer
     */
    std::array<size_t, N> sent;

private:
    ::etl::span<IWriter*, N> const _destinations;
    ::etl::span<uint8_t> _buffer;
    size_t _bufNum;
};

template<size_t N>
SplitWriter<N>::SplitWriter(::etl::span<IWriter*, N> const destinations)
: drops(), sent(), _destinations(destinations), _buffer(), _bufNum(INVALID_BUF)
{
    static_assert(N != 0, "Split writers cannot have size 0.");
    estd_assert(_destinations[0] != nullptr);
    auto const max = _destinations[0]->maxSize();
    for (size_t i = 1; i < N; i++)
    {
        estd_assert(_destinations[i] != nullptr);
        auto const maxi = _destinations[i]->maxSize();
        estd_assert(maxi == max);
    }
}

template<size_t N>
inline size_t SplitWriter<N>::maxSize() const
{
    return _destinations[0]->maxSize();
}

template<size_t N>
::etl::span<uint8_t> SplitWriter<N>::allocate(size_t const size)
{
    if (INVALID_BUF != _bufNum)
    {
        return _buffer;
    }
    for (size_t i = 0; i < N; i++)
    {
        auto const tmp = _destinations[i]->allocate(size);
        if (tmp.size() == size)
        {
            _buffer = tmp;
            _bufNum = i;
            return _buffer;
        }
    }
    return {};
}

template<size_t N>
void SplitWriter<N>::commit()
{
    if (_bufNum == INVALID_BUF)
    {
        return;
    }
    for (size_t i = 0; i < N; i++)
    {
        if (i == _bufNum)
        {
            // skip the one used as buffer to do it last
            continue;
        }
        auto const dest = _destinations[i];
        auto const tmp  = dest->allocate(_buffer.size());
        if (tmp.size() < _buffer.size())
        {
            drops[i] += 1;
            continue;
        }
        (void)::etl::mem_copy(_buffer.begin(), _buffer.size(), tmp.begin());
        sent[i] += 1;
        dest->commit();
    }

    sent[_bufNum] += 1;
    _destinations[_bufNum]->commit();
    _bufNum = INVALID_BUF;
}

template<size_t N>
inline void SplitWriter<N>::flush()
{}

} // namespace io

#endif /* GUARD_F7ED78E6_29F6_4D5B_89DE_C8EB8DCC3F1E */
