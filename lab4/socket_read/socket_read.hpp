#ifndef SERVER_SOCKET_READ_HPP
#define SERVER_SOCKET_READ_HPP

#include <stdlib.h>
#include <iterator>
#include <type_traits>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <string>

enum class SOCK_READ_STATE {
    SRS_INIT,
    SRS_FIRST_ARG,
    SRS_SECOND_ARG,
    SRS_BODY,
    SRS_EOF
};

struct data_batch {
    static const size_t BUFFER_SIZE = 2048;
    char data[BUFFER_SIZE];
    ssize_t data_len;
};

class socket_iterator : public std::iterator<std::bidirectional_iterator_tag,
        typename std::pair<SOCK_READ_STATE, std::string_view>> {
public:
    socket_iterator() = delete;
    socket_iterator(int sock_fd);

    void load_new_data_batch();

    socket_iterator& operator++();

    socket_iterator operator++(int);

    std::pair<SOCK_READ_STATE, std::string_view> operator*();

    auto operator-> ();

    bool operator==(const socket_iterator& r) const;

    bool operator!=(const socket_iterator& r) const;

    socket_iterator(int sock_fd, SOCK_READ_STATE read_state);

private:
    size_t seek_char(char c) const;

private:
    SOCK_READ_STATE m_read_state;
    data_batch m_data_batch;
    std::pair<SOCK_READ_STATE, std::string_view> res;
    size_t m_data_pointer;
    int m_sock_fd;
};

class socket_iterator_range  {
public:
    explicit socket_iterator_range(int sock_fd);

    socket_iterator begin() const;  // NOLINT

    socket_iterator end() const;  // NOLINT

private:
    socket_iterator m_begin, m_end;
};

socket_iterator_range socket_read(int sock_fd);


#endif //SERVER_SOCKET_READ_HPP
