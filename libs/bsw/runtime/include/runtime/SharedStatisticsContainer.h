// Copyright 2024 Accenture.

#ifndef GUARD_2A81615E_F0E2_47BD_AEAF_A8B88DA403D7
#define GUARD_2A81615E_F0E2_47BD_AEAF_A8B88DA403D7

#include <etl/array.h>
#include <etl/optional.h>
#include <etl/string.h>
#include <runtime/StatisticsContainer.h>
#include <util/estd/assert.h>

namespace runtime
{

/**
 * A wrapper for StatisticContainer with storage for the names. Intended for sharing runtime
 * statistics across cores, where the runtime access to the task names is not available. The names
 * are retrieved and cached upon the first call to copyFrom.
 *
 * Implementation note: since any particular core may call getIterator (which in turn uses
 * StatisticsContainer::getName), the function pointer has to be created each time, so that it
 * points to the implementation of SharedStatisticsContainer::getName residing in the callers
 * program memory.
 */
template<class Statistics, size_t N, class Entry = Statistics>
class SharedStatisticsContainer
{
public:
    using StatisticsContainer = ::runtime::declare::StatisticsContainer<Statistics, N, Entry>;
    using GetNameType         = typename StatisticsContainer::GetNameType;

    SharedStatisticsContainer() : _statisticsContainer(), _names{} {}

    template<class OtherEntry>
    void copyFrom(::runtime::StatisticsContainer<Entry, OtherEntry> const& src)
    {
        // if names not initialized, do so now
        if (!_names.has_value())
        {
            estd_assert(src.getEntries().size() <= N);
            _names.emplace();
            auto& names = *_names;
            for (uint8_t i = 0; i < src.getEntries().size(); ++i)
            {
                char const* const name = src.getName(i);
                if (name != nullptr)
                {
                    names[i] = name;
                }
            }
        }

        _statisticsContainer.copyFrom(src);
    }

    typename StatisticsContainer::IteratorType getIterator()
    {
        return typename StatisticsContainer::IteratorType(
            GetNameType::
                template create<SharedStatisticsContainer, &SharedStatisticsContainer::getName>(
                    *this),
            _statisticsContainer.getEntries());
    }

    GetNameType getGetName() const { return _statisticsContainer.getGetName(); }

private:
    static constexpr uint8_t MAX_NAME_LENGTH = 16U;

    char const* getName(size_t const index) const
    {
        if ((!_names.has_value()) || (_names->size() < index) || (_names.value()[index].empty()))
        {
            return nullptr;
        }

        return _names.value()[index].c_str();
    }

    StatisticsContainer _statisticsContainer;

    ::etl::optional<::etl::array<::etl::string<MAX_NAME_LENGTH>, N>> _names;
};

} // namespace runtime

#endif /* GUARD_2A81615E_F0E2_47BD_AEAF_A8B88DA403D7 */
