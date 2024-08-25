#pragma once

#include <string>

#include "EnumClass.h"

class Statement;
class InputBuffer {
public:
    InputBuffer() = default;
    ~InputBuffer() = default;

    InputBuffer(const InputBuffer &) = default;
    InputBuffer &operator=(const InputBuffer &) = default;
    InputBuffer(InputBuffer &&) = default;
    InputBuffer &operator=(InputBuffer &&) = default;

    // Member function
    void readLine();
    std::string buffer() const;
    [[nodiscard]] MetaCommandResult parseMetaCommand() const;
    StatementResult parseStatement(Statement &statement);

    StatementResult getStatementType(StatementType &statementType) const;

private:
    std::string m_buffer;
};
