
#define DEBUG_BUF_PERF_ 111
#include "../../include/iobuffer/my_iobuffer.hpp"
#include <iostream>

using namespace std;
typedef my::detail::counted_buffer<char> counted_buf_t;

int main() {

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
    const size_t ssize = spwc.size();
    ASSERT(ssize == 12);
    cout << spwc.begin() << endl;
    std::string bak(spwc.begin());
    ASSERT(bak == const_hello);
    ASSERT(bak.size() == 11);

    typedef counted_buf_t::span_t_const span_t_const;
    typedef counted_buf_t::spans_t_const spans_t_const;
    typedef counted_buf_t::span_t span_t;
    span_t spw(hello);
    size_t ms = spw.size();
    ASSERT(ms == 6);

    span_t_const string_span(const_hello);
    size_t szw = buf.write(string_span);
    (void)szw;
    ASSERT(szw == strlen(const_hello.c_str()) + 1);

    // read the whole buffer
    int wanted = -1;

    spans_t_const read_results;
    size_t got = buf.read(read_results, wanted);

    const size_t isz = read_results[0].size();
    ASSERT(got == isz);

    cout << read_results[0].begin() << endl;
    return 0;
}
/*/
int main() {
    using namespace std;
    cout << __cplusplus << endl;
}
/*/
