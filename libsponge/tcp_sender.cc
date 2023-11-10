#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>
#include <iostream>

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

uint64_t TCPSender::bytes_in_flight() const { 
    return in_flight;
}

void TCPSender::fill_window() {

    size_t win = win_size ? win_size : 1;

    if (!set_syn) {
        TCPSegment seg;
        seg.header().seqno = next_seqno();
        set_syn = seg.header().syn = true;

        _next_seqno += seg.length_in_sequence_space();
        in_flight += seg.length_in_sequence_space();

        timeout_gap = _initial_retransmission_timeout;
        time_ms = 0;

        _segments_out.push(seg);
        
        not_ack.insert(make_pair(_next_seqno, seg));

    }

    while (in_flight < win) {

        TCPSegment seg;
        seg.header().seqno = next_seqno();

        size_t read_len = min(TCPConfig::MAX_PAYLOAD_SIZE, win - in_flight );
        string payload = _stream.read(read_len);
        seg.payload() = Buffer(move(payload));

        if (!set_fin && _stream.eof() && win - in_flight - seg.length_in_sequence_space() > 0) {
            set_fin = seg.header().fin = true;
        }
        if (seg.length_in_sequence_space() == 0) {
            break;
        }

        _next_seqno += seg.length_in_sequence_space();
        in_flight += seg.length_in_sequence_space();

       if (not_ack.empty()) {
            timeout_gap = _initial_retransmission_timeout;
            time_ms = 0;
        }
        _segments_out.push(seg);
        
        not_ack.insert(make_pair(_next_seqno, seg));
        if (seg.header().fin) {
            break;
        }
    }
}



// ! \param ackno The remote receiver's ackno (acknowledgment number)
// ! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    size_t abs_ack = unwrap(ackno, _isn, _next_seqno);
    if (abs_ack > _next_seqno) {
        return;
    }
    auto it = not_ack.begin();
    while (it != not_ack.end()) {
        if (it -> first <= abs_ack) {
            in_flight -= it -> second.length_in_sequence_space();
            it = not_ack.erase(it);

            timeout_gap = _initial_retransmission_timeout;
            time_ms = 0;
            timeout_cnt = 0;
        }
        else {
            break;
        }
    }

    win_size = window_size;
    fill_window();

}



// ! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    time_ms += ms_since_last_tick;
    auto it = not_ack.begin();
    while (it != not_ack.end() && time_ms >= timeout_gap)
    {
        _segments_out.push(it -> second);
        if (win_size > 0) {
            timeout_gap *= 2;
            timeout_cnt ++;
        }
        time_ms = 0;
    }
    
}





unsigned int TCPSender::consecutive_retransmissions() const { return timeout_cnt; }

void TCPSender::send_empty_segment() {
    TCPSegment seg;
    seg.header().seqno = next_seqno();
    _segments_out.push(seg);
}