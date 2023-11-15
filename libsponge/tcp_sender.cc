#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity) {}

uint64_t TCPSender::bytes_in_flight() const { return _unacknowledged_bytes; }

void TCPSender::fill_window() 
{
    size_t window_size = _window_size ? _window_size : 1;
    while(_unacknowledged_bytes < window_size){
        TCPSegment send_seg;

        //如果尚未发送SYN包，则设置header的syn位
        if(!_syn_sent){
            _syn_sent = true;
            send_seg.header().syn = true;
        }

        //设置seqno
        send_seg.header().seqno = next_seqno();
        //设置payload
        size_t send_size = min(window_size - send_seg.header().syn - _unacknowledged_bytes, TCPConfig::MAX_PAYLOAD_SIZE);
        send_seg.payload() = Buffer(move(_stream.read(send_size)));

        //若满足条件则增加FIN
        if(!_fin_sent && _stream.eof() && send_seg.length_in_sequence_space() + _unacknowledged_bytes < window_size){
            _fin_sent = true;
            send_seg.header().fin = true;
        }

        //如果没有任何数据，则停止数据包发送
        if(send_seg.length_in_sequence_space() == 0){
            break;
        }

        //如果没有正在等待的数据包，则重设更新时间
        if(_unacknowledged_segments.empty()){
            _tick_time = 0;
            _timeout_tick = _initial_retransmission_timeout;
        }

        //发送
        _segments_out.push(send_seg);
        //跟踪数据包
        _unacknowledged_segments.insert(make_pair(_next_seqno, send_seg));
        _unacknowledged_bytes += send_seg.length_in_sequence_space();
        _next_seqno += send_seg.length_in_sequence_space();

        //如果发送的是FIN包，则停止发送
        if(send_seg.header().fin) break;
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) 
{ 
    //如果传入的ack不可靠，则直接丢弃
    if(unwrap(ackno, _isn, _next_seqno) > _next_seqno) return;

    //遍历数据结构，如果一个发送的segment已经被确认，则从数据结构中删除，并更新相关的数据
    auto it = _unacknowledged_segments.begin();
    while(it != _unacknowledged_segments.end()){
        if(it->first <= unwrap(ackno, _isn, _next_seqno)){
            _unacknowledged_bytes -= (it->second).length_in_sequence_space();
            _tick_time = 0;
            _retransmission_count = 0;
            _timeout_tick = _initial_retransmission_timeout;
            it = _unacknowledged_segments.erase(it);
        }else{
            it++;
        }
    }

    //更新窗口大小，调用fill_window继续发送数据
    _window_size = window_size;
    fill_window();
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) 
{ 
    _tick_time += ms_since_last_tick;

    //遍历追踪列表，如果有超时
    if(!_unacknowledged_segments.empty() && _tick_time >= _timeout_tick){
        _tick_time = 0;
        if(_window_size > 0) _timeout_tick *= 2;
        _segments_out.push(_unacknowledged_segments.begin()->second);
        _retransmission_count++;
    }
}

unsigned int TCPSender::consecutive_retransmissions() const { return _retransmission_count; }

void TCPSender::send_empty_segment() 
{
    TCPSegment send_seg;
    send_seg.header().seqno = next_seqno();
    _segments_out.push(send_seg);
}
