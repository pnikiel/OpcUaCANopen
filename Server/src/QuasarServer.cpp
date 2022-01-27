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


#include <thread>

#include "QuasarServer.h"
#include <LogIt.h>
#include <shutdown.h>

#include <DBus.h>
#include <DRoot.h>

#include <Configuration.hxx>
#include <Configurator.h>
#include <ConfigurationDecorationUtils.h>

using namespace Configuration;

QuasarServer::QuasarServer() : BaseQuasarServer()
{

}

QuasarServer::~QuasarServer()
{
 
}

void QuasarServer::mainLoop()
{
    printServerMsg("Press "+std::string(SHUTDOWN_SEQUENCE)+" to shutdown server");

    // Wait for user command to terminate the server thread.

    while(ShutDownFlag() == 0)
    {
       std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        for (Device::DBus* bus : Device::DRoot::getInstance()->buss())
        {
            bus->tick();
        }
    }
    printServerMsg(" Shutting down server");
}

void QuasarServer::initialize()
{
    LOG(Log::INF) << "Initializing Quasar server.";

}

void QuasarServer::shutdown()
{   
	LOG(Log::INF) << "Shutting down Quasar server.";
    LOG(Log::INF) << "Stopping processing CAN buses";
    for (Device::DBus* bus : Device::DRoot::getInstance()->buss())
    {
        bus->shutDown();
    }
}

void QuasarServer::initializeLogIt()
{
	BaseQuasarServer::initializeLogIt();
	Log::registerLoggingComponent("CanModule");
	Log::registerLoggingComponent("Spooky");
    Log::registerLoggingComponent("NodeMgmt");
    Log::registerLoggingComponent("Sdo");
    Log::registerLoggingComponent("Emergency");
    LOG(Log::INF) << "Logging initialized.";
}

bool QuasarServer::overridableConfigure(const std::string& fileName, AddressSpace::ASNodeManager *nm)
{
    return configure(fileName, nm, std::bind(&QuasarServer::processConfiguration, this, std::placeholders::_1));
}

bool QuasarServer::processConfiguration(Configuration::Configuration& configuration)
{
    if (configuration.GlobalSettings().size() < 1)
    {
        DecorationUtils::push_back(
            configuration,
            configuration.GlobalSettings(),
            Configuration::GlobalSettings("GlobalSettings"),
            Configuration::Configuration::GlobalSettings_id
        );
    }
    else
    {
    	// for a user-supplied GlobalSettings make sure it is called "GlobalSettings" as we can't force it via quasar
        if (configuration.GlobalSettings()[0].name() != "GlobalSettings")
            throw std::runtime_error("Error: user supplied GlobalSettings name must be GlobalSettings. Fix your configuration file.");
    }
    for (Bus& bus : configuration.Bus())
    {
        for (Node& node : bus.Node())
        {
            for (TpdoMultiplex& multiplex : node.TpdoMultiplex())
            {
                for (MultiplexedChannelConfigurationGenerator& generator : multiplex.MultiplexedChannelConfigurationGenerator())
                {
                    if (generator.MultiplexedChannel().size() != 1)
                        throw std::runtime_error ("Bad configuration: GenerateIdenticalChannels must contain precisely one MultiplexedChannel to server as a specimen.");
                    MultiplexedChannel& specimen = generator.MultiplexedChannel().front();
                    
                    if (specimen.name() != "specimen")
                        throw std::runtime_error ("Specimen must be called specimen ;-) ");
                    LOG(Log::INF) << "found GenerateIdenticalChannels!";
                    for (unsigned int chno=0; chno < generator.numberOfChannels(); chno++)
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
    
    return true;
}
