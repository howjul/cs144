#include "byte_stream.hh"

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity):
    buffer(capacity, '\0'),
    cap(capacity),
    begin(0),
    end(0),
    have_data(false),
    already_read(0),
    already_written(0),
    the_input_ended(false){
}

size_t ByteStream::write(const string &data) {
    //检查是否有空间可供写入
    if(remaining_capacity() == 0){
        return 0;
    }
    //计算可写入的字节数
    size_t write_size = min(data.size(), remaining_capacity());
    //计算并更新写入的位置
    size_t begin_before = begin;
    if(buffer_empty()){
        begin = end;
    }else{
        begin = (end + 1) % cap;
    }
    end = (begin + write_size) % cap;
    //替换字符串
    if(end < begin){
        buffer.replace(buffer.begin() + begin, buffer.begin() + cap, data.begin(), data.begin() + cap - begin);
        buffer.replace(buffer.begin(), buffer.begin() + end, data.begin() + cap - begin, data.begin() + write_size);
    }else{
        buffer.replace(buffer.begin() + begin, buffer.begin() + end, data.begin(), data.begin() + write_size);
    }
    //更新已写入的字节数
    already_written += write_size;
    //恢复写入的位置
    end = (begin + write_size - 1) % cap;
    begin = begin_before;
    //返回实际写入的字节数
    if(have_data == false && write_size > 0){
        have_data = true;
    }
    return write_size;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    //检查是否有数据
    if(buffer_empty()){
        return "";
    }
    //计算可读取的字节数
    size_t read_size = min(len, buffer_size());
    //获取返回的字符串
    string result = "";
    if(end > begin){
        result.append(buffer.begin() + begin, buffer.begin() + begin + read_size);
    }else{
        if(begin + read_size <= cap){
            result.append(buffer.begin() + begin, buffer.begin() + begin + read_size);
        }else{
            result.append(buffer.begin() + begin, buffer.begin() + cap);
            result.append(buffer.begin(), buffer.begin() + read_size - cap + begin);
        }
    }
    //返回字符串
    return result;
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    //检查是否有数据
    if(buffer_empty()){
        return;
    }
    //计算可读取的字节数
    size_t read_size = min(len, buffer_size());
    //检测是否还有数据
    size_t buffer_size_now = buffer_size() - read_size;
    //更新已读取的字节数
    already_read += read_size;
    //更新读取的位置
    begin = (begin + read_size) % cap;    
    if(buffer_size_now == 0){
        end = begin;
        have_data = false;
    }else{
        end = begin + buffer_size_now - 1;
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

void ByteStream::end_input() { the_input_ended = true;}

bool ByteStream::input_ended() const { return the_input_ended; }

size_t ByteStream::buffer_size() const { 
    if (buffer_empty()) {
        return 0;
    }else if (begin <= end) {
        return end - begin + 1;
    } else {
        return cap - begin + end + 1;
    }
}

bool ByteStream::buffer_empty() const { 
    return !have_data;
 }

bool ByteStream::eof() const { return (input_ended() && buffer_empty()); }

size_t ByteStream::bytes_written() const { return already_written; }

size_t ByteStream::bytes_read() const { return already_read; }

size_t ByteStream::remaining_capacity() const { return cap - buffer_size(); }
