#include "../include/Table.h"

#include <spdlog/spdlog.h>

#include <cassert>
#include <cstdint>
#include <cstring>
#include <memory>
#include <stdexcept>

#include "../include/Macros.h"
#include "Cursor.h"
#include "Pager.h"

Table::Table(const std::string& filename)
    : m_pager(new Pager(filename)),
      m_table_root_key(0) {
    if (m_pager->pageSize() == 0) {
        void* root_node = m_pager->getPage(0);
        initialize_leaf_node(root_node);
        set_node_root(root_node, true);
    }
};

Table::~Table() {
    db_close();
    delete m_pager;
};

void Table::db_close() const {
    // 计算需要写入的整页面数
    for (int i = 0; i < m_pager->pageSize(); i++) {
        void* page = m_pager->getPage(i, false);
        if (page) {
            m_pager->pager_flush(i);
            page = nullptr;
        }
    }
};

ExecuteResult Table::execute_statement(Statement& stmt) {
    switch (stmt.m_type) {
        case StatementType::STATEMENT_INSERT:
            return insert_row(stmt);
        case StatementType::STATEMENT_SELECT:
            return select_row(stmt);
        default:
            return ExecuteResult::EXECUTE_UNKNOWN;
    }
};

ExecuteResult Table::insert_row(Statement& stmt) {
    const auto& key = stmt.m_row_to_insert.m_id;
    auto cursor = std::make_unique<Cursor>(this, key);

    void* root_node = table_root_node();
    assert(cursor->curCellIdx() < *leaf_node_num_cells(root_node) + 1);

    uint32_t key_at_index = *leaf_node_key(root_node, cursor->curCellIdx());
    if (key_at_index == key) {
        return ExecuteResult::EXECUTE_DUPLICATE_KEY;
    }

    leaf_node_insert(cursor.get(), key, &stmt.m_row_to_insert);
    return ExecuteResult::EXECUTE_SUCCESS;
};

ExecuteResult Table::select_row(Statement& /*stmt*/) {
    Row row{};
    auto cursor = std::make_unique<Cursor>(this, 0);
    while (!cursor->isEndOfTable()) {
        void* page = cursor->curPos();
        deserialize_row_select(page, &row);
        row.print_row();
        cursor->next();
    }
    return ExecuteResult::EXECUTE_SUCCESS;
};

NodeType Table::get_node_type(void* node) {
    auto value = *static_cast<uint8_t*>(static_cast<uint8_t*>(node) + NODE_TYPE_OFFSET);
    return static_cast<NodeType>(value);
}

void Table::set_node_type(void* node, NodeType type) {
    auto value = static_cast<uint8_t>(type);
    *static_cast<uint8_t*>(static_cast<uint8_t*>(node) + NODE_TYPE_OFFSET) = value;
}

bool Table::is_node_root(void* node) {
    auto value = *static_cast<uint8_t*>(static_cast<uint8_t*>(node) + IS_ROOT_OFFSET);
    return static_cast<bool>(value);
}

void Table::set_node_root(void* node, bool is_root) {
    auto value = static_cast<uint8_t>(is_root);
    *static_cast<uint8_t*>(static_cast<uint8_t*>(node) + IS_ROOT_OFFSET) = value;
}

uint32_t* Table::node_parent_key(void* node) {
    return reinterpret_cast<uint32_t*>(static_cast<uint8_t*>(node) + PARENT_POINTER_OFFSET);
}

void* Table::table_root_node() const { return m_pager->getPage(m_table_root_key); }

void Table::leaf_node_insert(Cursor* cursor, uint32_t key, Row* value) {
    void* node = cursor->curRootNode();
    uint32_t num_cells = *leaf_node_num_cells(node);

    if (num_cells >= LEAF_NODE_MAX_CELLS) {
        leaf_node_split_and_insert(cursor, key, value);
        return;
    }

    if (cursor->curCellIdx() < num_cells) {
        for (uint32_t i = num_cells; i > cursor->curCellIdx(); i--) {
            memcpy(leaf_node_cell(node, i), leaf_node_cell(node, i - 1), LEAF_NODE_CELL_SIZE);
        }
    }

    *leaf_node_num_cells(node) += 1;
    *leaf_node_key(node, cursor->curCellIdx()) = key;
    serialize_row_insert(value, leaf_node_value(node, cursor->curCellIdx()));
}

uint32_t* Table::leaf_node_num_cells(void* node) {
    return reinterpret_cast<uint32_t*>(reinterpret_cast<uint8_t*>(node) +
                                       LEAF_NODE_NUM_CELLS_OFFSET);
}

uint32_t* Table::leaf_node_next_leaf(void* node) {
    return reinterpret_cast<uint32_t*>(reinterpret_cast<uint8_t*>(node) +
                                       LEAF_NODE_NEXT_LEAF_OFFSET);
}

void* Table::leaf_node_cell(void* node, uint32_t cell_num) {
    auto* base = reinterpret_cast<uint8_t*>(node);
    return base + LEAF_NODE_HEADER_SIZE + cell_num * LEAF_NODE_CELL_SIZE;
}

