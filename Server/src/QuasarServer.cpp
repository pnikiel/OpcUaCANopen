/* © Copyright CERN, Universidad de Oviedo, 2015.  All rights not expressly granted are reserved.
 * QuasarServer.cpp
 *
 *  Created on: Nov 6, 2015
 * 		Author: Damian Abalo Miron <damian.abalo@cern.ch>
 *      Author: Piotr Nikiel <piotr@nikiel.info>
 *
 *  This file is part of Quasar.
 *
 *  Quasar is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public Licence as published by
 *  the Free Software Foundation, either version 3 of the Licence.
 *
 *  Quasar is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public Licence for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with Quasar.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <signal.h>
#include <thread>
#include <iomanip>

#include "QuasarServer.h"
#include <LogIt.h>
#include <shutdown.h>


#include <fort.hpp>

#include <DRoot.h>
#include <DBus.h>
#include <DNode.h>
#include <DEmergencyParser.h>
#include <DOnlineConfigValidator.h>
#include <Sdo.h>

#include <ASBus.h>
#include <ASNode.h>
#include <ASEmergencyParser.h>

#include <Configuration.hxx>
#include <Configurator.h>
#include <ConfigurationDecorationUtils.h>

#include <Warnings.hxx>
#include <Utils.h>
#include <Logging.hpp>

using namespace Configuration;
using namespace Logging;

namespace po = boost::program_options;

struct LogComponent
{
    std::string name;
    std::string logLevelFromArgs;
    Log::LogComponentHandle handle;
    LogComponent(std::string _name) : name(_name) {}
};

std::vector<LogComponent> LogComponents =
    {
        {"CanModule"},
        {"Emergency"},
        {"NodeMgmt"},
        {"Sdo"},
        {"Spooky"},
        {"Spy"},
        {"SdoValidator"}
    };


void printLogo()
{

    // std::cout << " \

    //                                  ▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄ \
    //                            ▄▄▄▄▄▄▓▓███████████████████████▓▓▄▄▄▄▄▄  \
    //                        ▄▄▄▄▓████████████▀▀▀▀▀▀▀       ▀▀▀▀▀▀▀████▓▄▄▄▄  \
    //                      ▄▓▓███████████▀▀▀                        ▀▀▀█████▄▄    \
    //                   ▄▓████████████▌                                  ▀█████▓▄   \
    //                ▄▄▓███▌▀ ▀▓██████▓▓▄                                   ▐▀███▓▄▄   \
    //              ▄▄▓███▌▀░▄▄▓▓▀▀    ▀▀░▓▄▄                                  ▀▀███▓▄▄   \
    //            ▄▄▓█████▓▄▓▓▓▀          ▀▀░▄▄▄                                 ▀▀███▓▄▄  \
    //          ▄▄▓████████▓▌▀              ▀▀▀▓▄▄                       ▄▄▄▄▄▄▄▄▄▓█████▓▄
    //         ▄▓███▀▀▓████▀                   ▀▀░▄▄              ▄▄▄▄▄▄▄▓▓███████████████▌▄
    //        ▄▓██▌  ▐▓███░                      ▀▀▀         ▄▄▄▄▓███████▀▀▀▀▀▀▀▀   ▀▀██████▄
    //       ▄███▀   ▐▓███░                             ▄▄▄▓███████▀▀▀                ▐▓█████▄
    //      ▄▓██▀   ▐▓████▌                        ▄▄▓▓▓█████▀                         ▐█████▓▌
    //     ▄███▌   ▄▓░▀███▓▄                  ▄▄▄▄█████▀▀▀▀                            ▐███████▄
    //    ▐▓██▀   ▐▓▌  ▐███▓              ▄▄▄▄▓███▀▀ ▀    ▄▄                         ▄▄▓███▓███▌
    //    ▐██▌    ▐▓▌   ▐███▌         ▄▄▄▓▓███▀▀▀▀        ▀░▄▄                     ▄▄▓▓██▀▀░▓███▓
    //   ▐▓██░   ▄░░     ▀██▌▄    ▄▄▄▓▓███▀▀                ▀▀▄▄                 ▄▄▓▀▀▀    ░▓███▓░
    //  ▐▓███   ▐▓░       ▓██▓▄▄▄▄▓████▀▀                    ▐▓▓░              ▄▓█▀▀      ▐░▓▓▓██▌
    //  ▐▓██▌  ▐▓█░       ▐█████████▀                         ▐█▓▄          ▄░▓▓▀         ▐░░░▓██▌
    //  ▐███▌  ▐▓█        ▓██████▀                              ▐▓▌      ▄▓▓▓▀            ▐░░░▐██▌
    //  ▐███▓  ▐▐▌     ▄▄████████▌                               ▐▓▓▄▄▄▄▓▓▀▀               ░  ▐██▌
    //  ▐███▓░  ▐░▄ ▄▄▓███▀▀▀▀▀██▓▄                              ▐▓▓██▀▀▀               ▐░    ▐██▌
    //  ▐███▓░ ▐▓▓▓▓▓███▀▀     ▀██▓▄                         ▄▄▄▄░▓██▌                  ▓▓    ▓██▌
    //  ▐▓███▓ ▐▓████▀▀         ▀███▄                    ▄▄▄░░▀▀▀▀ ▀▀░▄▄               ▐▓▌   ▐▓██▌
    //   ▐▓██▓▄▓███▀▀            ▐███▄▄              ▄▄▄░▀▀▀▀▀       ▐▓▌               ▐▓▌   ▐███░
    //    ▐███████░               ▀███▓▄          ▄▓▓▓▀▀▀             ▀█░             ▄░▀▀  ▐▓██▀
    //    ▐███████                  ▀██▓▄    ▄▓▓▓▓░▀                   ▀░▄           ▐▓░    ▐██▌
    //     ▀██████▌                  ▀▓██▓▄                             ▐▓▌          ▐█░   ▄▓██░
    //     ▐▓█████▓▄        ▄▄▄▄▄▄▄▄   ▀██▓▄                            ▐█▌        ▄▄░▀   ▐███▀
    //      ▐██████▓▓▄▄▄▄▄▄ ░▀▀▀▀▀▀▀    ▀▓██▓▄                           ▀░       ▐▓▌    ▄▓██▀
    //      ▐▓████████▀▀ ▀▀               ▀███▓▄                         ▐░░▄   ▄▄░▀▀  ▐▓███▀
    //       ▐███████▌                      ▀█▀▀                          ▐▒▓▄▄▄▓█▀   ▄▓██▌▀
    //        ▀▀██████▓▄                                                  ▐▓██▓▓▀   ▄▓███▀
    //         ▐▀██████▓▄                    ▄▓████▓▄▄                    ▐███▌   ▐▓███▀
    //           ▐███████▓▄                 ▐████████▓▄                 ▄▄▓██▌ ▐▄▓███▀▀
    //             ▀████████▄▄▄▄▄           ▐█████████░▄▄▄▄▄         ▄▄▄▓█████▓▓███▀▀
    //               ▀▀█████████▓▓▄▄▄▄▄▄    ▐███████▀▀▀ ▐███▓▄▄▄▄▄▄▄▄▓▓█████████▀▀
    //                 ▀▀▀█████████████▓▄▄▄▄▓██████▀     ▀████████████▓▓██████▀
    //                    ▀▀▀█████████████████████▄▄▄▄▄▄▄▄▄██████████████████▀
    //                         ▀▀█████████████████████████████████████████▀▀
    //                             ▀███████████████████████████████████▀▀
    //                             ▐▓███████████████████████████████▀▀
    //                              ▐██████████████████▀▀▀▀▀▀▀▀▀▀▀▀
    //                              ▐█████████████████▀
    //                              ▐████████████████▌
    //                              ▐▓██████████████▌
    //                               ▓██████████████
    //                              ▐██████████████▌" << std::endl;


}


QuasarServer::QuasarServer() : BaseQuasarServer()
{
    printLogo();

    // the check for LittleEndian arch. Reason for that is most ValueMapper algorithms base on this assumption.
    uint32_t checkWordU32 = 0x12345678;
    uint8_t* checkPtr = reinterpret_cast<uint8_t*>(&checkWordU32);
    if (*checkPtr != (checkWordU32 & 0xff))
    {
        throw std::runtime_error("It seems that you're running this program on an architecture different from LittleEndian. With current ValueMapping algorithms this would deliver wrong results. Consider contributing or contact Piotr.");
    }
}

QuasarServer::~QuasarServer()
{
}

void QuasarServer::mainLoop()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    printNiceSummary();
    printServerMsg("Press " + std::string(SHUTDOWN_SEQUENCE) + " to shutdown server");

    // Wait for user command to terminate the server thread.

    while (ShutDownFlag() == 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        for (Device::DBus *bus : Device::DRoot::getInstance()->buss())
        {
            bus->tick();
        }
    }
    printServerMsg(" Shutting down server");
}

QuasarServer *quasarServer(nullptr);
void sigAction(int)
{
    quasarServer->signalAction();
}

void QuasarServer::initialize()
{
    LOG(Log::INF) << "Initializing Quasar server.";

    // TODO make it a separate method
    for (auto &logComponent : LogComponents)
    {
        if (logComponent.logLevelFromArgs != "")
        {
            Log::LOG_LEVEL level;
            if (!Log::logLevelFromString(logComponent.logLevelFromArgs, level))
            {
                throw std::runtime_error("Failed parsing log level [" + logComponent.logLevelFromArgs + "] received from the command line");
            }
            if (Log::setComponentLogLevel(logComponent.handle, level))
            {
                LOG(Log::INF) << "Note: overriden log component [" << logComponent.name << "] to [" << logComponent.logLevelFromArgs << "]";
            }
        }
    }

    for (Device::DBus *bus : Device::DRoot::getInstance()->buss())
        bus->initialize();

    quasarServer = this;
    signal(SIGTSTP, sigAction);
}

void QuasarServer::shutdown()
{
    LOG(Log::INF) << "Shutting down Quasar server.";
    LOG(Log::INF) << "Stopping processing CAN buses";
    for (Device::DBus *bus : Device::DRoot::getInstance()->buss())
    {
        bus->shutDown();
    }
}

void QuasarServer::initializeLogIt()
{
    BaseQuasarServer::initializeLogIt();
    for (auto &logComponent : LogComponents)
        logComponent.handle = Log::registerLoggingComponent(logComponent.name);
    LOG(Log::INF) << "Logging initialized.";
}

bool QuasarServer::overridableConfigure(const std::string &fileName, AddressSpace::ASNodeManager *nm)
{
    return configure(fileName, nm, std::bind(&QuasarServer::processConfiguration, this, std::placeholders::_1));
}

void QuasarServer::processMultiplexedChannelConfigurationGenerator(Configuration::Configuration &configuration)
{
    for (Bus &bus : configuration.Bus())
    {
        for (Node &node : bus.Node())
        {
            for (TpdoMultiplex &multiplex : node.TpdoMultiplex())
            {
                for (MultiplexedChannelConfigurationGenerator &generator : multiplex.MultiplexedChannelConfigurationGenerator())
                {
                    if (generator.MultiplexedChannel().size() != 1)
                        throw std::runtime_error("Bad configuration: GenerateIdenticalChannels must contain precisely one MultiplexedChannel to server as a specimen.");
                    MultiplexedChannel &specimen = generator.MultiplexedChannel().front();

                    if (specimen.name() != "specimen")
                        throw std::runtime_error("Specimen must be called specimen ;-) ");
                    LOG(Log::INF) << "found GenerateIdenticalChannels!";
                    for (unsigned int chno = 0; chno < generator.numberOfChannels(); chno++)
                    {
                        // Note from Piotr:
                        // we first make a copy and then fix name and id,
                        // reason for it is for cleaner memory handling for temporary objects we'd have to create,
                        // now we let XSD-CXX to handle it for us, so one problem less.
                        DecorationUtils::push_back(
                            multiplex,
                            multiplex.MultiplexedChannel(),
                            specimen,
                            TpdoMultiplex::MultiplexedChannel_id);
                        multiplex.MultiplexedChannel().back().name() = "ch" + std::to_string(chno);
                        multiplex.MultiplexedChannel().back().id() = chno;
                    }
                }
                //! Generators has been run, throw them away.
                DecorationUtils::clear(multiplex, multiplex.MultiplexedChannelConfigurationGenerator(), TpdoMultiplex::MultiplexedChannelConfigurationGenerator_id);
            }
        }
    }
}

template <typename Tcoll, typename Tparent, typename Tchild>
static void ensureObjectPresent(Tcoll &collection, Tparent &parent, Tchild object, size_t id, const std::string &expectedName)
{
    if (collection.size() < 1)
    {
        DecorationUtils::push_back(
            parent,
            collection,
            object,
            id);
    }
    else
    {
        // for a user-supplied object make sure it is called properly as we can't force it via quasar
        if (collection[0].name() != expectedName)
            throw std::runtime_error("Error: user supplied " + expectedName + " has wrong name attribute [" + collection[0].name() + "]. Fix your configuration file.");
    }
}

void QuasarServer::processGlobalSettings(Configuration::Configuration &configuration)
{
    ensureObjectPresent(
        configuration.GlobalSettings(),
        configuration,
        Configuration::GlobalSettings("GlobalSettings"),
        Configuration::Configuration::GlobalSettings_id,
        "GlobalSettings");
}
void QuasarServer::processWarnings(Configuration::Configuration &configuration)
{
    ensureObjectPresent(
        configuration.Warnings(),
        configuration,
        Configuration::Warnings("Warnings"),
        Configuration::Configuration::Warnings_id,
        "Warnings");
    Warnings::validateWarnings();
    Warnings::passToConfiguration(configuration.Warnings()[0]);
    Warnings::logWarningsConfiguration(configuration.Warnings()[0]);
}

bool QuasarServer::processConfiguration(Configuration::Configuration &configuration)
{
    processGlobalSettings(configuration);
    processWarnings(configuration);
    processMultiplexedChannelConfigurationGenerator(configuration);

    // now let's move all SdoValidators to the OnlineValidator group
    for (Configuration::Bus& bus : configuration.Bus())
    {
        for (Configuration::Node& node : bus.Node())
        {
            if (node.SdoValidator().size() > 0)
            {
                ensureObjectPresent(
                    node.OnlineConfigValidator(),
                    node,
                    Configuration::OnlineConfigValidator("onlineConfigValidator"),
                    Configuration::Node::OnlineConfigValidator_id,
                    "onlineConfigValidator");
                for (Configuration::SdoValidator& sdoValidator : node.SdoValidator())
                {
                    DecorationUtils::push_back(
                        node.OnlineConfigValidator().front(),
                        node.OnlineConfigValidator().front().SdoValidator(),
                        sdoValidator,
                        Configuration::OnlineConfigValidator::SdoValidator_id);
                }
                DecorationUtils::clear(
                    node,
                    node.SdoValidator(),
                    Configuration::Node::SdoValidator_id);
            }

        }
    }

    return true;
}

void QuasarServer::appendCustomCommandLineOptions(
    boost::program_options::options_description &commandLineOptions,
    boost::program_options::positional_options_description &positionalOptionsDescription)
{
    for (auto &logComponent : LogComponents)
        commandLineOptions.add_options()(("l" + logComponent.name).c_str(), po::value<std::string>(&logComponent.logLevelFromArgs), "log level from [ERR,WRN,INF,DBG,TRC]");

    for (auto &warningDefinition : Warnings::WarningsDefinitions)
    {
        commandLineOptions.add_options()(("W" + warningDefinition.first).c_str(), po::bool_switch(&warningDefinition.second.turnOn));
    }

    for (auto &warningDefinition : Warnings::WarningsDefinitions)
    {
        commandLineOptions.add_options()(("Wno_" + warningDefinition.first).c_str(), po::bool_switch(&warningDefinition.second.turnOff));
    }
    commandLineOptions.add_options()("Wall", po::bool_switch(&Warnings::allWarnings), "Turn on all warnings");
    commandLineOptions.add_options()("Wnone", po::bool_switch(&Warnings::noWarnings), "Turn off all warnings");
}

bool readSdoAsAscii(CANopen::SdoEngine &engine, uint16_t index, uint8_t subIndex, std::string& outString)
{
    std::vector<uint8_t> output;
    try
    {
        if (!engine.readExpedited("-", index, subIndex, output, 50))
            return false;
        outString.assign(output.size(), ' ');
        std::transform(output.begin(), output.end(), outString.begin(), [](uint8_t x)
                       { return (char)x; });
        return true;
    }
    catch (const std::exception &e)
    {
        return false;
    }
}


void QuasarServer::printNiceSummary() // TODO maybe move to another file ?
{
    fort::utf8_table table;
    table.set_border_style(FT_BOLD_STYLE);

    table << fort::header << "Node name" << "ID" << "State info" << "SW Version" << "Serial#" << "OnlConfValidation" << fort::endr;

    for (Device::DBus *bus : Device::DRoot::getInstance()->buss())
    {
        for (Device::DNode *node : bus->nodes())
        {
            bool assumeSuccessful = true;
            struct SdoBasedInfo
            {
                uint16_t index;
                uint8_t subIndex;
                std::string result;
            };
            std::vector<SdoBasedInfo> sdoBasedInfos = 
            {
                {0x100A, 0x00, "?"},  /* swVersion */
                {0x100A, 0x01, "?"},  /* swVersionMinor */
                {0x3100, 0x00, "?"}   /* serialNumber */
            };
            for (SdoBasedInfo& info : sdoBasedInfos)
            {
                if (bus->isInSpyMode())
                    info.result = "N/A: spy mode";
                else
                {
                    if (!readSdoAsAscii(node->sdoEngine(), info.index, info.subIndex, info.result))
                    {
                        assumeSuccessful = false;
                        break;
                    }
                }
            }

            std::string onlineConfigValidationResult ("no-tests-defined");
            if (node->onlineconfigvalidators().size() == 1)
            {
                if (assumeSuccessful)
                {
                    Device::DOnlineConfigValidator* validator = node->onlineconfigvalidators()[0];
                    std::vector<UaString> testResults;
                    OpcUa_Boolean passed;
                    OpcUa_UInt32 numberOfFailures;
                    validator->callValidate(testResults, passed, numberOfFailures);
                    onlineConfigValidationResult = (passed ? "✔" : "✖") + std::string(" (") + std::to_string(numberOfFailures) + " failed / " + std::to_string(validator->sdovalidators().size()) + " total)"; 
                }
                else
                {
                    onlineConfigValidationResult = "✖ (skipped)";
                }
            }
            
            std::string stateInfo = node->stateInfoSource() + " " + std::to_string(int(bus->getAddressSpaceLink()->getNodeGuardIntervalMs())) + "ms ";
            table << node->getFullName() << (unsigned int)node->id() << stateInfo << 
                sdoBasedInfos[0].result+"."+sdoBasedInfos[1].result << 
                sdoBasedInfos[2].result << 
                onlineConfigValidationResult << fort::endr;
        }
        table << fort::separator;
    }
    LOG(Log::INF) << "\n\n" << table.to_string() << std::endl;
}

void QuasarServer::signalAction()
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
        OpcUa_Int16 portError (0);
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