//
//  vertexrelabel_static.cpp
//  graphchi_xcode
//
//  Created by Michael Hahn on 2/15/17.
//
//

//Use struct as type values in edgelist format
//Income data format should be in the form: src_id dst_id src_type:dst_type:edge_type
//Unlike dynamic edge data, the struct size does not change
//We use an auxiliary in-memory data structure to update the type value after the first iteration
//types must be integers. For example, mmap_write should be 12.

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
#include "graphchi_basic_includes.hpp"
#include "logger/logger.hpp"

using namespace graphchi;

/**
 * Type definitions. Remember to create suitable graph shards using the
 * Sharder-program.
 */
struct type_label {
    int old_src;
    int old_dst;
    int new_src;
    int new_dst;
    int edge;
};


typedef int VertexDataType;
typedef type_label EdgeDataType;//src_type dst_type edge_type


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

/**
 * GraphChi programs need to subclass GraphChiProgram<vertex-type, edge-type>
 * class. The main logic is usually in the update function.
 */
struct VertexRelabel : public GraphChiProgram<VertexDataType, EdgeDataType> {
    
    std::map<int, int> label_map;
    std::map<std::string, int> relabel_map;
    std::mutex relabel_map_lock;
    std::mutex label_map_lock;
    int counter = 0;
    
