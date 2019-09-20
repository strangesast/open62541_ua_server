echo "#ifndef MTCONNECT_IDS_H" > mtconnect_ids.h
echo "#define MTCONNECT_IDS_H" >> mtconnect_ids.h
echo "" >> mtconnect_ids.h
echo "#include <map>" >> mtconnect_ids.h
echo "#include <string>" >> mtconnect_ids.h
echo " " >> mtconnect_ids.h

egrep -v "http|(Binary)|(XML)" MTConnect.NodeIds.csv | sort | uniq | awk -F, '{print "#define MT_" toupper($1) "\t\t" $2}' >> mtconnect_ids.h

echo " " >> mtconnect_ids.h
echo "extern std::map<std::string, UA_UInt32> mtIDMap; " >> mtconnect_ids.h
echo "#endif" >> mtconnect_ids.h


echo "#include <open62541/server.h>" > mtconnect_ids.cpp
echo "#include \"mtconnect_ids.h\" " >> mtconnect_ids.cpp
echo "std::map<std::string, UA_UInt32> mtIDMap = { " >> mtconnect_ids.cpp
egrep -v "http|(Binary)|(XML)" MTConnect.NodeIds.csv | sort | uniq | awk -F, '{print "    { \""$1"\","$2 " }, " }' >> mtconnect_ids.cpp
echo "    { \"\", 0 }" >> mtconnect_ids.cpp
echo "};" >> mtconnect_ids.cpp

