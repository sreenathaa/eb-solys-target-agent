/*******************************************************************************
 * Copyright 2017, Elektrobit Automotive GmbH. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. 
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *******************************************************************************/


#include "../gen/target_agent_version.hpp"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include "CMediator.h"
using namespace TargetAgent::Version;

#define RETAILMSG(cond,printf_exp)


class TargetAgentServer: public Poco::Util::ServerApplication {
public:
    TargetAgentServer() :
    _helpRequested(false) {
    }

    ~TargetAgentServer() {
    }

protected:
    void initialize(Application& self) {
        loadConfiguration();
        ServerApplication::initialize(self);
    }

    void uninitialize() {
        ServerApplication::uninitialize();
    }

    void defineOptions(Poco::Util::OptionSet& options) {
        ServerApplication::defineOptions(options);

        options.addOption(
            Poco::Util::Option("help", "h",
                               "display argument help information").required(false).repeatable(
                false).callback(
                Poco::Util::OptionCallback<TargetAgentServer>(this,
                        &TargetAgentServer::handleHelp)));
        options.addOption(
            Poco::Util::Option("prot_version", "p",
                               "display protocol version information").required(false).repeatable(
                false).callback(
                Poco::Util::OptionCallback<TargetAgentServer>(this,
                        &TargetAgentServer::handleRevision)));
        options.addOption(
            Poco::Util::Option("version", "v",
                               "display version information").required(false).repeatable(
                false).callback(
                Poco::Util::OptionCallback<TargetAgentServer>(this,
                        &TargetAgentServer::handleVersion)));
    }

    void handleHelp(const std::string& name, const std::string& value) {
        Poco::Util::HelpFormatter helpFormatter(options());
        helpFormatter.setCommand(commandName());
        helpFormatter.setUsage("OPTIONS");
        helpFormatter.setHeader("Target Agent Server Application");
        helpFormatter.format(std::cout);
        stopOptionsProcessing();
        _helpRequested = true;
    }

    void handleRevision(const std::string& name, const std::string& value) {
#ifndef _WIN32
        std::cout << "Target Agent->Solys Protocol Version  " << raceTaProtocolVersion<<" "<< std::endl;
#else

        RETAILMSG(1, (TEXT("TargetAgent[%s] Revision: %u \n"), _T(__FUNCTION__), raceTaProtocolVersion));
#endif

        stopOptionsProcessing();
        _helpRequested = true;
    }

    void handleVersion(const std::string& name, const std::string& value) {
        std::cout << "Target Agent Version  " << targetAgentVersion<<" "<< std::endl;
        stopOptionsProcessing();
        _helpRequested = true;
    }

    int main(const std::vector<std::string>& args) {
        if (!_helpRequested)
        {

            try
            {
#ifndef _WIN32
                std::cout << "Target Agent->Solys Protocol Version " << raceTaProtocolVersion<<" "<< std::endl;
#else

                RETAILMSG(1, (TEXT("TargetAgent[%s] Revision: %u \n"), _T(__FUNCTION__), raceTaProtocolVersion));
#endif

                TargetAgent::Mediator::CTargetAgentRuntime::getUniqueInstance()->run();
            }
            catch (Poco::Exception& e)
            {
#ifndef _WIN32
                std::cout << __func__ << "::" << __LINE__ << "::" << "Exception"
                << e.displayText() << " " << e.what() << std::endl;
#else

                wchar_t  ws[100];
                swprintf(ws, L"%hs", (e.displayText() + " " + e.what()).c_str());
                //    RETAILMSG(1, (TEXT("TargetAgent[%s] Exception: %s \n"), _T(__FUNCTION__), ws));
#endif

            }
            waitForTerminationRequest();
            std::cout << "Target Agent Begin Shutdown Phase"<< std::endl;
            TargetAgent::Mediator::CTargetAgentRuntime::getUniqueInstance()->shutdown();
            std::cout << "Target Agent End Shutdown Phase"<< std::endl;

        }
        return Application::EXIT_OK;
    }

private:
    bool _helpRequested;
};

#if defined(_WIN32)

int wmain(int argc, wchar_t* argv[])

#else
int main(int argc, char** argv)
#endif
{
#ifdef _POSIX_THREADS
    if (0 != pthread_setname_np(pthread_self(), "solys-agent"))
    {
        std::cout << "Failed to set thread name" << raceTaProtocolVersion<<" "<< std::endl;
    }
#endif
    TargetAgentServer app;
#ifndef _WIN32

    app.run(argc, argv);
#else

    std::vector<std::string> args;
    args.push_back("placeholder");
    app.run(args);
#endif

    app.terminate();

    return 0;
}

