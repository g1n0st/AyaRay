#ifndef AYA_CORE_MEMORY_H
#define AYA_CORE_MEMORY_H

#include "config.h"

#include <atomic>

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
		T *rp;

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
	__forceinline SharedPtr<T> MakeShared(TArgs&&... Args)
	{
		return SharedPtr<T>(new T(std::forward<TArgs>(Args)...));
	}
	template <typename T, typename... TArgs>
	__forceinline UniquePtr<T> MakeUnique(TArgs&&... Args)
	{
		return UniquePtr<T>(new T(std::forward<TArgs>(Args)...));
	}

	template<typename T> __forceinline T *AllocAligned(uint32_t count) {
		return (T *)_aligned_malloc(count * sizeof(T), AYA_L1_CACHE_LINE_SIZE);
	}
	void FreeAligned(void *ptr);

	template<class T>
	static __forceinline void SafeDelete(T*& ptr) {
		if (ptr != NULL) {
			delete ptr;
			ptr = NULL;
		}
	}

	template<class T>
	static __forceinline void SafeDeleteArray(T*& ptr) {
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

		void init(uint32_t nu, uint32_t nv, const T* data = nullptr) {
			u_res = nu;
			v_res = nv;
			auto roundUp = [this](const uint32_t x) {
				return (x + 3) & ~(3);
			};
			uint32_t n_alloc = roundUp(u_res) * roundUp(v_res);
			m_data = AllocAligned<T>(n_alloc);
			for (uint32_t i = 0; i < n_alloc; ++i)
				new (&m_data[i]) T();

			if (data) {
				for (uint32_t i = 0; i < u_res * v_res; i++)
					m_data[i] = data[i];
			}
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
		__forceinline int x() const {
			return u_res;
		}
		__forceinline int y() const {
			return v_res;
		}
		__forceinline int u() const {
			return u_res;
		}
		__forceinline int v() const {
			return v_res;
		}
	};
}

#endif