#pragma once
#include <cstdio>
#include <cstddef>
#include <cstdlib>
#include <exception>
#include <string>
#include <cstddef>
#include <cstring> // memset
#include <algorithm>

#include "my_cpp98_compat.hpp"

#ifndef MY_INVAL_BUF_SIZE
#define MY_INVAL_BUF_SIZE size_t(~0)
#endif
namespace my {

template <class T, class U> const T 
inline min(const T& a, const U& b) {
    return (b < a) ? b : a;
}

template <class T> const T 
inline min(const T a, const T b) {
    return (b < a) ? b : a;
}

template <class T> const T 
inline max(const T& a, const T& b) {
    return (comp(a, b)) ? b : a;
}
namespace detail {


    template <typename T> struct ptrs {
        T* m_beg;
        T* m_end;
        ptrs(T* p, size_t sz) : m_beg(p), m_end(m_beg + sz) noexcept {}
        ptrs(T* beg, T* end) : m_beg(beg), m_end(end) noexcept {
            if (beg > end) ASSERT("end > begin" == nullptr);
        }


        ptrs(const std::string& s)
            : m_beg(s.size() == 0 ? nullptr : &s[0])
            , m_end(s.size() == 0 ? nullptr : &s[0] + s.size() + 1) noexcept {}

        ptrs(std::string& s)
            : m_beg(s.size() == 0 ? nullptr : &s[0])
            , m_end(s.size() == 0 ? nullptr : &s[0] + s.size() + 1) noexcept {}

        inline size_t size() const noexcept { return (m_end - m_beg) / sizeof(T); }
        inline size_t size_in_bytes() const noexcept { return m_end - m_beg; }
        inline ptrdiff_type isize() const noexcept { return m_end - m_beg; }
        inline ptrdiff_type ssize() const noexcept { return m_end - m_beg; }

        inline T* begin() { return m_beg; }
        inline T* end() { return m_end; }
        inline const T* cbegin() const noexcept { return m_beg; }
        inline const T* cend() const noexcept { return m_end; }
        void clear() { m_beg = nullptr, m_end = nullptr; }
        inline bool empty() const noexcept { return size() == 0; }

		        
		T& operator[](size_t index) noexcept{
			ASSERT(index < size());
			return *(m_beg + index);
		}

		const T& operator[](size_t index) const noexcept{
			ASSERT(index < size());
			return *(m_beg + index);
		}
    };

    template <typename T> inline ptrs<T> ptrs_empty() {
        T* n = 0;
        ptrs<T> ps(n, n);
        return ps;
    }

    template <typename T> struct span_guts : ptrs<T> {
        protected:
        typedef ptrs<T> base_t;

        public:
        using ptrs<T>::size;
		using ptrs<T>::operator[];
        span_guts() : base_t(ptrs_empty<T>()) {
#ifdef DEBUG_BUF_PERF_
            TRACE("span_guts constructor [default]\n");
#endif
        }

        span_guts(T* ptr, const size_t sz) : base_t(ptr, sz) {
#ifdef DEBUG_BUF_PERF_
            TRACE("span_guts constructor [ptr, size_t]\n");
#endif
        }

#ifdef CPP11
        span_guts(span_guts&& rhs) {
#ifdef DEBUG_BUF_PERF_
            TRACE("span_guts move constructor [ptr, size_t]\n");
#endif
            using namespace std;
            std::move(rhs.m_begin, m_begin);
            std::move(rhs.m_end, m_end);
        }
#endif

#ifdef CPP11
        span_guts& operator=(span_guts&& rhs) {
#ifdef DEBUG_BUF_PERF_
            TRACE("span_guts move assign constructor [ptr, size_t]\n");
#endif
            using namespace std;
            std::move(rhs.m_begin, m_begin);
            std::move(rhs.m_end, m_end);
        }
#endif

        span_guts(T* beg, T* end) : base_t(beg, end) {

#ifdef DEBUG_BUF_PERF_
            TRACE("span_guts constructor [beg, end]\n");
#endif
        }

        span_guts(std::string& s) : base_t(s) noexcept {}

        // This constructor assumes it is OK to assume a terminator on the end of
        // std::string
        span_guts(const std::string& s) : base_t(s) noexcept {}
        span_guts(const span_guts& rhs) : base_t(rhs) noexcept {}

        span_guts& operator=(const span_guts& rhs) {
#ifdef DEBUG_BUF_PERF_
            TRACE("span_guts assignment\n");
#endif
            base_t::m_beg = rhs.m_beg;
            base_t::m_end = rhs.m_end;
            return *this;
        }

    }; // span_guts<T>

    // treat this like 2 pointers and a type, basically
    template <typename T> struct span : span_guts<T> {
        protected:
        typedef span_guts<T> guts_t;
        typedef span_guts<const T> guts_t_const;
        typedef span<T> my_type;

        public:
        using span_guts<T>::size;
        span() {}
        explicit span(std::string& s) : guts_t(s) {}
        explicit span(const std::string& s) : guts_t(s) {}
        span(T* data, const size_t sz) : guts_t(data, sz) {}
        span(T* beg, T* end) : guts_t(beg, end) {}
    };

