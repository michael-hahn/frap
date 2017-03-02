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
    
    int calculate_two_count_arrays(int method, std::vector<int> arr1, std::vector<int> arr2);
    
private:
    
    double mean;
    
    double std;
    
    std::vector<std::vector<int>> count_arrays;
};

#include "profile.cpp"
#endif /* profile_hpp */
