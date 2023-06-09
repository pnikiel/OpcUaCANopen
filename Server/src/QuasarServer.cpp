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
#include <ASMeta.h>
#include <ASNodeQueries.h>
#include <ASThreadPool.h>

#include <Configuration.hxx>
#include <Configurator.h>

#include <Warnings.hxx>
#include <Utils.h>
#include <Logging.hpp>
#include <CtrlZTable.hpp>
#include <ConfigurationProcessing.hxx>
#include "version.h"
#include <MyLogComponents.h>
#include <SourceVariables.h>

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
        {"Rpdo"},
        {"Sdo"},
        {"SdoValidator"},
        {"Spooky"},
        {"Spy"},
        {"SdoValidator"},
        {"NmTpdo"},
        {"MTpdo"}
    };

void printLogo()
{
    std::string versionStr(VERSION_STR);
    versionStr.resize(73, ' ');

    std::string text[] = {
        "                                                                                    ",
        "  ▄██▀▀██▄  ▀▀▀███▀▀  ▐██▌      ▄██▀▀█▄▄  ▄██▀▀██▄  ▀████▀██▄▄  ▄██▀▀██▄  ▄▄█▀▀██▄  ",
        "  ███  ███     ██▌    ▐██▌     ▐██▌ ▐██▌  ███▄▄▄     ▐██▌ ▐██▌  ██▌       ███▄▄▄▄   ",
        "  ████████     ██▌    ▐██▌     ▐███████▌   ▀▀▀▀██▄   ▐██▌ ▐██▌  ██▌         ▀▀▀███  ",
        "  ███  ███     ██▌    ▐███▄▄▄  ▐██▌ ▐██▌  ▀██▄▄██▀   ▐███▄▄█▀▀  ▀██▄▄██▀  ▀██▄▄██▀  ",
        "                                                                                    ",
        "  The CANopen NG OPCUA server by Piotr P. Nikiel et al., 2021-2023                  ",
        "  The CanModule for device-independent CAN support by V. Filimonov, P.P. Nikiel,    ",
        "    D. Abalo, M. Ludwig, 2012-2022                                                  ",
        "                                                                                    ",
        "  Version: " + versionStr,
        "                                                                                    "};

    std::cout << "starting up... " << std::endl;

    for (const std::string &line : text)
    {
        std::cout << "\033[1;97;44m" << line << "\033[0m" << std::endl;
    }
}

QuasarServer::QuasarServer() : BaseQuasarServer(),
                               m_forceDontReconfigure(false),
                               m_mapToVcan(false),
                               m_metaThreadPoolInfo(nullptr),
                               m_lastJobsAcceptedCounter(0),
                               m_lastJobsFinishedCounter(0),
                               m_lastMetaUpdate(std::chrono::steady_clock::now()),
                               m_threadPoolIngressRate(0),
                               m_threadPoolEgressRate(0)
{

    QuasarServer::s_instance = this;

    // the check for LittleEndian arch. Reason for that is most ValueMapper algorithms base on this assumption.
    uint32_t checkWordU32 = 0x12345678;
    uint8_t *checkPtr = reinterpret_cast<uint8_t *>(&checkWordU32);
    if (*checkPtr != (checkWordU32 & 0xff))
    {
        throw std::runtime_error("It seems that you're running this program on an architecture different from LittleEndian. With current ValueMapping algorithms this would deliver wrong results. Consider contributing or contact Piotr.");
    }
}

void QuasarServer::updateThreadPoolMetrics()
{
    auto now = std::chrono::steady_clock::now();
    double secsSince = std::chrono::duration_cast<std::chrono::milliseconds> (now - m_lastMetaUpdate).count() / 1000.0;
    Quasar::ThreadPool *quasarThreadPool(AddressSpace::SourceVariables_getThreadPool());
    size_t jobsAcceptedCtr = quasarThreadPool->getNumJobsAccepted();
    m_threadPoolIngressRate = (jobsAcceptedCtr - m_lastJobsAcceptedCounter) / secsSince;
    size_t jobsFinishedCtr = quasarThreadPool->getNumJobsFinished();
    m_threadPoolEgressRate = (jobsFinishedCtr - m_lastJobsFinishedCounter) / secsSince;
    
    m_metaThreadPoolInfo->setIngressJobRate(m_threadPoolIngressRate, OpcUa_Good);
    m_metaThreadPoolInfo->setEgressJobRate(m_threadPoolEgressRate, OpcUa_Good);
    m_metaThreadPoolInfo->setNumPendingJobs(quasarThreadPool->getNumPendingJobs(), OpcUa_Good);

    /* ... and for the new cycle ... */
    m_lastJobsAcceptedCounter = jobsAcceptedCtr;
    m_lastJobsFinishedCounter = jobsFinishedCtr;
    m_lastMetaUpdate = now;
}

