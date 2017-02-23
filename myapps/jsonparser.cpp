//
//  jsonparser.cpp
//  graphchi_xcode
//
//  Created by Michael Hahn on 2/22/17.
//
//

#include "jsonparser.hpp"
#include "jsoncpp/dist/jsoncpp.cpp"
#include "jsoncpp/dist/json/json.h"
#include "jsoncpp/dist/json/json-forwards.h"
#include <iostream>
#include <fstream>
#include <cassert>
#include <string>
#include <sstream>

class Maps {
public:
    Maps();
    ~Maps();
    bool insert_id(std::map<std::string, int>& map, std::string id_str, int id);
    bool insert_id_type(std::map<int, int>& map, int id, int type);
    std::map<std::string, int> vertex_map;//map string vertex ids to integer ids
    std::map<int, int> vertex_id_type_map;//map vertex int id to its int type
    std::map<std::string, int> vertex_type_map;//map string vertex type to integers
    std::map<std::string, int> edge_type_map;//map string edge type to integers
};

//Maps constructer constructs vertex_type_map and edge_type_map right away since they are fixed
Maps::Maps() {
    vertex_type_map["unknown"] = 0;
    vertex_type_map["task"] = 1;
    vertex_type_map["link"] = 2;
    vertex_type_map["socket"] = 3;
    vertex_type_map["iattr"] = 4;
    vertex_type_map["mmaped_file"] = 5;
    vertex_type_map["packet"] = 6;
    vertex_type_map["disc_node"] = 7;
    vertex_type_map["disc_agent"] = 8;
    vertex_type_map["disc_activity"] = 9;
    vertex_type_map["disc_entity"] = 10;
    vertex_type_map["file_name"] = 11;
    vertex_type_map["sb"] = 12;
    vertex_type_map["address"] = 13;
    vertex_type_map["sock"] = 14;
    vertex_type_map["shm"] = 15;
    vertex_type_map["msg"] = 16;
    vertex_type_map["fifo"] = 17;
    vertex_type_map["block"] = 18;
    vertex_type_map["char"] = 19;
    vertex_type_map["directory"] = 20;
    vertex_type_map["file"] = 21;
    vertex_type_map["inode_unknown"] = 22;
    vertex_type_map["relation"] = 23;
    vertex_type_map["string"] = 24;
    vertex_type_map["xattr"] = 25;
    vertex_type_map["packet_content"] = 26;
    edge_type_map["read"] = 0;
    edge_type_map["write"] = 1;
    edge_type_map["create"] = 2;
    edge_type_map["mmap_write"] = 3;
    edge_type_map["open"] = 4;
    edge_type_map["version_entity"] = 5;
    edge_type_map["named"] = 6;
    edge_type_map["exec"] = 7;
    edge_type_map["clone"] = 8;
    edge_type_map["mmap_read"] = 9;
    edge_type_map["mmap_exec"] = 10;
    edge_type_map["perm_read"] = 11;
    edge_type_map["perm_exec"] = 12;
    edge_type_map["unknown"] = 13;
    edge_type_map["change"] = 14;
    edge_type_map["bind"] = 15;
    edge_type_map["connect"] = 16;
    edge_type_map["listen"] = 17;
    edge_type_map["accept"] = 18;
    edge_type_map["link"] = 19;
    edge_type_map["search"] = 20;
    edge_type_map["send"] = 21;
    edge_type_map["receive"] = 22;
    edge_type_map["perm_write"] = 23;
    edge_type_map["sh_write"] = 24;
    edge_type_map["mmap"] = 25;
    edge_type_map["setattr"] = 26;
    edge_type_map["setxattr"] = 27;
    edge_type_map["removexattr"] = 28;
    edge_type_map["named_process"] = 29;
    edge_type_map["exec_process"] = 30;
    edge_type_map["version_activity"] = 31;
    edge_type_map["getattr"] = 32;
    edge_type_map["getxattr"] = 33;
    edge_type_map["listxattr"] = 34;
    edge_type_map["readlink"] = 35;
    edge_type_map["sh_read"] = 36;
    edge_type_map["send_packet"] = 37;
    edge_type_map["receive_packet"] = 38;
}