uint32_t* Table::leaf_node_key(void* node, uint32_t cell_num) {
    return reinterpret_cast<uint32_t*>(leaf_node_cell(node, cell_num));
}

void* Table::leaf_node_value(void* node, uint32_t cell_num) {
    auto* cell_ptr = reinterpret_cast<uint8_t*>(leaf_node_cell(node, cell_num));
    return static_cast<void*>(cell_ptr + LEAF_NODE_KEY_SIZE);
}

void Table::initialize_leaf_node(void* node) {
    set_node_type(node, NodeType::LEAF);
    set_node_root(node, false);
    *leaf_node_num_cells(node) = 0;
    *leaf_node_next_leaf(node) = 0;
}

uint32_t* Table::internal_node_num_keys(void* node) {
    return reinterpret_cast<uint32_t*>(reinterpret_cast<uint8_t*>(node) +
                                       INTERNAL_NODE_NUM_KEYS_OFFSET);
}

uint32_t* Table::internal_node_right_child(void* node) {
    return reinterpret_cast<uint32_t*>(reinterpret_cast<uint8_t*>(node) +
                                       INTERNAL_NODE_RIGHT_CHILD_OFFSET);
}

uint32_t* Table::internal_node_cell(void* node, uint32_t cell_num) {
    return reinterpret_cast<uint32_t*>(reinterpret_cast<uint8_t*>(node) +
                                       INTERNAL_NODE_HEADER_SIZE +
                                       cell_num * INTERNAL_NODE_CELL_SIZE);
}

uint32_t* Table::internal_node_child(void* node, uint32_t child_num) {
    uint32_t num_keys = *internal_node_num_keys(node);
    if (child_num > num_keys) {
        spdlog::error("Tried to access child_num: {},    num_keys: {}", child_num, num_keys);
        throw std::runtime_error("An error occurred");
    }

    if (child_num == num_keys) {
        return internal_node_right_child(node);
    }
    return internal_node_cell(node, child_num);
}

uint32_t* Table::internal_node_key(void* node, uint32_t key_num) {
    return reinterpret_cast<uint32_t*>(
        reinterpret_cast<uint8_t*>(internal_node_cell(node, key_num)) + INTERNAL_NODE_CHILD_SIZE);
}

void Table::initialize_internal_node(void* node) {
    set_node_type(node, NodeType::INTERNAL);
    set_node_root(node, false);
    *internal_node_num_keys(node) = 0;
}

uint32_t Table::get_node_max_key(void* node) {
    switch (get_node_type(node)) {
        case NodeType::INTERNAL:
            return *internal_node_key(node, *internal_node_num_keys(node) - 1);
        case NodeType::LEAF:
            return *leaf_node_key(node, *leaf_node_num_cells(node) - 1);
        default:
            assert(false);
            return 0;
    }
}

uint32_t Table::get_unused_page_num() { return m_pager->pageSize(); }

void Table::create_new_root(uint32_t right_child_page_num) {
    uint32_t left_child_page_num = get_unused_page_num();
    void* root = m_pager->getPage(left_child_page_num);
    void* right_child = m_pager->getPage(right_child_page_num);

    void* left_child = m_pager->getPage(left_child_page_num);
    initialize_internal_node(left_child);
    set_node_root(left_child, true);

    *internal_node_num_keys(left_child) = 1;
    *internal_node_child(left_child, 0) = right_child_page_num;
    uint32_t right_child_max_key = get_node_max_key(right_child);
    *internal_node_key(left_child, 0) = right_child_max_key;

    set_node_root(root, false);
    set_node_type(root, NodeType::INTERNAL);
    *internal_node_num_keys(root) = 1;
    *internal_node_child(root, 0) = left_child_page_num;
    *internal_node_key(root, 0) = right_child_max_key;

    *node_parent_key(left_child) = m_table_root_key;
    *node_parent_key(right_child) = m_table_root_key;
}

void Table::leaf_node_split_and_insert(Cursor* cursor, uint32_t key, Row* value) {
    void* old_node = cursor->curRootNode();
    uint32_t old_node_old_max = get_node_max_key(old_node);
    uint32_t new_page_num = get_unused_page_num();
    void* new_node = m_pager->getPage(new_page_num);
    initialize_leaf_node(new_node);
    *node_parent_key(new_node) = *node_parent_key(old_node);
    *leaf_node_next_leaf(new_node) = *internal_node_right_child(old_node);
    *leaf_node_next_leaf(old_node) = new_page_num;

    for (int32_t i = LEAF_NODE_MAX_CELLS; i >= 0; i--) {
        void* dest_node = i >= LEAF_NODE_LEFT_SPLIT_COUNT ? new_node : old_node;
        uint32_t idx = i % LEAF_NODE_LEFT_SPLIT_COUNT;
        void* dest_cell = leaf_node_cell(dest_node, idx);

        if (i == cursor->curCellIdx()) {
            serialize_row_insert(value, dest_cell);
            *leaf_node_key(dest_node, idx) = key;
        } else if (i > cursor->curCellIdx()) {
            memcpy(dest_cell, leaf_node_cell(old_node, i - 1), LEAF_NODE_CELL_SIZE);
        } else {
            memcpy(dest_cell, leaf_node_cell(old_node, i), LEAF_NODE_CELL_SIZE);
        }
    }

    *leaf_node_num_cells(old_node) = LEAF_NODE_LEFT_SPLIT_COUNT;
    *leaf_node_num_cells(new_node) = LEAF_NODE_RIGHT_SPLIT_COUNT;

    if (is_node_root(old_node)) {
        return create_new_root(new_page_num);
    }
    uint32_t parent_page_num = *node_parent_key(old_node);
    uint32_t new_max = get_node_max_key(old_node);
    update_internal_node_key(parent_page_num, old_node_old_max, new_max);
    internal_node_insert(parent_page_num, new_page_num);
}

