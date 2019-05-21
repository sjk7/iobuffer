#pragma once
#include <cstdio>
#include <cstddef>
#include <cstdlib>
#include <exception>
#include <new>
#include <string>
#include <cstddef>


#include "my_cpp98_compat.hpp"

#ifndef MY_INVAL_BUF_SIZE
#define MY_INVAL_BUF_SIZE size_t(~0)
#endif
namespace my {

	template<class T, class U> 
const T min(const T& a, const U& b)
{
    return (b < a) ? b : a;
}

		template<class T> 
const T min(const T a, const T b)
{
    return (b < a) ? b : a;
}

	template<class T> 
const T& max(const T& a, const T& b)
{
    return (comp(a, b)) ? b : a;
}
namespace detail {

    template <typename T> struct span_guts {
        protected:
        T* m_begin;
        T* m_end;
        int64_t m_written;
        int64_t m_read;
        typedef span_guts<T> self_type;

        public:
        span_guts() : m_begin(nullptr), m_end(nullptr), m_written(0), m_read(0) {
			TRACE("span_guts constructor\n");
		}

		span_guts(const T* const ptr, const size_t sz):
		m_begin(ptr), m_end(ptr+sz),m_written(0), m_read(0){}

        span_guts(T* beg, T* end) : m_begin(beg), m_end(end) {
            if (m_begin != m_end) ASSERT("my::span: end > begin" == nullptr);
       }

        span_guts(std::string& s)
            : m_begin(s.size() == 0 ? nullptr : &s[0])
            , m_end(s.size() == 0 ? nullptr : &s[0] + s.size()) {}
		
		// This constructor assumes it is OK to assume a terminator on the end of std::string
        span_guts(const std::string& s)
            : m_begin(s.size() == 0 ? nullptr : &s[0])
            , m_end(s.size() == 0 ? nullptr : &s[0] + s.size() + sizeof(T)) {}

        inline ptrdiff_type available_read() const noexcept {
            return m_written - m_read;
        }
        inline ptrdiff_type available_write() const noexcept {
            ptrdiff_type rv = size() - available_read();
            return rv;
        }
        inline size_t available_read_u() const noexcept {
            return static_cast<size_t>(available_read());
        }
        inline size_t available_write_u() const noexcept {
            return static_cast<size_t>(available_write());
        }
        inline size_t size_in_bytes() const noexcept { return m_end - m_begin; }
        inline size_t size() const noexcept { return size_in_bytes() / sizeof(T); }
        inline ptrdiff_type ssize() const noexcept { return m_end - m_begin; }

        inline T* begin() { return m_begin; }
        inline T* end() { return m_end; }
        inline const T* cbegin() const noexcept { return m_begin; }
        inline const T* cend() const noexcept { return m_end; }

        void clear(bool clear_accum = true) noexcept {
            m_begin = nullptr;
            m_end = nullptr;
            if (clear_accum) {
                m_written = 0;
                m_read = 0;
            }
        }
        inline int can_write() const noexcept { return available_write(); }
        inline int can_read() const noexcept { return available_read(); }
        inline bool empty() const noexcept { return size() == 0; }
    };

    // treat this like 2 pointers and a type, basically
    template <typename T> struct span : span_guts<T> {
        protected:
        typedef span_guts<T> guts_t;
		typedef span_guts<const T> guts_t_const;
        typedef span<T> my_type;

        public:
        span() {}
        explicit span(std::string& s) : guts_t(s) {}
        span(const std::string& s) : guts_t(s) {}
		span(const T* const data, const size_t sz) : guts_t(data, sz){}
		span(T* beg, T* end): guts_t(beg, end){}

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
        inline size_t resize(const size_t new_size) {

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

            span_t::m_begin = tmp;
            span_t::m_end = tmp + new_size;

            return span_t::size();
        }



		protected:
			// protected because only the superclass knows the
			// most efficient way to wrap read and write pos,
			// according to whether or not its pow2 sized buffer or not
		size_t write(const span_t_const sp){
			// FIXME: account for when we wrap

			size_t space = this->can_write();
			size_t write_size = min(sp.can_write(), space);
			T* dest = begin() + m_written;
			memcpy(dest, sp.cbegin(), write_size);
			m_written += write_size;
			return write_size;
		}

    };

    template <typename T> class pow2_buffer : public malloc_buffer<T> {

        typedef malloc_buffer<T> buf_t;

        public:
        pow2_buffer(size_t size = 0) : buf_t(size == 0 ? 0 : nextPowerOf2(size)), m_writepos(0), m_readpos(0) {}

		size_t write(const span_t_const sp){
			size_t ret = buf_t::write(sp);
			m_writepos += ret;
			m_writepos %= size();
			return ret;
		}

		 protected:
			int m_writepos;
			int m_readpos;


    };

    // a buffer that keeps track of read and write positions.
    template <typename T> struct counted_buffer : public pow2_buffer<T> {

        typedef pow2_buffer<T> base_t;
        typedef base_t self;
        typedef typename base_t::span_t span_t;
		typedef typename base_t::span_t spans_t[2];
		typedef typename base_t::span_t_const span_t_const;

        counted_buffer(size_t sz) : base_t(sz) {}

        inline int read(spans_t& spans, int wanted, int position, bool peek = false) const noexcept {
            (void)peek;
			TRACE("span[0].size() == %d\n", spans[0].size());
            return spans[0].ssize() + spans[1].ssize();
        }
        
		inline size_t write(const span_t_const& span) {
			return base_t::write(span);
        }


    };
} // namespace detail
} // namespace my
