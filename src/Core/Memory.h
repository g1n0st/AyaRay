#ifndef AYA_CORE_MEMORY_H
#define AYA_CORE_MEMORY_H

#include <Core/Config.h>

#include <atomic>
#include <vector>

namespace Aya {
	
	// https://lokiastari.com/blog/2015/01/15/c-plus-plus-by-example-smart-pointer-part-ii/
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
		SharedPtr(T *ptr = nullptr) : rp(new U_Ptr<T>(ptr)) {}
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

	// https://lokiastari.com/blog/2015/01/23/c-plus-plus-by-example-smart-pointer-part-iii/
	template<typename T>
	class UniquePtr {
	private:
		T *rp = nullptr;

	public:
		UniquePtr() : rp(nullptr) {}
		explicit UniquePtr(T *ptr) : rp(ptr) {}
		~UniquePtr() {
			delete rp;
		}

		UniquePtr(std::nullptr_t) : rp(nullptr) {}
		UniquePtr& operator = (std::nullptr_t) {
			reset();
			return *this;
		}

		UniquePtr(UniquePtr&& moving) noexcept {
			moving.swap(*this);
		}
		UniquePtr& operator = (UniquePtr&& moving) noexcept {
			moving.swap(*this);
			return *this;
		}

		// constructor/assignment for use with types derived from T
		template<typename U>
		UniquePtr(UniquePtr<U>&& moving) {
			UniquePtr<T> tmp(moving.release());
			tmp.swap(*this);
 		}
		template<typename U>
		UniquePtr& operator = (UniquePtr<U>&& moving) {
			UniquePtr<T> tmp(moving.release());
			tmp.swap(*this);
			return *this;
		}

		// remove compiler generated copy semantics
		UniquePtr(UniquePtr const&) = delete;
		UniquePtr& operator = (UniquePtr const&) = delete;

		void reset(T* ptr = nullptr) {
			T* oldptr = rp;
			rp = ptr;
			delete oldptr;
		}
		T * release() noexcept {
			T* result = nullptr;
			std::swap(result, rp);
			return result;
		}
		T * get() const {
			return rp;
		}
		void swap(UniquePtr& src) noexcept {
			std::swap(rp, src.rp);
		}

		explicit operator bool() const {
			return rp;
		}
		T & operator * () {
			assert(*this);
			return *rp;
		}
		T * operator -> () {
			return &*(*this);
		}
		const T *operator -> () const {
			return rp;
		}
	};

	template <typename T, typename... TArgs>
	AYA_FORCE_INLINE SharedPtr<T> MakeShared(TArgs&&... Args)
	{
		return SharedPtr<T>(new T(std::forward<TArgs>(Args)...));
	}
	template <typename T, typename... TArgs>
	AYA_FORCE_INLINE UniquePtr<T> MakeUnique(TArgs&&... Args)
	{
		return UniquePtr<T>(new T(std::forward<TArgs>(Args)...));
	}

	template<typename T> AYA_FORCE_INLINE T *AllocAligned(uint32_t count) {
		return (T *)_aligned_malloc(count * sizeof(T), AYA_L1_CACHE_LINE_SIZE);
	}
	void FreeAligned(void *ptr);

	template<class T>
	static AYA_FORCE_INLINE void SafeDelete(T*& ptr) {
		if (ptr != NULL) {
			delete ptr;
			ptr = NULL;
		}
	}

	template<class T>
	static AYA_FORCE_INLINE void SafeDeleteArray(T*& ptr) {
		if (ptr != NULL) {
			delete[] ptr;
			ptr = NULL;
		}
	}

	template <typename T> class BlockedArray {
	private:
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

		void init(uint32_t nu, uint32_t nv, const T* data) {
			u_res = nu;
			v_res = nv;
			auto roundUp = [this](const uint32_t x) {
				return (x + 3) & ~(3);
			};
			uint32_t n_alloc = roundUp(u_res) * roundUp(v_res);
			m_data = AllocAligned<T>(n_alloc);
			for (uint32_t i = 0; i < n_alloc; ++i)
				new (&m_data[i]) T();

			for (uint32_t i = 0; i < u_res * v_res; i++)
				m_data[i] = data[i];
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
		AYA_FORCE_INLINE uint32_t linearSize() const {
			return v_res * u_res;
		}
		AYA_FORCE_INLINE T &operator()(uint32_t u, uint32_t v) {
			return m_data[u * v_res + v];
		}
		AYA_FORCE_INLINE const T &operator()(uint32_t u, uint32_t v) const {
			return m_data[u * v_res + v];
		}
		AYA_FORCE_INLINE const T* data() const {
			return m_data;
		}
		AYA_FORCE_INLINE int x() const {
			return u_res;
		}
		AYA_FORCE_INLINE int y() const {
			return v_res;
		}
		AYA_FORCE_INLINE int u() const {
			return u_res;
		}
		AYA_FORCE_INLINE int v() const {
			return v_res;
		}
	};

	class MemoryPool {
	private:
		uint32_t m_size;
		uint32_t m_offset;
		uint8_t *mp_current;

		std::vector<uint8_t*> m_used, m_avail;

	public:
		MemoryPool(uint32_t size = 32768) {
			m_size = size;
			m_offset = 0;
			mp_current = AllocAligned<uint8_t>(m_size);
		}
		~MemoryPool() {
			FreeAligned(mp_current);
			for (auto p : m_used) FreeAligned(p);
			for (auto p : m_avail) FreeAligned(p);
		}

		template<class T>
		inline T* alloc(uint32_t count = 1) {
			uint32_t size = (count * sizeof(T) + 15) & (~15);

			if (m_offset + size > m_size) {
				m_used.emplace_back(mp_current);
				if (m_avail.size() > 0 && size < m_size) {
					mp_current = m_avail.back();
					m_avail.pop_back();
				}
				else {
					mp_current = AllocAligned<uint8_t>(Max(size, m_size));
				}
				m_offset = 0;
			}

			T *ret = (T*)(mp_current + m_offset);
			m_offset += size;

			return ret;
		}

		inline void freeAll() {
			m_offset = 0;
			while (m_used.size()) {
				m_avail.emplace_back(m_used.back());
				m_used.pop_back();
			}
		}
	};
}

#endif