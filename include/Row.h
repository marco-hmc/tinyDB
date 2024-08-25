#pragma once

#include "Macros.h"

class Row {
public:
    void print_row();

    uint32_t m_id;
    char m_username[COLUMN_USERNAME + 1];
    char m_email[COLUMN_EMAIL + 1];
};