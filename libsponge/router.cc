#include "router.hh"

#include <iostream>

using namespace std;

// Dummy implementation of an IP router

// Given an incoming Internet datagram, the router decides
// (1) which interface to send it out on, and
// (2) what next hop address to send it to.

// For Lab 6, please replace with a real implementation that passes the
// automated checks run by `make check_lab6`.

// You will need to add private members to the class declaration in `router.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

//! \param[in] route_prefix The "up-to-32-bit" IPv4 address prefix to match the datagram's destination address against
//! \param[in] prefix_length For this route to be applicable, how many high-order (most-significant) bits of the route_prefix will need to match the corresponding bits of the datagram's destination address?
//! \param[in] next_hop The IP address of the next hop. Will be empty if the network is directly attached to the router (in which case, the next hop address should be the datagram's final destination).
//! \param[in] interface_num The index of the interface to send the datagram out on.
void Router::add_route(const uint32_t route_prefix,
                       const uint8_t prefix_length,
                       const optional<Address> next_hop,
                       const size_t interface_num) {
    struct RouterEntry entry;
    entry.route_prefix = route_prefix;
    entry.prefix_length = prefix_length;
    entry.next_hop = next_hop;
    entry.interface_num = interface_num;
    router_table.push_back(entry);
}

//! \param[in] dgram The datagram to be routed
void Router::route_one_datagram(InternetDatagram &dgram) {
    int item = -1;
    int max_prefix_length = -1;
    //遍历路由表
    for(size_t i = 0; i < router_table.size(); i++){
        if(match(router_table[i].route_prefix, dgram.header().dst, router_table[i].prefix_length)){
            if(router_table[i].prefix_length > max_prefix_length){
                max_prefix_length = router_table[i].prefix_length;
                item = i;
            }
        }
    }
    //没有找到
    if(item == -1) return;
    //找到了
    //路由器减少数据包的ttl，若ttl在减少后为0，则丢弃该数据包
    //注意这里需要先判断！！再减去
    if(dgram.header().ttl <= 1) return;
    dgram.header().ttl--;
    //修改数据包的源地址和目的地址
    if(router_table[item].next_hop.has_value()){
        //下一跳地址不为空
        _interfaces[router_table[item].interface_num].send_datagram(dgram, router_table[item].next_hop.value());
    }else{
        //下一跳地址为空，说明IP和路由器直接相连
        _interfaces[router_table[item].interface_num].send_datagram(dgram, Address::from_ipv4_numeric(dgram.header().dst));
    }
    return;
}

void Router::route() {
    // Go through all the interfaces, and route every incoming datagram to its proper outgoing interface.
    for (auto &interface : _interfaces) {
        auto &queue = interface.datagrams_out();
        while (not queue.empty()) {
            route_one_datagram(queue.front());
            queue.pop();
        }
    }
}

bool Router::match(uint32_t cur_ip, uint32_t target_ip, uint8_t prefix_length){
    if(!prefix_length) return true;
    //获取掩码
    unit32_t the_mask;
    if(prefix_length == 0) the_mask = 0;
    else the_mask = ~((1 << (32 - prefix_length)) - 1);

    //获取前缀
    uint32_t the_prefix = cur_ip & the_mask;
    uint32_t the_target_prefix = target_ip & the_mask;

    //比较
    return the_prefix == the_target_prefix;
}
