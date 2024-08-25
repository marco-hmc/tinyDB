#pragma once

#include <cstdint>

const uint32_t COLUMN_USERNAME = 32;
const uint32_t COLUMN_EMAIL = 255;
//////////////////////////////////////

const uint32_t ID_OFFSET = 0;
const uint32_t ID_SIZE = sizeof(uint32_t);  // 4

const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;                 // 4
const uint32_t USERNAME_SIZE = sizeof(char) * (COLUMN_USERNAME + 1);  // 33

const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;  // 37
const uint32_t EMAIL_SIZE = sizeof(char) * (COLUMN_EMAIL + 1);  // 256

const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;  // 293

//////////////////////////////////////

const uint32_t PAGE_SIZE = 4096;
const uint32_t TABLE_MAX_PAGES = 100;                             // 模拟有100页
const uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;              // 13 = 4096/293
const uint32_t TABLE_MAX_ROWS = TABLE_MAX_PAGES * ROWS_PER_PAGE;  // 1300 = 100 * 13

//////////////////////////////////////

// 通用节点Header Layout
// | NodeType (1 byte) | IsRoot (1 byte) | ParentPointer (4 bytes) | = 6 bytes
const uint32_t NODE_TYPE_SIZE = sizeof(uint8_t);                       // size:     1
const uint32_t NODE_TYPE_OFFSET = 0;                                   // offset:   0
const uint32_t IS_ROOT_SIZE = sizeof(uint8_t);                         // size:     1
const uint32_t IS_ROOT_OFFSET = NODE_TYPE_SIZE;                        // offset:   1
const uint32_t PARENT_POINTER_SIZE = sizeof(uint32_t);                 // size:     4
const uint32_t PARENT_POINTER_OFFSET = IS_ROOT_OFFSET + IS_ROOT_SIZE;  // offset:   2
const uint8_t COMMON_NODE_HEADER_SIZE =
    NODE_TYPE_SIZE + IS_ROOT_SIZE + PARENT_POINTER_SIZE;  // size:  6

// 叶子节点的Header Layout
// | CommonNodeHeader (6 bytes) | NumCells (4 bytes) | NextLeaf (4 bytes) | = 14 bytes
const uint32_t LEAF_NODE_NUM_CELLS_SIZE = sizeof(uint32_t);                                 // 4
const uint32_t LEAF_NODE_NUM_CELLS_OFFSET = COMMON_NODE_HEADER_SIZE;                        // 6
const uint32_t LEAF_NODE_NEXT_LEAF_SIZE = sizeof(uint32_t);                                 // 4
const uint32_t LEAF_NODE_NEXT_LEAF_OFFSET =
    LEAF_NODE_NUM_CELLS_OFFSET + LEAF_NODE_NUM_CELLS_SIZE;  // 10
const uint32_t LEAF_NODE_HEADER_SIZE =
    COMMON_NODE_HEADER_SIZE + LEAF_NODE_NUM_CELLS_SIZE + LEAF_NODE_NEXT_LEAF_SIZE;  // 14

// 叶子节点Body Layout
// | Key (4 bytes) | Value (ROW_SIZE 293 bytes) |
const uint32_t LEAF_NODE_KEY_SIZE = sizeof(uint32_t);  // 4
const uint32_t LEAF_NODE_KEY_OFFSET = 0;               // 0

const uint32_t LEAF_NODE_VALUE_SIZE = ROW_SIZE;                                     // 293
const uint32_t LEAF_NODE_VALUE_OFFSET = LEAF_NODE_KEY_OFFSET + LEAF_NODE_KEY_SIZE;  // 4
const uint32_t LEAF_NODE_CELL_SIZE = LEAF_NODE_KEY_SIZE + LEAF_NODE_VALUE_SIZE;     // 297 = 4 + 293

// 内部节点Header Layout
// | CommonNodeHeader (6 bytes) | NumKeys (4 bytes) | RightChild (4 bytes) | = 14 bytes
const uint32_t INTERNAL_NODE_NUM_KEYS_SIZE = sizeof(uint32_t);           // 4
const uint32_t INTERNAL_NODE_NUM_KEYS_OFFSET = COMMON_NODE_HEADER_SIZE;  // 6
const uint32_t INTERNAL_NODE_RIGHT_CHILD_SIZE = sizeof(uint32_t);        // 4
const uint32_t INTERNAL_NODE_RIGHT_CHILD_OFFSET =
    INTERNAL_NODE_NUM_KEYS_OFFSET + INTERNAL_NODE_NUM_KEYS_SIZE;  // 10
const uint32_t INTERNAL_NODE_HEADER_SIZE =
    COMMON_NODE_HEADER_SIZE + INTERNAL_NODE_NUM_KEYS_SIZE + INTERNAL_NODE_RIGHT_CHILD_SIZE;  // 14

// 内部节点Body Layout
// | Child (4 bytes) | Key (4 bytes) | = 8 bytes
const uint32_t INTERNAL_NODE_CHILD_SIZE = sizeof(uint32_t);                                  // 4
const uint32_t INTERNAL_NODE_KEY_SIZE = sizeof(uint32_t);                                    // 4
const uint32_t INTERNAL_NODE_CELL_SIZE = INTERNAL_NODE_CHILD_SIZE + INTERNAL_NODE_KEY_SIZE;  // 8

// 叶子节点相关常量
const uint32_t LEAF_NODE_SPACE_FOR_CELLS = PAGE_SIZE - LEAF_NODE_HEADER_SIZE;  // 4086 = 4096 - 10
const uint32_t LEAF_NODE_MAX_CELLS =
    LEAF_NODE_SPACE_FOR_CELLS / LEAF_NODE_CELL_SIZE;  // 14 = 13.75 = (4096 - 10) / (4 + 293)
const uint32_t LEAF_NODE_RIGHT_SPLIT_COUNT = (LEAF_NODE_MAX_CELLS + 1) / 2;  // 叶子节点右分裂计数
const uint32_t LEAF_NODE_LEFT_SPLIT_COUNT =
    (LEAF_NODE_MAX_CELLS + 1) - LEAF_NODE_RIGHT_SPLIT_COUNT;  // 叶子节点左分裂计数

enum class NodeType {
    INTERNAL,
    LEAF,
};
/*
1. 缓存按整块读取大小4 kilobytes
   (极大多数系统架构的虚拟内存的page大小都为4kb)，如果每次都读整块，那读写效率是最大的
*/
