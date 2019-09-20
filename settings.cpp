#include <iostream>
#include <fstream>

#include <boost/thread.hpp>
#include <boost/tokenizer.hpp>
#include <boost/filesystem.hpp>
#include "settings.h"


Settings::Settings()
{
    m_nextns = 1;
}

string Settings::getSettingsName(string progName)
{
    const char* homeDrive = getenv("HOMEDRIVE");
    const char* homePath = getenv("HOMEPATH");
    string homeDir = "";

    if (homeDrive != nullptr && homePath != nullptr)
    {
        // Windows
        homeDir = homeDrive;
        homeDir += homePath;
    }
    else {
        // unix
        const char *home = getenv("HOME");
        if (home != nullptr)
            homeDir = home;

        // homeDir is "" (current directory) if no environment
    }


    boost::filesystem::path dir (homeDir);
    boost::filesystem::path file (progName + ".settings");
    boost::filesystem::path fullFilename = dir / file;

    return fullFilename.string();
}

void Settings::set(string &key, int ns)
{
    map<string, int>::iterator i = m_collection.find(key);

    if (i != m_collection.end())
        i->second = ns;
    else
        m_collection.insert(pair<string, int>(key, ns));
}

int Settings::get(string &key)
{
    std::map<string, int>::iterator i = m_collection.find(key);

    if (i != m_collection.end())
        return i->second;

    int ret = m_nextns++;
    m_collection.insert(pair<string, int>(key, ret));
    return ret;
}


void Settings::dump()
{
    for (map<string, int>::iterator p = m_collection.begin(); p != m_collection.end(); p++)
        std::cout << p->first << ": " << p->second << std::endl;
}

void Settings::restore(string filename)
{
    std::ifstream os(filename);
    string input;
    while (std::getline(os, input))
    {
        unsigned long pos = input.find_first_of("|");
        string key = input.substr(0, pos);
        int value = std::stoi(input.substr(pos + 1));

        if (value > m_nextns)
            m_nextns = value + 1;

        std::cout << "Restoring - " << key << " = " << value << std::endl;
        m_collection.insert(pair<string, int>(key, value));

    }
    os.close();
}

void Settings::save(string filename)
{
    std::ofstream os(filename);
    for (map<string, int>::iterator p = m_collection.begin(); p != m_collection.end(); p++)
    {
        std::cout << "Updating - " << p->first << " = " << p->second << std::endl;
        os << p->first << "|" << p->second << std::endl;
    }

    os.close();
}
