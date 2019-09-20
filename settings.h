#ifndef ITEMMANAGER_H
#define ITEMMANAGER_H

#include <string>
#include <map>

using namespace std;

class Settings
{
private:
    map< string, int> m_collection;
    int m_nextns;

public:
    static string getSettingsName(string progName);

    void restore(string filename);
    void save(string filename);
    void set(string &key, int ns);
    int get(string &key);

    void dump();

    Settings();
};

#endif // ITEMMANAGER_H
