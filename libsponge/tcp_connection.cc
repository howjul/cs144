#include "tcp_connection.hh"

#include <iostream>

// Dummy implementation of a TCP connection

// For Lab 4, please replace with a real implementation that passes the
// automated checks run by `make check`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

size_t TCPConnection::remaining_outbound_capacity() const { return _sender.stream_in().remaining_capacity(); }

size_t TCPConnection::bytes_in_flight() const { return _sender.bytes_in_flight(); }

size_t TCPConnection::unassembled_bytes() const { return _receiver.unassembled_bytes(); }

size_t TCPConnection::time_since_last_segment_received() const { return _time_since_last_segment_received; }

bool TCPConnection::active() const { return _is_active; }

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) { 
   _time_since_last_segment_received += ms_since_last_tick;
   //告诉TCPSender时间的流逝
   _sender.tick(ms_since_last_tick);
   //如果连续重传次数超过上限，则终止连接
    if(_sender.consecutive_retransmissions() > TCPConfig::MAX_RETX_ATTEMPTS){
        while(!_sender.segments_out().empty()){
            _sender.segments_out().pop();
        }
        _sender.stream_in().set_error();
        _receiver.stream_out().set_error();
        _is_active = false;
        _linger_after_streams_finish = false;
        //发送rst报文
        TCPSegment seg;
        seg.header().rst = true;
        _segments_out.push(seg);
        return;
    }
    send_segment();
   //当处于TCP的TIME_WAIT状态时，结束连接，也就是对象被释放，但是TCP仍然处于连接状态
    if (TCPState::state_summary(_receiver) == TCPReceiverStateSummary::FIN_RECV &&
        TCPState::state_summary(_sender) == TCPSenderStateSummary::FIN_ACKED &&
        _linger_after_streams_finish && _time_since_last_segment_received >= 10 * _cfg.rt_timeout) {
        _linger_after_streams_finish = false;
        _is_active = false;
    }
}


void TCPConnection::segment_received(const TCPSegment &seg) { 
    //将离上一次收到segment的时间清零
    _time_since_last_segment_received = 0;

    //是否需要发送空包回复 ACK，比如没有数据的时候收到 SYN/ACK 也要回一个 ACK
    bool need_send_ack = seg.length_in_sequence_space();
    
    //如果设置了rst标志
    if(seg.header().rst){
        _sender.stream_in().set_error();
        _receiver.stream_out().set_error();
        _is_active = false;
        _linger_after_streams_finish = false;
        return;
    }

    //把段交给TCPReceiver
    _receiver.segment_received(seg);

    if(seg.header().ack){
        //ack_received的最后会调用fill_window来发送报文
        _sender.ack_received(seg.header().ackno, seg.header().win);
        //如果队列中已经有数据报了就不需要专门的空包回复ACK了
        if(!_sender.segments_out().empty())
            need_send_ack = false;
    }

    //状态变化(按照个人的情况可进行修改)
    // 如果是 LISEN 到了 SYN
    if (TCPState::state_summary(_receiver) == TCPReceiverStateSummary::SYN_RECV &&
        TCPState::state_summary(_sender) == TCPSenderStateSummary::CLOSED) {
        // 此时肯定是第一次调用 fill_window，因此会发送 SYN + ACK
        connect();
        return;
    }

    // 判断 TCP 断开连接时是否时需要等待
    // CLOSE_WAIT
    if (TCPState::state_summary(_receiver) == TCPReceiverStateSummary::FIN_RECV &&
        TCPState::state_summary(_sender) == TCPSenderStateSummary::SYN_ACKED){
            _linger_after_streams_finish = false;
    }
        
    // 如果收到的数据包里没有任何数据，则这个数据包可能只是为了 keep-alive
    if (need_send_ack){
        _sender.send_empty_segment();
    }
    send_segment();

    // 如果到了准备断开连接的时候。服务器端先断
    // CLOSED
    if (TCPState::state_summary(_receiver) == TCPReceiverStateSummary::FIN_RECV &&
        TCPState::state_summary(_sender) == TCPSenderStateSummary::FIN_ACKED && !_linger_after_streams_finish) {
        _is_active = false;
        return;
    }

    
}

//写入相应的字节流并发送
size_t TCPConnection::write(const string &data) {
    size_t written_datalen = _sender.stream_in().write(data);
    _sender.fill_window();
    send_segment();
    return written_datalen;
}

//主动关闭，当上层没有数据要发送，将会调用此函数结束输入
//其实就是特殊的written，写入一个fin
void TCPConnection::end_input_stream() {
    _sender.stream_in().end_input();
    _sender.fill_window(); //写入的fin的条件其中之一_stream.eof()，_stream.eof()的条件为end_input标志为1且缓冲区无数据
    send_segment();
}

//主动连接，发送syn报文请求与服务端建立连接
//其实就是特殊的written，写入一个syn
void TCPConnection::connect() {
    _sender.fill_window();//因为这是第一次调用，所以会发送syn报文，并不携带数据
    send_segment();
}

//将segments从_sender的队列中取出，放到TCPConnection的队列中
void TCPConnection::send_segment(){
    std::queue<TCPSegment> &src_segment = _sender.segments_out();
    while(!src_segment.empty()){
        //将segments从_sender的队列中取出
        TCPSegment cur_segment = src_segment.front();
        src_segment.pop();
        //设置ack、确认应答号与接收窗口大小
        if(_receiver.ackno().has_value()){
            cur_segment.header().ack = true;
            cur_segment.header().ackno = _receiver.ackno().value();
            cur_segment.header().win = _receiver.window_size();
        }
        //放入TCPConnection的发送队列中
        _segments_out.push(cur_segment);
    }
}

TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";
            _sender.stream_in().set_error();
            _receiver.stream_out().set_error();
            _is_active = false;
            _linger_after_streams_finish = false;
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}

    