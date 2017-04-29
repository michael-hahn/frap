//
//  profile.hpp
//  graphchi_xcode
//
//  Created by Michael Hahn on 3/2/17.
//
//

#ifndef profile_hpp
#define profile_hpp

#include <vector>
#include <map>

class profile {
public:
    
    double get_mean() {
        return this->mean;
    }
    
    double get_std() {
        return this->std;
    }
    
    void set_mean(double mean) {
        this->mean = mean;
    }
    
    void set_std(double std) {
        this->std = std;
    }
    
    void add_array(std::vector<int> array) {
        this->count_arrays.push_back(array);
    }
    
    std::vector<std::vector<int>> get_count_arrays() {
        return this->count_arrays;
    }
    
    void reset_arrays() {
        this->count_arrays.clear();
        this->cluster_groups.clear();
    }
    
    void reset_map() {
        this->instance_distance_map.clear();
    }
    
    int calculate_two_count_arrays(int method, std::vector<int> arr1, std::vector<int> arr2);//for old simple normal distribution analysis only
    
    double calculate_distance(int method, std::vector<int> count_array1, std::vector<int> count_array2);
    
    void remove_array(int pos) {
        std::vector<std::vector<int>>::iterator itr = count_arrays.begin();
        this->count_arrays.erase(itr + pos);
    }
    
private:
    
    double mean;
    
    double std;
    
    std::vector<std::vector<int>> count_arrays;
    
    //Centroids of all clusters
    std::vector<std::vector<double>> centroids;
    
    //map that records the longest distance between a well-behaved instance and the rest of the well-behaved ones
    std::map<int, double> instance_distance_map;
    
    //each vector contains the id of instances that belongs together
    std::vector<std::vector<int>> cluster_groups;
};

#include "profile.cpp"
#endif /* profile_hpp */
