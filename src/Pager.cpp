#include "../include/Pager.h"

#include <spdlog/spdlog.h>

#include <cassert>
#include <cerrno>
#include <cstring>
#include <iostream>

#include "../include/Macros.h"

Pager::Pager(const std::string& filename) {
    m_file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
    if (m_file.fail()) {
        spdlog::error("Failed to open file: {}.", filename);
        std::exit(EXIT_FAILURE);
    }

    m_file.seekg(0, std::ios::end);
    m_file_bytes_length = m_file.tellg();
    m_file.seekg(0, std::ios::beg);
    m_pages_size = m_file_bytes_length / PAGE_SIZE;
    if (m_file_bytes_length % PAGE_SIZE != 0) {
        spdlog::error("Db file is not a whole number of pages. Corrupt file.");
        std::exit(EXIT_FAILURE);
    }
    m_pages.resize(TABLE_MAX_PAGES, nullptr);
}

Pager::~Pager() {
    for (void* page : m_pages) {
        if (page) {
            free(page);
        }
    }

    m_file.close();
}

size_t Pager::pageSize() const { return m_pages_size; }

void* Pager::getPage(uint32_t page_num, bool needToReadFromFile /*= true*/) {
    if (m_pages.size() <= page_num) {
        spdlog::error("out of limitation: {}", std::strerror(errno));
        std::exit(EXIT_FAILURE);
    }

    if (needToReadFromFile && m_pages[page_num] == nullptr) {
        void* page = malloc(PAGE_SIZE);

        if (m_file_bytes_length > 0 && m_file_bytes_length >= page_num * PAGE_SIZE) {
            m_file.seekg(page_num * PAGE_SIZE, std::ios::beg);
            if (m_file.fail()) {
                spdlog::error("get page error seek");
                std::exit(EXIT_FAILURE);
            }

            m_file.read(static_cast<char*>(page), PAGE_SIZE);
            if (m_file.fail() && !m_file.eof()) {
                spdlog::error("get page error seek");
                std::exit(EXIT_FAILURE);
            }
        }

        m_pages[page_num] = page;
        if (page_num >= m_pages_size) {
            m_pages_size = page_num + 1;
        }
    }

    return m_pages[page_num];
}

void Pager::pager_flush(uint32_t page_num) {
    if (!m_pages[page_num]) {
        spdlog::error("flush error by empty page at: {},    size: {}", page_num, PAGE_SIZE);
        std::exit(EXIT_FAILURE);
    }

    // 将文件指针移动到正确的位置
    m_file.seekp(page_num * PAGE_SIZE, std::ios::beg);
    if (m_file.fail()) {
        spdlog::error("flush seek page at: {},      error: {}", page_num, std::strerror(errno));
        std::exit(EXIT_FAILURE);
    }

    // 写入数据到文件
    m_file.write(static_cast<char*>(m_pages[page_num]), PAGE_SIZE);
    if (m_file.fail()) {
        spdlog::error("flush write page at: {},      error: {}", page_num, std::strerror(errno));
        std::exit(EXIT_FAILURE);
    }
}
