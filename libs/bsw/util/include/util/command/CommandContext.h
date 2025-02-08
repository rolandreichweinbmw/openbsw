// Copyright 2024 Accenture.

#ifndef GUARD_9713AE4F_6FEE_4AE0_8DDA_473083B26D2B
#define GUARD_9713AE4F_6FEE_4AE0_8DDA_473083B26D2B

#include "util/command/ICommand.h"
#include "util/stream/ISharedOutputStream.h"
#include "util/stream/NullOutputStream.h"
#include "util/string/ConstString.h"

#include <etl/span.h>

#include <cstdint>

namespace util
{
namespace command
{
class CommandContext : public ::util::stream::ISharedOutputStream
{
public:
    template<class T>
    class IdentifierChecker
    {
    public:
        IdentifierChecker(CommandContext& context, ::util::string::ConstString const& identifier);

        IdentifierChecker& check(char const* identifier, T value);
        T getValue();

    private:
        CommandContext& _context;
        ::util::string::ConstString _identifier;
        bool _match;
        T _value;
    };

    explicit CommandContext(
        ::util::string::ConstString const& line,
        ::util::stream::ISharedOutputStream* sharedOutputStream = nullptr);

    bool hasToken() const;

    ::util::string::ConstString scanToken();
    ::util::string::ConstString scanIdentifierToken();
    ::etl::span<uint8_t> scanByteBufferToken(::etl::span<uint8_t> const& buf);
    template<class T>
    IdentifierChecker<T> scanEnumToken();
    template<class T>
    T scanIntToken();

    bool checkEol();
    bool check(bool condition, ICommand::Result result = ICommand::Result::BAD_VALUE);

    ICommand::Result getResult() const;
    ::util::string::ConstString getSuffix() const;

    ::util::stream::IOutputStream& startOutput(IContinuousUser* user) override;
    void endOutput(IContinuousUser* user) override;
    void releaseContinuousUser(IContinuousUser& user) override;

private:
    bool isValid() const;

    bool ignoreWhitespace();

    static bool isWhitespace(char c);
    static bool isIdentifierChar(char c, bool firstChar);
    static int32_t getDigit(char c, uint32_t base);

    ::util::stream::NullOutputStream _nullStream;
    ::util::stream::ISharedOutputStream* _sharedOutputStream;
    ::util::stream::IOutputStream* _activeStream;
    char const* _start;
    char const* _current;
    char const* _end;
    char const* _tokenStart;
    ICommand::Result _result;
};

/**
 * Implementation.
 */
template<class T>
CommandContext::IdentifierChecker<T> CommandContext::scanEnumToken()
{
    return IdentifierChecker<T>(*this, scanIdentifierToken());
}

template<class T>
T CommandContext::scanIntToken()
{
    T result          = static_cast<T>(0);
    bool negative     = false;
    char const* start = _current;
    if (isValid())
    {
        _tokenStart   = _current;
        uint32_t base = 10U;
        switch (*_current)
        {
            case '+':
            case '-':
            {
                negative = *_current == '-';
                ++_current;
                break;
            }
            case '0':
            {
                if (((_current + 1) < _end) && ((_current[1] == 'x') || (_current[1] == 'X')))
                {
                    base = 16U;
                    _current += 2;
                }
                else
                {
                    base = 8U;
                }
                break;
            }
            default:
            {
                break;
            }
        }
        start = _current;
        while (_current != _end)
        {
            int32_t const digit = getDigit(*_current, base);
            if (digit < 0)
            {
                break;
            }
            result = result * static_cast<T>(base) + static_cast<T>(digit);
            ++_current;
        }
    }
    bool checkCondition = (_current != start);
    if (checkCondition)
    {
        checkCondition = ignoreWhitespace();
    }
    if (check(checkCondition, ICommand::Result::BAD_TOKEN))
    {
        return negative ? -result : result;
    }

    return static_cast<T>(0);
}

template<class T>
CommandContext::IdentifierChecker<T>::IdentifierChecker(
    CommandContext& context, ::util::string::ConstString const& identifier)
: _context(context), _identifier(identifier), _match(false), _value()
{}

template<class T>
CommandContext::IdentifierChecker<T>&
CommandContext::IdentifierChecker<T>::check(char const* const identifier, T const value)
{
    if ((!_match) && (_identifier.compareIgnoreCase(::util::string::ConstString(identifier)) == 0))
    {
        _match = true;
        _value = value;
    }
    return *this;
}

template<class T>
T CommandContext::IdentifierChecker<T>::getValue()
{
    (void)_context.check(_match);
    return _value;
}

} /* namespace command */
} /* namespace util */

#endif /* GUARD_9713AE4F_6FEE_4AE0_8DDA_473083B26D2B */
