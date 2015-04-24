#ifndef comet_task_ptr_h__
#define comet_task_ptr_h__

#include <combaseapi.h>

namespace comet {
    template<typename T>
    class task_ptr {
    public:
        task_ptr() : _ptr(0) {}
        task_ptr(T* ptr) : _ptr(ptr) {}

        task_ptr( const task_ptr& other ); // non construction-copyable
        task_ptr( task_ptr&& other ); // non construction-movable
        task_ptr& operator=( const task_ptr& ); // non copyable
        task_ptr& operator=( task_ptr&& ); // non movable

        ~task_ptr() {
            free();
        }

        void free() {
            if (_ptr) {
                CoTaskMemFree(_ptr);
            }
            _ptr = 0;
        }

        bool alloc(size_t size) {
            free();
            _ptr = ::CoTaskMemAlloc(size);
            return _ptr != NULL;
        }

        bool realloc(size_t size) {
            _ptr = ::CoTaskMemRealloc(_ptr, size);
            return _ptr != NULL;
        }

        T* in() {
            return _ptr;
        }

        T** inout() {
            return &_ptr;
        }

        T** out() {
            free();
            return &_ptr;
        }

#ifdef _TYPEDEF_BOOL_TYPE
        typedef task_ptr<T> _Myt;
        _TYPEDEF_BOOL_TYPE;
        _OPERATOR_BOOL() const _NOEXCEPT
        {    // test for non-null pointer
            return (_ptr != 0 ? _CONVERTIBLE_TO_TRUE : 0);
        }
#else
        explicit operator bool() const
        {    // test for non-null pointer
            return _ptr != 0;
        }
#endif

        T& operator[](size_t i) {
            return _ptr[i];
        }
        const T& operator[](size_t i) const {
            return _ptr[i];
        }
        T* operator->() {
            return _ptr;
        }
        T operator*() {
            return _ptr;
        }
        
    private:
        T* _ptr;
    };
}

#endif // comet_task_ptr_h__
