#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    TCPHeader header = seg.header();
    if(!_set_syn_flag){
        // 判断是否为listen状态，如果是，也就是在等待syn包
        if(!header.syn){
            // 不是syn包，直接返回
            return;
        }else{
            // 是syn包，设置isn
            isn = header.seqno;
            _set_syn_flag = true;
        }
    }
    if(_reassembler.stream_out().input_ended()){
        // 若已经停止写入，则丢弃
        return;
    }
    
    // 计算abs_seqno
    WrappingInt32 seqno = header.syn ? header.seqno + 1 : header.seqno; // 第一个有效字符的序列号
    uint64_t check_point = _reassembler.stream_out().bytes_written();
    uint64_t abs_seqno = unwrap(seqno, isn, check_point); // 绝对序列号
    // 将片段插入到reassembler中，注意绝对序列号需要减去一才能成为流序列号
    _reassembler.push_substring(seg.payload().copy(), abs_seqno - 1, header.fin);
    return;
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    // 返回接收方尚未获取到的第一个字节的字节索引
    uint64_t bytes_written = _reassembler.stream_out().bytes_written();
    if(!_set_syn_flag) return {}; // 当前为Listen状态，直接返回空
    else if(!_reassembler.stream_out().input_ended()) return {wrap(bytes_written + 1, isn)}; // 当前处于SYN_RECV状态，只需要加上SYN标志长度
    else return {wrap(bytes_written + 2, isn)}; // 如果当前处于FIN_RECV状态，还需要加上FIN标志长度
}

size_t TCPReceiver::window_size() const { 
    // 返回接收窗口的大小，也就是第一个未组装的字节索引和第一个不可接受的字节索引之间的长度
    return stream_out().remaining_capacity(); 
}
