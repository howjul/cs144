#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity) : cap(capacity) {}

size_t ByteStream::write(const string &data) {
    //检测是否能够写入
    if (error() || input_ended() || remaining_capacity() == 0) return 0;
    //计算可写入的字节数
    size_t write_size = min(remaining_capacity(), data.size());
    //写入数据
    for (size_t i = 0; i < write_size; i++) {
        buffer.push_back(data[i]);
        already_written++;
    }
    return write_size;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    //检查是否有数据
    if (error() ||buffer_empty()) return "";
    //计算可读取的字节数
    size_t read_size = min(len, buffer.size());
    //获取返回的字符串
    string result;
    size_t i = 0;
    auto it = buffer.begin();
    while(1){
        if(it == buffer.end() || i++ >= read_size) break;
        result.push_back(*it);
        it++;
    }
    //返回字符串
    return result;
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    //检查是否有数据
    if (error() || buffer_empty()) return;
    //获取可读取的字节数
    size_t read_size = min(len, buffer.size());
    //一个一个推出数据
    for (size_t i = 0; i < read_size; i++) {
        buffer.pop_front();
        already_read++;
    }
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    string result = peek_output(len);
    pop_output(len);
    return result;
}

void ByteStream::end_input() { the_input_ended = true; }

bool ByteStream::input_ended() const { return the_input_ended; }

size_t ByteStream::buffer_size() const { return buffer.size(); }

bool ByteStream::buffer_empty() const { return buffer.size() == 0; }

bool ByteStream::eof() const { return input_ended() && buffer_empty(); }

size_t ByteStream::bytes_written() const { return already_written; }

size_t ByteStream::bytes_read() const { return already_read; }

size_t ByteStream::remaining_capacity() const { return cap - buffer.size(); }