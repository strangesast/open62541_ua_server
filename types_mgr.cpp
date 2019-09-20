#include <iostream>

#include <open62541/server.h>
#include <open62541/types.h>

#include "types_mgr.h"
#include "mtconnect_ids.h"

int TypesMgr::m_namespace = 2;

TypesMgr::TypesMgr()
{
}

void TypesMgr::setup(UA_Server *server)
{
    m_uaServer = server;
}

int TypesMgr::lookup(string typeName, string data)
{
    map<string, int> &dict = getMap(typeName);
    auto searchDict = dict.find(data);
    if (searchDict == dict.end())
        return -1;

    return searchDict->second;
}

int TypesMgr::lookup(string typeName, UA_NodeId node, string data)
{
    auto search = m_store.find(typeName);
    if (search == m_store.end())
    {
        // try to load the dictionary on the fly from UA_Nodes
        if (!loadDictionary(typeName, node))
            return -1;

        search = m_store.find(typeName);
        if (search == m_store.end())
            return -1; // should be loaded, something is wrong
    }

    map<string, int> &dict = search->second;
    auto searchDict = dict.find(data);
    if (searchDict == dict.end())
        return -1;

    return searchDict->second;
}


bool TypesMgr::getDictionary(string typeName, UA_Variant &enumStrings)
{
    map<string, int> & dict = getMap(typeName);
    int numEnumValues = dict.size();

    if (numEnumValues == 0)
        return false;

    UA_LocalizedText *data = (UA_LocalizedText *)UA_malloc(sizeof(UA_LocalizedText) * numEnumValues);

    int i = 0;
    for (std::map<string,int>::iterator it = dict.begin(); it != dict.end(); it++, i++)
        data[i] = UA_LOCALIZEDTEXT_ALLOC("en", it->first.c_str());

    UA_Variant_setArray(&enumStrings, data, numEnumValues, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]);
    return true;
}

bool TypesMgr::loadDictionary(string typeName)
{
    auto search = mtIDMap.find(typeName);
    if (search == mtIDMap.end())
        return false;

    UA_NodeId node = UA_NODEID_NUMERIC(m_namespace, search->second);
    return loadDictionary(typeName, node);
}

bool TypesMgr::loadDictionary(string key, UA_NodeId node)
{
    // look up its "EnumStrings" property
    UA_NodeId childID;
    if (!findChildId(node,
                UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY),
                UA_QUALIFIEDNAME(0, "EnumStrings"), &childID))
    {
        if (!findChildId(node,
                    UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY),
                    UA_QUALIFIEDNAME(m_namespace, "EnumStrings"), &childID))
            return false;
    }

    // load the enum table from its value
    UA_Variant attr;
    UA_Server_readValue(m_uaServer, childID, &attr);

    if (attr.type->typeIndex != UA_TYPES_LOCALIZEDTEXT)
        return false;

    map<string, int> dict;
    int len = attr.arrayLength;

    UA_LocalizedText *results = (UA_LocalizedText *)attr.data;
    for (int i=0; i<len; i++, results++)
    {
        string data = (const char*)results->text.data;

        dict.insert(std::pair<string, int>(data, i) );
    }

    m_store.insert(std::pair<string, map<string, int>>(key, dict));
    return true;
}


bool TypesMgr::findChildId(UA_NodeId &parentNode, UA_NodeId referenceType,
            const UA_QualifiedName targetName, UA_NodeId *result)
{
    UA_RelativePathElement rpe;
    UA_RelativePathElement_init(&rpe);
    rpe.referenceTypeId = referenceType;
    rpe.isInverse = false;
    rpe.includeSubtypes = false;
    rpe.targetName = targetName;

    UA_BrowsePath bp;
    UA_BrowsePath_init(&bp);
    bp.startingNode = parentNode;
    bp.relativePath.elementsSize = 1;
    bp.relativePath.elements = &rpe;    //clion complains but is ok

    UA_BrowsePathResult bpr = UA_Server_translateBrowsePathToNodeIds(m_uaServer, &bp);
    if(bpr.statusCode != UA_STATUSCODE_GOOD ||
        bpr.targetsSize < 1)
        return false;


    UA_NodeId_copy(&bpr.targets[0].targetId.nodeId, result);
    UA_BrowsePathResult_deleteMembers(&bpr);

    return true;
}

map<string, int>& TypesMgr::getMap(string typeName)
{
    auto search = m_store.find(typeName);
    if (search == m_store.end())
    {
        // try to load the dictionary on the fly from UA_Nodes
        if (!loadDictionary(typeName))
            return m_store[""];

        search = m_store.find(typeName);
        if (search == m_store.end())
            return m_store[""]; // should be loaded, something is wrong
    }

    return search->second;
}
