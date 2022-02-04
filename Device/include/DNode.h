
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


#ifndef __DNode__H__
#define __DNode__H__

#include <Definitions.hpp>
#include <NodeGuarding.hpp>
#include <Sdo.h>

#include <stdint.h>

#include <Base_DNode.h>

#include <CanMessage.h>

#include <NodeStateEngine.hpp>

namespace Device
{

class
    DNode
    : public Base_DNode
{

public:
    /* sample constructor */
    explicit DNode (
        const Configuration::Node& config,
        Parent_DNode* parent
    ) ;
    /* sample dtr */
    ~DNode ();

    /* delegators for
    cachevariables and sourcevariables */
    /* Note: never directly call this function. */
    UaStatus writeRequestedState ( const UaString& v);


    /* delegators for methods */
    UaStatus callReset (
        OpcUa_Boolean& bootupReceived
    ) ;

private:
    /* Delete copy constructor and assignment operator */
    DNode( const DNode& other );
    DNode& operator=(const DNode& other);

    // ----------------------------------------------------------------------- *
    // -     CUSTOM CODE STARTS BELOW THIS COMMENT.                            *
    // -     Don't change this comment, otherwise merge tool may be troubled.  *
    // ----------------------------------------------------------------------- *

public:
    void initialize();
    
    // the generic reception handler for this Node
    void onMessageReceived (const CanMessage& msg);
    void onTpdoReceived (const CanMessage& msg);
    void tick();

    CANopen::SdoEngine& sdoEngine() { return m_sdoEngine; }

    void addNodeStateChangeCallBack(CANopen::NodeStateChangeCallBack callBack) { m_nodeStateEngine.addNodeStateChangeNotification(callBack); }

    void publishState (uint8_t rawState, CANopen::NodeState state);

    //! To be called just before the server emits sync
    void notifySync ();

    const CANopen::NodeStateEngine& nodeStateEngine () const { return m_nodeStateEngine; }

private:
    void onBootupReceived ();
    void onEmergencyReceived (const CanMessage& msg);

    std::chrono::steady_clock::time_point m_lastNodeGuardingTimePoint;
    CANopen::NodeState m_previousState;
    CANopen::NodeGuardingOperationsState m_nodeGuardingOperationsState;
    CANopen::StateInfoModel m_stateInfoModel;
    CANopen::SdoEngine m_sdoEngine;
    
    std::vector<CANopen::NodeStateChangeCallBack>  m_nodeStateChangeCallBacks;

    CANopen::NodeStateEngine m_nodeStateEngine;

};

}

#endif // __DNode__H__