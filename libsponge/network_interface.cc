#include "network_interface.hh"

#include "arp_message.hh"
#include "ethernet_frame.hh"

#include <iostream>

// Dummy implementation of a network interface
// Translates from {IP datagram, next hop address} to link-layer frame, and from link-layer frame to IP datagram

// For Lab 5, please replace with a real implementation that passes the
// automated checks run by `make check_lab5`.

// You will need to add private members to the class declaration in `network_interface.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] ethernet_address Ethernet (what ARP calls "hardware") address of the interface
//! \param[in] ip_address IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface(const EthernetAddress &ethernet_address, const Address &ip_address)
    : _ethernet_address(ethernet_address), _ip_address(ip_address) {
    cerr << "DEBUG: Network interface has Ethernet address " << to_string(_ethernet_address) << " and IP address "
         << ip_address.ip() << "\n";
}

//! \param[in] dgram the IPv4 datagram to be sent
//! \param[in] next_hop the IP address of the interface to send it to (typically a router or default gateway, but may also be another host if directly connected to the same network as the destination)
//! (Note: the Address type can be converted to a uint32_t (raw 32-bit IP address) with the Address::ipv4_numeric() method.)
void NetworkInterface::send_datagram(const InternetDatagram &dgram, const Address &next_hop) {
    // convert IP address of next hop to raw 32-bit representation (used in ARP header)
    const uint32_t next_hop_ip = next_hop.ipv4_numeric();

    std::map<unit32_t, struct ArpEntry>::iterator it = arp_table.find(next_hop_ip);
    if (it != arp_table.end()){
        //目标以太网地址已经找到，直接发送
        EthernetHeader header;
        header.dst = (it->second).ethernet_address;
        header.src = _ethernet_address;
        header.type = EthernetHeader::TYPE_IPv4;
        
        //组装为以太网帧
        EthernetFrame frame;
        frame.header() = header;
        frame.payload() = dgram.serialize();
        _frames_out.push(frame);
    }else{
        //找不到目标以太网地址，先查询之前是否发送过ARP请求
        std::map<uint32_t, size_t>::iterator it2 = arp_request.find(next_hop_ip);
        if(it2 == arp_request.end()){
            //之前没有发送过ARP请求，发送ARP请求，构造ARP请求报文
            ARPMessage arp_message;
            arp_message.hardware_type = ARPMessage::TYPE_ETHERNET;
            arp_message.protocol_type = EthernetHeader::TYPE_IPv4;
            arp_message.hardware_address_size = sizeof(EthernetHeader::src);
            arp_message.protocol_address_size = sizeof(IPv4Header::src);
            arp_message.opcode = ARPMessage::OPCODE_REQUEST;
            arp_message.sender_ethernet_address = _ethernet_address;
            arp_message.sender_ip_address = _ip_address.ipv4_numeric();
            arp_message.target_ethernet_address = {};//未知
            arp_message.target_ip_address = next_hop_ip;

            //构造ARP请求头
            EthernetHeader header;
            header.dst = ETHERNET_BROADCAST;
            header.src = _ethernet_address;
            header.type = EthernetHeader::TYPE_ARP;
            
            //构造ARP请求帧
            EthernetFrame frame;
            frame.header() = header;
            frame.payload() = arp_message.serialize();
            
            //发送ARP请求
            _frames_out.push(frame);

            //将此请求加入请求表
            arp_request.insert(std::pair<uint32_t, size_t>(next_hop_ip, tick_counter));
        }

        //不管之前没有发送过ARP请求，都要将数据包加入缓存
        struct NetworkInterface::data_cache cur_datagram_cache = {dgram, next_hop};
        datagram_cache.push(cur_datagram_cache);
    }
}

//! \param[in] frame the incoming Ethernet frame
optional<InternetDatagram> NetworkInterface::recv_frame(const EthernetFrame &frame) {
    //广播地址或者目的地不是本地直接返回
    if()
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void NetworkInterface::tick(const size_t ms_since_last_tick) 
{ 
    tick_counter += ms_since_last_tick;

    //遍历arp表，删除过期的条目
    std::map<unit32_t, struct ArpEntry>::iterator it = arp_table.begin();
    for(; it != arp_table.end();){
        if((it->second).last_used + 30 * 1000 < tick_counter){//过期，删除
            it = arp_table.erase(it);
        }else{//未过期，继续遍历
            it++;
        }
    }

    //遍历arp请求表，重新发送过期的条目
    std::map<uint32_t, size_t>::iterator it2 = arp_request.begin();
    for(; it2 != arp_request.end();){
        if(it2->second + 5 * 1000 < tick_counter){
            //过期，重新发送
            ARPMessage arp_message;
            arp_message.hardware_type = ARPMessage::TYPE_ETHERNET;
            arp_message.protocol_type = EthernetHeader::TYPE_IPv4;
            arp_message.hardware_address_size = sizeof(EthernetHeader::src);
            arp_message.protocol_address_size = sizeof(IPv4Header::src);
            arp_message.opcode = ARPMessage::OPCODE_REQUEST;
            arp_message.sender_ethernet_address = _ethernet_address;
            arp_message.sender_ip_address = _ip_address.ipv4_numeric();
            arp_message.target_ethernet_address = {};//未知
            arp_message.target_ip_address = it2->first;

            //构造ARP请求头
            EthernetHeader header;
            header.dst = ETHERNET_BROADCAST;
            header.src = _ethernet_address;
            header.type = EthernetHeader::TYPE_ARP;
            
            //构造ARP请求帧
            EthernetFrame frame;
            frame.header() = header;
            frame.payload() = arp_message.serialize();
            
            //发送ARP请求
            _frames_out.push(frame);

            //更新请求表时间
            it2->second = tick_counter;
        }
    }
}
