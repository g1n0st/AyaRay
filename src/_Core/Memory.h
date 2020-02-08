#ifndef AYA_CORE_MEMORY_H
#define AYA_CORE_MEMORY_H

#include "config.h"

#include <atomic>

namespace Aya {
	// std::shared_ptr implement
	template <typename T>
	class SharedPtr;

	template <typename T>
	class U_Ptr {
	public:
		friend class SharedPtr<T>;

		U_Ptr(T *ptr) : p(ptr), cnt(1) {}
		~U_Ptr() { delete p; }

		std::atomic<uint32_t> cnt;
		T *p;
	};

	template <typename T>
	class SharedPtr {
	public:
		SharedPtr(T *ptr = NULL) : rp(new U_Ptr<T>(ptr)) {}
		SharedPtr(const SharedPtr<T> &sp) :rp(sp.rp) {
			rp->cnt++;
		}

		SharedPtr & operator = (const SharedPtr<T>& rhs) {
			rhs.rp->cnt++;
			if (!--rp->cnt) delete rp;
			rp = rhs.rp;
			return *this;
		}
		SharedPtr & operator = (T *p) {
			rp = new U_Ptr<T>(p);
			return *this;
		}

		T & operator * () {
			assert(*this);
			return *(rp->p);
		}
		T * operator -> () {
			return rp->p;
		}
		const T *operator -> () const {
			return rp->p;
		}

		~SharedPtr() {
			if (!--rp->cnt) delete rp;
		}
	private:
		U_Ptr<T> *rp;
	};
	
	// std::unique_ptr implement
	template<typename T>
	struct DefaultDeleter {
		void operator() (T *p) {
			if (p) {
				delete p;
				p = NULL;
			}
		}
	};

	template<typename T, typename Deleter = DefaultDeleter<T>>
	class UniquePtr {
	public:
		UniquePtr(T *ptr = NULL) : rp(ptr) {}
		~UniquePtr() {
			del();
		}

	private:
		//not allow copyable
		UniquePtr(const UniquePtr &);
		UniquePtr & operator = (const UniquePtr &);

	public:
		void reset(T *p) {
			del();
			rp = p;
		}
		T * release() {
			T *p = rp;
			rp = NULL;
			return p;
		}
		T * get() {
			return rp;
		}

	public:
		operator bool() const {
			return NULL != rp;
		}
		T & operator * () {
			assert(*this);
			return *rp;
		}
		T * operator -> () {
			return &*(*this);
		}
		const T *operator -> () const {
			return rp->p;
		}

	private:
		T *rp;
		Deleter m_deleter;

		void del() {
			if (*this) {
				m_deleter(rp);
				rp = NULL;
			}
		}
	};

	template<typename T> __forceinline T *AllocAligned(uint32_t count) {
		return (T *)_aligned_malloc(count * sizeof(T), AYA_L1_CACHE_LINE_SIZE);
	}
	void FreeAligned(void *ptr);

	template <typename T> class BlockedArray {
	public:
		T *m_data;
		uint32_t u_res, v_res;

	public:
		BlockedArray() {
			m_data = NULL;
		}
		BlockedArray(uint32_t nu, uint32_t nv) {
			init(nu, nv);
		}
		~BlockedArray() {
			free();
		}

		void init(uint32_t nu, uint32_t nv) {
			u_res = nu;
			v_res = nv;
			auto roundUp = [this](const uint32_t x) {
				return (x + 3) & ~(3);
			};
			uint32_t n_alloc = roundUp(u_res) * roundUp(v_res);
			m_data = AllocAligned<T>(n_alloc);
			for (uint32_t i = 0; i < n_alloc; ++i)
				new (&m_data[i]) T();
		}
		void free() {
			for (uint32_t i = 0; i < u_res * v_res; ++i)
				m_data[i].~T();
			FreeAligned(m_data);
		}
		__forceinline uint32_t linearSize() const {
			return v_res * u_res;
		}
		__forceinline T &operator()(uint32_t u, uint32_t v) {
			return m_data[u * v_res + v];
		}
		__forceinline const T &operator()(uint32_t u, uint32_t v) const {
			return m_data[u * v_res + v];
		}
		__forceinline const T* data() const {
			return m_data;
		}
	};
}

#endif