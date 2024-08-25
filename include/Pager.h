#pragma once

#include <sys/types.h>

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <vector>

class Pager {
public:
    explicit Pager(const std::string &filename);
    ~Pager();

    Pager(const Pager &) = delete;
    Pager &operator=(const Pager &) = delete;
    Pager(Pager &&) = delete;
    Pager &operator=(Pager &&) = delete;

    // Member function
    size_t pageSize() const;
    void *getPage(uint32_t page_num, bool needToReadFromFile = true);
    void pager_flush(uint32_t page_num);

private:
    std::vector<void *> m_pages;  // 存储指向页面数据的指针的向量，每个元素对应一个页面
    std::fstream m_file;          // 用于文件操作的文件流对象
    size_t m_file_bytes_length;   // 文件的总长度（以字节为单位）
    size_t m_pages_size;          // 文件中的页面总数
};