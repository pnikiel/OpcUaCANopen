
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


#ifndef __DTpdoMultiplex__H__
#define __DTpdoMultiplex__H__

#include <Base_DTpdoMultiplex.h>

#include <Enumerator.hpp>
#include <CanMessage.h>

namespace Device
{

class
    DTpdoMultiplex
    : public Base_DTpdoMultiplex
{

public:
    /* sample constructor */
    explicit DTpdoMultiplex (
        const Configuration::TpdoMultiplex& config,
        Parent_DTpdoMultiplex* parent
    ) ;
    /* sample dtr */
    ~DTpdoMultiplex ();

    /* delegators for
    cachevariables and sourcevariables */


    /* delegators for methods */

private:
    /* Delete copy constructor and assignment operator */
    DTpdoMultiplex( const DTpdoMultiplex& other );
    DTpdoMultiplex& operator=(const DTpdoMultiplex& other);

    // ----------------------------------------------------------------------- *
    // -     CUSTOM CODE STARTS BELOW THIS COMMENT.                            *
    // -     Don't change this comment, otherwise merge tool may be troubled.  *
    // ----------------------------------------------------------------------- *

public:
    void initialize();

    void onReplyReceived(const CanMessage& msg);

    //! To be called just before the server emits sync
    void notifySync ();

    //! To be called when the bus goes down
    void propagateNull ();

    Enumerator::TpdoMultiplex::TransportMechanism transportMechanismEnum() const { return m_transportMechanism; }

private:
    const std::string m_name;
    Enumerator::TpdoMultiplex::TransportMechanism m_transportMechanism;



};

}

#endif // __DTpdoMultiplex__H__