Maps::~Maps(){
    vertex_map.clear();
    vertex_type_map.clear();
    edge_type_map.clear();
    vertex_id_type_map.clear();
}

bool Maps::insert_id(std::map<std::string, int>& map, std::string id_str, int id) {
    std::pair<std::map<std::string, int>::iterator, bool> rst;
    rst = map.insert(std::pair<std::string, int>(id_str, id));
    if (rst.second == false)
        std::cout << "ID: " << id_str << " is already in the map." << std::endl;
    return rst.second;
}

bool Maps::insert_id_type(std::map<int, int>& map, int id, int type) {
    std::pair<std::map<int, int>::iterator, bool> rst;
    rst = map.insert(std::pair<int, int>(id, type));
    if (rst.second == false)
        std::cout << "ID: " << id << " is already in the id_type map." << std::endl;
    return rst.second;
}


int main (int argc, const char ** argv) {
    std::cout << "JSON parser use jsoncpp..." << std::endl;
    
    int counter = 0;//To create unique integer ids for vertices
    
    Maps myMaps;
    
    std::string provData;
    std::ifstream provFile("../../dataset/prov1.txt");
    std::ofstream edgelistFile("../../dataset/prov1_edgelist.txt");
    if (!edgelistFile.is_open()) {
        std::cerr << "Opening file to write failed..." <<std::endl;
        assert(edgelistFile.is_open() == true);
    }
    if (provFile.is_open()) {
        while (getline(provFile, provData)) {
            Json::Value root;//the root of the JSON object; In PROV-JSON, it contains: prefix, activity, entity, used, wasGeneratedBy, wasInformedBy, wasDerivedFrom
            Json::Reader reader;
            bool parsingSuccessful = reader.parse(provData, root);
            if (!parsingSuccessful) {
                std::cout  << "Failed to parse provData" << std::endl << reader.getFormattedErrorMessages();
                assert(parsingSuccessful == true);
            }
            if (root.isMember("activity")){
                Json::Value activities = root["activity"];
                for (Json::Value::iterator it = activities.begin(); it != activities.end(); it++) {
                    Json::Value id = it.key();
                    std::string id_str = id.asString();
                    bool rtn = myMaps.insert_id(myMaps.vertex_map, id_str, counter);
                    if (rtn) counter++;
                    Json::Value vertex = *it;
                    std::string vertex_type = vertex["prov:type"].asString();
                    int vertex_type_int = myMaps.vertex_type_map[vertex_type];
                    int vertex_id_int = myMaps.vertex_map[id_str];
                    myMaps.insert_id_type(myMaps.vertex_id_type_map, vertex_id_int, vertex_type_int);
                    //edgelistFile << vertex_id_int << "\t" << vertex_type_int << std::endl; //for debugging only
                }
            }
            if (root.isMember("entity")) {
                Json::Value entities = root["entity"];
                for (Json::Value::iterator it = entities.begin(); it != entities.end(); it++) {
                    Json::Value id = it.key();
                    std::string id_str = id.asString();
                    bool rtn = myMaps.insert_id(myMaps.vertex_map, id_str, counter);
                    if (rtn) counter++;
                    Json::Value vertex = *it;
                    std::string vertex_type = vertex["prov:type"].asString();
                    int vertex_type_int = myMaps.vertex_type_map[vertex_type];
                    int vertex_id_int = myMaps.vertex_map[id_str];
                    myMaps.insert_id_type(myMaps.vertex_id_type_map, vertex_id_int, vertex_type_int);
                    //edgelistFile << vertex_id_int << "\t" << vertex_type_int << std::endl; //for debugging only
                }
            }
            
        }
        provFile.close();
    } else {
        std::cerr << "Opening file to read failed..." << std::endl;
        assert(provFile.is_open() == true);
    }
    //read the file again to create edgelist file
    provFile.open("../../dataset/prov1.txt");
    if (provFile.is_open()) {
        while (getline(provFile, provData)) {
            Json::Value sroot;//the root of the JSON object; In PROV-JSON, it contains: prefix, activity, entity, used, wasGeneratedBy, wasInformedBy, wasDerivedFrom
            Json::Reader sreader;
            bool sparsingSuccessful = sreader.parse(provData, sroot);
            if (!sparsingSuccessful) {
                std::cout  << "Failed to parse provData again" << std::endl << sreader.getFormattedErrorMessages();
                assert(sparsingSuccessful == true);
            }
            if (sroot.isMember("used")) {
                Json::Value useds = sroot["used"];
                for (Json::Value::iterator it = useds.begin(); it != useds.end(); it++) {
                    Json::Value edge = *it;
                    std::string edge_type_string = edge["prov:type"].asString();
                    int edge_type_int = myMaps.edge_type_map[edge_type_string];
                    std::string src_vertex_id_string = edge["prov:entity"].asString();
                    std::string dst_vertex_id_string = edge["prov:activity"].asString();
                    int src_id = myMaps.vertex_map[src_vertex_id_string];
                    int dst_id = myMaps.vertex_map[dst_vertex_id_string];
                    int src_type = myMaps.vertex_id_type_map[src_id];
                    int dst_type = myMaps.vertex_id_type_map[dst_id];
                    
                    //**********For debugging purposes
                    //std::cout << "Iterating through Used:" << std::endl;
                    std::map<std::string, int>::iterator itf;
                    std::map<int, int>::iterator itf2;
                    itf = myMaps.vertex_map.find(src_vertex_id_string);
                    if (itf == myMaps.vertex_map.end()) {
                        std::cout << src_vertex_id_string << " (source) is not found in the vertex_map. " << std::endl;
                    }
                    itf = myMaps.vertex_map.find(dst_vertex_id_string);
                    if (itf == myMaps.vertex_map.end()) {
                        std::cout << dst_vertex_id_string << " (destination) is not found in the vertex_map. " << std::endl;
                    }
                    itf2 = myMaps.vertex_id_type_map.find(src_id);
                    if (itf2 == myMaps.vertex_id_type_map.end()) {
                        std::cout << src_id << " (source) is not found in the vertex_id_type_map. " << std::endl;
                    }
                    itf2 = myMaps.vertex_id_type_map.find(dst_id);
                    if (itf2 == myMaps.vertex_id_type_map.end()) {
                        std::cout << dst_id << " (destination) is not found in the vertex_id_type_map. " << std::endl;
                    }
                    //********************************
                    
                    std::string src_type_str_format;
                    std::string dst_type_str_format;
                    std::string edge_type_str_format;
                    std::ostringstream convert_src;
                    std::ostringstream convert_dst;
                    std::ostringstream convert_edge;
                    convert_src << src_type;
                    convert_dst << dst_type;
                    convert_edge << edge_type_int;
                    src_type_str_format = convert_src.str();
                    dst_type_str_format = convert_dst.str();
                    edge_type_str_format = convert_edge.str();
                    
                    
                    std::string types_str = "";
                    types_str += src_type_str_format;
                    types_str += ":";
                    types_str += dst_type_str_format;
                    types_str += ":";
                    types_str += edge_type_str_format;
                    edgelistFile << src_id << "\t" << dst_id << "\t" << types_str << std::endl;
                }
            }
            if (sroot.isMember("wasGeneratedBy")) {
                Json::Value useds = sroot["wasGeneratedBy"];
                for (Json::Value::iterator it = useds.begin(); it != useds.end(); it++) {
                    Json::Value edge = *it;
                    std::string edge_type_string = edge["prov:type"].asString();
                    int edge_type_int = myMaps.edge_type_map[edge_type_string];
                    std::string src_vertex_id_string = edge["prov:activity"].asString();
                    std::string dst_vertex_id_string = edge["prov:entity"].asString();
                    int src_id = myMaps.vertex_map[src_vertex_id_string];
                    int dst_id = myMaps.vertex_map[dst_vertex_id_string];
                    int src_type = myMaps.vertex_id_type_map[src_id];
                    int dst_type = myMaps.vertex_id_type_map[dst_id];
                    
                    //**********For debugging purposes
                    //std::cout << "Iterating through wasGeneratedBy:" << std::endl;
                    std::map<std::string, int>::iterator itf;
                    std::map<int, int>::iterator itf2;
                    itf = myMaps.vertex_map.find(src_vertex_id_string);
                    if (itf == myMaps.vertex_map.end()) {
                        std::cout << src_vertex_id_string << " (source) is not found in the vertex_map. " << std::endl;
                    }
                    itf = myMaps.vertex_map.find(dst_vertex_id_string);
                    if (itf == myMaps.vertex_map.end()) {
                        std::cout << dst_vertex_id_string << " (destination) is not found in the vertex_map. " << std::endl;
                    }
                    itf2 = myMaps.vertex_id_type_map.find(src_id);
                    if (itf2 == myMaps.vertex_id_type_map.end()) {
                        std::cout << src_id << " (source) is not found in the vertex_id_type_map. " << std::endl;
                    }
                    itf2 = myMaps.vertex_id_type_map.find(dst_id);
                    if (itf2 == myMaps.vertex_id_type_map.end()) {
                        std::cout << dst_id << " (destination) is not found in the vertex_id_type_map. " << std::endl;
                    }
                    //********************************
                    
                    std::string src_type_str_format;
                    std::string dst_type_str_format;
                    std::string edge_type_str_format;
                    std::ostringstream convert_src;
                    std::ostringstream convert_dst;
                    std::ostringstream convert_edge;
                    convert_src << src_type;
                    convert_dst << dst_type;
                    convert_edge << edge_type_int;
                    src_type_str_format = convert_src.str();
                    dst_type_str_format = convert_dst.str();
                    edge_type_str_format = convert_edge.str();
                    
                    
                    std::string types_str = "";
                    types_str += src_type_str_format;
                    types_str += ":";
                    types_str += dst_type_str_format;
                    types_str += ":";
                    types_str += edge_type_str_format;
                    edgelistFile << src_id << "\t" << dst_id << "\t" << types_str << std::endl;
                }
            }
            if (sroot.isMember("wasInformedBy")) {
                Json::Value useds = sroot["wasInformedBy"];
                for (Json::Value::iterator it = useds.begin(); it != useds.end(); it++) {
                    Json::Value edge = *it;
                    std::string edge_type_string = edge["prov:type"].asString();
                    int edge_type_int = myMaps.edge_type_map[edge_type_string];
                    std::string src_vertex_id_string = edge["prov:informant"].asString();
                    std::string dst_vertex_id_string = edge["prov:informed"].asString();
                    int src_id = myMaps.vertex_map[src_vertex_id_string];
                    int dst_id = myMaps.vertex_map[dst_vertex_id_string];
                    int src_type = myMaps.vertex_id_type_map[src_id];
                    int dst_type = myMaps.vertex_id_type_map[dst_id];
                    
                    //**********For debugging purposes
                    //std::cout << "Iterating through wasInformedBy:" << std::endl;
                    std::map<std::string, int>::iterator itf;
                    std::map<int, int>::iterator itf2;
                    itf = myMaps.vertex_map.find(src_vertex_id_string);
                    if (itf == myMaps.vertex_map.end()) {
                        std::cout << src_vertex_id_string << " (source) is not found in the vertex_map. " << std::endl;
                    }
                    itf = myMaps.vertex_map.find(dst_vertex_id_string);
                    if (itf == myMaps.vertex_map.end()) {
                        std::cout << dst_vertex_id_string << " (destination) is not found in the vertex_map. " << std::endl;
                    }
                    itf2 = myMaps.vertex_id_type_map.find(src_id);
                    if (itf2 == myMaps.vertex_id_type_map.end()) {
                        std::cout << src_id << " (source) is not found in the vertex_id_type_map. " << std::endl;
                    }
                    itf2 = myMaps.vertex_id_type_map.find(dst_id);
                    if (itf2 == myMaps.vertex_id_type_map.end()) {
                        std::cout << dst_id << " (destination) is not found in the vertex_id_type_map. " << std::endl;
                    }
                    //********************************
                    
                    std::string src_type_str_format;
                    std::string dst_type_str_format;
                    std::string edge_type_str_format;
                    std::ostringstream convert_src;
                    std::ostringstream convert_dst;
                    std::ostringstream convert_edge;
                    convert_src << src_type;
                    convert_dst << dst_type;
                    convert_edge << edge_type_int;
                    src_type_str_format = convert_src.str();
                    dst_type_str_format = convert_dst.str();
                    edge_type_str_format = convert_edge.str();
                    
                    
                    std::string types_str = "";
                    types_str += src_type_str_format;
                    types_str += ":";
                    types_str += dst_type_str_format;
                    types_str += ":";
                    types_str += edge_type_str_format;
                    edgelistFile << src_id << "\t" << dst_id << "\t" << types_str << std::endl;
                }
            }
            if (sroot.isMember("wasDerivedFrom")) {
                Json::Value useds = sroot["wasDerivedFrom"];
                for (Json::Value::iterator it = useds.begin(); it != useds.end(); it++) {
                    Json::Value edge = *it;
                    std::string edge_type_string = edge["prov:type"].asString();
                    int edge_type_int = myMaps.edge_type_map[edge_type_string];
                    std::string src_vertex_id_string = edge["prov:usedEntity"].asString();
                    std::string dst_vertex_id_string = edge["prov:generatedEntity"].asString();
                    int src_id = myMaps.vertex_map[src_vertex_id_string];
                    int dst_id = myMaps.vertex_map[dst_vertex_id_string];
                    int src_type = myMaps.vertex_id_type_map[src_id];
                    int dst_type = myMaps.vertex_id_type_map[dst_id];
                    
                    //**********For debugging purposes
                    //std::cout << "Iterating through wasDerivedFrom:" << std::endl;
                    std::map<std::string, int>::iterator itf;
                    std::map<int, int>::iterator itf2;
                    itf = myMaps.vertex_map.find(src_vertex_id_string);
                    if (itf == myMaps.vertex_map.end()) {
                        std::cout << src_vertex_id_string << " (source) is not found in the vertex_map. " << std::endl;
                    }
                    itf = myMaps.vertex_map.find(dst_vertex_id_string);
                    if (itf == myMaps.vertex_map.end()) {
                        std::cout << dst_vertex_id_string << " (destination) is not found in the vertex_map. " << std::endl;
                    }
                    itf2 = myMaps.vertex_id_type_map.find(src_id);
                    if (itf2 == myMaps.vertex_id_type_map.end()) {
                        std::cout << src_id << " (source) is not found in the vertex_id_type_map. " << std::endl;
                    }
                    itf2 = myMaps.vertex_id_type_map.find(dst_id);
                    if (itf2 == myMaps.vertex_id_type_map.end()) {
                        std::cout << dst_id << " (destination) is not found in the vertex_id_type_map. " << std::endl;
                    }
                    //********************************
        
                    std::string src_type_str_format;
                    std::string dst_type_str_format;
                    std::string edge_type_str_format;
                    std::ostringstream convert_src;
                    std::ostringstream convert_dst;
                    std::ostringstream convert_edge;
                    convert_src << src_type;
                    convert_dst << dst_type;
                    convert_edge << edge_type_int;
                    src_type_str_format = convert_src.str();
                    dst_type_str_format = convert_dst.str();
                    edge_type_str_format = convert_edge.str();
                    
                    
                    std::string types_str = "";
                    types_str += src_type_str_format;
                    types_str += ":";
                    types_str += dst_type_str_format;
                    types_str += ":";
                    types_str += edge_type_str_format;
                    edgelistFile << src_id << "\t" << dst_id << "\t" << types_str << std::endl;                }
            }
        }
        provFile.close();
    } else {
        std::cerr << "Opening file to read again failed..." << std::endl;
        assert(provFile.is_open() == true);
    }
    edgelistFile.close();
    return 0;
}