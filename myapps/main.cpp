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
    //This tells us how many instances are only part of the monitoring, not the learning
    //Therefore the learning graphs would be (num_graphs - num_monitor)
    int num_monitor = get_option_int("nmonitor");
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
    int niters           = get_option_int("niters", 4); // Number of iterations, or default of 4
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
    
    //Generate label maps of all learning instances
    for (int i = 0; i < num_graphs - num_monitor; i++) {
        km->insert_label_map();
        
        VertexRelabel program;
        graphchi_engine<VertexDataType, EdgeDataType> engine(filenames[i], nshards_arr[i], scheduler, m);
        engine.run(program, niters);
    }
    
    //generate count arrays and put them in the profile
    std::vector<std::map<int, int>> label_maps = km->get_label_maps();
    assert((int)label_maps.size() == num_graphs - num_monitor);

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
    
    for (int i = 0 ; i < num_graphs - num_monitor; i++) {
        for (int j = 1; j < num_graphs - num_monitor - i; j++) {
            double distance = pf.calculate_distance(KULLBACKLEIBLER, count_arrays[i], count_arrays[i+j]);
            distance_matrix.push_back(distance);
        }
    }
    
    //print out distance matrix
//    std::cout << "Distance matrix: ";
//    for (std::vector<double>::iterator itr = distance_matrix.begin(); itr != distance_matrix.end(); itr++) {
//        std::cout << *itr << " ";
//    }
//    std::cout << std::endl;
    
    //kmean clustering to detect outliers and to form clusters
    //if many instances do not fit into a cluster, then we can create multiple clusters to encapsulate many normal behaviors that are quite divergent
    //Before apply kmeans, we have a kmean-prior algorithm that helps determine the optimal cluster size when clustering distribution
    //We cluster pair-wise distances
    //The number of cluster will be used as the value k when clustering distributions
    //This algorithm also helps to determine the inital centroid value to use
    std::pair<std::vector<std::vector<int>>, std::vector<std::vector<double>>> cluster_prior_results = kmeans_prior(num_graphs - num_monitor, distance_matrix);
    std::vector<std::vector<int>> cluster_prior = cluster_prior_results.first;
    //prior_distance not used for now
    std::vector<std::vector<double>> prior_distance = cluster_prior_results.second;
    
    //obtain the value of k for later clustering of distributions
    int total_number_of_valid_clusters_estimate = 0;
    for (std::vector<std::vector<int>>::iterator itr = cluster_prior.begin(); itr != cluster_prior.end(); itr++) {
        if (itr->size() > 0)
            total_number_of_valid_clusters_estimate++;
    }
    std::cout << "# of Clusters (estimate):" << total_number_of_valid_clusters_estimate << std::endl;
 
    //Initialize a vector that will hold the instance IDs of the ones that will be the initial centroild of the clustering of distributions
    std::vector<int> cluster_ids;
    
    //print out the cluster:
    //format: graph_x - graph_y
    
    //this vector contains for each cluster the instance that appears and the number of distance value of that instance
    //e.g. if cluster 0 has 1-0 1-2 1-4, then we have [(1 -> 3, 2 -> 1, 4 -> 1), <other_maps>]
    std::vector<std::map<int, int>> cluster_temps;
    
    for (std::vector<std::vector<int>>::iterator itr = cluster_prior.begin(); itr != cluster_prior.end(); itr++) {
        std::cout << "Prior Cluster: ";
        std::map<int, int> temp;
        for (std::vector<int>::iterator itr2 = itr->begin(); itr2 != itr->end(); itr2++) {
            for (int x = 0; x < num_graphs - num_monitor - 1; x++) {
                for (int y = 0; y < num_graphs - num_monitor - 1 - x; y++) {
                    if ((((( (num_graphs - num_monitor - 1) + ( num_graphs - num_monitor - x )) * x ) / 2 ) + y ) == *itr2) {
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
    
    //for debugging: print cluster_temps
//    std::cout << std::endl;
//    for (size_t i = 0; i < cluster_temps.size(); i++) {
//        for (std::map<int, int>::iterator it = cluster_temps[i].begin(); it != cluster_temps[i].end(); it++) {
//            std::cout << it->first << "," << it->second << " ";
//        }
//        std::cout << std::endl;
//    }
    
    for (size_t i = 0; i < cluster_temps.size(); i++) {
        if (cluster_temps[i].size() > 0) {
            int id = -1;
            int max_occur = -1;
            for (std::map<int, int>::iterator it = cluster_temps[i].begin(); it != cluster_temps[i].end(); it++) {
                if (it->second > max_occur) {
                    max_occur = it->second;
                    id = it->first;
                }
            }
            assert (id >= 0);
            assert (max_occur > 0);
            cluster_ids.push_back(id);
        }
    }
    
    //for debugging: print cluster_ids
//    for (size_t i = 0; i < cluster_ids.size(); i++) {
//        std::cout << cluster_ids[i] << "..";
//    }
//    std::cout << std::endl;
    
    //this is the centroids of all the clusters in the profile
    std::vector<std::vector<int>> final_centroids;

    std::pair<std::vector<std::vector<int>>, std::vector<std::vector<double>>> cluster_results = kmeans(total_number_of_valid_clusters_estimate, cluster_ids, count_arrays, final_centroids);
    
    //for debugging: print the centroids of the results:
//    for (std::vector<std::vector<int>>::iterator it = final_centroids.begin(); it != final_centroids.end(); it++) {
//        std::cout << "Centroids:" << std::endl;
//        for (std::vector<int>::iterator itr2 = it->begin(); itr2 != it->end(); itr2++) {
//            std::cout << *itr2 << " ";
//        }
//        std::cout << std::endl;
//    }
    
    std::vector<std::vector<int>> cluster = cluster_results.first;
    std::vector<std::vector<double>> cluster_distances = cluster_results.second;
    
    //for debugging: print out elements in a cluster
    for (std::vector<std::vector<int>>::iterator it = cluster.begin(); it != cluster.end(); it++) {
        std::cout << "Cluster: ";
        for (std::vector<int>::iterator itr2 = it->begin(); itr2 != it->end(); itr2++) {
            std::cout << *itr2 << " ";
        }
        std::cout << std::endl;
    }
    //for debugging: print out distances of each instance with its centroid in each cluster
//    for (std::vector<std::vector<double>>::iterator it = cluster_distances.begin(); it != cluster_distances.end(); it++) {
//        std::cout << "Cluster Distances: ";
//        for (std::vector<double>::iterator itr2 = it->begin(); itr2 != it->end(); itr2++) {
//            std::cout << *itr2 << " ";
//        }
//        std::cout << std::endl;
//    }
    
    //the final number of clusters, put final centroids and radius of the clusters in the profile
    //count arrays in the profile are updated as well
    pf.reset_count_arrays();
    int number_of_clusters = 0;
    for (size_t i = 0; i < cluster.size(); i++) {
        if (cluster[i].size() > (num_graphs - num_monitor) * 0.2) {
            number_of_clusters++;
            pf.add_centroid(final_centroids[i]);
            double max_dis = 0.0;
            for (std::vector<double>::iterator it = cluster_distances[i].begin(); it != cluster_distances[i].end(); it++) {
                if (*it > max_dis)
                    max_dis = *it;
            }
            pf.add_max_distance_from_centroid(max_dis);
            for (size_t j = 0; j < cluster[i].size(); j++) {
                pf.add_array(count_arrays[cluster[i][j]]);
            }
        }
    }
    
    assert(number_of_clusters > 0);
    assert(number_of_clusters == (int)pf.get_centroids().size());
    assert(number_of_clusters == (int)pf.get_distances().size());
    //for debugging, print out final number of clusters, max distance and centroids
    std::cout << "Final number of clusters: " << number_of_clusters << std::endl;
    std::cout << "Final size of centroids: " << pf.get_centroids().size() << std::endl;
    std::cout << "Final size of distances: " << pf.get_distances().size() << std::endl;
    std::cout << "Final number of count array in the profile: " << pf.get_count_arrays().size() << std::endl;
    std::vector<std::vector<int>> profile_centroids = pf.get_centroids();
//    for (std::vector<std::vector<int>>::iterator it = profile_centroids.begin(); it != profile_centroids.end(); it++) {
//        std::cout << "Centroids: ";
//        for (std::vector<int>::iterator itr2 = it->begin(); itr2 != it->end(); itr2++) {
//            std::cout << *itr2 << " ";
//        }
//        std::cout << std::endl;
//    }
    std::vector<double> profile_distances = pf.get_distances();
    std::cout << "Max distance of each cluster: ";
    for (std::vector<double>::iterator it = profile_distances.begin(); it != profile_distances.end(); it++) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
    
    //Detection stage: Now use the kernelmap from the learning stage to get the count arrays of the monitoring instances
    for (int i = 0; i < num_monitor; i++) {
        VertexRelabelDetection program2;
        graphchi_engine<VertexDataType, EdgeDataType> engine(filenames[num_graphs-num_monitor+i], nshards_arr[num_graphs-num_monitor+i], scheduler, m);
        engine.run(program2, niters);
        
        monitored.count_array = km->generate_count_array(monitored.label_map);
        
        std::vector<double> monitor_distances;
        //calculate distance between the monitored count array and the centroid
        for (int i = 0 ; i < number_of_clusters; i++) {
            double monitor_distance = pf.calculate_distance(KULLBACKLEIBLER, profile_centroids[i], monitored.count_array);
            monitor_distances.push_back(monitor_distance);
        }
        
        //debug only:
        std::cout << "Distances of monitored instance: ";
        for (size_t i = 0; i < monitor_distances.size(); i++) {
            std::cout << monitor_distances[i] << " ";
        }
        std::cout << std::endl;
        
        //test if the monitored program belonged to any of the cluster (i.e., within the radius)
        bool need_recluster = true;
        for (size_t i = 0; i < monitor_distances.size(); i++) {
            if (monitor_distances[i] <= profile_distances[i]) {
                need_recluster = false;
            }
        }
        
        if (!need_recluster)
            std::cout << "This monitored instance is normal..." << std::endl;
        else {
            std::cout << "This monitored instance is outside the radius of any cluster... Recluster now..." << std::endl;
            std::vector<std::vector<int>> total_count_arrays = pf.get_count_arrays();
            total_count_arrays.push_back(monitored.count_array);
            std::cout << "# of arrays in total_count_arrays: " << total_count_arrays.size() << std::endl;
            std::vector<std::vector<int>> total_centroids = pf.get_centroids();
            total_centroids.push_back(monitored.count_array);
            std::cout << "# of arrays in total_centroids: " << total_centroids.size() << std::endl;
            
            std::pair<std::vector<std::vector<int>>, std::vector<std::vector<double>>> cluster_monitor_results = kmeans_monitor(total_centroids.size(), total_count_arrays, total_centroids);
            std::vector<std::vector<int>> cluster_monitor = cluster_monitor_results.first;
            
            //for debugging: print out elements in a cluster
            for (std::vector<std::vector<int>>::iterator it = cluster_monitor.begin(); it != cluster_monitor.end(); it++) {
                std::cout << "ReCluster (Monitoring): ";
                for (std::vector<int>::iterator itr2 = it->begin(); itr2 != it->end(); itr2++) {
                    std::cout << *itr2 << " ";
                }
                std::cout << std::endl;
            }
            
            bool bad_instance = false;
            for (size_t j = 0; j < cluster_monitor.size(); j++) {
                if (cluster_monitor[j].size() == 1 && cluster_monitor[j][0] == (int)pf.get_count_arrays().size()) {
                    std::cout << "This monitored instance is abnormal!" << std::endl;
                    bad_instance = true;
                }
            }
            if (!bad_instance) std::cout << "This monitored instance is actually normal..." << std::endl;
            
        }
        
        monitored.count_array.clear();
        monitored.label_map.clear();
    }
    
    //this map the vector cluster_temps
    //if instance 0 has 3 in cluster 0 and 4 in cluster 1 the map entry will be 0 -> [<0, 3> <1, 4>]
//    std::map<int, std::vector<std::pair<int, int>>> instance_temp;
//    for (int i = 0; i < (int) cluster_temps.size(); i++) {
//        for (std::map<int, int>::iterator itr = cluster_temps[i].begin(); itr != cluster_temps[i].end(); itr++) {
//            std::pair<std::map<int, std::vector<std::pair<int, int>>>::iterator,bool> ret;
//            std::vector<std::pair<int, int>> start_vec;
//            start_vec.push_back(std::pair<int, int>(i, itr->second));
//            ret = instance_temp.insert ( std::pair<int,std::vector<std::pair<int, int>>>(itr->first, start_vec) );
//            if (ret.second==false) {
//                ret.first->second.push_back(std::pair<int, int>(i, itr->second));
//            }
//        }
//    }
    
    //for debugging; print instance_temp
//    for (std::map<int, std::vector<std::pair<int, int>>>::iterator it = instance_temp.begin(); it != instance_temp.end(); it++) {
//        std::cout << it->first << ":";
//        for (std::vector<std::pair<int, int>>::iterator itr = it->second.begin(); itr != it->second.end(); itr++) {
//            std::cout << itr->first << "," << itr->second << "  ";
//        }
//        std::cout << std::endl;
//    }
 
    //this map shows what instance belongs to what group
//    std::map<int, int> instance_belongs;
//    for (std::map<int, std::vector<std::pair<int, int>>>::iterator it = instance_temp.begin(); it != instance_temp.end(); it++) {
//        int max_clutser_population = 0;
//        int instance_goes_to = -1;
//        for (std::vector<std::pair<int, int>>::iterator itr = it->second.begin(); itr != it->second.end(); itr++) {
//            if (itr->second >= max_clutser_population) {
//                max_clutser_population = itr->second;
//                instance_goes_to = itr->first;
//            }
//        }
//        assert (instance_goes_to >= 0);
//        instance_belongs.insert(std::pair<int, int>(it->first, instance_goes_to));
//    }
//    //for debugging: print instance_belongs;
//    std::cout << "InstanceID -> ClusterID" << std::endl;
//    for (std::map<int, int>::iterator it = instance_belongs.begin(); it != instance_belongs.end(); it++) {
//        std::cout << it->first << "->" << it->second << std::endl;
//    }
    
    //cluster_id -> # of instances in this cluster
//    std::map<int, int> cluster_has_instance_number;
//    for (std::map<int, int>::iterator it = instance_belongs.begin(); it != instance_belongs.end(); it++) {
//        std::pair<std::map<int,int>::iterator,bool> ret;
//        ret = cluster_has_instance_number.insert ( std::pair<int,int>(it->second,1) );
//        if (ret.second==false) {
//            ret.first->second++;
//        }
//    }
    //for debugging: print cluster_has_instance_number
//    for (std::map<int, int>::iterator it = cluster_has_instance_number.begin(); it != cluster_has_instance_number.end(); it++) {
//        std::cout << it->first << "->" << it->second << std::endl;
//    }
    
    //bad_cluster has all cluster numbers that contain bad instances
//    std::vector<int> bad_cluster;
//    for (std::map<int, int>::iterator it = cluster_has_instance_number.begin(); it != cluster_has_instance_number.end(); it++) {
//        if ((double)it->second < num_graphs * 0.15) {
//            //std::cout << it->first << std::endl;
//            bad_cluster.push_back(it->first);
//        }
//    }
    
    
//    for (std::vector<int>::iterator it = bad_cluster.begin(); it != bad_cluster.end(); it++) {
//        for (std::map<int, int>::iterator itr = instance_belongs.begin(); itr != instance_belongs.end(); itr++) {
//            if (itr->second == *it) {
//                std::cout << itr->first << std::endl;
//            }
//        }
//    }
    
    
    
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