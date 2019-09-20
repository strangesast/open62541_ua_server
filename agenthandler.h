#ifndef AGENTHANDLER_H
#define AGENTHANDLER_H

#include <boost/property_tree/ptree.hpp>

#include <string>
#include <iostream>
#include <set>

#include "types_mgr.h"
#include "settings.h"

using namespace std;
using boost::property_tree::ptree;

class agentHandler
{
private:
    UA_Server *m_uaServer;
    Settings m_settings;
    UA_NodeId m_topNode;

    ptree m_ptree;
    string m_xml;
    std::map<string, int> m_ids;
    map<string, UA_NodeId> m_fieldDataTypes;

    UA_UInt16 m_namespace;
    TypesMgr m_typesMgr;

    map<string, UA_NodeId> m_eventTypes;

public:
    agentHandler();

private:

    void setupMetaInfo(string deviceUUID, UA_NodeId &topNode, ptree &ptree, string path, vector<UA_NodeId> &nodePath);

    string getJSON_data(ptree &tree, string path);
    bool isLeafNode(ptree::iterator &p);
    UA_NodeId addDeviceDataItem(string deviceUUID, UA_NodeId &topNode, ptree &dataItem, bool appendName, string path, vector<UA_NodeId> &nodePath);

    void setProperties(UA_NodeId &topNode, ptree &dataItem);
    void writeData(string variable, string dateTime, string data);
    bool findChildId(UA_NodeId &parentNode, UA_NodeId referenceType,
                const UA_QualifiedName targetName, UA_NodeId *result);
    bool writeNodeData(UA_NodeId &nodeId, string result);
    string toCamelCase(string &input);
    void getEventTypes(UA_NodeId eventRootnode, UA_NodeId eventTypeNode, map<string, UA_NodeId> &results);
    UA_NodeId addProperty(UA_NodeId parentNode, string propertyName, string value);
    void addNotifier(UA_NodeId targetNode, UA_NodeId srcNode);
    void addNotifierToAll(vector<UA_NodeId> & nodePath);
    int updateDeviceData(const string &deviceName, const string &deviceUUID, const string &componentId, ptree &pt);


public:
    void setup(UA_Server *uaServer, UA_NodeId topNode, int ns);
    void setProbeInfo(string probeXml);
    bool process(string xmlText);
    string getJSON_data(string path) { return getJSON_data(m_ptree, path); }
    bool updateData();
};

#endif // AGENTHANDLER_H
