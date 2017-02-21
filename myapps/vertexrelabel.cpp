//
//  vertexrelabel.cpp
//  graphchi_xcode
//
//  Created by Michael Hahn on 2/15/17.
//
//

//Use dynamic edge data
//Income data format should be in the format: src_id dst_id src_type:dst_type:edge_type
//types must be integers in the form of strings. For example, mmap_write should be "12" (string)
#define _GNU_SOURCE
#define DYNAMICEDATA 1

#include <string>
#include <iostream>
#include <stdlib.h>
#include <map>
#include <thread>
#include <mutex>
#include <cstdlib>
#include <vector>
#include <sstream>
#include "graphchi_basic_includes.hpp"

using namespace graphchi;

/**
 * Type definitions. Remember to create suitable graph shards using the
 * Sharder-program.
 */


typedef std::string VertexDataType;
typedef chivector<std::string> EdgeDataType;//src_type dst_type edge_type

/**
 * GraphChi programs need to subclass GraphChiProgram<vertex-type, edge-type>
 * class. The main logic is usually in the update function.
 */
struct VertexRelabel : public GraphChiProgram<VertexDataType, EdgeDataType> {
    
    std::map<std::string, int> label_map;
    std::mutex map_lock;
    
    void updateOrInsert(std::string label) {
        std::pair<std::map<std::string, int>::iterator, bool> rst;
        rst = label_map.insert(std::pair<std::string, int>(label, 1));
        if (rst.second == false) {
            std::cout << "Label " + label + " is already in the map. Updating the value...\n";
            rst.first->second = rst.first->second + 1;
        }
        return;
    }
    
    void print_map () {
        std::map<std::string, int>::iterator map_itr;
        for (map_itr = label_map.begin(); map_itr != label_map.end(); map_itr++)
            std::cout << map_itr->first << ":" << map_itr->second << std::endl;
    }
    /**
     *  Vertex update function.
     */
    void update(graphchi_vertex<VertexDataType, EdgeDataType> &vertex, graphchi_context &gcontext) {
        
        if (gcontext.iteration == 0) {
            /* On first iteration, initialize vertex (and its edges). This is usually required, because
             on each run, GraphChi will modify the data files. To start from scratch, it is easiest
             do initialize the program in code. Alternatively, you can keep a copy of initial data files. */
            // First for each vertex, set its label as its w3ctype
            std::string vertex_label = "";
            // The value can be obtained from any outedge (from src_type) or in_edge from (dst_type)
            graphchi_edge<EdgeDataType> * outedge = vertex.outedge(0);
            //if the node has no outedge, we get a random inedge //TODO: This may not do what I expect
            if (outedge == NULL) {
                graphchi_edge<EdgeDataType> * inedge = vertex.inedge(0);
                //get the dst_type from inedge
                vertex_label = inedge->get_vector()->get(1);
            } else {
                //get the src_type from outedge
                vertex_label = outedge->get_vector()->get(0);
            }
            //make sure vertex must have a valid string, not an empty string
            if (vertex_label != "") {
                vertex.set_data(vertex_label);
                map_lock.lock();
                updateOrInsert(vertex_label);
                map_lock.unlock();
            } else {
                std::cerr << "Invalid vertex type: empty vertex. Check import file." << std::endl;
                exit(1); //TODO: This may be a bad practice
            }
            
        } else {
            std::vector<int> label_vec;
            for(int i=0; i < vertex.num_inedges(); i++) {
                graphchi_edge<EdgeDataType> * in_edge = vertex.inedge(i);
                chivector<std::string> * in_vector = in_edge->get_vector();
                std::string in_label_string = in_vector->get(0);
                char * in_token;
                in_token = strtok((char*)in_label_string.c_str(), ",");
                while (in_token != NULL) {
                    int in_one_label = std::atoi(in_token);
                    label_vec.push_back(in_one_label);
                    in_token = strtok(NULL, ",");
                }
            }
            for (int i=0; i < vertex.num_outedges(); i++) {
                graphchi_edge<EdgeDataType> * out_edge = vertex.outedge(i);
                chivector<std::string> * out_vector = out_edge->get_vector();
                std::string out_label_string = out_vector->get(1);
                char * out_token;
                out_token = strtok((char*)out_label_string.c_str(), ",");
                while (out_token != NULL) {
                    int out_one_label = std::atoi(out_token);
                    label_vec.push_back(out_one_label);
                    out_token = strtok(NULL, ",");
                }
            }
            std::string self_label = vertex.get_data();
            char * self_token;
            self_token = strtok((char*)self_label.c_str(), ",");
            while (self_token != NULL) {
                int self_one_label = std::atoi(self_token);
                label_vec.push_back(self_one_label);
                self_token = strtok(NULL, ",");
            }
            std::sort(label_vec.begin(), label_vec.end());
            
            std::string new_label = "";
            for (std::vector<int>::iterator it = label_vec.begin(); it != label_vec.end() - 1; ++it) {
                std::string num_string;
                std::stringstream out;
                out << *it;
                num_string = out.str();
                new_label += num_string + ",";
            }
            std::string last_num_string;
            std::stringstream last_out;
            last_out << *(label_vec.end() - 1);
            last_num_string = last_out.str();
            new_label += last_num_string;
            
            vertex.set_data(new_label);
            map_lock.lock();
            updateOrInsert(new_label);
            map_lock.unlock();
        }
        
        // broadcast new label to neighbors by writing the value to the edges
        // write to the src_type if the vertex is the source vertex of the incident edge
        // write to the dst_type if the vertex is the destination vertex of the incident edge
        if (gcontext.iteration > 0) {
            std::string label = vertex.get_data();
            for(int i=0; i < vertex.num_inedges(); i++) {
                graphchi_edge<EdgeDataType> * in_edge = vertex.inedge(i);
                chivector<std::string> * in_vector = in_edge->get_vector();
                in_vector->set(1, label);
            }
            for (int i=0; i < vertex.num_outedges(); i++) {
                graphchi_edge<EdgeDataType> * out_edge = vertex.outedge(i);
                chivector<std::string> * out_vector = out_edge->get_vector();
                out_vector->set(0, label);
            }
        }
    }
    
