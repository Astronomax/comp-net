#include <iostream>
#include "buffered_channel.h"
#include <pthread.h>
#include <unordered_map>
#include <cstring>
#include <unistd.h>

#pragma pack(push, 1)
struct ipv4_header {
    int8_t version_IHL;
    int8_t TOS;
    int16_t total_length;
    int16_t identification;
    int16_t flags_frag_offset;
    int8_t ttl;
    int8_t protocol;
    int16_t checksum;
    uint32_t src_ip;
    uint32_t dst_ip;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct udp_header {
    int16_t src_port;
    int16_t dst_port;
    int16_t length;
    int16_t checksum;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct rip_header {
    int8_t command;
    int8_t version;
    int16_t routing_domain;
};
#pragma pack(pop)

struct rip_entry {
    int16_t addr_family;
    //16-bit padding
    uint32_t ip;
    uint32_t _[2];
    uint32_t metric;
};

const int INF = 16;

using ip_data = std::pair<void*, int>;

struct router_args {
    pthread_mutex_t *print_mutex;
    uint32_t addr;
    BufferedChannel<ip_data>* ch;
    std::unordered_map<uint32_t, BufferedChannel<ip_data>*> edges;
};

uint32_t parse_addr(std::string s) {
    uint32_t addr = 0;
    int shift = 32;
    for(int i = 0; i < 3; i++) {
        shift -= 8;
        auto it = s.find('.');
        int tmp = stoi(s.substr(0, it));
        addr += (tmp << shift);
        s.erase(s.begin(), s.begin() + it + 1);
    }
    addr += stoi(s);
    return addr;
}

std::string show_addr(uint32_t addr) {
    int mask = (1 << 8) - 1;
    uint8_t octet[4];
    for(int i = 0; i < 4; i++) {
        octet[3 - i] = addr & mask;
        addr >>= 8;
    }
    int len = snprintf(nullptr, 0, "%d.%d.%d.%d", octet[0], octet[1], octet[2], octet[3]);
    std::string pretty_addr(len, 0);
    sprintf(pretty_addr.data(), "%d.%d.%d.%d", octet[0], octet[1], octet[2], octet[3]);
    return pretty_addr;
}

void print_table(uint32_t addr, const std::unordered_map<uint32_t, std::pair<int, uint32_t>> &d) {
    printf("[Source IP]     [Destination IP]      [Next Hop]   [Metric]\n");
    for(auto &i : d) {
        printf("%-16s%-22s%-16s%5d\n",
               show_addr(addr).c_str(),
               show_addr(i.first).c_str(),
               show_addr(i.second.second).c_str(),
               i.second.first);
    }
    printf("\n");
}

void* router_routine(void* _) {
    auto args = reinterpret_cast<router_args*>(_);
    std::unordered_map<uint32_t, std::pair<int, uint32_t>> d;
    for (auto &u : args->edges)
        d[u.first] = {1, u.first};
    d[args->addr] = {0, args->addr};

    auto get = [&](uint32_t v) {
        if(d.find(v) == d.end()) return d[v] = {INF, 0};
        return d[v];
    };
    auto broadcast = [&](ip_data data) {
        for(auto &u : args->edges) {
            u.second->Send(data);
        }
    };
    auto send = [&](uint32_t dst, ip_data data) {
        args->edges[dst]->Send(data);
    };
    auto recv = [&]() {
        return args->ch->Recv();
    };

    int step = 0;

    for(;;) {
        char *data, *beg, *ptr;
        int len = sizeof(ipv4_header) + sizeof(udp_header) + sizeof(rip_header);
        ptr = data = static_cast<char *>(malloc(len));
        //std::cout << "after malloc" << std::endl;
        std::fill(data, data + len, 0);
        reinterpret_cast<ipv4_header*>(ptr)->src_ip = args->addr;
        ptr += sizeof(ipv4_header) + sizeof(udp_header);
        reinterpret_cast<rip_header*>(ptr)->command = 1;  //request
        reinterpret_cast<rip_header*>(ptr)->version = 1;
        broadcast({data, len});

        ++step;

        for(int p = 0; p < args->edges.size(); p++) {
            auto boxed_value = recv();
            if (!boxed_value.has_value()) {
                return new std::unordered_map<uint32_t, std::pair<int, uint32_t>>(d);
            }
            auto ip_data = boxed_value.value();

            ptr = beg = reinterpret_cast<char *>(ip_data.first);
            uint32_t src_addr = reinterpret_cast<ipv4_header *>(ptr)->src_ip;
            ptr += sizeof(ipv4_header) + sizeof(udp_header);
            int8_t cmd = reinterpret_cast<rip_header *>(ptr)->command;
            ptr += sizeof(rip_header);

            switch (cmd) {
                case 1: { //request
                    data = static_cast<char *>(malloc(532));
                    std::fill(data, data + 532, 0);
                    ptr = beg = reinterpret_cast<char *>(data);
                    reinterpret_cast<ipv4_header *>(ptr)->src_ip = args->addr;
                    ptr += sizeof(ipv4_header) + sizeof(udp_header);
                    reinterpret_cast<rip_header *>(ptr)->command = 2; //response
                    reinterpret_cast<rip_header *>(ptr)->version = 1;
                    ptr += sizeof(rip_header);

                    for (auto &u : d) {
                        reinterpret_cast<rip_entry *>(ptr)->ip = u.first;
                        reinterpret_cast<rip_entry *>(ptr)->metric = u.second.first;
                        ptr += sizeof(rip_entry);
                    }
                    send(src_addr, {data, (int) (ptr - beg)});
                    break;
                }
                case 2: { //response
                    for (; ptr < beg + ip_data.second; ptr += sizeof(rip_entry)) {
                        uint32_t dst_addr = reinterpret_cast<rip_entry *>(ptr)->ip;
                        uint32_t metric = reinterpret_cast<rip_entry *>(ptr)->metric;
                        if (get(src_addr).first + metric < get(dst_addr).first)
                            d[dst_addr] = {get(src_addr).first + metric, src_addr};
                    }
                    break;
                }
                default:
                    break;
            }
        }

        pthread_mutex_lock(args->print_mutex);
        printf("Simulation step %d of router %s table:\n", step, show_addr(args->addr).c_str());
        print_table(args->addr, d);
        fflush(stdout);
        pthread_mutex_unlock(args->print_mutex);

        //free(ip_data.first);
        usleep(500000);
    }
}

int main() {
    //std::ios::sync_with_stdio(false);
    //std::cin.tie(0);
#ifndef ONLINE_JUDGE
    freopen("input.txt", "r", stdin);
#endif
    int n; std::cin >> n;
    std::unordered_map<uint32_t, int> inds;
    std::vector<std::pair<pthread_t, router_args>> args(n);

    pthread_mutex_t print_mutex;
    pthread_mutex_init(&print_mutex, nullptr);

    for(int i = 0; i < n; i++) {
        std::string addr_;
        std::cin >> addr_;
        uint32_t addr = parse_addr(addr_);
        inds[addr] = i;
        args[i].first = addr;
        args[i].second.print_mutex = &print_mutex;
        args[i].second.addr = addr;
        args[i].second.ch = new BufferedChannel<ip_data>(1e9);
    }
    int m; std::cin >> m;
    for(int i = 0; i < m; i++) {
        std::string addra_, addrb_;
        std::cin >> addra_ >> addrb_;
        uint32_t addra = parse_addr(addra_);
        uint32_t addrb = parse_addr(addrb_);
        args[inds[addra]].second.edges[addrb] = args[inds[addrb]].second.ch;
        args[inds[addrb]].second.edges[addra] = args[inds[addra]].second.ch;
    }

    for(auto &arg : args) {
        pthread_create(&arg.first, nullptr, router_routine, &arg.second);
    }
    usleep(5000000); //work 5 seconds
    for(auto &arg : args) {
        arg.second.ch->Close();
        std::unordered_map<uint32_t, std::pair<int, uint32_t>>* d;
        pthread_join(arg.first, (void**)(&d));
        pthread_mutex_lock(&print_mutex);
        printf("Final state of router %s table:\n", show_addr(arg.second.addr).c_str());
        print_table(arg.second.addr, *d);
        fflush(stdout);
        pthread_mutex_unlock(&print_mutex);
        delete d;
    }
    pthread_mutex_destroy(&print_mutex);
    return 0;
}
