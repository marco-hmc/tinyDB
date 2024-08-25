#pragma once

#include <functional>
#include <map>
#include <string>

#include "EnumClass.h"
#include "Row.h"

class InputBuffer;

// 执行结果状态码
enum class ExecuteResult {
    EXECUTE_SUCCESS,
    EXECUTE_DUPLICATE_KEY,
    EXECUTE_FULL_TABLE,
    EXECUTE_UNKNOWN,
};

class Statement {
public:
    Statement() = default;

    static std::map<StatementResult, std::function<void()>> statementWarnMap();
    static std::map<ExecuteResult, std::function<void()>> executeWarnMap();

    StatementResult init(std::string &bufferString, StatementType type);
    // [[nodiscard]] StatementType type() const;

    StatementResult initInsert(std::string &bufferString);
    StatementResult initSelect(std::string &bufferString);
    StatementResult initUpdate(std::string &bufferString);
    StatementResult initDelete(std::string &bufferString);

public:
    StatementType m_type{StatementType::STATEMENT_UNKNOWN};
    Row m_row_to_insert;
};