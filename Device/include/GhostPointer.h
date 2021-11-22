/*
 * GhostReference.h
 *
 *  Created on: 16 Aug 2018
 *      Author: pnikiel
 */

#ifndef DEVICE_INCLUDE_GHOSTPOINTER_H_
#define DEVICE_INCLUDE_GHOSTPOINTER_H_

#include <stdexcept>
#include <memory>
#include <string>
#include <LogIt.h>

class GhostException: public std::runtime_error
{
public:
    GhostException(): std::runtime_error("GhostPointer: resource-not-available") {}
    GhostException(const std::string& s): std::runtime_error(s) {}
};

/*
 * Note: if the pointers are atomic (which normally should be the case,
 * this reference is re-entrant and threadsafe.
 */

template<typename T>
class GhostPointer
{
public:
    GhostPointer() :
        m_internal( new Internal() )
    {
        LOG(Log::TRC) << "Create GhostPointer internal=" << m_internal.get();
    }

    T* operator-> () const {
        if (m_internal->m_accessEnabled && m_internal->m_ptr)
            return m_internal->m_ptr;
        else
        {
            LOG(Log::TRC) << "GhostPointer: resource identified by internal " << m_internal.get() << " not available";
            throw GhostException(); }
        }

    T& operator* () const { if (m_internal->m_accessEnabled && m_internal->m_ptr) return *(m_internal->m_ptr); else throw GhostException(); }

    void assign( T* newptr )
    {
        if (m_internal->m_ptr)
            throw std::logic_error("Illegal to assign GhostPointer for the 2nd time!");
        m_internal->m_ptr = newptr;
    }

    void enableAccess ()
    {
        LOG(Log::TRC) << "GhostPointer: enabling access to resource identified by internal " << m_internal.get();
        m_internal->m_accessEnabled = true;
    }

    void disableAccess () { m_internal->m_accessEnabled = false; }

    bool isInitialized() const { return m_internal->m_ptr != nullptr; }

    T* get () const { return m_internal->m_ptr;}

private:

    struct Internal
    {
        Internal(): m_ptr(nullptr), m_accessEnabled(false) {}
        T*      m_ptr;
        bool    m_accessEnabled;
    };

    std::shared_ptr<Internal> m_internal;

};



#endif /* DEVICE_INCLUDE_GHOSTPOINTER_H_ */
