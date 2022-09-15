/**
 * @file SharedArray.h
 *
 * @author Manuel Tobias Schiller <manuel.schiller@nikhef.nl>
 * @date 2012-08-29
 */
#ifndef _SHAREDARRAY_H
#define _SHAREDARRAY_H

#include <vector>

#include <TObject.h>

/** @brief class to hold array data, refcounted
 *
 * @author Manuel Tobias Schiller <manuel.schiller@nikhef.nl>
 * @date 2012-08-30
 */
template <class TYPE>
class SharedArrayImp : public TObject
{
    public:
    /// for ROOT I/O
    SharedArrayImp() : refcount(0)
        { }
    /// constructor
    SharedArrayImp(unsigned size);
    /// copy constructor
    SharedArrayImp(const SharedArrayImp<TYPE>& other);
    /// clone method
    virtual SharedArrayImp<TYPE>* Clone(
        const char* newname = 0) const;
    /// destructor
    virtual ~SharedArrayImp();

    /// return reference count
    unsigned refCount() const { return refcount; }
    /// acquire a reference
    SharedArrayImp<TYPE>* acquire()
    { ++refcount; return this; }
    /// release a reference
    void release() { if (!--refcount) delete this; }

    /// return size of array
    unsigned size() const { return arr.size(); }
    /// read-only access
    const TYPE& operator[](unsigned idx) const
    { return arr[idx]; }
    /// read-write access
    TYPE& operator[](unsigned idx)
    { return arr[idx]; }
    private:
    /// reference count
    unsigned refcount;
    /// array
    std::vector<TYPE> arr;

    ClassDef(SharedArrayImp, 1);
};

/** @brief copy on write TYPE array class to make cloning cheap
 * 
 * @author Manuel Tobias Schiller <manuel.schiller@nikhef.nl>
 * @date 2012-08-30
 */
template <class TYPE>
class SharedArray : public TObject
{
    public:
    // forward declarations
    class RWProxy;
    friend class RWProxy;
    private:
    /// for ROOT I/O
    SharedArray() : pimpl(0) { }
    public:
    /// constructor
    SharedArray(unsigned sz) :
        pimpl(sz ? (new SharedArrayImp<TYPE>(sz)) : 0)
        { }
    /// copy constructor
    SharedArray(const SharedArray<TYPE>& other) :
        TObject(other), pimpl(other.pimpl->acquire())
        { }
    /// clone method
    virtual SharedArray<TYPE>* Clone(const char* newname = 0) const;
    /// destructor
    virtual ~SharedArray(); 
    /// assignment
    SharedArray& operator=(const SharedArray<TYPE>& other);
    /// get size of array
    unsigned size() const { return pimpl ? pimpl->size() : 0u; }
    /// read-only access to array elements
    const RWProxy operator[](unsigned idx) const
    { return RWProxy(const_cast<SharedArray<TYPE>*>(this), idx); }
    /// read-write access to array elements
    RWProxy operator[](unsigned idx)
    { return RWProxy(this, idx); }

    private:
    /// trigger copy-on-write
    void triggerCOW()
    {
        SharedArrayImp<TYPE> *oldpimpl = pimpl;
        pimpl = new SharedArrayImp<TYPE>(*pimpl);
        oldpimpl->release();
    }
    /// pointer to array implementation
    SharedArrayImp<TYPE>* pimpl;
    
    public:
    /** @brief proxy class to distinguish reads from writes
     *
     * @author Manuel Tobias Schiller <manuel.schiller@nikhef.nl>
     * @date 2012-08-30
     *
     * since it's only used within SharedArray itself and is
     * never persistified, there is no need to make ROOT aware of
     * it
     */
    class RWProxy
    {
        public:
        /// constructor
        RWProxy(SharedArray* par, unsigned i) :
            parent(par), idx(i)
            { }

        /// rvalue use
        operator TYPE() const
        { return parent->pimpl->operator[](idx); }

        /// lvalue use
        RWProxy& operator=(const TYPE d)
        {
            // trigger copy on write if we have to
            if (1 < parent->pimpl->refCount()) {
            // non-value-changing assignments should not
            // trigger deep copy
            if (d == parent->pimpl->operator[](idx))
                return *this;
            parent->triggerCOW();
            }
            // perform assignment
            parent->pimpl->operator[](idx) = d;
            return *this;
        }

        /// lvalue use
        RWProxy& operator=(const RWProxy& other)
        {
            if (&other == this) return *this;
            // don't need to work if we assign character to itself
            if (other.idx == idx &&
                other.parent->pimpl == parent->pimpl)
            return *this;
            return (*this = TYPE(other));
        }

        /// addition and assignment
        RWProxy& operator+=(const TYPE d)
        { return *this = TYPE(*this) + d; }
        /// addition and assignment
        RWProxy& operator+=(const RWProxy& other)
        { return *this += TYPE(other); }

        /// subtraction and assignment
        RWProxy& operator-=(const TYPE d)
        { return *this = TYPE(*this) - d; }
        /// subtraction and assignment
        RWProxy& operator-=(const RWProxy& other)
        { return *this -= TYPE(other); }

        /// multiplication and assignment
        RWProxy& operator*=(const TYPE d)
        { return *this = TYPE(*this) * d; }
        /// multiplication and assignment
        RWProxy& operator*=(const RWProxy& other)
        { return *this *= TYPE(other); }

        /// division and assignment
        RWProxy& operator/=(const TYPE d)
        { return *this = TYPE(*this) * d; }
        /// division and assignment
        RWProxy& operator/=(const RWProxy& other)
        { return *this /= TYPE(other); }

        /// prefix increment
        RWProxy& operator++() { return *this += 1.; }
        /// prefix decrement
        RWProxy& operator--() { return *this -= 1.; }

        /// postfix increment
        TYPE operator++(int)
        { const TYPE d = *this; ++*this; return d; }
        /// postfix decrement
        TYPE operator--(int)
        { const TYPE d = *this; --*this; return d; }

        /// equality
        bool operator==(const TYPE d) const
        { return TYPE(*this) == d; }
        /// equality
        bool operator==(const RWProxy& other) const
        { return *this == TYPE(other); }

        /// non-equality
        bool operator!=(const TYPE d) const
        { return !(*this == d); }
        /// non-equality
        bool operator!=(const RWProxy& other) const
        { return !(*this == other); }

        /// comparison
        bool operator<(const TYPE d) const
        { return TYPE(*this) < d; }
        /// comparison
        bool operator<(const RWProxy& other) const
        { return *this < TYPE(other); }

        /// comparison
        bool operator>(const TYPE d) const
        { return TYPE(*this) > d; }
        /// comparison
        bool operator>(const RWProxy& other) const
        { return *this > TYPE(other); }

        /// comparison
        bool operator<=(const TYPE d) const
        { return !(*this > d); }
        /// comparison
        bool operator<=(const RWProxy& other) const
        { return !(*this > other); }

        /// comparison
        bool operator>=(const TYPE d) const
        { return !(*this < d); }
        /// comparison
        bool operator>=(const RWProxy& other) const
        { return !(*this < other); }

        private:
        SharedArray* parent;
        unsigned idx;
    };


    private:
    ClassDef(SharedArray, 1);
};

#ifdef __GCCXML__
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

#endif // _SHAREDARRAY_H

// vim: ft=cpp:sw=4:tw=78
