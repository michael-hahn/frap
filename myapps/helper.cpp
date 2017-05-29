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

double calculate_distance2(int method, std::vector<int> count_array1, std::vector<int> count_array2) {
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

//k-mean clustering
//cluster indices
//k: number of cluster

std::pair<std::vector<std::vector<int>>, std::vector<std::vector<double>>> kmeans_prior(int k, std::vector<double> distance_matrix) {
    int matrix_size = distance_matrix.size();
    double cluster[k];
    double newCluster[k];
    double group[k][matrix_size];
    std::vector<double> newGroup[k];
    std::vector<std::vector<int>> rtn;
    std::vector<std::vector<double>> rtn_distance;
    bool converge = true;

    for (int i = 0; i < k; i++) {
        std::vector<int> vec;
        std::vector<double> dis;
        rtn.push_back(vec);
        rtn_distance.push_back(dis);

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
            rtn_distance[groupNum].push_back(min);
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
                rtn_distance[d].clear();
                newGroup[d].clear();
            }
        }

//        std::cout << "newCluster: ";
//        for (int i = 0; i < k; i++) {
//            std::cout << newCluster[i] << " ";
//        }
//        std:: cout << std::endl;

    } while (!converge);

    return std::pair<std::vector<std::vector<int>>, std::vector<std::vector<double>>>(rtn, rtn_distance);
}

std::pair<std::vector<std::vector<int>>, std::vector<std::vector<double>>> kmeans(int k, std::vector<int>seeds, std::vector<std::vector<int>> count_array, std::vector<std::vector<int>>& centroids) {
    int nvec = count_array.size();
    int array_size = count_array[0].size();

    std::vector<int> cluster[k];
    std::vector<int> newCluster[k];
    double group[k][nvec];

    std::vector<std::vector<int>> newGroup[k];
    std::vector<std::vector<int>> rtn;  // Final variable to return
    std::vector<std::vector<double>> rtn_distance;


    bool converge = true;

    //Initialize rtn

    for (int i = 0; i < k; i++) {
        std::vector<int> vec;
        std::vector<double> dis;
        rtn.push_back(vec);
        rtn_distance.push_back(dis);

        std::vector<int> temp (array_size,0);
        for (int ii = 0; ii < array_size; ii++) {
            cluster[i].push_back(count_array[seeds[i]][ii]);
            newCluster[i].push_back(count_array[seeds[i]][ii]);
        }
    }

    // Calculate distance to each cluster
    do {
        converge = true;
        for (int i = 0; i < k; i++) {
            int j = 0;
            for (std::vector<std::vector<int>>::iterator itr = count_array.begin(); itr != count_array.end(); itr++) {
                group[i][j] = calculate_distance2(0, *itr, cluster[i]);  // distance of j-th entry to i-th cluster
                j++;
            }
        }

        for (int i = 0; i < nvec; i++) {
            int groupNum = 0;
            double min = group[0][i];
            for (int p = 1; p < k; p++) {
                if (group[p][i] < min) {
                    min = group[p][i];
                    groupNum = p;
                }
            }
            rtn[groupNum].push_back(i);
            rtn_distance[groupNum].push_back(min);
            newGroup[groupNum].push_back(count_array[i]);
        }

        for (int q = 0; q < k; q++) {
            if (newGroup[q].size() != 0) {
              std::vector<int> sum(array_size,0);
              for (std::vector<std::vector<int>>::iterator it = newGroup[q].begin(); it != newGroup[q].end(); it++) {
                for(int f = 0; f < array_size; f++) {
                  sum[f] += (*it)[f];
                }
              }
              for(int g = 0; g < array_size; g++) {
                sum[g] /= newGroup[q].size();
              }
              for (int d = 0; d < array_size; d++) {
                newCluster[q][d] = sum[d];// calculate mean of distribution for a cluster
              }
            }
        }

        for (int t = 0; t < k; t++) {
          for (int r = 0; r < array_size; r++) {
            if (newCluster[t][r] != cluster[t][r])
                converge = false;
          }
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
                rtn_distance[d].clear();
                newGroup[d].clear();
            }
        } else {
            for (int t = 0; t < k; t++) {
                centroids.push_back(newCluster[t]);
            }
        }

        //        std::cout << "newCluster: ";
        //        for (int i = 0; i < k; i++) {
        //            std::cout << newCluster[i] << " ";
        //        }
        //        std:: cout << std::endl;

    } while (!converge);
    

    return std::pair<std::vector<std::vector<int>>, std::vector<std::vector<double>>>(rtn, rtn_distance);
}

std::pair<std::vector<std::vector<int>>, std::vector<std::vector<double>>> kmeans_monitor(int k, std::vector<std::vector<int>> count_array, std::vector<std::vector<int>> centroids) {
    int nvec = count_array.size();
    int array_size = count_array[0].size();
    
    std::vector<int> cluster[k];
    std::vector<int> newCluster[k];
    double group[k][nvec];
    
    std::vector<std::vector<int>> newGroup[k];
    std::vector<std::vector<int>> rtn;  // Final variable to return
    std::vector<std::vector<double>> rtn_distance;
    
    
    bool converge = true;
    
    //Initialize rtn
    
    for (int i = 0; i < k; i++) {
        std::vector<int> vec;
        std::vector<double> dis;
        rtn.push_back(vec);
        rtn_distance.push_back(dis);
        
        for (int ii = 0; ii < array_size; ii++) {
            cluster[i].push_back(centroids[i][ii]);
            newCluster[i].push_back(centroids[i][ii]);
        }
    }
    
    // Calculate distance to each cluster
    do {
        converge = true;
        for (int i = 0; i < k; i++) {
            int j = 0;
            for (std::vector<std::vector<int>>::iterator itr = count_array.begin(); itr != count_array.end(); itr++) {
                group[i][j] = calculate_distance2(0, *itr, cluster[i]);  // distance of j-th entry to i-th cluster
                j++;
            }
        }
        
        for (int i = 0; i < nvec; i++) {
            int groupNum = 0;
            double min = group[0][i];
            for (int p = 1; p < k; p++) {
                if (group[p][i] < min) {
                    min = group[p][i];
                    groupNum = p;
                }
            }
            rtn[groupNum].push_back(i);
            rtn_distance[groupNum].push_back(min);
            newGroup[groupNum].push_back(count_array[i]);
        }
        
        for (int q = 0; q < k; q++) {
            if (newGroup[q].size() != 0) {
                std::vector<int> sum(array_size,0);
                for (std::vector<std::vector<int>>::iterator it = newGroup[q].begin(); it != newGroup[q].end(); it++) {
                    for(int f = 0; f < array_size; f++) {
                        sum[f] += (*it)[f];
                    }
                }
                for(int g = 0; g < array_size; g++) {
                    sum[g] /= newGroup[q].size();
                }
                for (int d = 0; d < array_size; d++) {
                    newCluster[q][d] = sum[d];// calculate mean of distribution for a cluster
                }
            }
        }
        
        for (int t = 0; t < k; t++) {
            for (int r = 0; r < array_size; r++) {
                if (newCluster[t][r] != cluster[t][r])
                    converge = false;
            }
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
                rtn_distance[d].clear();
                newGroup[d].clear();
            }
        }
        //        std::cout << "newCluster: ";
        //        for (int i = 0; i < k; i++) {
        //            std::cout << newCluster[i] << " ";
        //        }
        //        std:: cout << std::endl;
        
    } while (!converge);
    
    
    return std::pair<std::vector<std::vector<int>>, std::vector<std::vector<double>>>(rtn, rtn_distance);
}
