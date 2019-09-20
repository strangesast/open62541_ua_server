#include <open62541/server.h>
#include <open62541/types.h>

#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>

#include <cstdio>
#include <ctime>
#include <cstdlib>
#include <math.h>
#include <map>
#include <ctype.h>

#include "agenthandler.h"
#include "settings.h"
#include "mtconnect_ids.h"
#include "mteuInfo.h"

namespace fs = boost::filesystem;

agentHandler::agentHandler()
{
    m_uaServer = NULL;
}

void agentHandler::setup(UA_Server *uaServer, UA_NodeId topNode, int ns)
{
    m_uaServer = uaServer;
    m_topNode = topNode;
    m_namespace = ns;

    m_typesMgr.setup(uaServer);
    getEventTypes(UA_NODEID_NUMERIC(2, MT_MTCONTROLLEDVOCABEVENTCLASSTYPE), UA_NODEID_NUMERIC(2, MT_MTCONTROLLEDVOCABEVENTTYPE), m_eventTypes);
    getEventTypes(UA_NODEID_NUMERIC(2, MT_MTSTRINGEVENTCLASSTYPE), UA_NODEID_NUMERIC(2, MT_MTSTRINGEVENTTYPE), m_eventTypes);
    getEventTypes(UA_NODEID_NUMERIC(2, MT_MTNUMERICEVENTCLASSTYPE), UA_NODEID_NUMERIC(2, MT_MTNUMERICEVENTTYPE), m_eventTypes);
}


