#ifndef comet_com_ptr_array_h__
#define comet_com_ptr_array_h__

#include <comet_task_ptr.h>
#include <comet/ptr.h>

namespace comet {
    template<typename TInf, typename TCount = UINT32>
    class com_ptr_array {
    public:
        com_ptr_array() : _ptr(nullptr), _count(0) {}
        ~com_ptr_array() {
            clear();
        }


        TInf** in() {
            return _ptr.in();
        }

        TInf*** inout() {
            return _ptr.inout();
        }

        TInf*** out() {
            clear();
            return _ptr.out();
        }

        TCount count() {
            return _count;
        }

        TCount* inout_count() {
            return &_count;
        }

        TCount* out_count() {
            clear();
            return &_count;
        }

        com_ptr<TInf> operator[](size_t i) {
            return com_ptr<TInf>(_ptr[i]);
        }



    private:
        void clear() {
            if (_ptr) {
                for (DWORD i = 0; i < _count; i++) {
                    _ptr[i]->Release();
                }
                _count = 0;
                _ptr.free();
            }
        }

        task_ptr<TInf*> _ptr;
        TCount _count;
    };
}

#endif // comet_com_ptr_array_h__