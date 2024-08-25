#include <iostream>
#include <memory>

#include "include/EnumClass.h"
#include "include/InputBuffer.h"
#include "include/Statement.h"
#include "include/Table.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/spdlog.h"

int main(int argc, char *argv[]) {
    std::ofstream ofs("logs/logfile.log", std::ofstream::out | std::ofstream::trunc);
    ofs.close();

    auto file_logger = spdlog::basic_logger_mt("file_logger", "logs/logfile.log");
    spdlog::set_default_logger(file_logger);
    spdlog::set_level(spdlog::level::info);  // 设置日志级别

    std::string filename;
    std::string exportPath;
    if (argc == 1) {
        filename =
            "/home/marco/0_codeRepo/2_project/3_cppProject/4_tinytinyDB/demo/demo_0/build/"
            "demo_debug.db";
        exportPath =
            "/home/marco/0_codeRepo/2_project/3_cppProject/4_tinytinyDB/demo/demo_0/test/bin/"
            "export_debug.csv";
    } else {
        filename = argv[1];
        exportPath = argv[2];
    }

    spdlog::info("<filename>: {}, <exportPath>: {}", filename, exportPath);
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