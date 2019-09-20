#ifndef TYPES_MGR_H
#define TYPES_MGR_H

#include <string>
#include <map>

using namespace std;

class TypesMgr
{
private:
    static int m_namespace; // MTConnect Nodes namespace
    UA_Server *m_uaServer;
    map<string, map<string, int>> m_store;

    bool loadDictionary(string typeName);
    bool loadDictionary(string key, UA_NodeId node);
    bool findChildId(UA_NodeId &parentNode, UA_NodeId referenceType,
                const UA_QualifiedName targetName, UA_NodeId *result);
    map<string, int>& getMap(string typeName);

public:
    TypesMgr();

    void setup(UA_Server *server);
    int lookup(string typeName, string data);
    int lookup(string typeName, UA_NodeId node, string data);
    bool getDictionary(string typeName, UA_Variant &enumStrings);

};



#endif // TYPES_MGR_H
