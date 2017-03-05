//
//  kernelmaps.cpp
//  graphchi_xcode
//
//  Created by Michael Hahn on 3/2/17.
//
//

#include "kernelmaps.hpp"

KernelMaps* KernelMaps::single_instance;

KernelMaps* KernelMaps::get_instance() {
    if (!single_instance)
        single_instance = new KernelMaps();
    return single_instance;
}

KernelMaps::~KernelMaps() {
    this->relabel_map.clear();
    this->label_maps.clear();
    this->counter = 0;
    delete single_instance;
}

void KernelMaps::resetMaps() {
    this->relabel_map.clear();
    this->label_maps.clear();
    this ->counter = 0;
}

void KernelMaps::insert_label_map() {
    std::map<int, int> label_map;
    this->label_maps.push_back(label_map);
}

//insert a label into the relabel map if it does not exist in the relabel map and then return the new relabeled label
//if it does exist, return the existing relabeled label
int KernelMaps::insert_relabel(std::string label) {
    std::pair<std::map<std::string, int>::iterator, bool> rst;
    rst = relabel_map.insert(std::pair<std::string, int>(label, counter));
    if (rst.second == false) {
        logstream(LOG_INFO) << "Label " + label + " is already in the map." << std::endl;
        return rst.first->second;
    } else {
        counter++;
        return counter - 1;
    }
}

//insert int to label_map if it does not exist, or update the mapped value otherwise.
//the label in the parameter is the mapped/relabeled label in the relabel map
//always insert to the last map of the label_maps vector
void KernelMaps::insert_label(int label) {
    std::pair<std::map<int, int>::iterator, bool> rst;
    rst = this->label_maps.back().insert(std::pair<int, int>(label, 1));
    if (rst.second == false) {
        logstream(LOG_INFO) << "Label is already in the map. Updating the value..." << std::endl;
        rst.first->second++;
    }
    return;
}

std::vector<int> KernelMaps::generate_count_array(std::map<int, int>& map) {
    std::vector<int> rtn;
    std::map<int, int>::iterator itr = map.begin();
    for (int i = 0; i < this->counter; i++) {
        if (itr != map.end()) {
            if (itr->first == i) {
                rtn.push_back(itr->second);
                itr++;
            } else rtn.push_back(0);
        } else {
            rtn.push_back(0);
        }
    }
    return rtn;
}

//The rest functions are for debugging purpose only
void KernelMaps::print_relabel_map () {
    std::map<std::string, int>::iterator map_itr;
    logstream(LOG_INFO) << "Printing relabel map..." << std::endl;
    for (map_itr = this->relabel_map.begin(); map_itr != this->relabel_map.end(); map_itr++)
        logstream(LOG_INFO) << map_itr->first << ":" << map_itr->second << std::endl;
}

void KernelMaps::print_label_map (std::map<int, int>lmap) {
    std::map<int, int>::iterator map_itr;
    logstream(LOG_INFO) << "Printing label map..." << std::endl;
    for (map_itr = lmap.begin(); map_itr != lmap.end(); map_itr++)
        logstream(LOG_INFO) << map_itr->first << ":" << map_itr->second << std::endl;
}

