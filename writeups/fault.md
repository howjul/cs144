[100%] Testing the TCP receiver and the TCP sender...
Test project /home/cs144/sponge/build
      Start  1: t_wrapping_ints_cmp
 1/34 Test  #1: t_wrapping_ints_cmp ..............   Passed    0.00 sec
      Start  2: t_wrapping_ints_unwrap
 2/34 Test  #2: t_wrapping_ints_unwrap ...........   Passed    0.00 sec
      Start  3: t_wrapping_ints_wrap
 3/34 Test  #3: t_wrapping_ints_wrap .............   Passed    0.01 sec
      Start  4: t_wrapping_ints_roundtrip
 4/34 Test  #4: t_wrapping_ints_roundtrip ........   Passed    0.11 sec
      Start  5: t_recv_connect
 5/34 Test  #5: t_recv_connect ...................   Passed    0.00 sec
      Start  6: t_recv_transmit
 6/34 Test  #6: t_recv_transmit ..................   Passed    0.04 sec
      Start  7: t_recv_window
 7/34 Test  #7: t_recv_window ....................   Passed    0.01 sec
      Start  8: t_recv_reorder
 8/34 Test  #8: t_recv_reorder ...................   Passed    0.00 sec
      Start  9: t_recv_close
 9/34 Test  #9: t_recv_close .....................   Passed    0.00 sec
      Start 10: t_recv_special
10/34 Test #10: t_recv_special ...................   Passed    0.00 sec
      Start 11: t_send_connect
11/34 Test #11: t_send_connect ...................***Failed    0.01 sec
Test Failure on expectation:
        Expectation: in state `stream started but nothing acknowledged`

Failure message:
        The TCPSender was in state `stream ongoing`, but it was expected to be in state `stream started but nothing acknowledged`

List of steps that executed successfully:
        Initialized with (retx-timeout=1000) and called fill_window()
        Expectation: in state `stream started but nothing acknowledged`
        Expectation: segment sent with (A=0,R=0,S=1,F=0,seqno=1691829577,payload_size=0,...)
        Expectation: 1 bytes in flight
        Action:      ack 1691829577 winsize 137

The test "SYN -> wrong ack test" failed

      Start 12: t_send_transmit
12/34 Test #12: t_send_transmit ..................   Passed    0.04 sec
      Start 13: t_send_retx
13/34 Test #13: t_send_retx ......................***Failed    0.00 sec
Test Failure on expectation:
        Expectation: no (more) segments

Failure message:
        The TCPSender sent a segment, but should not have. Segment info:
        Header(flags=S,seqno=2257485361,ack=0,win=0) with 0 bytes

List of steps that executed successfully:
        Initialized with (retx-timeout=150) and called fill_window()
        Expectation: segment sent with (A=0,R=0,S=1,F=0,seqno=2257485361,payload_size=0,...)
        Expectation: no (more) segments
        Expectation: in state `stream started but nothing acknowledged`
        Action:      149 ms pass
        Expectation: no (more) segments
        Action:      1 ms pass
        Expectation: segment sent with (A=0,R=0,S=1,F=0,seqno=2257485361,payload_size=0,...)
        Expectation: in state `stream started but nothing acknowledged`
        Expectation: 1 bytes in flight
        Action:      299 ms pass

The test "Retx SYN twice at the right times, then ack" failed

      Start 14: t_send_window
14/34 Test #14: t_send_window ....................   Passed    0.05 sec
      Start 15: t_send_ack
15/34 Test #15: t_send_ack .......................   Passed    0.01 sec
      Start 16: t_send_close
16/34 Test #16: t_send_close .....................***Failed    0.01 sec
Test Failure on expectation:
        Expectation: in state `stream finished (FIN sent) but not fully acknowledged`

Failure message:
        The TCPSender was in state `stream finished and fully acknowledged`, but it was expected to be in state `stream finished (FIN sent) but not fully acknowledged`

List of steps that executed successfully:
        Initialized with (retx-timeout=1000) and called fill_window()
        Expectation: segment sent with (A=0,R=0,S=1,F=0,seqno=1851455273,payload_size=0,...)
        Action:      ack 1851455274 winsize 137
        Expectation: in state `stream ongoing`
        Action:      close
        Expectation: in state `stream finished (FIN sent) but not fully acknowledged`
        Expectation: segment sent with (F=1,seqno=1851455274,...)
        Action:      ack 1851455274 winsize 137