    //insert string to relabel_map if it does not exist, then return the mapped int value
    //otherwise, return the existing mapped int value
    int insert_relabel(std::string label) {
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
    void insert_label(int label) {
        std::pair<std::map<int, int>::iterator, bool> rst;
        rst = label_map.insert(std::pair<int, int>(label, 1));
        if (rst.second == false) {
            logstream(LOG_INFO) << "Label is already in the map. Updating the value..." << std::endl;
            rst.first->second++;
        }
        return;
    }
    
    void print_relabel_map () {
        std::map<std::string, int>::iterator map_itr;
        logstream(LOG_INFO) << "Printing relabel map..." << std::endl;
        for (map_itr = relabel_map.begin(); map_itr != relabel_map.end(); map_itr++)
            logstream(LOG_INFO) << map_itr->first << ":" << map_itr->second << std::endl;
    }
    
    void print_label_map () {
        std::map<int, int>::iterator map_itr;
        logstream(LOG_INFO) << "Printing label map..." << std::endl;
        for (map_itr = label_map.begin(); map_itr != label_map.end(); map_itr++)
            logstream(LOG_INFO) << map_itr->first << ":" << map_itr->second << std::endl;
    }
    /**
     *  Vertex update function.
     */
    void update(graphchi_vertex<VertexDataType, EdgeDataType> &vertex, graphchi_context &gcontext) {
        //TODO: can scheduling make a difference so that label updates can be synchronized?
        //assert(gcontext.scheduler != NULL);
        if (gcontext.iteration % 2 == 1) {
            for(int i=0; i < vertex.num_inedges(); i++) {
                graphchi_edge<EdgeDataType> * in_edge = vertex.inedge(i);
                type_label in_type = in_edge->get_data();
                in_type.old_dst = in_type.new_dst;
                in_edge->set_data(in_type);
                logstream(LOG_INFO) << "Swapped in edges of " << vertex.id() << " to " << in_type.old_dst << std::endl;
            }
            for (int i=0; i < vertex.num_outedges(); i++) {
                graphchi_edge<EdgeDataType> * out_edge = vertex.outedge(i);
                type_label out_type = out_edge->get_data();
                out_type.old_src = out_type.new_src;
                out_edge->set_data(out_type);
                logstream(LOG_INFO) << "Swapped out edges of " << vertex.id() << " to " << out_type.old_src << std::endl;
            }
        } else {
            if (gcontext.iteration == 0) {
                /* On first iteration, initialize vertex (and its edges). This is usually required, because
                 on each run, GraphChi will modify the data files. To start from scratch, it is easiest
                 do initialize the program in code. Alternatively, you can keep a copy of initial data files. */
                // First for each vertex, set its label as its w3ctype
                std::string vertex_label = "";
                // The value can be obtained from any outedge (from src_type) or in_edge from (dst_type)
                graphchi_edge<EdgeDataType> * outedge = vertex.random_outedge();
                //if the node has no outedge, we get a first inedge in the queue
                if (outedge == NULL) {
                    graphchi_edge<EdgeDataType> * inedge = vertex.inedge(0);
                    //get the dst_type from inedge
                    int vertex_int_label = inedge->get_data().new_dst;
                    std::string num_string;
                    std::stringstream out;
                    out << vertex_int_label;
                    vertex_label = out.str();
                } else {
                    //get the src_type from outedge
                    int vertex_int_label = outedge->get_data().new_src;
                    std::string num_string;
                    std::stringstream out;
                    out << vertex_int_label;
                    vertex_label = out.str();
                }
                //make sure vertex must have a valid string, not an empty string
                if (vertex_label != "") {
                    relabel_map_lock.lock();
                    int label_map_label = insert_relabel(vertex_label);
                    relabel_map_lock.unlock();
                    label_map_lock.lock();
                    insert_label(label_map_label);
                    label_map_lock.unlock();
                    vertex.set_data(label_map_label);
                    logstream(LOG_INFO) << "The value of label " << vertex.id() << " is: " << label_map_label << std::endl;
                } else {
                    logstream(LOG_FATAL) << "Invalid vertex_label in relabel_map. " << std::endl;
                    assert (vertex_label != "");
                }
            } else {
                std::vector<int> label_vec;
                for(int i=0; i < vertex.num_inedges(); i++) {
                    graphchi_edge<EdgeDataType> * in_edge = vertex.inedge(i);
                    int int_in_type = in_edge->get_data().old_src;
                    label_vec.push_back(int_in_type);
                    logstream(LOG_INFO) << "Vertex " << vertex.id() << " getting " << int_in_type << " from in edges" << std::endl;
                }
                for (int i=0; i < vertex.num_outedges(); i++) {
                    graphchi_edge<EdgeDataType> * out_edge = vertex.outedge(i);
                    int int_out_type = out_edge->get_data().old_dst;
                    label_vec.push_back(int_out_type);
                    logstream(LOG_INFO) << "Vertex " << vertex.id() << " getting " << int_out_type << " from out edges" << std::endl;
                }
                std::sort(label_vec.begin(), label_vec.end());
                int self_label = vertex.get_data();
                label_vec.push_back(self_label);
                
                std::string new_label = "";
                std::string first_num_string;
                std::stringstream first_out;
                first_out << *(label_vec.end() - 1);
                first_num_string = first_out.str();
                new_label += first_num_string;
                new_label += ",";

                for (std::vector<int>::iterator it = label_vec.begin(); it != label_vec.end() - 1; ++it) {
                    std::string num_string;
                    std::stringstream out;
                    out << *it;
                    num_string = out.str();
                    new_label += num_string + " ";
                }
                
                relabel_map_lock.lock();
                int label_map_label = insert_relabel(new_label);
                relabel_map_lock.unlock();
                label_map_lock.lock();
                insert_label(label_map_label);
                label_map_lock.unlock();
                vertex.set_data(label_map_label);
                logstream(LOG_INFO) << "The value of label " << vertex.id() << " is: " << label_map_label << std::endl;
            }
            
            // broadcast new label to neighbors by writing the value to the edges
            // write to the src_type if the vertex is the source vertex of the incident edge
            // write to the dst_type if the vertex is the destination vertex of the incident edge
            int label = vertex.get_data();
            if (gcontext.iteration == 0) {
                for(int i=0; i < vertex.num_inedges(); i++) {
                    graphchi_edge<EdgeDataType> * in_edge = vertex.inedge(i);
                    type_label in_type = in_edge->get_data();
                    in_type.new_dst = label;
                    in_edge->set_data(in_type);
                }
                for (int i=0; i < vertex.num_outedges(); i++) {
                    graphchi_edge<EdgeDataType> * out_edge = vertex.outedge(i);
                    type_label out_type = out_edge->get_data();
                    out_type.new_src = label;
                    out_edge->set_data(out_type);
                }
            } else {
                for(int i=0; i < vertex.num_inedges(); i++) {
                    graphchi_edge<EdgeDataType> * in_edge = vertex.inedge(i);
                    type_label in_type = in_edge->get_data();
                    in_type.new_dst = label;
                    in_edge->set_data(in_type);
                    logstream(LOG_INFO) << "Updated in edges of " << vertex.id() << " to " << in_type.old_dst << std::endl;
                }
                for (int i=0; i < vertex.num_outedges(); i++) {
                    graphchi_edge<EdgeDataType> * out_edge = vertex.outedge(i);
                    type_label out_type = out_edge->get_data();
                    out_type.new_src = label;
                    out_edge->set_data(out_type);
                    logstream(LOG_INFO) << "Updated out edges of " << vertex.id() << " to " << out_type.old_src << std::endl;
                }
            }
            /* Scheduler myself for next iteration */
            //gcontext.scheduler->add_task(vertex.id());
        }
    }
    
    /**
     * Called before an iteration starts.
     */
    void before_iteration(int iteration, graphchi_context &gcontext) {
    }
    
    /**
     * Called after an iteration has finished.
     */
    //For debugging purpose:
    void after_iteration(int iteration, graphchi_context &gcontext) {
        logstream(LOG_INFO) << "After " << iteration << "th iteration:" << std::endl;
        print_relabel_map();
        print_label_map();
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
    int niters           = get_option_int("niters", 10); // Number of iterations
    //bool scheduler       = get_option_int("scheduler", 0); // Whether to use selective scheduling
    //TODO: should I use selective scheduling?
    bool scheduler       = false;
    
    /* Detect the number of shards or preprocess an input to create them */
    int nshards          = convert_if_notexists<EdgeDataType>(filename,
                                                             get_option_string("nshards", "auto"));
    
    /* Run */
    VertexRelabel program;
    graphchi_engine<VertexDataType, EdgeDataType> engine(filename, nshards, scheduler, m);
    engine.run(program, niters);
    
    /* Report execution metrics */
    metrics_report(m);
    return 0;
}
