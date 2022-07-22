
/*  © Copyright CERN, 2015. All rights not expressly granted are reserved.

    The stub of this file was generated by quasar (https://github.com/quasar-team/quasar/)

    Quasar is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public Licence as published by
    the Free Software Foundation, either version 3 of the Licence.
    Quasar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public Licence for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with Quasar.  If not, see <http://www.gnu.org/licenses/>.


 */


#ifndef __DSdoVariable__H__
#define __DSdoVariable__H__

#include <Base_DSdoVariable.h>

namespace Device
{

    class DBus;
    class DNode;

class
    DSdoVariable
    : public Base_DSdoVariable
{

public:
    /* sample constructor */
    explicit DSdoVariable (
        const Configuration::SdoVariable& config,
        Parent_DSdoVariable* parent
    ) ;
    /* sample dtr */
    ~DSdoVariable ();

    /* delegators for
    cachevariables and sourcevariables */

    /* ASYNCHRONOUS !! */
    UaStatus readValue (
        UaVariant& value,
        UaDateTime& sourceTime
    );
    /* ASYNCHRONOUS !! */
    UaStatus writeValue (
        UaVariant& value
    );

    /* delegators for methods */

private:
    /* Delete copy constructor and assignment operator */
    DSdoVariable( const DSdoVariable& other );
    DSdoVariable& operator=(const DSdoVariable& other);

    // ----------------------------------------------------------------------- *
    // -     CUSTOM CODE STARTS BELOW THIS COMMENT.                            *
    // -     Don't change this comment, otherwise merge tool may be troubled.  *
    // ----------------------------------------------------------------------- *

public:
    void initialize (DBus* bus, DNode* node);

    void setIndex (uint16_t index);

    std::string name() const { return m_name; }

private:
    uint16_t m_index;
    uint16_t m_subIndex;

    //! We need this references for different stuff like locks etc and can't get it via getParent
    DBus* m_bus;
    DNode* m_node;

    const std::string m_name;

    const bool m_expeditedReadTimeoutInheritsGlobal;
    unsigned int m_expeditedReadTimeoutSecondsFromConfig; // valid only if not global
    const bool m_expeditedWriteTimeoutInheritsGlobal;
    unsigned int m_expeditedWriteTimeoutSecondsFromConfig; // valid only if not global


};

}

#endif // __DSdoVariable__H__