#include <ConfigurationProcessing.hxx>
#include <ConfigurationDecorationUtils.h>
#include <LogIt.h>
#include <Warnings.hxx>


using namespace Configuration;

namespace ConfigurationProcessing
{

void processMultiplexedChannelConfigurationGenerator(Configuration::Configuration &configuration)
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

void processGlobalSettings(Configuration::Configuration &configuration)
{
    ensureObjectPresent(
        configuration.GlobalSettings(),
        configuration,
        Configuration::GlobalSettings("GlobalSettings"),
        Configuration::Configuration::GlobalSettings_id,
        "GlobalSettings");
}
void processWarnings(Configuration::Configuration &configuration)
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

void processConfiguration(Configuration::Configuration& configuration)
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
}

}