void QuasarServer::printThreadPoolMetrics()
{
    Quasar::ThreadPool *quasarThreadPool(AddressSpace::SourceVariables_getThreadPool());
    if (!quasarThreadPool)
        return;
    std::cout << "Quasar ThreadPool: pending jobs:" << 
        wrapValue(std::to_string(quasarThreadPool->getNumPendingJobs())) <<
        " ingressRate[1/s]:" << wrapValue(std::to_string(m_threadPoolIngressRate)) <<
        " egressRate[1/s]:" << wrapValue(std::to_string(m_threadPoolEgressRate)) <<   std::endl;
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

        updateThreadPoolMetrics();
    }
    printServerMsg(" Shutting down server");
}

QuasarServer *QuasarServer::s_instance = nullptr;

void handleCtrlZ(int)
{
    printCtrlZTable();
}

void QuasarServer::initialize()
{
    LOG(Log::INF) << "Initializing Quasar server.";

    // fill up product version.
    AddressSpace::ASMeta *meta = AddressSpace::findByStringId<AddressSpace::ASMeta>(getNodeManager(), "Meta");
    if (meta)
        meta->setProductVersion(VERSION_STR, OpcUa_Good);

    m_metaThreadPoolInfo = AddressSpace::findByStringId<AddressSpace::ASThreadPool>(getNodeManager(), "Meta.ThreadPool");
    if (!m_metaThreadPoolInfo)
        throw std::logic_error("Sth wrong with Meta address-space ?? .. ??");

    // TODO make it a separate method to parse components
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

    signal(SIGTSTP, handleCtrlZ);

    printLogo();
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
    {
        logComponent.handle = Log::registerLoggingComponent(logComponent.name);
        if (logComponent.name == "Spooky")
            MyLogComponents::s_spooky = logComponent.handle;
        if (logComponent.name == "NodeMgmt")
            MyLogComponents::s_nodemgmt = logComponent.handle;
        if (logComponent.name == "Spy")
            MyLogComponents::s_spy = logComponent.handle;
        if (logComponent.name == "NmTpdo")
            MyLogComponents::s_nm_tpdo = logComponent.handle;
        if (logComponent.name == "MTpdo")
            MyLogComponents::s_m_tpdo = logComponent.handle;
    }
    LOG(Log::INF) << "Logging initialized.";
}

bool QuasarServer::overridableConfigure(const std::string &fileName, AddressSpace::ASNodeManager *nm)
{
    return configure(fileName, nm, std::bind(&QuasarServer::processConfiguration, this, std::placeholders::_1));
}

bool QuasarServer::processConfiguration(Configuration::Configuration &configuration)
{
    ConfigurationProcessing::processConfiguration(configuration);

    return true; //! throws if problem anyway
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

    commandLineOptions.add_options()("force_dont_reconfigure", po::bool_switch(&m_forceDontReconfigure)->default_value(false), "Force DontReconfigure CanModule option per all declared buses which will avoid elevated privileges on some platforms");
    commandLineOptions.add_options()("map_to_vcan", po::bool_switch(&m_mapToVcan)->default_value(false), "Map to VCAN port names whenever possible");
    commandLineOptions.add_options()("print_cobids_tables", po::bool_switch(&m_printCobidsTables)->default_value(false), "Print cobid tables");
}

bool readSdoAsAscii(const std::string &where, CANopen::SdoEngine &engine, uint16_t index, uint8_t subIndex, std::string &outString)
{
    std::vector<uint8_t> output;
    try
    {
        if (!engine.readExpeditedOrSegmented(where, index, subIndex, output, 50 /*ms*/))
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

    table << fort::header << "Node name"
          << "ID"
          << "State info"
          << "SW Version"
          << "Serial#"
          << "OnlConfValidation" << fort::endr;

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
                std::string where;
            };
            std::vector<SdoBasedInfo> sdoBasedInfos =
                {
                    {0x100A, 0x00, "?", "swVersion"},
                    {0x3100, 0x00, "?", "serialNumber"},
                    //! Sequence is as is because the first two are mandatory of CANopen 
                    //! while swVersionMinor is a local extension.
                    {0x100A, 0x01, "?", "swVersionMinor"}}; 
            for (SdoBasedInfo &info : sdoBasedInfos)
            {
                if (bus->isInSpyMode())
                    info.result = "N/A: spy mode";
                else if (!node->sdoEngine())
                    info.result = "N/A: SDO switched off";
                else
                {
                    if (!readSdoAsAscii(node->getFullName() + "." + info.where, *node->sdoEngine(), info.index, info.subIndex, info.result))
                    {
                        assumeSuccessful = false;
                        break;
                    }
                }
            }

            std::string onlineConfigValidationResult("no-tests-defined");
            if (node->onlineconfigvalidators().size() == 1)
            {
                if (assumeSuccessful)
                {
                    Device::DOnlineConfigValidator *validator = node->onlineconfigvalidators()[0];
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
            table << node->getFullName() <<
                (unsigned int)node->id() <<
                stateInfo <<
                sdoBasedInfos[0].result + "." + sdoBasedInfos[2].result <<
                sdoBasedInfos[1].result <<
                onlineConfigValidationResult << fort::endr;
        }
        table << fort::separator;
    }
    LOG(Log::INF) << "\n\n"
                  << table.to_string() << std::endl;
}
