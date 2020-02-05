#ifndef AYA_MEMORY_H
#define AYA_MEMORY_H

#include "config.h"

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

		uint32_t cnt;
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
}

#endif