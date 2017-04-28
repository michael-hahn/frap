//
//  main.cpp
//  graphchi_xcode
//
//  Created by Michael Hahn on 3/2/17.
//
//

#include <string>
#include <iostream>
#include <stdlib.h>
#include <map>
#include <thread>
#include <mutex>
#include <cstdlib>
#include <vector>
#include <sstream>
#include <cassert>
#include <cmath>
#include <algorithm>
#include <stdio.h>
#include "kernelmaps.hpp"
#include "profile.hpp"
#include "global.h"
#include "vertex.hpp"
#include "graphchi_basic_includes.hpp"
#include "logger/logger.hpp"

using namespace graphchi;

#define KULLBACKLEIBLER 0
#define HELLINGER 1
#define EUCLIDEAN 2
#define METRIC 1 //for old simple normal distribution analysis only. Deprecated


// Parse the type value in the file to the type_label structure for reading
void parse(type_label &x, const char * s) {
    char * ss = (char *) s;
    char delims[] = ":";
    char * t;
    t = strtok(ss, delims);
    if (t == NULL)
        logstream(LOG_FATAL) << "Source Type info does not exist" << std::endl;
    assert(t != NULL);
    x.new_src = atoi(t);
    //TODO: We can make sure type value is never 0 so we can check if parse goes wrong here
    t = strtok(NULL, delims);
    if (t == NULL)
        logstream(LOG_FATAL) << "Destination Type info does not exist" << std::endl;
    assert (t != NULL);
    x.new_dst = atoi(t);
    t = strtok(NULL, delims);
    if (t == NULL)
        logstream(LOG_FATAL) << "Edge Type info does not exist" << std::endl;
    assert (t != NULL);
    x.edge = atoi(t);
    t = strtok(NULL, delims);
    if (t != NULL)
        logstream(LOG_FATAL) << "Extra info will be ignored" << std::endl;
    return;
}

