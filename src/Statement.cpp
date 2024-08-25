
#include "../include/Statement.h"

#include <cstring>
#include <iostream>

#include "../include/InputBuffer.h"
#include "../include/Macros.h"
#include "EnumClass.h"
#include "spdlog/spdlog.h"

std::map<StatementResult, std::function<void()>> Statement::statementWarnMap() {
    return {
        {StatementResult::PREPARE_SUCCESS, []() { spdlog::info("Statement parse successfully."); }},
        {StatementResult::PREPARE_SYNTAX_ERROR,
         []() { spdlog::error("Syntax error. Could not parse statement."); }},
        {StatementResult::PREPARE_STRING_TOO_LONG, []() { spdlog::error("String is too long."); }},
        {StatementResult::PREPARE_NEGATIVE_ID, []() { spdlog::error("ID must be positive."); }},
        {StatementResult::PREPARE_UNRECOGNIZED_STATEMENT,
         []() {
             //  spdlog::error("Unrecognized keyword at start of '{}'.", InputBuffer::buffer());
             spdlog::error("Unrecognized keyword");
         }},
    };
}

std::map<ExecuteResult, std::function<void()>> Statement::executeWarnMap() {
    return {
        {ExecuteResult::EXECUTE_SUCCESS, []() { spdlog::info("Executed."); }},
        {ExecuteResult::EXECUTE_DUPLICATE_KEY, []() { spdlog::error("Error: Duplicate key."); }},
        {ExecuteResult::EXECUTE_FULL_TABLE, []() { spdlog::error("Error: Table full."); }},
        {ExecuteResult::EXECUTE_UNKNOWN, []() { spdlog::error("Error: Unknown error."); }},

    };
}

StatementResult Statement::init(std::string &bufferString, StatementType type) {
    switch (type) {
        case StatementType::STATEMENT_INSERT:
            return initInsert(bufferString);
        case StatementType::STATEMENT_SELECT:
        case StatementType::STATEMENT_RANGED_SELECT:
            return initSelect(bufferString);
        case StatementType::STATEMENT_UPDATE:
            return initUpdate(bufferString);
        case StatementType::STATEMENT_DELETE:
            return initDelete(bufferString);
        default:
            return StatementResult::PREPARE_UNRECOGNIZED_STATEMENT;
    }
}

StatementResult Statement::initInsert(std::string &bufferString) {
    size_t pos = 0;
    size_t startPos = 0;
    std::string token = " ";

    pos = bufferString.find(token, startPos);
    std::string command = bufferString.substr(startPos, pos - startPos);
    spdlog::info("  command: {}", command);
    startPos = pos + token.length();

    pos = bufferString.find(token, startPos);
    std::string idStr = bufferString.substr(startPos, pos - startPos);
    spdlog::info("      idStr: {}", idStr);
    startPos = pos + token.length();

    pos = bufferString.find(token, startPos);
    std::string username = bufferString.substr(startPos, pos - startPos);
    spdlog::info("      username: {}", username);
    startPos = pos + token.length();

    std::string email = bufferString.substr(startPos);
    spdlog::info("      email: {}", email);
    if (idStr.empty() || username.empty() || email.empty()) {
        return StatementResult::PREPARE_SYNTAX_ERROR;
    }

    uint32_t id = std::stoi(idStr);
    if (id < 0) {
        return StatementResult::PREPARE_NEGATIVE_ID;
    }
    if (username.length() > COLUMN_USERNAME) {
        return StatementResult::PREPARE_STRING_TOO_LONG;
    }
    if (email.length() > COLUMN_EMAIL) {
        return StatementResult::PREPARE_STRING_TOO_LONG;
    }

    m_type = StatementType::STATEMENT_INSERT;
    m_row_to_insert.m_id = id;
    std::strncpy(m_row_to_insert.m_username, username.c_str(),
                 sizeof(m_row_to_insert.m_username) - 1);
    // Ensure null-termination
    m_row_to_insert.m_username[sizeof(m_row_to_insert.m_username) - 1] = '\0';

    std::strncpy(m_row_to_insert.m_email, email.c_str(), sizeof(m_row_to_insert.m_email) - 1);
    // Ensure null-termination
    m_row_to_insert.m_email[sizeof(m_row_to_insert.m_email) - 1] = '\0';
    return StatementResult::PREPARE_SUCCESS;
}

StatementResult Statement::initSelect(std::string &bufferString) {
    // judge if it is a ranged select
    // if()
    // m_type = StatementType::STATEMENT_RANGED_SELECT;
    m_type = StatementType::STATEMENT_SELECT;
    return StatementResult::PREPARE_SUCCESS;
}

StatementResult Statement::initUpdate(std::string &bufferString) {
    m_type = StatementType::STATEMENT_UPDATE;
    return StatementResult::PREPARE_SUCCESS;
}

StatementResult Statement::initDelete(std::string &bufferString) {
    m_type = StatementType::STATEMENT_DELETE;
    return StatementResult::PREPARE_SUCCESS;
}