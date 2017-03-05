//
//  profile.cpp
//  graphchi_xcode
//
//  Created by Michael Hahn on 3/2/17.
//
//
#include <cassert>
#include <cmath>
#include "profile.hpp"
#include "helper.cpp"

//multiple methods available now. OLD METHOD. Deprecated
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


double profile::calculate_distance(int method, std::vector<int> count_array1, std::vector<int> count_array2) {
    double distance = 0;
    if (method == 0) {//symmetric kullback-leibler divergence
        std::vector<double> count_distribution_1 = count_distribution(count_array1, true);
        std::vector<double> count_distribution_2 = count_distribution(count_array2, true);
        assert(count_distribution_1.size() == count_distribution_2.size());
        
        std::vector<double>::iterator itr_1 = count_distribution_1.begin();
        std::vector<double>::iterator itr_2 = count_distribution_2.begin();
        while (itr_1 != count_distribution_1.end()) {
            distance += (*itr_1 - *itr_2) * log(*itr_1 / *itr_2);
            itr_1++;
            itr_2++;
        }
    }
    if (method == 1) {//hellinger distance
        std::vector<double> count_distribution_1 = count_distribution(count_array1, false);
        std::vector<double> count_distribution_2 = count_distribution(count_array2, false);
        assert(count_distribution_1.size() == count_distribution_2.size());
        
        std::vector<double>::iterator itr_1 = count_distribution_1.begin();
        std::vector<double>::iterator itr_2 = count_distribution_2.begin();
        while (itr_1 != count_distribution_1.end()) {
            distance += (sqrt(*itr_1) - sqrt(*itr_2)) * (sqrt(*itr_1) - sqrt(*itr_2));
            itr_1++;
            itr_2++;
        }
        distance = sqrt(distance) / sqrt(2);
    }
    if (method == 2) {//euclidian distance
        assert(count_array1.size() == count_array2.size());
        std::vector<int>::iterator itr_1 = count_array1.begin();
        std::vector<int>::iterator itr_2 = count_array2.begin();
        while (itr_1 != count_array1.end()) {
            distance += (*itr_1 - *itr_2) * (*itr_1 - *itr_2);
            itr_1++;
            itr_2++;
        }
        distance = sqrt(distance);
    }
    return distance;
}










