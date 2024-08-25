#pragma once
#include <cstdint>

#include "Cursor.h"
#include "Pager.h"
#include "Statement.h"

class Table {
public:
    explicit Table(const std::string &filename);
    ~Table();
    void db_close() const;

    Table(const Table &) = default;
    Table &operator=(const Table &) = default;
    Table(Table &&) = default;
    Table &operator=(Table &&) = default;

    ExecuteResult execute_statement(Statement &stmt);
    ExecuteResult insert_row(Statement &stmt);
    ExecuteResult select_row(Statement &stmt);

    static NodeType get_node_type(void *node);
    static void set_node_type(void *node, NodeType type);
    static bool is_node_root(void *node);
    static void set_node_root(void *node, bool is_root);
    static uint32_t *node_parent_key(void *node);
    void *table_root_node() const;

    void leaf_node_insert(Cursor *cursor, uint32_t key, Row *value);
    static uint32_t *leaf_node_num_cells(void *node);
    static uint32_t *leaf_node_next_leaf(void *node);
    static void *leaf_node_cell(void *node, uint32_t cell_num);
    static uint32_t *leaf_node_key(void *node, uint32_t cell_num);
    static void *leaf_node_value(void *node, uint32_t cell_num);
    static void initialize_leaf_node(void *node);

    static uint32_t *internal_node_num_keys(void *node);
    static uint32_t *internal_node_right_child(void *node);
    static uint32_t *internal_node_cell(void *node, uint32_t cell_num);

    static uint32_t *internal_node_child(void *node, uint32_t child_num);
    static uint32_t *internal_node_key(void *node, uint32_t key_num);
    static void initialize_internal_node(void *node);

    static uint32_t get_node_max_key(void *node);
    uint32_t get_unused_page_num();

    void create_new_root(uint32_t right_child_page_num);
    void leaf_node_split_and_insert(Cursor *cursor, uint32_t key, Row *value);

    uint32_t internal_node_find_child(const uint32_t &rootPageKey, const uint32_t &targetLeafKey);
    void update_internal_node_key(const uint32_t &rootPageKey, uint32_t old_key, uint32_t new_key);
    void internal_node_insert(uint32_t parent_page_num, uint32_t child_page_num);

    static void serialize_row_insert(Row *source, void *destination);
    static void deserialize_row_select(void *source, Row *destination);

    // print
    void exportToCsvFile(const std::string &csvPath) const;
    void printBtree();
    static void print_leaf_node(void *node);

private:
    friend Cursor;
    void *getPage(uint32_t page_num) const;

private:
    Pager *m_pager;
    uint32_t m_table_root_key;  // 记录root页面坐标
};