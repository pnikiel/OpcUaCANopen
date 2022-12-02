#include <iostream>

#include <CtrlZTable.hpp>

#include <DRoot.h>
#include <DBus.h>
#include <DNode.h>
#include <DEmergencyParser.h>
#include <Utils.h>
#include <ASBus.h>
#include <ASNode.h>
#include <ASEmergencyParser.h>

#include <fort.hpp>

void printCtrlZTable ()
{
    fort::char_table table;
    table.set_border_style(FT_BOLD_STYLE);

    table << fort::header << "(Bus) Object" << "CAN state" << "#toErr" << "txRate" << "rxRate" << "#totalTx" << "#totalRx" << fort::endr;
    table << fort::header << "(Node) Object" << "CANopen state" << "#Bootups" << "#Emergs" << "-" << "-" << "-" << fort::endr;

    std::cout << std::endl
              << Quasar::TermColors::ForeGreen() << "*** Ctrl-Z: list info ***" << Quasar::TermColors::StyleReset() 
              << std::endl;
    for (Device::DBus *bus : Device::DRoot::getInstance()->buss())
    {
        OpcUa_UInt32 portError (0);
        std::string portErrorStr;
        if (bus->getAddressSpaceLink()->getPortError(portError) == OpcUa_Good)
        {
            portErrorStr = std::to_string(portError);
        }
        else
            portErrorStr = "?"; // from null
        
        CanModule::CanStatistics statistics;
        bus->getStatistics(statistics);

        table << "(Bus) " + bus->getFullName() << portErrorStr << bus->getAddressSpaceLink()->getStatsTransitionsIntoErrorCounter() << 
            statistics.txRate () << statistics.rxRate() << statistics.totalTransmitted() << statistics.totalReceived() <<  fort::endr;
        for (Device::DNode *node : bus->nodes())
        {
            table << 
                "---> " + node->getFullName() << 
                CANopen::stateEnumToText(node->nodeStateEngine().currentState()) <<
                node->getAddressSpaceLink()->getBootupCounter() << 
                node->emergencyparsers()[0]->getAddressSpaceLink()->getEmergencyErrorCounter() << "-" << "-" << "-" << fort::endr;
            
            table[table.cur_row()-1][1].set_cell_content_text_style(fort::text_style::bold);
            if (node->nodeStateEngine().currentState() == CANopen::NodeState::DISCONNECTED)
                table[table.cur_row()-1][1].set_cell_content_bg_color(fort::color::red);
            else if (node->nodeStateEngine().currentState() == CANopen::NodeState::OPERATIONAL)
                table[table.cur_row()-1][1].set_cell_content_fg_color(fort::color::green);
        }
        table << fort::separator;
    }
    std::cout << table.to_string() << std::endl;
}