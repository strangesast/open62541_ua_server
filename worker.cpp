#include <stdlib.h>
#include <open62541/server.h>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp>
#include <boost/chrono.hpp>
#include <boost/filesystem.hpp>

#include "httpreader.h"
#include "agenthandler.h"
#include "worker.h"

Worker::Worker()
{
    m_interval = 60;
    m_poll_count = 0;
}

Worker::~Worker()
{
    m_reader.close();
}

bool Worker::setup(UA_Server *uaServer, UA_NodeId topNode, Settings *settings, string uri, string interval)
{
    m_uri = uri;
    m_settings = settings;
    m_uaServer = uaServer;
    m_topNode = topNode;

    m_interval = atoi(interval.c_str());
    if (m_interval <= 0) {
        std::cerr << "interval [" << interval << "] is invalid" << endl;
        return false;
    }

    m_namespace = UA_Server_addNamespace(m_uaServer, uri.c_str());
    std::cout << "----------------" << endl;
    std::cout << "Agent Uri:       " << uri << endl;
    std::cout << "Poll Interval:   " << interval << endl;
    std::cout << "namespace : " << m_namespace << endl;

    m_handler.setup(m_uaServer, m_topNode, m_namespace);
    if (m_reader.parseUri(uri))
        return setMetaInfo();

    return false;
}

bool Worker::setMetaInfo()
{
    m_reader.setQuery("/probe");
    string probeXml = m_reader.read();
    if (probeXml.length() == 0)
    {
        std::cerr << "No data!" << endl;
        return false;
    }

    m_handler.processProbeInfo(probeXml);
    m_reader.close();
    return true;
}

void Worker::run()
{
    while (true)
    {
        poll();
        boost::this_thread::sleep_for( boost::chrono::seconds(m_interval) );
    }
}

void Worker::poll()
{
    m_poll_count++;

//    if (m_next_sequence.length() == 0)
        m_reader.setQuery("/current");
//    else
//        m_reader.setQuery("/sample?count=10000&from="+m_next_sequence);

    string xmlData = m_reader.read();

    if (xmlData.length() == 0)
    {
        std::cerr << "No data!" << endl;
        return;
    }

    try {
        m_handler.parseStreamData(xmlData);
    }
    catch (exception & e)
    {
        std::cerr << e.what() << endl;
        return;
    }

    // don't output if data has not changed
    string sequence = m_handler.getJSON_data("MTConnectStreams.Header.<xmlattr>.nextSequence");
    if (sequence.compare(m_next_sequence) == 0)
    {
        std::cout << "========== { " << m_uri << " - "<< m_poll_count << ", [SKIPPED] next sequence = " << m_next_sequence << " } ==========" << std::endl;
        return;
    }

    if (sequence.length() == 0)
    {
        // last next_sequence may be invalid, reset to using "current" to fetch the latest data
        std::cout << "========== { " << m_uri << " - "<< m_poll_count << ", [SKIPPED] reset to fetch current data } ==========" << std::endl;
        return;
    }

    m_handler.processStreamData();
    m_next_sequence = sequence;
    std::cout << "========== { " << m_uri << " - updated " << m_poll_count << ", next sequence = " << m_next_sequence << " } ==========" << std::endl;
}