void agentHandler::setupMetaInfo(string deviceUUID, UA_NodeId &topNode, ptree &probeInfo, string path, vector<UA_NodeId> &nodePath)
{
    for (ptree::iterator p = probeInfo.begin(); p != probeInfo.end(); p++)
    {
        string display = p->first;

        if (display.compare("<xmlattr>") == 0 || display.compare("Compositions") == 0
                || display.compare("Description") == 0)
            continue;

        if (!isLeafNode(p))
        {
            if (display.compare("DataItems") == 0)
            {
                ptree& dataItems = p->second;
                set <string> types;
                set <string> checkList;
                for (ptree::iterator pos = dataItems.begin(); pos != dataItems.end(); pos++)
                {
                    string s = getJSON_data(pos->second, "<xmlattr>.type");
                    string category = getJSON_data(pos->second, "<xmlattr>.category");
                    if (category.compare("CONDITION") == 0)
                        s += "Condition";

                    if (types.count(s) > 0)
                        checkList.insert(s);
                    else
                        types.insert(s);
                }

                for (ptree::iterator pos = dataItems.begin(); pos != dataItems.end(); pos++)
                {
                    string s = getJSON_data(pos->second, "<xmlattr>.type");
                    string category = getJSON_data(pos->second, "<xmlattr>.category");
                    if (category.compare("CONDITION") == 0)
                        s += "Condition";

                    UA_NodeId itemNode = addDeviceDataItem(deviceUUID, topNode, pos->second.get_child("<xmlattr>"), checkList.count(s) > 0, path, nodePath);
                    // check for Source
                    string source = getJSON_data(pos->second, "Source");
                    if (source.length() > 0)
                        addProperty(itemNode, "SourceData", source);


                    // check for Constraints
                    try {
                        ptree& constraints = pos->second.get_child("Constraints");
                        if (constraints.size() > 0)
                        {
                            UA_ObjectAttributes oAttr = UA_ObjectAttributes_default;
                            oAttr.displayName = UA_LOCALIZEDTEXT_ALLOC("en-US", "Constraints");

                            UA_NodeId constraintNode;
                            UA_Server_addObjectNode(m_uaServer,
                                                      UA_NODEID_NUMERIC(m_namespace, 0),
                                                      itemNode,
                                                      UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                                                      UA_QUALIFIEDNAME(m_namespace, "Constraints"),
                                                      UA_NODEID_NUMERIC(2, MT_MTCONSTRAINTTYPE),
                                                      oAttr,
                                                      NULL,
                                                      &constraintNode);

                            vector <string> values;
                            for (ptree::iterator p = constraints.begin(); p != constraints.end(); p++)
                            {
                                string key = p->first;

                                if (key.compare("Value") == 0)
                                    values.push_back(p->second.data());
                                else
                                {
                                    UA_VariableAttributes mnAttr = UA_VariableAttributes_default;
                                    mnAttr.dataType = UA_NODEID_NUMERIC(0, UA_NS0ID_DOUBLE);
                                    UA_Double value = atof(p->second.data().c_str());

                                    UA_Variant_setScalar(&mnAttr.value, &value, &UA_TYPES[UA_TYPES_DOUBLE]);
                                    mnAttr.displayName = UA_LOCALIZEDTEXT_ALLOC("en-US", key.c_str());
                                    UA_Server_addVariableNode(m_uaServer, UA_NODEID_NULL, constraintNode,
                                                              UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY),
                                                              UA_QUALIFIEDNAME_ALLOC(m_namespace, key.c_str()),
                                                              UA_NODEID_NUMERIC(0, UA_NS0ID_PROPERTYTYPE), mnAttr, NULL, NULL);

                                }
                            }

                            size_t size = values.size();
                            if (size > 0)
                            {

                                UA_String *data = (UA_String *)UA_malloc(sizeof(UA_String) * size);

                                int i = 0;
                                for (vector<string>::iterator q = values.begin(); q != values.end(); q++, i++)
                                    data[i] = UA_STRING_ALLOC((*q).c_str());

                                UA_VariableAttributes mnAttr = UA_VariableAttributes_default;
                                mnAttr.dataType = UA_NODEID_NUMERIC(0, UA_NS0ID_STRING);
                                UA_Variant_setArray(&mnAttr.value, data, size, &UA_TYPES[UA_TYPES_STRING]);

                                mnAttr.displayName = UA_LOCALIZEDTEXT("en-US", "Values");
                                UA_Server_addVariableNode(m_uaServer, UA_NODEID_NULL, constraintNode,
                                                          UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY),
                                                          UA_QUALIFIEDNAME(m_namespace, "Values"),
                                                          UA_NODEID_NUMERIC(0, UA_NS0ID_PROPERTYTYPE),
                                                          mnAttr, NULL, NULL);
                            }
                        }

                    } catch (...) {
                    }

                }
            }
            else
            {
                UA_NodeId nextId = topNode;

                UA_NodeId nodeType = UA_NODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE);

                UA_UInt32 typeId = mtIDMap[p->first + "Type"];
                if (typeId != 0)
                    nodeType = UA_NODEID_NUMERIC(2, typeId);

                string name = getJSON_data(p->second, "<xmlattr>.name");
                string id = getJSON_data(p->second, "<xmlattr>.id");

                bool setNotifiers = false;
                if (display.compare("Linear") == 0 || display.compare("Rotary") == 0)
                {
                    if (name.length() > 0)
                        display += "[" + name + "]";

                    setNotifiers = true;
                }

                UA_ObjectAttributes oAttr = UA_ObjectAttributes_default;
                oAttr.displayName = UA_LOCALIZEDTEXT_ALLOC("en-US", display.c_str());
                UA_Server_addObjectNode(m_uaServer, UA_NODEID_NULL,
                                        topNode,
                                        UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
                                        UA_QUALIFIEDNAME_ALLOC(m_namespace, display.c_str()),
                                        nodeType,
                                        oAttr,
                                        NULL,
                                        &nextId);


                if (name.length() > 0)
                    addProperty(nextId, "Name", name);

                UA_String value;
                value = UA_STRING_ALLOC(id.c_str());
                UA_Server_writeObjectProperty_scalar(m_uaServer, nextId,
                                              UA_QUALIFIEDNAME(2, "XmlId"),
                                              &value, &UA_TYPES[UA_TYPES_STRING]);

                vector<UA_NodeId> copyNodePath(nodePath);
                copyNodePath.push_back(nextId);

                if (setNotifiers)
                    addNotifierToAll(copyNodePath);

                setupMetaInfo(deviceUUID, nextId, p->second, path + "." + display, copyNodePath);
            }
        }
        else
        {
            UA_VariableAttributes mnAttr = UA_VariableAttributes_default;
            UA_String value = UA_STRING_ALLOC(p->second.data().c_str());
            UA_Variant_setScalar(&mnAttr.value, &value, &UA_TYPES[UA_TYPES_STRING]);
            mnAttr.displayName = UA_LOCALIZEDTEXT_ALLOC("en-US", display.c_str());
            UA_Server_addVariableNode(m_uaServer, UA_NODEID_NULL, topNode,
                                      UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY),
                                      UA_QUALIFIEDNAME_ALLOC(m_namespace, display.c_str()),
                                      UA_NODEID_NUMERIC(0, UA_NS0ID_PROPERTYTYPE), mnAttr, NULL, NULL);
        }
    }
}

