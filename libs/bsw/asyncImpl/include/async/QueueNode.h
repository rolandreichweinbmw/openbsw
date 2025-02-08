// Copyright 2024 Accenture.

/**
 * \ingroup async
 */
#ifndef GUARD_1ED8DBD4_BEC2_4869_AC44_11E89BB45AD1
#define GUARD_1ED8DBD4_BEC2_4869_AC44_11E89BB45AD1

namespace async
{
template<typename T>
class QueueNode
{
public:
    QueueNode();

    bool isEnqueued() const;

    T* getNext() const;
    void setNext(T* next);

    void enqueue();
    T* dequeue();

private:
    T* _next;
};

/**
 * Inline implementations.
 */
template<typename T>
inline QueueNode<T>::QueueNode() : _next(reinterpret_cast<T*>(1U))
{}

template<typename T>
inline bool QueueNode<T>::isEnqueued() const
{
    return _next != reinterpret_cast<T*>(1U);
}

template<typename T>
inline T* QueueNode<T>::getNext() const
{
    return _next;
}

template<typename T>
inline void QueueNode<T>::setNext(T* const next)
{
    _next = next;
}

template<typename T>
inline void QueueNode<T>::enqueue()
{
    _next = nullptr;
}

template<typename T>
inline T* QueueNode<T>::dequeue()
{
    T* const prevNext = _next;
    _next             = reinterpret_cast<T*>(1U);
    return prevNext;
}

} // namespace async

#endif // GUARD_1ED8DBD4_BEC2_4869_AC44_11E89BB45AD1
