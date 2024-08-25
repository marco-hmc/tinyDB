#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>

namespace {

std::string readCmdFile(const std::string& fileName) {
    std::string dirPath =
        "/home/marco/0_codeRepo/2_project/3_cppProject/4_tinytinyDB/demo/demo_0/test/cmd/";
    std::filesystem::path filePath = std::filesystem::path(dirPath) / fileName;

    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("无法打开文件: " + filePath.string());
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string readCsvFile(const std::string& fileName) {
    std::string dirPath =
        "/home/marco/0_codeRepo/2_project/3_cppProject/4_tinytinyDB/demo/demo_0/test/bin/";
    std::filesystem::path filePath = std::filesystem::path(dirPath) / fileName;

    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("无法打开文件: " + filePath.string());
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}
}  // namespace

class Case1Test : public ::testing::Test {
protected:
    const std::string outputDir =
        "/home/marco/0_codeRepo/2_project/3_cppProject/4_tinytinyDB/demo/demo_0/test/bin/";
    std::string fileName = (std::filesystem::path(outputDir) / "test.db").string();
    std::string exportPath = (std::filesystem::path(outputDir) / "export_test.csv").string();
    std::string standardExportPath = "standard_export.csv";

    // 在每个测试前执行
    virtual void SetUp() {
        // 运行同级文件夹下的 Python 脚本
        int result = system(
            "python "
            "/home/marco/0_codeRepo/2_project/3_cppProject/4_tinytinyDB/demo/demo_0/test/testCase/"
            "testCase1/init.py");
        if (result != 0) {
            throw std::runtime_error("Failed to run Python script");
        }

        // 确保测试开始前，文件不存在
        std::remove(fileName.c_str());
        std::remove(exportPath.c_str());

        std::ofstream file(fileName, std::ios::out);
        if (!file) {
            throw std::system_error(errno, std::system_category(), "Failed to create " + fileName);
        }
        file.close();
    }

    // 在每个测试后执行
    virtual void TearDown() {
        // 测试完成后，删除文件
        std::remove(fileName.c_str());
        std::remove(exportPath.c_str());
    }

    // 执行测试命令
    void executeTestCommand(const std::string& command) {
        std::string fullCommand =
            "cat <<EOF | ../demo_0 " + fileName + " " + exportPath + "\n" + command + "\nEOF";
        int result = system(fullCommand.c_str());
        ASSERT_EQ(result, 0) << "Command failed: " << fullCommand;
    }

    // 读取文件内容
    std::string readFileContents() {
        std::ifstream file(fileName, std::ios::in | std::ios::binary);
        if (!file) {
            throw std::system_error(errno, std::system_category(), "Failed to open " + fileName);
        }

        std::string contents((std::istreambuf_iterator<char>(file)),
                             std::istreambuf_iterator<char>());
        return contents;
    }

    bool exportFileCompare() {
        std::string exportFile = readCsvFile(exportPath);
        std::string standardExportFile = readCsvFile(standardExportPath);
        return exportFile == standardExportFile;
    }
};

TEST_F(Case1Test, InsertCommand) {
    std::string insertCommands = readCmdFile("insertCmd");
    executeTestCommand(insertCommands);
}

TEST_F(Case1Test, SelectCommand) {
    std::string selectCommand = readCmdFile("selectCmd");
    executeTestCommand(selectCommand);
}

TEST_F(Case1Test, ExportCommand) {
    std::string exportCommand = readCmdFile("exportCmd");
    executeTestCommand(exportCommand);

    bool isSame = exportFileCompare();
    ASSERT_TRUE(isSame) << "Exported file is not the same as expected";
}