The test "FIN not acked test" failed

      Start 17: t_send_extra
17/34 Test #17: t_send_extra .....................***Failed    0.02 sec
Test Failure on expectation:
        Expectation: segment sent with (seqno=1690866018,payload_size=3,"def",...)

Failure message:
        The Sender should have produced a segment that existed, but it did not

List of steps that executed successfully:
        Initialized with (retx-timeout=5579) and called fill_window()
        Expectation: segment sent with (A=0,R=0,S=1,F=0,seqno=1690866014,payload_size=0,...)
        Action:      ack 1690866015 winsize 1000
        Expectation: in state `stream ongoing`
        Action:      write bytes: "abc"
        Expectation: segment sent with (seqno=1690866015,payload_size=3,"abc",...)
        Action:      5574 ms pass
        Action:      write bytes: "def"
        Expectation: segment sent with (seqno=1690866018,payload_size=3,"def",...)
        Action:      ack 1690866018 winsize 1000
        Action:      5578 ms pass
        Expectation: no (more) segments
        Action:      2 ms pass

The test "Timer restarts on ACK of new data" failed

      Start 18: t_strm_reassem_single
18/34 Test #18: t_strm_reassem_single ............   Passed    0.01 sec
      Start 19: t_strm_reassem_seq
19/34 Test #19: t_strm_reassem_seq ...............   Passed    0.01 sec
      Start 20: t_strm_reassem_dup
20/34 Test #20: t_strm_reassem_dup ...............   Passed    0.02 sec
      Start 21: t_strm_reassem_holes
21/34 Test #21: t_strm_reassem_holes .............   Passed    0.01 sec
      Start 22: t_strm_reassem_many
22/34 Test #22: t_strm_reassem_many ..............   Passed    0.04 sec
      Start 23: t_strm_reassem_overlapping
23/34 Test #23: t_strm_reassem_overlapping .......   Passed    0.01 sec
      Start 24: t_strm_reassem_win
24/34 Test #24: t_strm_reassem_win ...............   Passed    0.04 sec
      Start 25: t_strm_reassem_cap
25/34 Test #25: t_strm_reassem_cap ...............   Passed    0.10 sec
      Start 26: t_byte_stream_construction
26/34 Test #26: t_byte_stream_construction .......   Passed    0.01 sec
      Start 27: t_byte_stream_one_write
27/34 Test #27: t_byte_stream_one_write ..........   Passed    0.00 sec
      Start 28: t_byte_stream_two_writes
28/34 Test #28: t_byte_stream_two_writes .........   Passed    0.00 sec
      Start 29: t_byte_stream_capacity
29/34 Test #29: t_byte_stream_capacity ...........   Passed    0.34 sec
      Start 30: t_byte_stream_many_writes
30/34 Test #30: t_byte_stream_many_writes ........   Passed    0.01 sec
      Start 31: t_webget
31/34 Test #31: t_webget .........................   Passed    1.19 sec
      Start 53: t_address_dt
32/34 Test #53: t_address_dt .....................   Passed    0.04 sec
      Start 54: t_parser_dt
33/34 Test #54: t_parser_dt ......................   Passed    0.00 sec
      Start 55: t_socket_dt
34/34 Test #55: t_socket_dt ......................   Passed    0.00 sec

88% tests passed, 4 tests failed out of 34

Total Test time (real) =   2.19 sec

The following tests FAILED:
         11 - t_send_connect (Failed)
         13 - t_send_retx (Failed)
         16 - t_send_close (Failed)
         17 - t_send_extra (Failed)
Errors while running CTest
CMakeFiles/check_lab2.dir/build.make:57: recipe for target 'CMakeFiles/check_lab2' failed
make[3]: *** [CMakeFiles/check_lab2] Error 8
CMakeFiles/Makefile2:163: recipe for target 'CMakeFiles/check_lab2.dir/all' failed
make[2]: *** [CMakeFiles/check_lab2.dir/all] Error 2
CMakeFiles/Makefile2:170: recipe for target 'CMakeFiles/check_lab2.dir/rule' failed
make[1]: *** [CMakeFiles/check_lab2.dir/rule] Error 2
Makefile:168: recipe for target 'check_lab2' failed
make: *** [check_lab2] Error 2