void agentHandler::setProbeInfo(string probeXml)
{
    ptree probeInfo;
    try {
         // Read the stringstream into a Boost property tree, m_ptree
        istringstream iss;
        iss.str (probeXml);

        boost::property_tree::read_xml( iss, probeInfo );
    }
    catch (exception & e)
    {
        std::cerr << e.what() << endl;
        return;
    }

    ptree& devices = probeInfo.get_child("MTConnectDevices.Devices");
    for (ptree::iterator pos = devices.begin(); pos != devices.end(); pos++)
    {
        ptree& device = pos->second;

        string deviceName = getJSON_data(device, "<xmlattr>.name");
        string deviceUUID = getJSON_data(device, "<xmlattr>.uuid");
        string deviceId = getJSON_data(device, "<xmlattr>.id");
        string description = getJSON_data(device, "Description");
        string displayName = deviceName + " [" + deviceUUID + "]";

        UA_NodeId deviceNodeId; /* get the nodeid assigned by the server */
        UA_ObjectAttributes oAttr = UA_ObjectAttributes_default;
        oAttr.displayName = UA_LOCALIZEDTEXT_ALLOC("en-US", deviceName.c_str());

        UA_Server_addObjectNode(m_uaServer, UA_NODEID_NUMERIC(m_namespace, 0),
                                m_topNode,
                                UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
                                UA_QUALIFIEDNAME_ALLOC(m_namespace, deviceName.c_str()),
                                UA_NODEID_NUMERIC(2, MT_MTDEVICETYPE),
                                oAttr, NULL, &deviceNodeId);


        UA_String value;
        value = UA_STRING_ALLOC(deviceId.c_str());
        UA_Server_writeObjectProperty_scalar(m_uaServer, deviceNodeId,
                                      UA_QUALIFIEDNAME(2, "XmlId"),
                                      &value, &UA_TYPES[UA_TYPES_STRING]);

        value = UA_STRING_ALLOC(deviceUUID.c_str());
        UA_Server_writeObjectProperty_scalar(m_uaServer, deviceNodeId,
                                      UA_QUALIFIEDNAME(2, "Uuid"),
                                      &value, &UA_TYPES[UA_TYPES_STRING]);

        value = UA_STRING_ALLOC(deviceName.c_str());
        UA_Server_writeObjectProperty_scalar(m_uaServer, deviceNodeId,
                                      UA_QUALIFIEDNAME(2, "Name"),
                                      &value, &UA_TYPES[UA_TYPES_STRING]);

        try {

            UA_NodeId descriptionNodeId;

            ptree& deviceDescription = device.get_child("Description.<xmlattr>");
            oAttr.displayName = UA_LOCALIZEDTEXT_ALLOC("en-US", "Description");
            UA_Server_addObjectNode(m_uaServer, UA_NODEID_NUMERIC(m_namespace, 0),
                                    deviceNodeId,
                                    UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                                    UA_QUALIFIEDNAME(m_namespace, "Description"),
                                    UA_NODEID_NUMERIC(2, MT_MTDESCRIPTIONTYPE),
                                    oAttr, NULL, &descriptionNodeId);

            setProperties(descriptionNodeId, deviceDescription);

        } catch (...) {
        }


        addNotifier(UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER), deviceNodeId);

#if 0
        try {

            ptree& deviceComponents = device.get_child("DataItems");
            for (ptree::iterator pos = deviceComponents.begin(); pos != deviceComponents.end(); pos++)
                addDeviceDataItem(deviceNodeId, pos->second.get_child("<xmlattr>"));

        } catch (...) {
        }
#endif

#if 0
        if (findChildId(deviceNodeId,
                    UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY),
                    UA_QUALIFIEDNAME(2, "XmlId"), &childID))
        {
            writeNodeData(childID, deviceId);
        }

        if (findChildId(deviceNodeId,
                    UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY),
                    UA_QUALIFIEDNAME(2, "Uuid"), &childID))
        {
            writeNodeData(childID, deviceUUID);
        }

        if (findChildId(deviceNodeId,
                    UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY),
                    UA_QUALIFIEDNAME(2, "Name"), &childID))
        {
            writeNodeData(childID, deviceName);
        }
#endif


        string settingsName = Settings::getSettingsName("OPCUA-" + deviceUUID);
//        m_settings.restore(settingsName);

        vector<UA_NodeId> nodePath;

        nodePath.push_back(deviceNodeId);
        setupMetaInfo(deviceUUID, deviceNodeId, device, "", nodePath);
//        m_settings.save(settingsName);
    }

}

bool agentHandler::process(string xmlText)
{
    m_xml = xmlText;

    try {
         // Read the stringstream into a Boost property tree, m_ptree
        istringstream iss;
        iss.str (m_xml);

        boost::property_tree::read_xml( iss, m_ptree );
    }
    catch (exception & e)
    {
        std::cerr << e.what() << endl;
        return false;
    }

    return true;
}



string agentHandler::getJSON_data(ptree & tree, string path)
{
    try {
        ptree& node = tree.get_child(path);
        return node.data();

    } catch (...) {
        return "";
    }
}

