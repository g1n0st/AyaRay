#ifndef AYA_MEMORY_H
#define AYA_MEMORY_H

#include "config.h"
#include "parallel.h"

namespace Aya {
	template <typename T>
	class SharedPtr;

	/** @brief Assist class for SharedPtr */
	template <typename T>
	class U_Ptr
	{
	public:
		friend class SharedPtr<T>;

		U_Ptr(T *ptr) : p(ptr), cnt(1) {}
		~U_Ptr() { delete p; }

		atom32_t cnt;
		T *p;
	};

	/** @brief Auto count references pointer class, copy and release memory automatically */
	template <typename T>
	class SharedPtr
	{
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
}

#endif