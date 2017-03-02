//
//  profile.cpp
//  graphchi_xcode
//
//  Created by Michael Hahn on 3/2/17.
//
//
#include <cassert>
#include "profile.hpp"

//multiple methods available now
int profile::calculate_two_count_arrays(int method, std::vector<int> arr1, std::vector<int> arr2) {
    int rtn = 0;
    std::vector<int>::iterator arr1_itr = arr1.begin();
    std::vector<int>::iterator arr2_itr = arr2.begin();
    if (method == 0) {//Sum of multiplication
        //both vectors should have the same size
        while (arr1_itr != arr1.end()) {
            rtn += *arr1_itr * (*arr2_itr);
            arr1_itr++;
            assert(arr2_itr != arr2.end());
            arr2_itr++;
        }
    } else if (method == 1) {//Sum of geometric distance squared
        while (arr1_itr != arr1.end()) {
            rtn += (*arr1_itr - *arr2_itr) * (*arr1_itr - *arr2_itr);
            arr1_itr++;
            assert(arr2_itr != arr2.end());
            arr2_itr++;
        }
    }
    return rtn;
}