UA_NodeId agentHandler::addDeviceDataItem(string deviceUUID, UA_NodeId &topNode, ptree &dataItem, bool appendName, string path, vector<UA_NodeId> &nodePath)
{
    std::map<string, string> m;

    for (ptree::iterator pos = dataItem.begin(); pos != dataItem.end(); pos++)
    {
        string key = pos->first;
        string value = pos->second.data();

         m[key] = value;
    }

    string id = m["id"];
    string type = m["type"];
    string subType = m["subType"];
    string category = m["category"];
    string name = m["name"];

    UA_NodeId nextId;
    string subTypeCamelCase;
    string displayName = toCamelCase(type);

    if (subType.length() > 0)
        subTypeCamelCase = toCamelCase(subType);

    string actualDisplayName = subTypeCamelCase + displayName;
    if (category.compare("CONDITION") == 0)
        actualDisplayName += "Condition";

    if (name.length() > 0 && appendName)
        actualDisplayName += "[" + name + "]";

    string nodeIdentifier = deviceUUID + "." + id;
    if (category.compare("EVENT") == 0)
    {        
        string eventClassType = displayName+"ClassType";

        UA_NodeId eventNodeType = m_eventTypes[eventClassType];
        if (UA_NodeId_isNull(&eventNodeType))
            eventNodeType = UA_NODEID_NUMERIC(2, MT_MTSTRINGEVENTTYPE);

        UA_VariableAttributes oAttr = UA_VariableAttributes_default;
        oAttr.valueRank = UA_VALUERANK_SCALAR;
        oAttr.displayName = UA_LOCALIZEDTEXT_ALLOC("en-US", actualDisplayName.c_str());

        UA_UInt32 eventClassID = eventNodeType.identifier.numeric;
        if (eventClassID == MT_MTSTRINGEVENTTYPE)
        {
             oAttr.dataType = UA_NODEID_NUMERIC(0, UA_NS0ID_STRING);
             m_fieldDataTypes.insert(pair<string, UA_NodeId>(nodeIdentifier, UA_NODEID_NUMERIC(0, UA_NS0ID_STRING)));
        }
        else if (eventClassID == MT_MTNUMERICEVENTTYPE)
        {
            oAttr.dataType = UA_NODEID_NUMERIC(0, UA_NS0ID_DOUBLE);
            m_fieldDataTypes.insert(pair<string, UA_NodeId>(nodeIdentifier, UA_NODEID_NUMERIC(0, UA_NS0ID_DOUBLE)));
        }
        else if (eventClassID == MT_MTCONTROLLEDVOCABEVENTTYPE)
        {
            oAttr.dataType = UA_NODEID_NUMERIC(0, UA_NS0ID_UINT32);
            m_fieldDataTypes.insert(pair<string, UA_NodeId>(nodeIdentifier, UA_NODEID_NUMERIC(0, UA_NS0ID_ENUMVALUETYPE)));
        }


        if (type.compare("ASSET_CHANGED") == 0 || type.compare("ASSET_REMOVED") == 0)
        {
            eventNodeType = UA_NODEID_NUMERIC(2, MT_MTASSETEVENTTYPE);
            oAttr.dataType = UA_NODEID_NUMERIC(2, MT_ASSETEVENTDATATYPE);
        }

        UA_StatusCode statusCode = UA_Server_addVariableNode(m_uaServer,
                                UA_NODEID_STRING_ALLOC(m_namespace, nodeIdentifier.c_str()),
                                topNode,
                                UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                                UA_QUALIFIEDNAME_ALLOC(m_namespace, actualDisplayName.c_str()),
                                eventNodeType,
                                oAttr, NULL, &nextId);

        UA_ExpandedNodeId targetExpId;

        targetExpId.nodeId       = nextId;
        targetExpId.namespaceUri = UA_STRING_NULL;
        targetExpId.serverIndex  = 0;

        UA_Server_addReference(m_uaServer, UA_NODEID_NUMERIC(2, mtIDMap[displayName + "ClassType"]), UA_NODEID_NUMERIC(2, MT_HASMTCLASSTYPE), targetExpId, false);

        if (subType.length() > 0)
        {
            // add MTSubTypeName property
            addProperty(nextId, "MTSubTypeName", subType);
            UA_Server_addReference(m_uaServer, UA_NODEID_NUMERIC(2, mtIDMap[subTypeCamelCase + "SubClassType"]), UA_NODEID_NUMERIC(2, MT_HASMTSUBCLASSTYPE), targetExpId, false);
        }

        addNotifierToAll(nodePath);

    }
    else if (category.compare("CONDITION") == 0)
    {       
        UA_ObjectAttributes oAttr = UA_ObjectAttributes_default;
        oAttr.displayName = UA_LOCALIZEDTEXT_ALLOC("en-US", actualDisplayName.c_str());

        m_fieldDataTypes.insert(pair<string, UA_NodeId>(nodeIdentifier, UA_NODEID_NUMERIC(0, UA_NS0ID_CONDITIONTYPE)));

        UA_Server_addObjectNode(m_uaServer,
                                UA_NODEID_STRING_ALLOC(m_namespace, nodeIdentifier.c_str()),
                                topNode ,
                                UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                                UA_QUALIFIEDNAME_ALLOC(m_namespace, actualDisplayName.c_str()),
                                 UA_NODEID_NUMERIC(2, MT_MTCONDITIONTYPE),
                                oAttr, NULL, &nextId);

        UA_ExpandedNodeId targetExpId;

        targetExpId.nodeId       = nextId;
        targetExpId.namespaceUri = UA_STRING_NULL;
        targetExpId.serverIndex  = 0;

        UA_Server_addReference(m_uaServer, topNode, UA_NODEID_NUMERIC(0, UA_NS0ID_HASCONDITION), targetExpId, true);

        UA_Server_addReference(m_uaServer, UA_NODEID_NUMERIC(2, MT_SYSTEMCLASSTYPE), UA_NODEID_NUMERIC(2, MT_HASMTCLASSTYPE), targetExpId, false);
        addNotifier(topNode, nextId);
    }
    else if (category.compare("SAMPLE") == 0)
    {
        UA_NodeId nodeType = UA_NODEID_NUMERIC(2, MT_MTSAMPLETYPE);

        UA_VariableAttributes oAttr = UA_VariableAttributes_default;
        oAttr.valueRank = UA_VALUERANK_SCALAR;
        oAttr.displayName = UA_LOCALIZEDTEXT_ALLOC("en-US", actualDisplayName.c_str());

        if (type.compare("PATH_POSITION") == 0)
        {
            nodeType = UA_NODEID_NUMERIC(2, MT_MTTHREESPACESAMPLETYPE);
            oAttr.dataType = UA_NODEID_NUMERIC(2, MT_THREESPACESAMPLEDATATYPE);
        }
        else
            oAttr.dataType = UA_NODEID_NUMERIC(0, UA_NS0ID_DOUBLE);

        m_fieldDataTypes.insert(pair<string, UA_NodeId>(nodeIdentifier, oAttr.dataType));


        UA_StatusCode statusCode = UA_Server_addVariableNode(m_uaServer,
                                UA_NODEID_STRING_ALLOC(m_namespace, nodeIdentifier.c_str()),
                                topNode,
                                UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                                UA_QUALIFIEDNAME_ALLOC(m_namespace, actualDisplayName.c_str()),
                                nodeType,
                                oAttr, NULL, &nextId);

        UA_ExpandedNodeId targetExpId;

        targetExpId.nodeId       = nextId;
        targetExpId.namespaceUri = UA_STRING_NULL;
        targetExpId.serverIndex  = 0;

        UA_Server_addReference(m_uaServer, UA_NODEID_NUMERIC(2, mtIDMap[displayName + "ClassType"]), UA_NODEID_NUMERIC(2, MT_HASMTCLASSTYPE), targetExpId, false);

        if (subType.length() > 0)
        {
            // add MTSubTypeName property
            addProperty(nextId, "MTSubTypeName", subType);

            UA_Server_addReference(m_uaServer, UA_NODEID_NUMERIC(2, mtIDMap[subTypeCamelCase + "SubClassType"]), UA_NODEID_NUMERIC(2, MT_HASMTSUBCLASSTYPE), targetExpId, false);
        }


        UA_NodeId nodeId;
        if (findChildId(nextId, UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY), UA_QUALIFIEDNAME(0, "EngineeringUnits"), &nodeId))
        {
            UA_EUInformation euData;

            EuInfo *euInfo = euStore[m["units"]];

            if (euInfo)
            {
                euData.namespaceUri = UA_STRING("http://www.opcfoundation.org/UA/units/un/cefact");
                euData.unitId = euInfo->m_unitId;
                euData.displayName = UA_LOCALIZEDTEXT_ALLOC("", euInfo->m_displayName.c_str());
                euData.description = UA_LOCALIZEDTEXT_ALLOC("", euInfo->m_description.c_str());

                UA_Variant v;
                UA_Variant_setScalar(&v, &euData, &UA_TYPES[UA_TYPES_EUINFORMATION]);

                UA_Server_writeValue(m_uaServer, nodeId, v);
            }
        }
    }

    if (category.compare("CONDITION") != 0)
    {
        UA_WriteValue wv;

        UA_WriteValue_init(&wv);
        wv.nodeId = nextId;
        wv.attributeId = UA_ATTRIBUTEID_VALUE;
        wv.value.status = UA_STATUSCODE_BADDATAUNAVAILABLE;
        wv.value.hasStatus = true;
        UA_StatusCode status = UA_Server_write(m_uaServer, &wv);
    }

    m.erase("id");
    m.erase("type");
    m.erase("subType");
    m.erase("category");