    /**
     * Called before an iteration starts.
     */
    void before_iteration(int iteration, graphchi_context &gcontext) {
        std::cout << "Before " << iteration << "th iteration:" << std::endl;
        print_map();
        return;
    }
    
    /**
     * Called after an iteration has finished.
     */
    //For debugging purpose:
    void after_iteration(int iteration, graphchi_context &gcontext) {
        std::cout << "After " << iteration << "th iteration:" << std::endl;
        print_map();
        return;
    }
    
    /**
     * Called before an execution interval is started.
     */
    void before_exec_interval(vid_t window_st, vid_t window_en, graphchi_context &gcontext) {
    }
    
    /**
     * Called after an execution interval has finished.
     */
    void after_exec_interval(vid_t window_st, vid_t window_en, graphchi_context &gcontext) {
    }
    
};

int main(int argc, const char ** argv) {
    /* GraphChi initialization will read the command line
     arguments and the configuration file. */
    graphchi_init(argc, argv);
    
    /* Metrics object for keeping track of performance counters
     and other information. Currently required. */
    metrics m("vertexlabel");
    
    /* Basic arguments for application */
    std::string filename = get_option_string("file");  // Base filename
    int niters           = get_option_int("niters", 4); // Number of iterations
    //bool scheduler       = get_option_int("scheduler", 0); // Whether to use selective scheduling
    //TODO: should I use selective scheduling?
    bool scheduler       = false;//not for now
    
    /* Detect the number of shards or preprocess an input to create them */
    int nshards          = convert_if_notexists<std::string>(filename,
                                                              get_option_string("nshards", "auto"));
    
    /* Run */
    VertexRelabel program;
    graphchi_engine<VertexDataType, EdgeDataType> engine(filename, nshards, scheduler, m);
    engine.run(program, niters);
    
    /* Report execution metrics */
    metrics_report(m);
    return 0;
}
