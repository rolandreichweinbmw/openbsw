// Copyright 2024 Accenture.

#ifndef GUARD_2467B834_2D81_45A2_B2C9_6CE5ADF2D312
#define GUARD_2467B834_2D81_45A2_B2C9_6CE5ADF2D312

#include "util/command/CommandContext.h"
#include "util/command/ICommand.h"

#include <etl/delegate.h>

namespace util
{
namespace command
{
class SimpleCommand : public ICommand
{
public:
    using ExecuteFunction = ::etl::delegate<void(CommandContext& context)>;

    SimpleCommand(char const* id, char const* description, ExecuteFunction executeFunc);

    char const* getDescription() const;

    char const* getId() const override;
    ExecuteResult execute(
        ::util::string::ConstString const& arguments,
        ::util::stream::ISharedOutputStream* sharedOutputStream) override;
    void getHelp(IHelpCallback& callback) const override;

private:
    char const* _id;
    char const* _description;
    ExecuteFunction _execute;
};

} /* namespace command */
} /* namespace util */

#endif /* GUARD_2467B834_2D81_45A2_B2C9_6CE5ADF2D312 */
