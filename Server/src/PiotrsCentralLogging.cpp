#include <PiotrsCentralLogging.hpp>
#include <iostream>
#include <unistd.h>

PiotrsCentralLogging::PiotrsCentralLogging():
    m_logStorage(1000),
    m_dataLost(false),
    m_configuration(loadConfiguration())
    {

    }

PiotrsCentralLogging::~PiotrsCentralLogging()
{

}

bool PiotrsCentralLogging::initialize()
{
    std::cout << "PiotrsSink: initializing now. " << std::endl;

    mqtt::connect_options options;
    options.set_automatic_reconnect(true);
    options.set_automatic_reconnect(1, 10); // no more than 10 secs for reconnect attempt

    
    char hostname_ch [128];
    gethostname (hostname_ch, sizeof hostname_ch -1);

    m_mqttClient = new mqtt::client(m_configuration.mqttBrokerHostName, std::string(hostname_ch)+":"+std::to_string(getpid()));
    m_mqttClient->connect(options);

    m_dispatcher = std::thread(&PiotrsCentralLogging::dispatchThread, this);

    return true;
}

void PiotrsCentralLogging::dispatchThread()
{
    char hostname_ch [128];
    gethostname (hostname_ch, sizeof hostname_ch -1);

    // the application name should be of course coming from the ServerConfig.xml but that I leave to people responsible for ServerConfig.xml
    
    

    std::string topic = "log/" + std::string(hostname_ch) + "/" + m_configuration.appInstanceName;

    std::cout << "PiotrsCentralLogging: the MQTT topic would be [" << topic << "]" << std::endl;

    while (true)
    {
        try
        {
            bool dataWasLost = m_dataLost;
            m_dataLost = false;
            std::string mqttMsgAsText;
            mqttMsgAsText.reserve(1024);
            size_t numMsgs (0);

            for (unsigned int i=0; i < m_logStorage.size(); i++)
            {
                OneLogItem* storedMessage;
                if (m_logStorage.investigate(i, &storedMessage)) // there was a msg at this slot
                {
                    // std::cout << "Recovered: " << storedMessage->message << std::endl;
                    mqttMsgAsText.append(storedMessage->message);
                    mqttMsgAsText.append("#@#");
                    m_logStorage.pop(i);
                    numMsgs++;
                }
            }
            if (numMsgs > 0)
            {
                auto pubmsg = mqtt::make_message(topic, mqttMsgAsText);
                m_mqttClient->publish(pubmsg);
            }
        }
        catch (const std::exception& e)
        {
            std::cout << "PiotrsCentralLogging: exception: " << e.what() << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void PiotrsCentralLogging::logMessage(const std::string& msg)
{
    // std::cout << "PiotrsSink: " << msg << std::endl;


    OneLogItem logItem;
    logItem.message = msg;

    bool success = m_logStorage.store(logItem); // not efficient -- creation and deletion of data, could be more optimal
    if (!success)
        m_dataLost = true;
}

PiotrsCentralLogging::Configuration PiotrsCentralLogging::loadConfiguration ()
{
    Configuration configuration;

    const char* ENV_VAR_BROKER_HOST_NAME = "PIOTRS_CENTRAL_LOGGING_BROKER_HOSTNAME";
    const char* ENV_VAR_APP_INSTANCE = "PIOTRS_CENTRAL_LOGGING_APP_INSTANCE";

    if (!getenv(ENV_VAR_BROKER_HOST_NAME))
        throw std::runtime_error("Can't initialize PiotrsCentralLogging because env var is not defined: " + (std::string)ENV_VAR_BROKER_HOST_NAME);
    
    configuration.mqttBrokerHostName = getenv(ENV_VAR_BROKER_HOST_NAME); 

    if (!getenv(ENV_VAR_APP_INSTANCE))
        throw std::runtime_error("Can't initialize PiotrsCentralLogging because env var is not defined: " + (std::string)ENV_VAR_APP_INSTANCE);

    configuration.appInstanceName = getenv(ENV_VAR_APP_INSTANCE);

    return configuration;
}
