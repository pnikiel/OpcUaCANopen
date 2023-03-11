
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


#ifndef __DStateSwitcher__H__
#define __DStateSwitcher__H__

#include <Base_DStateSwitcher.h>
#include <Definitions.hpp>

namespace Device
{

class
    DStateSwitcher
    : public Base_DStateSwitcher
{

public:
    /* sample constructor */
    explicit DStateSwitcher (
        const Configuration::StateSwitcher& config,
        Parent_DStateSwitcher* parent
    ) ;
    /* sample dtr */
    ~DStateSwitcher ();

    /* delegators for
    cachevariables and sourcevariables */


    /* delegators for methods */
    UaStatus callGoToPreOperational (
        const UaString&  nodeNamePattern,
        UaString& info,
        std::vector<UaString>& affectedNodes
    ) ;
    UaStatus callGoToOperational (
        const UaString&  nodeNamePattern,
        UaString& info,
        std::vector<UaString>& affectedNodes
    ) ;

private:
    /* Delete copy constructor and assignment operator */
    DStateSwitcher( const DStateSwitcher& other );
    DStateSwitcher& operator=(const DStateSwitcher& other);

    // ----------------------------------------------------------------------- *
    // -     CUSTOM CODE STARTS BELOW THIS COMMENT.                            *
    // -     Don't change this comment, otherwise merge tool may be troubled.  *
    // ----------------------------------------------------------------------- *

public:

    //! affectedNodes is an optional output.
    void requestSwitch(const std::string& pattern, CANopen::NodeState intendedState, std::vector<UaString>* affectedNodes = nullptr);
private:



};

}

#endif // __DStateSwitcher__H__