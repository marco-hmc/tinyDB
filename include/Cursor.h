#pragma once

#include <gtest/gtest.h>
#include <sys/types.h>

#include <cstdint>

class Table;

class Cursor {
public:
    // explicit Cursor(Table *table, bool isStartCursor = true);
    Cursor(const Table *table, uint32_t key);
    ~Cursor() = default;

    Cursor &operator=(const Cursor &) = delete;
    Cursor(const Cursor &) = delete;

    Cursor(Cursor &&) = delete;
    Cursor &operator=(Cursor &&) = delete;

    bool isEndOfTable() const;
    uint32_t curCellIdx() const;
    void *curRootNode() const;
    void *curPos() const;
    void next();

private:
    void leaf_node_find(void *rootNode, const uint32_t &targetLeafKey);
    void internal_node_find(void *rootNode, const uint32_t &targetLeafKey);
    static uint32_t internal_node_find_child(void *rootNode, const uint32_t &targetLeafKey);

private:
    const Table *const m_table;
    void *m_curRootNode;
    uint32_t m_leaf_idx;
    bool m_endOfTable;
};