    template <typename T> class malloc_buffer : public span<T> {

        public:
        typedef span<T> span_t;
        typedef span<const T> span_t_const;
        malloc_buffer(const size_t size) { resize(size); }
        malloc_buffer(const malloc_buffer&);
        malloc_buffer& operator=(const malloc_buffer& rhs);
        ~malloc_buffer() {
            if (span_t::begin()) delete span_t::begin();
            span_t::clear();
        }

        inline T* begin() { return span_t::begin(); }
        inline T* end() { return span_t::end(); }

        // returns the size of the buffer actually created,
        // or MY_INVAL_BUF_SIZE (leaving the object in a valid state, with the old size
        // and nothing changed). NOTE: MY_INVAL_SIZE is a size_t, but -1 if cast to int.
        inline size_t resize(size_t new_size) {
			
			ASSERT(sizeof(T) == 1 && "Not designed nor tested for multi-byte Ts");
			new_size *= sizeof(T);
            T* tmp = nullptr;
            if (span_t::begin() == 0) {
                tmp = static_cast<T*>(malloc(new_size * sizeof(T)));
                memset(tmp, 0, new_size * sizeof(T));
                if (!tmp) {
                    return MY_INVAL_BUF_SIZE;
                }
            } else {
                tmp = static_cast<T*>(realloc(span_t::begin(), (new_size * sizeof(T))));
                if (!tmp) {
                    return MY_INVAL_BUF_SIZE;
                }
            }

            span_t::m_beg = tmp;
            span_t::m_end = tmp + new_size;

            return span_t::size();
        }

        protected:
    };

    struct buf_ctrs {
        uint64_t read;
        uint64_t written;
        uint64_t writepos;
        uint64_t readpos;
        uint64_t m_mask;
    };

    buf_ctrs make_buf_ctrs(size_t sz) {
        buf_ctrs b;
        memset(&b, 0, sizeof(b));
        if (sz) b.m_mask = sz - 1;
        return b;
    }

    template <typename T> class pow2_buffer : public malloc_buffer<T> {

        typedef malloc_buffer<T> buf_t;

        public:
        typedef malloc_buffer<T> base_t;
        typedef typename base_t::span_t_const spans_t_const[2];
		typedef typename base_t::span_t spans_t[2];
        typedef typename base_t::span_t_const span_t_const;
		

        // typedef typename span_t spans_t[2];
        pow2_buffer(size_t size = 0)
            : buf_t(size == 0 ? 0 : nextPowerOf2(size))
            , m_ctrs(make_buf_ctrs(buf_t::size())) {}

        inline size_t write(const span_t_const& sp) {

            size_t space = this->can_write();
            size_t write_size = min(sp.size(), space);
            T* dest = this->begin() + m_ctrs.written;
            memcpy(dest, sp.cbegin(), write_size);
            m_ctrs.written += write_size;

            int remain = sp.isize() - (int)write_size;
            while (remain > 0 && this->can_write() > 0) {
                remain = sp.isize() - (int)write_size;
                const span_t_const extra(sp.cbegin() + remain, remain);
                return write_size + write(extra);
            }
            return write_size;
        }

        // returns pointers to my internal memory.
        // For multithreaded use, you probably want to (*copy*) the memory into your own.
		// SP can be span_t spans_t[2], or spans_t_const[2], or perhaps your own type?
		template <typename SP>
        inline size_t read(SP results[2], int how_many, int idx = 0)
            noexcept {
			
			using namespace std;

            ASSERT(idx < 2);
			size_t avail = this->can_read();
            how_many = how_many < 0 ? avail : how_many;
                        
            size_t sz = CAST(size_t, how_many);
            int how_much = CAST(int, min(sz, avail));

            SP tmp(this->begin() + m_ctrs.read, how_much);
            results[idx] = tmp;
            m_ctrs.read += how_much;
            int remain = results[idx].isize() - (int)how_much;

            if (remain > 0) {
                return how_much + read(results, idx + 1);
            }

            return CAST(size_t, how_much);
        }

        inline ptrdiff_type available_read() const noexcept {
            return m_ctrs.written - m_ctrs.read;
        }
        inline ptrdiff_type available_write() const noexcept {
            ptrdiff_type rv = buf_t::size() - available_read();
            return rv;
        }
        inline size_t available_read_u() const noexcept {
            return static_cast<size_t>(available_read());
        }
        inline size_t available_write_u() const noexcept {
            return static_cast<size_t>(available_write());
        }

        void clear(bool clear_ctrs = true) noexcept {
            this->m_begin = nullptr;
            this->m_end = nullptr;
            if (clear_ctrs) {
                memset(&m_ctrs, 0, sizeof(m_ctrs));
            }
        }
        inline int can_write() const noexcept { return available_write(); }
        inline int can_read() const noexcept { return available_read(); }
		
		inline int tot_written() const noexcept		{ return m_ctrs.written; }
		inline int tot_read() const noexcept		{ return m_ctrs.read; }
		inline int writepos() const noexcept	{ return m_ctrs.writepos; }
		inline int readpos() const noexcept	{ return m_ctrs.readpos; }

        protected:
        mutable buf_ctrs m_ctrs;
    };

    // a buffer that keeps track of read and write positions.
    template <typename T> struct counted_buffer : public pow2_buffer<T> {

        typedef pow2_buffer<T> base_t;
        typedef base_t self;
        typedef typename base_t::span_t span_t;
		typedef typename base_t::spans_t_const;
		typedef typename base_t::spans_t spans_t;
        typedef typename base_t::span_t_const span_t_const;

        typedef typename base_t::span_t_const span_t_const;

        counted_buffer(size_t sz) : base_t(sz) {}
		counted_buffer(const span_t_const& sp) : base_t(sp.size()){
			base_t::write(sp);
		}
		counted_buffer(T* d, size_t sz) : base_t(sz){
			const span_t_const sp(d, sz);
			base_t::write(sp);
		}
    };
} // namespace detail
} // namespace my
