#ifndef SERVER_DATA_BATCH_HPP
#define SERVER_DATA_BATCH_HPP
#include <cstdlib>

struct data_batch {
    data_batch() = default;
    static const size_t BUFFER_SIZE = 2048;
    char data[BUFFER_SIZE];
    ssize_t data_len;
};

#endif //SERVER_DATA_BATCH_HPP
