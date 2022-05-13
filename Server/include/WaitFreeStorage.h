//@author pnikiel

#ifndef __WAITFREESTORAGE__
#define __WAITFREESTORAGE__

#include <atomic>

template<typename T>
class WaitFreeStorage
{

public:
    WaitFreeStorage ( unsigned int size ):
	m_whereToTryWrite(0),
    m_size(size)

    {
	m_slots = new Slot [size];
    }
	

    bool store ( const T& what )
    {
	unsigned int tryAt = m_whereToTryWrite.fetch_add( 1 ) % m_size;
	unsigned int attempts = m_size;
	while (attempts > 0)
	{
	    typename WaitFreeStorage<T>::Slot::State expected = WaitFreeStorage<T>::Slot::State::NOTHING;
	    typename WaitFreeStorage<T>::Slot::State desired = WaitFreeStorage<T>::Slot::State::TAKEN_CONTENTS_INVALID;
	    if (m_slots[tryAt].state.compare_exchange_strong( expected, desired ) == true)
			{
			// cmp-xchg successful, this slot is booked
			m_slots[tryAt].data = what;
			m_slots[tryAt].state = WaitFreeStorage<T>::Slot::State::TAKEN_CONTENTS_VALID;
			return true;
			}
	    tryAt = (tryAt+1) % m_size;
	    --attempts;
	}
	return false;

    }

    bool investigate( unsigned int position, T** out )
    {
	if (position >= m_size)
	    throw std::runtime_error("position-out-of-range");
	if ( m_slots[position].state.load() == WaitFreeStorage<T>::Slot::State::TAKEN_CONTENTS_VALID )
	{
	    *out = &m_slots[position].data;
	    return true;
	}
	else
	    return false;

    }

    bool pop ( unsigned int position )
    {
	if (position >= m_size)
	    throw std::runtime_error("position-out-of-range");
	if ( m_slots[position].state.load() == WaitFreeStorage<T>::Slot::State::TAKEN_CONTENTS_VALID )
	{
	    m_slots[position].state = WaitFreeStorage<T>::Slot::State::NOTHING;
	    return true;
	}
	else
	    return false;
    }

    unsigned int size() const { return m_size; }

private:

    struct Slot
    {
	enum State
	{
	    NOTHING = 0,
	    TAKEN_CONTENTS_INVALID = 1,
	    TAKEN_CONTENTS_VALID = 2
	};
	std::atomic<State> state;
	T data;
    Slot():
	state( NOTHING )
	    {}
    };

    // this keeps on rising each write attempt
    std::atomic<unsigned int> m_whereToTryWrite;
    const unsigned int m_size;

    Slot* m_slots ;
};


#endif // __WAITFREESTORAGE__
