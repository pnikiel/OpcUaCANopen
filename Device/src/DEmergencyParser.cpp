
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


#include <Configuration.hxx> // TODO; should go away, is already in Base class for ages

#include <DEmergencyParser.h>
#include <ASEmergencyParser.h>

#include <Logging.hpp>

using namespace Logging;

namespace Device
{
// 1111111111111111111111111111111111111111111111111111111111111111111111111
// 1     GENERATED CODE STARTS HERE AND FINISHES AT SECTION 2              1
// 1     Users don't modify this code!!!!                                  1
// 1     If you modify this code you may start a fire or a flood somewhere,1
// 1     and some human being may possible cease to exist. You don't want  1
// 1     to be charged with that!                                          1
// 1111111111111111111111111111111111111111111111111111111111111111111111111






// 2222222222222222222222222222222222222222222222222222222222222222222222222
// 2     SEMI CUSTOM CODE STARTS HERE AND FINISHES AT SECTION 3            2
// 2     (code for which only stubs were generated automatically)          2
// 2     You should add the implementation but dont alter the headers      2
// 2     (apart from constructor, in which you should complete initializati2
// 2     on list)                                                          2
// 2222222222222222222222222222222222222222222222222222222222222222222222222

/* sample ctr */
DEmergencyParser::DEmergencyParser (
    const Configuration::EmergencyParser& config,
    Parent_DEmergencyParser* parent
):
    Base_DEmergencyParser( config, parent)

    /* fill up constructor initialization list here */
{
    /* fill up constructor body here */
}

/* sample dtr */
DEmergencyParser::~DEmergencyParser ()
{
}

/* delegates for cachevariables */



/* delegators for methods */

// 3333333333333333333333333333333333333333333333333333333333333333333333333
// 3     FULLY CUSTOM CODE STARTS HERE                                     3
// 3     Below you put bodies for custom methods defined for this class.   3
// 3     You can do whatever you want, but please be decent.               3
// 3333333333333333333333333333333333333333333333333333333333333333333333333

void DEmergencyParser::onEmergencyReceived (const CanMessage& msg)
{
    uint16_t errorCode = msg.c_data[0] | (msg.c_data[1] << 8);
    getAddressSpaceLink()->setLastErrorCode(errorCode, OpcUa_Good);
    getAddressSpaceLink()->setLastErrorRegister(msg.c_data[2], OpcUa_Good);
    getAddressSpaceLink()->setLastErrorByte3(msg.c_data[3], OpcUa_Good);
    getAddressSpaceLink()->setLastErrorByte4(msg.c_data[4], OpcUa_Good);
    getAddressSpaceLink()->setLastErrorByte5(msg.c_data[5], OpcUa_Good);
    getAddressSpaceLink()->setLastErrorByte6(msg.c_data[6], OpcUa_Good);
    getAddressSpaceLink()->setLastErrorByte7(msg.c_data[7], OpcUa_Good);

    // FE2.1: Count emergencies
    getAddressSpaceLink()->setEmergencyErrorCounter(getAddressSpaceLink()->getEmergencyErrorCounter()+1, OpcUa_Good);

    //TODO: we should print that we received emergency messages with error severity ERR
    // do it!
    LOG(Log::ERR, "Emergency") << wrapId(getFullName()) << ": received emergency, code 0x" << wrapValue(Utils::toHexString(errorCode)) << 
        ", counter for this node is " << wrapValue(std::to_string(getAddressSpaceLink()->getEmergencyErrorCounter()));
}

}