#include "socket_read.hpp"


socket_iterator::socket_iterator(int sock_fd) :
        m_read_state(SOCK_READ_STATE::SRS_INIT),
        m_sock_fd(sock_fd),
        m_data_pointer(0),
        res(SOCK_READ_STATE::SRS_INIT, std::string_view()) {
    m_data_batch.data_len = 0;
}

socket_iterator::socket_iterator(int sock_fd, SOCK_READ_STATE read_state) :
        m_sock_fd(sock_fd),
        m_data_pointer(0),
        m_read_state(read_state),
        res(read_state, std::string_view()) {
    m_data_batch.data_len = 0;
}

size_t socket_iterator::seek_char(char c) const {
    for (auto i = (ssize_t)m_data_pointer; i < m_data_batch.data_len; ++i) {
        if (m_data_batch.data[i] == c) {
            return i;
        }
    }
    return m_data_batch.data_len;
}

void socket_iterator::load_new_data_batch() {
    if (m_read_state == SOCK_READ_STATE::SRS_INIT) {
        fcntl(m_sock_fd, F_SETFL, (fcntl(m_sock_fd, F_GETFL, 0) & ~O_NONBLOCK));
    } else {
        fcntl(m_sock_fd, F_SETFL, (fcntl(m_sock_fd, F_GETFL, 0) | O_NONBLOCK));
    }
    m_data_batch.data_len = read(m_sock_fd, m_data_batch.data, data_batch::BUFFER_SIZE);
    m_data_pointer = 0;
}

socket_iterator& socket_iterator::operator++() {
    if (m_read_state == SOCK_READ_STATE::SRS_EOF) {
        res = std::make_pair(SOCK_READ_STATE::SRS_EOF, std::string_view(""));
        return *this;
    }

    if (m_data_pointer >= m_data_batch.data_len) {
        load_new_data_batch();
    }

    switch(m_read_state) {
        case SOCK_READ_STATE::SRS_INIT: {
            m_read_state = SOCK_READ_STATE::SRS_FIRST_ARG;
        }
        case SOCK_READ_STATE::SRS_FIRST_ARG: {
            size_t pos = seek_char(' ');
            res = std::make_pair(SOCK_READ_STATE::SRS_FIRST_ARG, std::string_view(m_data_batch.data + m_data_pointer, pos - m_data_pointer));
            m_data_pointer = pos;
            if (m_data_pointer < m_data_batch.data_len) {
                ++m_data_pointer;
                m_read_state = SOCK_READ_STATE::SRS_SECOND_ARG;
            }
            break;
        }
        case SOCK_READ_STATE::SRS_SECOND_ARG: {
            size_t pos = seek_char(' ');
            res = std::make_pair(SOCK_READ_STATE::SRS_SECOND_ARG, std::string_view(m_data_batch.data + m_data_pointer, pos - m_data_pointer));
            m_data_pointer = pos;
            if (m_data_pointer < m_data_batch.data_len) {
                size_t cnt = 0;
                std::string CRLF = "\r\n";

                while(true) {
                    for (; (ssize_t)m_data_pointer < m_data_batch.data_len; ++m_data_pointer) {
                        if (CRLF[cnt % 2] == m_data_batch.data[m_data_pointer]) {
                            ++cnt;
                        } else {
                            cnt = 0;
                        }
                        if (cnt == 4) {
                            ++m_data_pointer;
                            m_read_state = SOCK_READ_STATE::SRS_BODY;
                            return *this;
                        }
                    }
                    load_new_data_batch();
                }
            }
            break;
        }
        case SOCK_READ_STATE::SRS_BODY: {
            if (m_data_batch.data_len <= 0) {
                m_read_state = SOCK_READ_STATE::SRS_EOF;
            } else {
                res = std::make_pair(SOCK_READ_STATE::SRS_BODY, std::string_view(m_data_batch.data + m_data_pointer,
                                                m_data_batch.data_len - m_data_pointer));
                m_data_pointer = m_data_batch.data_len;
            }
            break;
        }
        default: {
            m_read_state = SOCK_READ_STATE::SRS_EOF;
            break;
        }
    }
    return *this;
}

socket_iterator socket_iterator::operator++(int) {
    socket_iterator old(*this);
    this->operator++();
    return old;
}

std::pair<SOCK_READ_STATE, std::string_view> socket_iterator::operator*() {
    return res;
}

auto socket_iterator::operator-> () {
    return (this->operator*());
}

bool socket_iterator::operator==(const socket_iterator& r) const {
    return m_read_state == r.m_read_state;
}

bool socket_iterator::operator!=(const socket_iterator& r) const {
    return m_read_state != r.m_read_state;
}


socket_iterator_range::socket_iterator_range(int sock_fd) :
    m_begin(socket_iterator(sock_fd, SOCK_READ_STATE::SRS_INIT)),
    m_end(socket_iterator(sock_fd, SOCK_READ_STATE::SRS_EOF)) {}

socket_iterator socket_iterator_range::begin() const {  // NOLINT
    return m_begin;
}

socket_iterator socket_iterator_range::end() const {  // NOLINT
    return m_end;
}


socket_iterator_range socket_read(int sock_fd) {
    return socket_iterator_range(sock_fd);
}