//        m.erase("name");

    for (std::map<string, string>::iterator it=m.begin(); it !=m.end(); ++it)
    {
        string key = it->first;
        string value = it->second;

        if (value.length() > 0)
            addProperty(nextId, toCamelCase(key), value);
    }

    UA_Variant v;
    UA_NodeId nodeId;
    UA_String value;

//cerr << type << endl;
//cerr << id << endl;
    value = UA_STRING_ALLOC(id.c_str());
    UA_Variant_setScalar(&v, &value, &UA_TYPES[UA_TYPES_STRING]);

    if (findChildId(nextId, UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY), UA_QUALIFIEDNAME(2, "XmlId"), &nodeId))
            UA_Server_writeValue(m_uaServer, nodeId, v);

    value = UA_STRING_ALLOC(type.c_str());
    UA_Variant_setScalar(&v, &value, &UA_TYPES[UA_TYPES_STRING]);
    if (findChildId(nextId, UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY), UA_QUALIFIEDNAME(2, "MTTypeName"), &nodeId))
            UA_Server_writeValue(m_uaServer, nodeId, v);


#if 0
    UA_EnumValueType enumData;

    UA_NodeId nodeTypeId = UA_NODEID_NUMERIC(2, typeId);
    const UA_DataType *dataType = UA_findDataType(&nodeTypeId);

    enumData.value = 0;
    enumData.displayName = UA_LOCALIZEDTEXT_ALLOC("", "");
    enumData.description = UA_LOCALIZEDTEXT_ALLOC("", category.c_str());
    UA_Server_writeObjectProperty_scalar(m_uaServer, nextId,
                                  UA_QUALIFIEDNAME(2, "Category"),
                                  &enumData,
                                  &UA_TYPES[UA_TYPES_ENUMVALUETYPE]);