uint32_t Table::internal_node_find_child(const uint32_t& rootPageKey,
                                         const uint32_t& targetLeafKey) {
    void* rootNode = getPage(rootPageKey);
    uint32_t num_keys = *internal_node_num_keys(rootNode);

    uint32_t min = 0;
    uint32_t max = num_keys;
    while (max != min) {
        uint32_t mid = (min + max) / 2;
        uint32_t key_at_index = *internal_node_key(rootNode, mid);
        if (key_at_index >= targetLeafKey) {
            max = mid;  // m_cell_num = mid;
        } else {
            min = mid + 1;
        }
    }
    return min;
}

void Table::update_internal_node_key(const uint32_t& rootPageKey,
                                     uint32_t old_key,
                                     uint32_t new_key) {
    uint32_t old_child_idx = internal_node_find_child(rootPageKey, old_key);
    void* node = getPage(rootPageKey);
    *internal_node_key(node, old_child_idx) = new_key;
}

void Table::internal_node_insert(uint32_t parent_page_num, uint32_t child_page_num) {
    void* parent = getPage(parent_page_num);
    void* child = getPage(child_page_num);
    uint32_t child_max_key = get_node_max_key(child);
    uint32_t child_idx = internal_node_find_child(parent_page_num, child_max_key);

    uint32_t original_num_keys = *internal_node_num_keys(parent);
    *internal_node_num_keys(parent) = original_num_keys + 1;

    uint32_t right_child_page_num = *internal_node_right_child(parent);
    void* right_child = getPage(right_child_page_num);

    if (child_max_key > get_node_max_key(right_child)) {
        *internal_node_child(parent, original_num_keys) = right_child_page_num;
        *internal_node_key(parent, original_num_keys) = get_node_max_key(right_child);
        *internal_node_right_child(parent) = child_page_num;
    } else {
        for (uint32_t i = original_num_keys; i > child_idx; i--) {
            void* dest = internal_node_cell(parent, i);
            void* src = internal_node_cell(parent, i - 1);
            memcpy(dest, src, INTERNAL_NODE_CELL_SIZE);
        }
        *internal_node_child(parent, child_idx) = child_page_num;
        *internal_node_key(parent, child_idx) = child_max_key;
    }
}

void* Table::getPage(uint32_t page_num) const { return m_pager->getPage(page_num); }

void Table::serialize_row_insert(Row* source, void* destination) {
    memcpy(destination, &(source->m_id), ID_SIZE);
    memcpy(static_cast<char*>(destination) + USERNAME_OFFSET, source->m_username, USERNAME_SIZE);
    memcpy(static_cast<char*>(destination) + EMAIL_OFFSET, source->m_email, EMAIL_SIZE);
};

void Table::deserialize_row_select(void* source, Row* destination) {
    memcpy(&(destination->m_id), source, ID_SIZE);
    memcpy(destination->m_username, static_cast<char*>(source) + USERNAME_OFFSET, USERNAME_SIZE);
    memcpy(destination->m_email, static_cast<char*>(source) + EMAIL_OFFSET, EMAIL_SIZE);
};

void Table::print_leaf_node(void* node) {
    uint32_t num_cells = *leaf_node_num_cells(node);
    std::cout << "leaf (size " << num_cells << ")\n";
    for (int i = 0; i < num_cells; i++) {
        uint32_t key = *leaf_node_key(node, i);
        std::cout << "  - " << i << " : " << key << '\n';
    }
}

void Table::exportToCsvFile(const std::string& csvPath) const {
    std::ofstream csvFile(csvPath, std::ios::out | std::ios::trunc);
    if (!csvFile.is_open()) {
        throw std::runtime_error("无法打开CSV文件");
    }

    // 写入CSV文件头
    csvFile << "ID,Username,Email\n";

    Row row{};
    auto cursor = std::make_unique<Cursor>(this, 0);

    while (!cursor->isEndOfTable()) {
        void* page = cursor->curPos();
        deserialize_row_select(page, &row);
        csvFile << row.m_id << "," << row.m_username << "," << row.m_email << "\n";
        cursor->next();
    }
    csvFile.close();

    spdlog::info("导出CSV文件成功");
}

void Table::printBtree() {
    auto cursor = std::make_unique<Cursor>(this, 0);
    while (!cursor->isEndOfTable()) {
        void* page = cursor->curPos();
        print_leaf_node(page);
        cursor->next();
    }
};
