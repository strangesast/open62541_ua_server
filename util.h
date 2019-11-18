#ifndef UTIL_H
#define UTIL_H

class util
{
public:
    static string toString(UA_String& ua_str) {
        return std::string(reinterpret_cast<char*>(ua_str.data), ua_str.length);
    }

    static UA_String toUAString(string &str) {
        return UA_STRING((char*) str.c_str());
    }

    static UA_LocalizedText toUALocalizedText(string &str) {
        return UA_LOCALIZEDTEXT("en-US", (char*) str.c_str());
    }

    static UA_QualifiedName toUAQualifiedName(UA_UInt16 nsIndex, string &str) {
        return UA_QUALIFIEDNAME(nsIndex, (char*) str.c_str());
    }

    static UA_NodeId toUANodeId(UA_UInt16 nsIndex, string &str) {
        return UA_NODEID_STRING (nsIndex, (char*) str.c_str());
    }
};

#endif // UTIL_H
