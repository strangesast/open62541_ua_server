#include <signal.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>

#include <open62541/plugin/log_stdout.h>
#include <open62541/server.h>
#include <open62541/server_config_default.h>
#include <open62541/types.h>
#include <open62541/types_generated_handling.h>

#include <boost/thread.hpp>
#include <boost/tokenizer.hpp>
#include <boost/filesystem.hpp>

#include "worker.h"
#include "nodeset.h"
#include "mtconnect_ids.h"

using namespace boost;

static Settings settings;
static string settingsName;

static Worker *createWorker(UA_Server *server, UA_NodeId &nodeId, string outputLocation, string uri, string poll)
{
    Worker *worker = new Worker();

    if (!worker->setup(server, nodeId, &settings, outputLocation, uri, poll))
        return nullptr;

    return worker;
}


static volatile UA_Boolean running = true;
static void stopHandler(int sig) {
    UA_LOG_INFO(UA_Log_Stdout, UA_LOGCATEGORY_USERLAND, "received ctrl-c");
    running = false;
}


int main(int argc, char** argv) {
    signal(SIGINT, stopHandler);
    signal(SIGTERM, stopHandler);

    // Check command line arguments.
    if (argc < 2 || argc > 3)
    {
        std::cerr <<
            "Usage: " << endl <<
            "    OPCUA-MTServer <uri> [poll cycle in seconds - default is 60]" << endl <<
            "Example:" << endl <<
            "    OPCUA-MTServer https://smstestbed.nist.gov/vds" << endl <<
            "or" << endl <<
            "    OPCUA-MTServer <config file>" << endl;
        return -1;
    }

    // settingsName = Settings::getSettingsName("OPCUA-MTServer");
    // settings.restore(settingsName);

    UA_Server *server = UA_Server_new();
    UA_ServerConfig_setDefault(UA_Server_getConfig(server));
    if (nodeset(server) != UA_STATUSCODE_GOOD) {
        UA_LOG_ERROR(UA_Log_Stdout, UA_LOGCATEGORY_SERVER, "Could not add the nodeset. "
            "Check previous output for any error.");
        return -1;
    }


    UA_NodeId topId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    std::vector<Worker*> worker_pool;

    boost::thread *bthread = nullptr;
    if (argc != 2)
    {
        string outputLocation = "/tmp";
        string uri = argv[1];
        string poll = argc > 2 ? argv[2] : "60";

        Worker *worker = createWorker(server, topId, outputLocation, uri, poll);

        worker_pool.push_back(worker);

        bthread = new boost::thread(boost::bind(&Worker::run, worker));
    }
    else {

        ifstream in(argv[1]);
        if (!in.is_open()) {
            std::cerr << "Cannot open config file " << argv[1] << "!" << endl;
            return -1;
        }

        typedef tokenizer< escaped_list_separator<char> > Tokenizer;
        vector< string > vec;
        string line;

        while (getline(in, line))
        {
            Tokenizer tok(line);
            vec.assign(tok.begin(), tok.end());

            unsigned long argc = vec.size();

            // empty line
            if (argc == 0)
                continue;

            if (argc != 1 && argc != 2)
            {
                std::cerr << "Invalid input [" << line << "]" << endl;
                return -1;
            }

            string outputLocation = "/tmp";

            string uri = vec[0];
            if (uri[0] == '#')
                continue;

            string poll = argc > 1 ? vec[1] : "60";

            Worker *worker = createWorker(server, topId, outputLocation, uri, poll);

            worker_pool.push_back(worker);

            bthread = new boost::thread(boost::bind(&Worker::run, worker));

            if (bthread == nullptr)
                return -1;
        }

    }

    // settings.save(settingsName);
    UA_StatusCode retval = UA_Server_run(server, &running);

    UA_Server_delete(server);
    return retval == UA_STATUSCODE_GOOD ? EXIT_SUCCESS : EXIT_FAILURE;
}