int main(int argc, const char ** argv) {
    /* GraphChi initialization will read the command line
     arguments and the configuration file. */
    graphchi_init(argc, argv);
    
    /* Metrics object for keeping track of performance counters
     and other information. Currently required. */
    metrics m("Detection Framework");
    
    /* Basic arguments for application */
    //First, we must know how many graphs will be used for computation:
    int num_graphs = get_option_int("ngraphs");
    //We then use the for loop to get all the filenames, and store them in an array
    std::string filenames[num_graphs] = {};
    for (int i = 0; i < num_graphs; i++) {
        std::string f_string;
        std::stringstream f_out;
        f_out << i;
        f_string = f_out.str();
        const std::string name_of_file = "file" + f_string;
        filenames[i] = get_option_string(name_of_file.c_str());
    }
    
    //TODO: Make iterations dynamic based on convergence instead
    int niters           = get_option_int("niters", 10); // Number of iterations
    bool scheduler       = false;
    
    /* Detect the number of shards or preprocess an input to create them */
    //for each file, detect shards or preprocess an input to create them
    //put results in an array
    int nshards_arr[num_graphs] = {};
    for (int i = 0; i < num_graphs; i++) {
        nshards_arr[i] = convert_if_notexists<EdgeDataType>(filenames[i], get_option_string("nshards", "auto"));
    }

    /* Run */
    //create the single instance of KernelMap
    KernelMaps* km = KernelMaps::get_instance();
    km->resetMaps();
    
    //create a tentative profile
    profile pf;
    
    pf.reset_arrays();
    pf.reset_map();
    
    //Generate label maps of all instances
    for (int i = 0; i < num_graphs; i++) {
        km->insert_label_map();
        
        VertexRelabel program;
        graphchi_engine<VertexDataType, EdgeDataType> engine(filenames[i], nshards_arr[i], scheduler, m);
        engine.run(program, niters);
    }
    
    //generate count arrays and put them in the profile
    std::vector<std::map<int, int>> label_maps = km->get_label_maps();
    assert((int)label_maps.size() == num_graphs);

    for (std::vector<std::map<int, int>>::iterator it = label_maps.begin(); it != label_maps.end(); it++) {
        pf.add_array(km->generate_count_array(*it));
    }

    //calculate distance matrix between every two count arrays
    //the matrix is implemented as a vector. With 3 graphs, A, B, and C, we have [D(A, B), D(A, C), D(B, C)]
    std::vector<double> distance_matrix;
    //get all the count arrays
    std::vector<std::vector<int>> count_arrays = pf.get_count_arrays();
    
    //print out all count arrays - debugging
//    for (std::vector<std::vector<int>>::iterator itr = count_arrays.begin(); itr != count_arrays.end(); itr++) {
//        std::cout << "Count Array: ";
//        for (std::vector<int>::iterator itr2 = itr->begin(); itr2 != itr->end(); itr2++) {
//            std::cout << *itr2 << " ";
//        }
//        std::cout << std::endl;
//    }
    for (int i = 0 ; i < num_graphs; i++) {
        for (int j = 1; j < num_graphs - i; j++) {
            double distance = pf.calculate_distance(KULLBACKLEIBLER, count_arrays[i], count_arrays[i+j]);
            distance_matrix.push_back(distance);
        }
    }
    
    //print out distance matrix
    std::cout << "Distance matrix: ";
    for (std::vector<double>::iterator itr = distance_matrix.begin(); itr != distance_matrix.end(); itr++) {
        std::cout << *itr << " ";
    }
    std::cout << std::endl;
    
    //kmean clustering to detect outliers and to form clusters
    //if many instances do not fit into a cluster, then we can create multiple clusters to encapsulate many normal behaviors that are quite divergent
    std::vector<std::vector<int>> cluster = kmean(num_graphs, distance_matrix);
    
    //print out the cluster:
    //format: graph_x - graph_y
    
    //this vector contains for each clutser the instance that appears and the number of distance value of that instance
    //e.g. if cluster 0 has 1-0 1-2 1-4, then we have [(1 -> 3, 2 -> 1, 4 -> 1), <other_maps>]
    std::vector<std::map<int, int>> cluster_temps;
    
    for (std::vector<std::vector<int>>::iterator itr = cluster.begin(); itr != cluster.end(); itr++) {
        std::cout << "Cluster: ";
        std::map<int, int> temp;
        for (std::vector<int>::iterator itr2 = itr->begin(); itr2 != itr->end(); itr2++) {
            for (int x = 0; x < num_graphs - 1; x++) {
                for (int y = 0; y < num_graphs - 1 - x; y++) {
                    if ((((( (num_graphs - 1) + ( num_graphs - x )) * x ) / 2 ) + y ) == *itr2) {
                        std::cout << x << "-" << x + 1 + y << " ";
                        std::pair<std::map<int,int>::iterator,bool> ret;
                        ret = temp.insert ( std::pair<int,int>(x,1) );
                        if (ret.second==false) {
                            ret.first->second++;
                        }
                        ret = temp.insert ( std::pair<int,int>(x+1+y,1) );
                        if (ret.second==false) {
                            ret.first->second++;
                        }
                    }
                }
            }
//            std::cout << *itr2 << " ";
        }
        std::cout << std::endl;
        cluster_temps.push_back(temp);
    }
    //for debugging; print cluster_temps
//    std::cout << std::endl;
//    for (size_t i = 0; i < cluster_temps.size(); i++) {
//        for (std::map<int, int>::iterator it = cluster_temps[i].begin(); it != cluster_temps[i].end(); it++) {
//            std::cout << it->first << "," << it->second << " ";
//        }
//        std::cout << std::endl;
//    }
    
    //this map the vector cluster_temps
    //if instance 0 has 3 in cluster 0 and 4 in cluster 1 the map entry will be 0 -> [<0, 3> <1, 4>]
    std::map<int, std::vector<std::pair<int, int>>> instance_temp;
    for (int i = 0; i < (int) cluster_temps.size(); i++) {
        for (std::map<int, int>::iterator itr = cluster_temps[i].begin(); itr != cluster_temps[i].end(); itr++) {
            std::pair<std::map<int, std::vector<std::pair<int, int>>>::iterator,bool> ret;
            std::vector<std::pair<int, int>> start_vec;
            start_vec.push_back(std::pair<int, int>(i, itr->second));
            ret = instance_temp.insert ( std::pair<int,std::vector<std::pair<int, int>>>(itr->first, start_vec) );
            if (ret.second==false) {
                ret.first->second.push_back(std::pair<int, int>(i, itr->second));
            }
        }
    }
    //for debugging; print instance_temp
//    for (std::map<int, std::vector<std::pair<int, int>>>::iterator it = instance_temp.begin(); it != instance_temp.end(); it++) {
//        std::cout << it->first << ":";
//        for (std::vector<std::pair<int, int>>::iterator itr = it->second.begin(); itr != it->second.end(); itr++) {
//            std::cout << itr->first << "," << itr->second << "  ";
//        }
//        std::cout << std::endl;
//    }
    
    //this map shows what instance belongs to what group
    std::map<int, int> instance_belongs;
    for (std::map<int, std::vector<std::pair<int, int>>>::iterator it = instance_temp.begin(); it != instance_temp.end(); it++) {
        int max_clutser_population = 0;
        int instance_goes_to = -1;
        for (std::vector<std::pair<int, int>>::iterator itr = it->second.begin(); itr != it->second.end(); itr++) {
            if (itr->second >= max_clutser_population) {
                max_clutser_population = itr->second;
                instance_goes_to = itr->first;
            }
        }
        assert (instance_goes_to >= 0);
        instance_belongs.insert(std::pair<int, int>(it->first, instance_goes_to));
    }
    //for debugging: print instance_belongs;
//    for (std::map<int, int>::iterator it = instance_belongs.begin(); it != instance_belongs.end(); it++) {
//        std::cout << it->first << "->" << it->second << std::endl;
//    }
    
    //cluster_id -> # of instances in this cluster
    std::map<int, int> cluster_has_instance_number;
    for (std::map<int, int>::iterator it = instance_belongs.begin(); it != instance_belongs.end(); it++) {
        std::pair<std::map<int,int>::iterator,bool> ret;
        ret = cluster_has_instance_number.insert ( std::pair<int,int>(it->second,1) );
        if (ret.second==false) {
            ret.first->second++;
        }
    }
    //for debugging: print cluster_has_instance_number
//    for (std::map<int, int>::iterator it = cluster_has_instance_number.begin(); it != cluster_has_instance_number.end(); it++) {
//        std::cout << it->first << "->" << it->second << std::endl;
//    }
    .
    
/*
    //values between two count arrays
    std::vector<int> val_two_arrays;
    std::vector<std::vector<int>> count_arrays = pf.get_count_arrays();
    
    for (int i = 0 ; i < num_graphs; i++) {
        for (int j = 0; j < num_graphs - i; j++) {
            int val = pf.calculate_two_count_arrays(METRIC, count_arrays[i], count_arrays[i+j]);
            val_two_arrays.push_back(val);
        }
    }
    
    //print kv
    std::cout << "Kernel vector values: ";
    for(size_t i = 0; i < val_two_arrays.size(); i++) {
        std::cout << val_two_arrays[i] << " ";
    }
    std::cout << std::endl;
    
    //produce normalized kernel value for each graph
    double normalized_kv[num_graphs] = {};
    //sum of multiplication
    if (METRIC == 0) {
        for (int i = 0; i < num_graphs; i++) {
            double total = 0.0;
            for (int j = 0; j < num_graphs; j++) {
                if (i != j) {
                    int self = val_two_arrays[i*num_graphs-i*(i-1)/2+j-i];
                    int base1 = val_two_arrays[i*num_graphs-i*(i-1)/2+i-i];
                    int base2 = val_two_arrays[j*num_graphs-j*(j-1)/2+j-j];
                    double normalized = (double) self/((double)base1*(double)base2);
                    total += normalized;
                }
            }
            normalized_kv[i] = total/num_graphs;
        }
    } else if (METRIC == 1) {//geometric distance
        for (int i = 0; i < num_graphs; i++) {
            double total = 0.0;
            for (int j = 0; j < num_graphs; j++) {
                if (i != j) {
                    int dist = sqrt(val_two_arrays[i*num_graphs-i*(i-1)/2+j-i]);
                    total += dist;
                }
            }
            normalized_kv[i] = total/(num_graphs - 1);
        }
    }
    //print normalize_kv
    std::cout << "Normalized kernel vector values: ";
    for(int i = 0; i < num_graphs; i++) {
        std::cout << normalized_kv[i] << " ";
    }
    std::cout << std::endl;
    
    //calculate mean and std of normalized_kv
    double sum = 0.0, mean, standardDeviation = 0.0;
    int i;
    for(i = 0; i < num_graphs; ++i){
        sum += normalized_kv[i];
    }
    mean = sum/num_graphs;
    for(i = 0; i < num_graphs; ++i)
        standardDeviation += pow(normalized_kv[i] - mean, 2);
    standardDeviation = sqrt(standardDeviation / num_graphs);
    std::cout << "Mean: " << mean << std::endl;
    std::cout << "STD: " << standardDeviation << std::endl;
    
    //Determine instances with unusual behavior, and then regenerate the mean and STD without them as part of the profile
    double two_stds_upper_bound = mean + 2 * standardDeviation;
    double two_stds_lower_bound = mean - 2 * standardDeviation;
    std::cout << "boundary is: " << two_stds_lower_bound << " - " << two_stds_upper_bound << std::endl;
    std::vector<double> good_normalized_kv;
    for(int i = 0; i < num_graphs; ++i) {
        if (normalized_kv[i] <= two_stds_upper_bound && normalized_kv[i] >= two_stds_lower_bound)
            good_normalized_kv.push_back(normalized_kv[i]);
        else {
            pf.remove_array(i);//remove arrays from profile that are not suppose to be part of it
        }
    }
    
    double good_sum = 0.0, good_mean, good_standardDeviation = 0.0;
    for(std::vector<double>::iterator it = good_normalized_kv.begin(); it != good_normalized_kv.end(); it++){
        good_sum += *it;
    }
    good_mean = good_sum/good_normalized_kv.size();
    for(std::vector<double>::iterator it = good_normalized_kv.begin(); it != good_normalized_kv.end(); it++)
        good_standardDeviation += pow(*it - mean, 2);
    good_standardDeviation = sqrt(good_standardDeviation / good_normalized_kv.size());
    std::cout << "Good Mean: " << good_mean << std::endl;
    std::cout << "Good STD: " << good_standardDeviation << std::endl;
*/
    
    /* Report execution metrics */
    //metrics_report(m);
    return 0;
}