// Copyright 2024 Accenture.

#ifndef GUARD_76CA5B26_B5A0_4D06_8D0B_4C528493A626
#define GUARD_76CA5B26_B5A0_4D06_8D0B_4C528493A626

#include <etl/delegate.h>

#include <cstdint>

namespace nvstorage
{
/** Block ID datatype */
using NvBlockIdType = uint8_t;
using NvAppIdType   = uint8_t;

namespace consts
{
constexpr NvBlockIdType INVALID_NV_BLOCK_ID = 0xffU;
} // namespace consts

/** Operation kind */
enum NvStorageOperation
{
    NVSTORAGE_READ,
    NVSTORAGE_WRITE,
    NVSTORAGE_READ_DEFAULTS,
    NVSTORAGE_INVALIDATE,
    NVSTORAGE_RADDR,
    NVSTORAGE_SWRITE,
    NVSTORAGE_READ_ALL,
    NVSTORAGE_WRITE_ALL,
    NVSTORAGE_NONE
};

/** Internal return code for validity info */
enum NvValidity
{
    NV_VALIDITY_OK,
    NV_VALIDITY_INVALID,
    NV_VALIDITY_READ_FAILED
};

/** Job priority */
enum NvPriority
{
    NVSTORAGE_PRIORITY_HIGH,
    NVSTORAGE_PRIORITY_MEDIUM,
    NVSTORAGE_PRIORITY_LOW
};

/** Return code */
enum NvStorageReturnCode
{
    NVSTORAGE_REQ_OK,
    NVSTORAGE_REQ_NOT_OK,
    NVSTORAGE_REQ_PENDING,
    NVSTORAGE_REQ_INTEGRITY_FAILED,
    NVSTORAGE_REQ_RESTORED_FROM_ROM,
    NVSTORAGE_REQ_DRIVER_ERROR,
    NVSTORAGE_REQ_NO_DRIVER,
    NVSTORAGE_REQ_INVALID_PARAMETER,
    NVSTORAGE_REQ_INVALIDATED,
    NVSTORAGE_REQ_DECRYPTION_FAILED,
    NVSTORAGE_REQ_ENCRYPTION_FAILED,
    NVSTORAGE_REQ_NOT_INITIALIZED,
    NVSTORAGE_REQ_NO_MILEAGE,
    NVSTORAGE_REQ_NV_BUSY
};

using applJobFinishedCallback = ::etl::delegate<void(
    NvStorageOperation const, NvStorageReturnCode const, NvBlockIdType const)>;
using applInitBlockCallback = ::etl::delegate<void(NvStorageOperation const, NvBlockIdType const)>;
} /* namespace nvstorage */
#endif // GUARD_76CA5B26_B5A0_4D06_8D0B_4C528493A626
