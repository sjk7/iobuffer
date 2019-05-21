#include "../../include/iobuffer/my_iobuffer.hpp"
#include <iostream>

using namespace std;
typedef my::detail::counted_buffer<char> counted_buf_t;
int main() {
	
	int x = _MSC_VER;
    counted_buf_t tmpbuf(0);
    // zero MEANS zero
    ASSERT(tmpbuf.size() == 0);

    // For everything else, the next higest power of two:
    counted_buf_t buf(8000);
    size_t sz = buf.size();
    ASSERT(sz == 8192);
    const char* beg = buf.begin();
    const char* end = buf.end();
    ASSERT(beg && end);
    ASSERT(end - beg == buf.ssize());
    ASSERT(sz == buf.size_in_bytes());
    cout << buf.size() << endl;
    ASSERT(end - sz == buf.begin());

    cout << buf.available_read() << endl;
    ASSERT(buf.available_read() == 0);

    cout << buf.available_write() << endl;
    ASSERT(buf.available_write() == buf.ssize());

    ASSERT(buf.can_write() == buf.ssize());
    ASSERT(buf.can_read() == 0);

    std::string hello = "hello";
    const std::string const_hello = "const hello";
    typedef my::detail::span<const char> rospan_t;
    rospan_t spwc(const_hello);
    ASSERT(spwc.size() == 11);
    cout << spwc.begin() << endl;
    std::string bak(spwc.begin());
    ASSERT(bak == const_hello);
    ASSERT(bak.size() == 11);

    typedef counted_buf_t::span_t_const span_t_const;
	typedef counted_buf_t::span_t span_t;
    span_t spw(hello);
    ASSERT(spw.size() == 5);

	span_t_const string_span (const_hello);
	size_t szw = buf.write(string_span);
	
    buf.write( span_t_const(hello) );

    // read the whole buffer
	typedef counted_buf_t::spans_t spans_t;
    int wanted = -1;
	spans_t spans;
    spans_t read_results;
	int got = buf.read(spans, wanted, 0);
    // buf.write("Hello", 5);
	

    return 0;
}
