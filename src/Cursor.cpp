#include "../include/Cursor.h"

#include <sys/types.h>

#include <cstdint>

#include "Table.h"

Cursor::Cursor(const Table* table, uint32_t targetLeafKey)
    : m_table(table) {
    void* rootNode = table->table_root_node();
    auto rootNodeType = Table::get_node_type(rootNode);

    if (rootNodeType == NodeType::LEAF) {
        leaf_node_find(rootNode, targetLeafKey);
    } else if (rootNodeType == NodeType::INTERNAL) {
        internal_node_find(rootNode, targetLeafKey);
    }

    m_endOfTable = m_leaf_idx >= *Table::leaf_node_num_cells(m_curRootNode);
}

void Cursor::leaf_node_find(void* rootNode, const uint32_t& targetLeafKey) {
    m_curRootNode = rootNode;
    uint32_t num_cells = *Table::leaf_node_num_cells(rootNode);

    uint32_t min = 0;
    uint32_t max = num_cells;
    while (max != min) {
        uint32_t mid = (min + max) / 2;
        uint32_t key_at_index = *Table::leaf_node_key(rootNode, mid);
        if (key_at_index == targetLeafKey) {
            min = mid;  // m_cell_num = mid;
            break;
        }

        if (targetLeafKey < key_at_index) {
            max = mid;
        } else {
            min = mid + 1;
        }
    }
    m_leaf_idx = min;
}

void Cursor::internal_node_find(void* rootNode, const uint32_t& targetLeafKey) {
    uint32_t child_index = internal_node_find_child(rootNode, targetLeafKey);
    uint32_t childPageKey = *Table::internal_node_child(rootNode, child_index);

    void* childRootNode = m_table->getPage(childPageKey);
    switch (Table::get_node_type(childRootNode)) {
        case NodeType::INTERNAL:
            internal_node_find(childRootNode, targetLeafKey);
            break;
        case NodeType::LEAF:
            leaf_node_find(childRootNode, targetLeafKey);
            break;
    }
}

uint32_t Cursor::internal_node_find_child(void* rootNode, const uint32_t& targetLeafKey) {
    uint32_t num_keys = *Table::internal_node_num_keys(rootNode);

    uint32_t min = 0;
    uint32_t max = num_keys;
    while (max != min) {
        uint32_t mid = (min + max) / 2;
        uint32_t key_at_index = *Table::internal_node_key(rootNode, mid);
        if (key_at_index >= targetLeafKey) {
            max = mid;  // m_cell_num = mid;
        } else {
            min = mid + 1;
        }
    }
    return min;
}

uint32_t Cursor::curCellIdx() const { return m_leaf_idx; }

void* Cursor::curRootNode() const { return m_curRootNode; }

bool Cursor::isEndOfTable() const { return m_endOfTable; }

void* Cursor::curPos() const { return Table::leaf_node_value(m_curRootNode, m_leaf_idx); };

void Cursor::next() {
    m_leaf_idx++;
    if (m_leaf_idx >= *(Table::leaf_node_num_cells(m_curRootNode))) {
        uint32_t next_page_num = *Table::leaf_node_next_leaf(m_curRootNode);
        if (next_page_num == 0) {
            m_endOfTable = true;
        } else {
            m_curRootNode = m_table->getPage(next_page_num);
            m_leaf_idx = 0;
        }
    }
}
