//
//  helper.cpp
//  graphchi_xcode
//
//  Created by Michael Hahn on 3/4/17.
//
//

#include <stdio.h>
#include <cstdlib>
#include <cassert>
#include <cmath>
#include <vector>

//back-off probability can be optionally included in the count distribution
std::vector<double> count_distribution(std::vector<int> count_array, bool back_off) {
    std::vector<double> count_distr;
    int sum = 0;
    int zero_count = 0;
    
    for (std::vector<int>::iterator itr = count_array.begin(); itr != count_array.end(); itr++) {
        if (*itr != 0)
            sum += *itr;
        else zero_count++;
    }
    bool min_exist = false;
    double min = 0.0;
    for (std::vector<int>::iterator itr = count_array.begin(); itr != count_array.end(); itr++) {
        double val = *itr / (double)sum;
        count_distr.push_back(val);
        if (val > 0) {
            if (min_exist) {
                if (min > val)
                    min = val;
            } else {
                min = val;
                min_exist = true;
            }
        }
    }
    assert(min != 0.0);
    if (back_off) {
        double back_off_probability = (min / 2) / zero_count;
        double deduct_probability = (min / 2) / (count_array.size() - zero_count);
        for (std::vector<double>::iterator itr = count_distr.begin(); itr != count_distr.end(); itr++) {
            if (*itr == 0)
                *itr = back_off_probability;
            else *itr = *itr - deduct_probability;
        }
    }
    return count_distr;
}

double mean(std::vector<double> vec) {
    double sum = 0.0;
    for (std::vector<double>::iterator itr = vec.begin(); itr != vec.end(); itr++) {
        sum += *itr;
    }
    if (vec.size() == 0)
        return 0.0;
    else return sum / vec.size();
}

//k-mean clustering
//cluster indices
//k: number of cluster

std::vector<std::vector<int>> kmean(int k, std::vector<double> distance_matrix) {
    int matrix_size = distance_matrix.size();
    double cluster[k];
    double newCluster[k];
    double group[k][matrix_size];
    std::vector<double> newGroup[k];
    std::vector<std::vector<int>> rtn;
    bool converge = true;

    for (int i = 0; i < k; i++) {
        std::vector<int> vec;
        rtn.push_back(vec);
        
        // BAD, might be putting two clusters in the same place.
        cluster[i] = distance_matrix[rand() % matrix_size];
        newCluster[i] = 0.0;
    }
    do {
        converge = true;
        for (int i = 0; i < k; i++) {
            int j = 0;
            for (std::vector<double>::iterator itr = distance_matrix.begin(); itr != distance_matrix.end(); itr++) {
                group[i][j] = abs(*itr - cluster[i]);
                j++;
            }
        }
        
        for (int i = 0; i < matrix_size; i++) {
            int groupNum = 0;
            double min = group[0][i];
            for (int p = 1; p < k; p++) {
                if (group[p][i] < min) {
                    min = group[p][i];
                    groupNum = p;
                }
            }
            rtn[groupNum].push_back(i);
            newGroup[groupNum].push_back(distance_matrix[i]);
        }
        
        for (int q = 0; q < k; q++) {
            newCluster[q] = mean(newGroup[q]);
        }
        
        for (int t = 0; t < k; t++) {
            if (newCluster[t] != cluster[t])
                converge = false;
        }
        
//        std::cout << "rtn value: " << std::endl;
//        for (std::vector<std::vector<int>>::iterator itr = rtn.begin(); itr != rtn.end(); itr++) {
//            std::cout << "Cluster: ";
//            for (std::vector<int>::iterator itr2 = itr->begin(); itr2 != itr->end(); itr2++) {
//                std::cout << *itr2 << " ";
//            }
//            std::cout << std::endl;
//        }
//        std:: cout << std::endl;
        
        if (!converge) {
            for (int d = 0; d < k; d++) {
                cluster[d] = newCluster[d];
                rtn[d].clear();
                newGroup[d].clear();
            }
        }
        
//        std::cout << "newCluster: ";
//        for (int i = 0; i < k; i++) {
//            std::cout << newCluster[i] << " ";
//        }
//        std:: cout << std::endl;
        
    } while (!converge);
    
    return rtn;
}



