#endif

    UA_Int32 dd = m_typesMgr.lookup("MTCategoryType", category);
    UA_Variant_setScalar(&v, &dd, &UA_TYPES[UA_TYPES_INT32]);
    if (findChildId(nextId, UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY), UA_QUALIFIEDNAME(2, "Category"), &nodeId))
            UA_Server_writeValue(m_uaServer, nodeId, v);

    if (findChildId(nextId, UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY), UA_QUALIFIEDNAME(0, "EnumStrings"), &nodeId))
    {
        UA_Variant enumValues;
        bool enumLoaded = true;
        if (!m_typesMgr.getDictionary(displayName + "DataType", enumValues))
        {
            enumLoaded = m_typesMgr.getDictionary(displayName + "ClassType", enumValues);
            if (!enumLoaded)
                cerr << "Cannot load enums for " << displayName << endl;
        }

        if (enumLoaded == true)
            UA_Server_writeValue(m_uaServer, nodeId, enumValues);
    }

    return nextId;
}

void agentHandler::setProperties(UA_NodeId &topNode, ptree &dataItem)
{
    for (ptree::iterator pos = dataItem.begin(); pos != dataItem.end(); pos++)
    {
        string key = pos->first;
        string value = pos->second.data();

        UA_NodeId nextId;

        UA_VariableAttributes oAttr = UA_VariableAttributes_default;
        oAttr.displayName = UA_LOCALIZEDTEXT_ALLOC("en-US", key.c_str());

        UA_Server_addVariableNode(m_uaServer, UA_NODEID_NUMERIC(m_namespace, 0),
                                topNode,
                                UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY),
                                UA_QUALIFIEDNAME_ALLOC(m_namespace, key.c_str()),
                                UA_NODEID_NUMERIC(0, UA_NS0ID_PROPERTYTYPE),
                                oAttr, NULL, &nextId);


        writeNodeData(nextId, value);
    }
}


bool agentHandler::isLeafNode(ptree::iterator &p)
{
    size_t size = p->second.size();

    if (size > 1)
        return false;

    if (size == 0)
        return true;

    try {
        ptree& node = p->second.get_child("<xmlattr>");
        return true;

    } catch (...) {
        return false;
    }
}


