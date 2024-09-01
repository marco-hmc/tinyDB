#include <iostream>
#include <memory>

#include "include/EnumClass.h"
#include "include/Initializer.h"
#include "include/InputBuffer.h"
#include "include/Statement.h"
#include "include/Table.h"
#include "spdlog/spdlog.h"

int main(int argc, char *argv[]) {
    std::string filename;
    std::string exportPath;
    if (!initializeProgram(argc, argv, filename, exportPath)) {
        return 1;
    }

    auto table = std::make_unique<Table>(filename);
    bool isRun = true;
    while (isRun) {
        std::cout << "> ";
        auto inputBuffer = std::make_unique<InputBuffer>();
        inputBuffer->readLine();

        if (inputBuffer->buffer()[0] == '.') {
            switch (inputBuffer->parseMetaCommand()) {
                case MetaCommandResult::META_COMMAND_SUCCESS:
                    continue;
                case MetaCommandResult::META_COMMAND_EXIT:
                    table.reset();
                    isRun = false;
                    continue;
                case MetaCommandResult::META_COMMAND_BTREE_PRINT:
                    table->printBtree();
                    continue;
                case MetaCommandResult::META_COMMAND_CONST_EXPORT:
                    table->exportToCsvFile(exportPath);
                    continue;
                case MetaCommandResult::META_COMMAND_UNRECOGNIZED:
                    spdlog::error("Unrecognized command '{}'.", inputBuffer->buffer());
                    continue;
            }
        }

        Statement statement;
        switch (auto parseState = inputBuffer->parseStatement(statement)) {
            case StatementResult::PREPARE_SUCCESS:
                spdlog::info("  Statement parse successfully.");
                break;
            default:
                Statement::statementWarnMap()[parseState]();
                continue;
        }

        switch (auto executeState = table->execute_statement(statement)) {
            case ExecuteResult::EXECUTE_SUCCESS:
                spdlog::info("  Executed.");
                break;
            default:
                Statement::executeWarnMap()[executeState]();
                continue;
        }
    }

    return 0;
}