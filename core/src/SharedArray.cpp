/**
 * @author Manuel Tobias Schiller <manuel.schiller@nikhef.nl>
 * @date 2012-08-29
 */
#include "SharedArray.h"

template <class TYPE>
SharedArray<TYPE>::~SharedArray()
{ if (pimpl) pimpl->release(); }

template <class TYPE>
SharedArray<TYPE>* SharedArray<TYPE>::Clone(const char*) const
{ return new SharedArray<TYPE>(*this); }

template <class TYPE>
SharedArray<TYPE>& SharedArray<TYPE>::operator=(
    const SharedArray<TYPE>& other)
{
    if (other.pimpl == pimpl) return *this;
    TObject::operator=(other);
    SharedArrayImp<TYPE>* oldpimpl = pimpl;
    pimpl = other.pimpl->acquire();
    if (oldpimpl) oldpimpl->release();
    return *this;
}

template <class TYPE>
SharedArrayImp<TYPE>::SharedArrayImp(unsigned size) :
    refcount(1), arr(size)
{ }

template <class TYPE>
SharedArrayImp<TYPE>::~SharedArrayImp()
{ }

template <class TYPE>
SharedArrayImp<TYPE>* SharedArrayImp<TYPE>::Clone(const char*) const
{ return new SharedArrayImp<TYPE>(*this); }

template <class TYPE>
SharedArrayImp<TYPE>::SharedArrayImp(
    const SharedArrayImp<TYPE>& other) :
    TObject(other), refcount(1), arr(other.arr)
{ }

#if ! defined(__GCCXML__) && ! defined(__CINT__)
template class SharedArrayImp<char>;
template class SharedArrayImp<short>;
template class SharedArrayImp<int>;
template class SharedArrayImp<long>;
template class SharedArrayImp<long long>;
template class SharedArrayImp<unsigned char>;
template class SharedArrayImp<unsigned short>;
template class SharedArrayImp<unsigned int>;
template class SharedArrayImp<unsigned long>;
template class SharedArrayImp<unsigned long long>;
template class SharedArrayImp<float>;
template class SharedArrayImp<double>;
template class SharedArray<char>;
template class SharedArray<short>;
template class SharedArray<int>;
template class SharedArray<long>;
template class SharedArray<long long>;
template class SharedArray<unsigned char>;
template class SharedArray<unsigned short>;
template class SharedArray<unsigned int>;
template class SharedArray<unsigned long>;
template class SharedArray<unsigned long long>;
template class SharedArray<float>;
template class SharedArray<double>;
#endif // __GCCXML__

// vim: sw=4:tw=78:ft=cpp