void agentHandler::writeData(string variable, string dateTime, string data)
{
    UA_NodeId nodeId = UA_NODEID_STRING_ALLOC(m_namespace, variable.c_str());

    if (data.compare("UNAVAILABLE") == 0)
    {
        UA_WriteValue wv;

        UA_WriteValue_init(&wv);
        wv.nodeId = nodeId;
        wv.attributeId = UA_ATTRIBUTEID_VALUE;
        wv.value.status = UA_STATUSCODE_BADNOTCONNECTED;
        wv.value.hasStatus = true;
        UA_Server_write(m_uaServer, &wv);
        return;
    }

    UA_Variant myVar;
    UA_Variant_init(&myVar);

    UA_NodeId typeNode  = m_fieldDataTypes[variable];

    UA_Double myDouble;
    UA_String myString;
    UA_UInt32 myInt32;
    UA_NodeId n;

    UA_UInt32 dataType = typeNode.identifier.numeric;
    switch (dataType)
    {
    case UA_NS0ID_CONDITIONTYPE:
        // to do
        return;
        break;

    case UA_NS0ID_DOUBLE:
        myDouble = atof(data.c_str());
        UA_Variant_setScalar(&myVar, &myDouble, &UA_TYPES[UA_TYPES_DOUBLE]);
        break;

    case MT_THREESPACESAMPLEDATATYPE:
        // to do
        return;
        break;

    case MT_ASSETEVENTDATATYPE:
        // to do
        return;
        break;

    case UA_NS0ID_ENUMVALUETYPE:
        myInt32 = m_typesMgr.lookup(variable, nodeId, data);

        // unknown
        if (myInt32 < 0)
        {
            cerr << "cannot map " << data << endl;
            return;
        }

//        cerr << "map " << data << " to " << myInt32 << endl;
        UA_Variant_setScalar(&myVar, &myInt32, &UA_TYPES[UA_TYPES_UINT32]);
        break;

    case UA_NS0ID_STRING:
        myString = UA_STRING_ALLOC(data.c_str());
        UA_Variant_setScalar(&myVar, &myString, &UA_TYPES[UA_TYPES_STRING]);
        break;

    default:
        if (findChildId(nodeId, UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY), UA_QUALIFIEDNAME(0, "EnumStrings"), &n))
        {
            myInt32 = m_typesMgr.lookup(variable, nodeId, data);
            if (myInt32 >= 0)
            {
                UA_Variant_setScalar(&myVar, &myInt32, &UA_TYPES[UA_TYPES_UINT32]);

                UA_Variant v;

                myString = UA_STRING_ALLOC(data.c_str());
                UA_Variant_setScalar(&v, &myString, &UA_TYPES[UA_TYPES_STRING]);

                if (findChildId(nodeId, UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY), UA_QUALIFIEDNAME(2, "ValueAsText"), &n))
                        UA_Server_writeValue(m_uaServer, n, v);

                cerr << "map " << data << " to " << myInt32 << endl;

                // set it to UA_NS0ID_ENUMVALUETYPE, so next time skip the enum load process
                m_fieldDataTypes[variable] = UA_NODEID_NUMERIC(0, UA_NS0ID_ENUMVALUETYPE);
                break;
            }
            else
                // treat it as string next time
                m_fieldDataTypes[variable] = UA_NODEID_NUMERIC(0, UA_NS0ID_STRING);
        }
        myString = UA_STRING_ALLOC(data.c_str());
        UA_Variant_setScalar(&myVar, &myString, &UA_TYPES[UA_TYPES_STRING]);
        break;
    }

    struct tm tm;
    strptime(dateTime.c_str(), "%Y-%m-%dT%H:%M:%S", &tm);
    time_t dt = mktime(&tm);

    unsigned long pos = dateTime.find_first_of(".");
    int milliSec = std::stoi(dateTime.substr(pos + 1, 3));

    UA_WriteValue wv;
    UA_WriteValue_init(&wv);
    wv.nodeId = nodeId;
    wv.attributeId = UA_ATTRIBUTEID_VALUE;

    wv.value.sourceTimestamp = UA_DateTime_fromUnixTime(dt) + milliSec * UA_DATETIME_MSEC;
    wv.value.hasSourceTimestamp = true;
    wv.value.hasStatus = false;
    wv.value.value = myVar;
    wv.value.hasValue = true;
    UA_StatusCode status = UA_Server_write(m_uaServer, &wv);
}


bool agentHandler::findChildId(UA_NodeId &parentNode, UA_NodeId referenceType,
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

bool agentHandler::writeNodeData(UA_NodeId &nodeId, string result)
{
    UA_Variant data;
    UA_Variant_init(&data);

    UA_String value = UA_STRING_ALLOC(result.c_str());

    UA_Variant_setScalar(&data, &value, &UA_TYPES[UA_TYPES_STRING]);

    UA_Server_writeValue(m_uaServer, nodeId, data);

    return true;
}

string agentHandler::toCamelCase(string &input)
{
    const char *line = input.c_str();
    char *ret = (char *)malloc(input.size() + 1);
    char *p = ret;
    bool wordStart = true;

    for (size_t index = 0; index < input.size(); index++)
    {
        char c = line[index];
        if (isalpha(c))
        {
            if (wordStart)
            {
                *p++ = toupper(c);
                wordStart = false;
            }
            else
                *p++ = tolower(c);
        }
        else
        {
            // allow only ' ' and _ to separate words
            // otherwise take the last word
            if (c != ' ' && c != '_')
            {
                *p = '\0';
                p = ret;
            }
            wordStart = true;
        }
    }

    *p = '\0';
    string result = string(ret);
    free(ret);

    return result;
}

void agentHandler::getEventTypes(UA_NodeId eventRootNode, UA_NodeId eventTypeNode, map<string, UA_NodeId> &results)
{
    UA_NodeId ret = { 0 };
    UA_ReferenceDescription *ref;

    UA_BrowseResult res = { 0 };
    UA_BrowseDescription b_desc = { 0 };

    UA_BrowseDescription_init(&b_desc);
    b_desc.nodeId = eventRootNode;
    b_desc.resultMask = UA_BROWSERESULTMASK_ALL;
    b_desc.referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE);

    res = UA_Server_browse(m_uaServer, 0, &b_desc);
    if (res.referencesSize < 1)
        goto err;

    for (int i=0; i<res.referencesSize; i++)
    {
        ref = &(res.references[i]);
        results.insert(make_pair((const char *)ref->displayName.text.data, eventTypeNode));
    }

  err:
    UA_BrowseDescription_deleteMembers(&b_desc);
    UA_BrowseResult_deleteMembers(&res);
}


UA_NodeId agentHandler::addProperty(UA_NodeId parentNode, string propertyName, string value)
{
    UA_NodeId outNodeId;
    UA_VariableAttributes mnAttr = UA_VariableAttributes_default;
    mnAttr.dataType = UA_NODEID_NUMERIC(0, UA_NS0ID_STRING);

    UA_String data = UA_STRING_ALLOC(value.c_str());
    UA_Variant_setScalar(&mnAttr.value, &data, &UA_TYPES[UA_TYPES_STRING]);

    mnAttr.displayName = UA_LOCALIZEDTEXT_ALLOC("en-US", propertyName.c_str());
    UA_Server_addVariableNode(m_uaServer,
                              UA_NODEID_NUMERIC(m_namespace, 0),
                              parentNode,
                              UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY),
                              UA_QUALIFIEDNAME_ALLOC(2, propertyName.c_str()),
                              UA_NODEID_NUMERIC(0, UA_NS0ID_PROPERTYTYPE),
                              mnAttr,
                              NULL,
                              &outNodeId);

    return outNodeId;
}


void agentHandler::addNotifier(UA_NodeId targetNode, UA_NodeId srcNode)
{
//    cout << targetNode.identifier.numeric << " -> " << srcNode.identifier.numeric << endl;
    if (targetNode.identifier.numeric == srcNode.identifier.numeric)
        return;

    if (srcNode.identifier.numeric == 0)
        return;

    UA_ExpandedNodeId targetExpId;

    targetExpId.nodeId       = srcNode;
    targetExpId.namespaceUri = UA_STRING_NULL;
    targetExpId.serverIndex  = 0;

    UA_Server_addReference(m_uaServer, targetNode,
                           UA_NODEID_NUMERIC(0, UA_NS0ID_HASNOTIFIER), targetExpId, true);

    UA_Server_writeEventNotifier(m_uaServer, targetNode, UA_EVENTNOTIFIERTYPE_SUBSCRIBETOEVENTS);
}

void agentHandler::addNotifierToAll(vector<UA_NodeId> & nodePath)
{
    for (vector<UA_NodeId>::iterator it = nodePath.begin() ; it != nodePath.end(); )
    {
        vector<UA_NodeId>::iterator n = it++;
        if (it == nodePath.end())
            break;

        addNotifier(*n, *it);
    }
}


bool agentHandler::updateData()
{
    bool ret = true;

    try {

        // each line is one component stream
        ptree& devices = m_ptree.get_child("MTConnectStreams.Streams");
        for (ptree::iterator pos = devices.begin(); pos != devices.end(); pos++)
        {
            ptree& device = pos->second;

            string deviceName = getJSON_data(device, "<xmlattr>.name");
            string deviceUUID = getJSON_data(device, "<xmlattr>.uuid");

            int rec_count = 0;

            for (ptree::iterator p = device.begin(); p != device.end(); p++)
            {
                string key = p->first;
                if (key.compare("ComponentStream") == 0)
                {
                    ptree &stream = p->second;

                    string componentId = getJSON_data(stream, "<xmlattr>.componentId");

                    for (ptree::iterator s = stream.begin(); s != stream.end(); s++)
                    {
                        if (s->first.compare("<xmlattr>"))
                        {
                            rec_count += updateDeviceData(deviceName, deviceUUID, componentId, s->second);
                        }
                    }
                }
            }
        }
    }
    catch (exception& e)
    {
        cerr << e.what() << endl;
        cerr << m_xml << endl;
        ret = false;
    }

    return ret;
}

int agentHandler::updateDeviceData(const string &deviceName, const string &deviceUUID, const string &componentId, ptree &pt)
{
    int rec_count = 0;

    // output first any unique subtrees
    for (ptree::iterator pos = pt.begin(); pos != pt.end(); pos++)
    {
        string dataItemId = getJSON_data(pos->second, "<xmlattr>.dataItemId");

        if (dataItemId.length() > 0)
        {
            string timestamp = getJSON_data(pos->second, "<xmlattr>.timestamp");
            string dataPoint = pos->second.data();

            string key = deviceUUID + "." + dataItemId;

            writeData(key, timestamp, dataPoint);
        }
        else
            rec_count += updateDeviceData(deviceName, deviceUUID, componentId, pos->second);
    }

    return rec_count;
}
