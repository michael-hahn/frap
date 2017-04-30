//
//  vertex.cpp
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
#include "graphchi_basic_includes.hpp"
#include "logger/logger.hpp"
#include "vertex.hpp"
#include "kernelmaps.hpp"
#include "global.h"

using namespace graphchi;

/**
 * GraphChi programs need to subclass GraphChiProgram<vertex-type, edge-type>
 * class. The main logic is usually in the update function.
 */
struct VertexRelabel : public GraphChiProgram<VertexDataType, EdgeDataType> {
    
    //get the singleton kernelMaps
    KernelMaps* km = KernelMaps::get_instance();
    
    //locks for sync update
    std::mutex relabel_map_lock;
    std::mutex label_map_lock;
    /**
     *  Vertex update function.
     */
    void update(graphchi_vertex<VertexDataType, EdgeDataType> &vertex, graphchi_context &gcontext) {
        //this should not happen if data is clean - node id should be sequential and there should not be any skipped ids
        //we skip here for dirty data
        if (vertex.num_inedges() <= 0 && vertex.num_outedges() <= 0) {
            logstream(LOG_INFO) << "Isolated vertex "<<  vertex.id() <<" detected" << std::endl;
            return;
        }
        //swap phase in odd-numbered iterations
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
        } else {//update phase in even-numbered iterations
            if (gcontext.iteration == 0) {
                /* On first iteration, initialize vertex (and its edges). This is usually required, because
                 on each run, GraphChi will modify the data files. To start from scratch, it is easiest
                 do initialize the program in code. Alternatively, you can keep a copy of initial data files. */
                // for each vertex, set its label as its w3c type
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
                    int label_map_label = km->insert_relabel(vertex_label);
                    relabel_map_lock.unlock();
                    label_map_lock.lock();
                    km->insert_label(label_map_label);
                    label_map_lock.unlock();
                    vertex.set_data(label_map_label);
                    logstream(LOG_INFO) << "The value of label " << vertex.id() << " is: " << label_map_label << std::endl;
                } else {
                    logstream(LOG_FATAL) << "Invalid vertex_label in relabel_map. " << std::endl;
                    assert (vertex_label != "");
                }
            } else {//include edge type during relabeling in the second update phase iteration
                if (gcontext.iteration == 2) {
                    std::vector<std::pair<int, int>> incoming_pair_label_vec;
                    std::vector<std::pair<int, int>> outgoing_pair_label_vec;
                    for(int i=0; i < vertex.num_inedges(); i++) {
                        graphchi_edge<EdgeDataType> * in_edge = vertex.inedge(i);
                        int int_in_type = in_edge->get_data().old_src;
                        int int_in_edge_type = in_edge->get_data().edge;
                        std::pair<int, int> pair_label_in (int_in_type, int_in_edge_type);
                        incoming_pair_label_vec.push_back(pair_label_in);
                    }
                    for (int i=0; i < vertex.num_outedges(); i++) {
                        graphchi_edge<EdgeDataType> * out_edge = vertex.outedge(i);
                        int int_out_type = out_edge->get_data().old_dst;
                        int int_out_edge_type = out_edge->get_data().edge;
                        std::pair<int, int> pair_label_out (int_out_type, int_out_edge_type);
                        outgoing_pair_label_vec.push_back(pair_label_out);
                    }
                    struct {
                        bool operator()(std::pair<int, int> a, std::pair<int, int> b) {
                            return a.first < b.first;
                        }
                    } comparePair;
                    
                    std::sort(incoming_pair_label_vec.begin(), incoming_pair_label_vec.end(), comparePair);
                    std::sort(outgoing_pair_label_vec.begin(), outgoing_pair_label_vec.end(), comparePair);
                    
                    std::string new_incoming_label = "";
                    std::string first_incoming_num_string;
                    std::stringstream first_incoming_out;
                    first_incoming_out << vertex.get_data();
                    first_incoming_num_string = first_incoming_out.str();
                    new_incoming_label += first_incoming_num_string;
                    new_incoming_label += ",";
                    
                    for (std::vector<std::pair<int, int>>::iterator it = incoming_pair_label_vec.begin(); it != incoming_pair_label_vec.end(); ++it) {
                        std::string incoming_num_string_first;
                        std::stringstream incoming_out_first;
                        incoming_out_first << it->first;
                        incoming_num_string_first = incoming_out_first.str();
                        new_incoming_label += incoming_num_string_first + " ";
                        std::string incoming_num_string_second;
                        std::stringstream incoming_out_second;
                        incoming_out_second << it->second;
                        incoming_num_string_second = incoming_out_second.str();
                        new_incoming_label += incoming_num_string_second + " ";
                    }
                    
                    std::string new_outgoing_label = "";
                    std::string first_outgoing_num_string;
                    std::stringstream first_outgoing_out;
                    first_outgoing_out << vertex.get_data();
                    first_outgoing_num_string = first_outgoing_out.str();
                    new_outgoing_label += first_outgoing_num_string;
                    new_outgoing_label += ",";
                    
                    for (std::vector<std::pair<int, int>>::iterator it = outgoing_pair_label_vec.begin(); it != outgoing_pair_label_vec.end(); ++it) {
                        std::string outgoing_num_string_first;
                        std::stringstream outgoing_out_first;
                        outgoing_out_first << it->first;
                        outgoing_num_string_first = outgoing_out_first.str();
                        new_incoming_label += outgoing_num_string_first + " ";
                        std::string outgoing_num_string_second;
                        std::stringstream outgoing_out_second;
                        outgoing_out_second << it->second;
                        outgoing_num_string_second = outgoing_out_second.str();
                        new_incoming_label += outgoing_num_string_second + " ";
                    }
                    
                    relabel_map_lock.lock();
                    int label_map_label_incoming = km->insert_relabel(new_incoming_label);
                    int label_map_label_outgoing = km->insert_relabel(new_outgoing_label);
                    relabel_map_lock.unlock();
                    
                    std::string new_combined_label = ""; //label_map_label_incoming,label_map_label_outgoing
                    std::string incoming_label_in_combined;
                    std::stringstream incoming_label_in_combined_out;
                    incoming_label_in_combined_out << label_map_label_incoming;
                    incoming_label_in_combined = incoming_label_in_combined_out.str();
                    new_combined_label += incoming_label_in_combined;
                    new_combined_label += ",";
                    std::string outgoing_label_in_combined;
                    std::stringstream outgoing_label_in_combined_out;
                    outgoing_label_in_combined_out << label_map_label_outgoing;
                    outgoing_label_in_combined = outgoing_label_in_combined_out.str();
                    new_combined_label += outgoing_label_in_combined;
                    
                    relabel_map_lock.lock();
                    int label_map_label_combined = km->insert_relabel(new_combined_label);
                    relabel_map_lock.unlock();
                    
                    label_map_lock.lock();
                    km->insert_label(label_map_label_combined);
                    label_map_lock.unlock();
                    vertex.set_data(label_map_label_combined);
                    logstream(LOG_INFO) << "The value of label " << vertex.id() << " is: " << label_map_label_combined << std::endl;
                } else {//only takes incoming and outgoing vertex labels
                    std::vector<int> incoming_label_vec;
                    std::vector<int> outgoing_label_vec;
                    for(int i=0; i < vertex.num_inedges(); i++) {
                        graphchi_edge<EdgeDataType> * in_edge = vertex.inedge(i);
                        int int_in_type = in_edge->get_data().old_src;
                        incoming_label_vec.push_back(int_in_type);
                        logstream(LOG_INFO) << "Vertex " << vertex.id() << " getting " << int_in_type << " from in edges" << std::endl;
                    }
                    for (int i=0; i < vertex.num_outedges(); i++) {
                        graphchi_edge<EdgeDataType> * out_edge = vertex.outedge(i);
                        int int_out_type = out_edge->get_data().old_dst;
                        outgoing_label_vec.push_back(int_out_type);
                        logstream(LOG_INFO) << "Vertex " << vertex.id() << " getting " << int_out_type << " from out edges" << std::endl;
                    }
                    std::sort(incoming_label_vec.begin(), incoming_label_vec.end());
                    std::sort(outgoing_label_vec.begin(), outgoing_label_vec.end());
                    
                    int self_label = vertex.get_data();
                    incoming_label_vec.push_back(self_label);
                    outgoing_label_vec.push_back(self_label);
                    
                    std::string new_incoming_label = "";
                    std::string first_incoming_num_string;
                    std::stringstream first_incoming_out;
                    first_incoming_out << *(incoming_label_vec.end() - 1);
                    first_incoming_num_string = first_incoming_out.str();
                    new_incoming_label += first_incoming_num_string;
                    new_incoming_label += ",";
                    
                    for (std::vector<int>::iterator it = incoming_label_vec.begin(); it != incoming_label_vec.end() - 1; ++it) {
                        std::string incoming_num_string;
                        std::stringstream incoming_out;
                        incoming_out << *it;
                        incoming_num_string = incoming_out.str();
                        new_incoming_label += incoming_num_string + " ";
                    }
                    
                    std::string new_outgoing_label = "";
                    std::string first_outgoing_num_string;
                    std::stringstream first_outgoing_out;
                    first_outgoing_out << *(outgoing_label_vec.end() - 1);
                    first_outgoing_num_string = first_outgoing_out.str();
                    new_outgoing_label += first_outgoing_num_string;
                    new_outgoing_label += ",";
                    
                    for (std::vector<int>::iterator it = outgoing_label_vec.begin(); it != outgoing_label_vec.end() - 1; ++it) {
                        std::string outgoing_num_string;
                        std::stringstream outgoing_out;
                        outgoing_out << *it;
                        outgoing_num_string = outgoing_out.str();
                        new_outgoing_label += outgoing_num_string + " ";
                    }
                    
                    relabel_map_lock.lock();
                    int label_map_label_incoming = km->insert_relabel(new_incoming_label);
                    int label_map_label_outgoing = km->insert_relabel(new_outgoing_label);
                    relabel_map_lock.unlock();
                    
                    std::string new_combined_label = ""; //label_map_label_incoming,label_map_label_outgoing
                    std::string incoming_label_in_combined;
                    std::stringstream incoming_label_in_combined_out;
                    incoming_label_in_combined_out << label_map_label_incoming;
                    incoming_label_in_combined = incoming_label_in_combined_out.str();
                    new_combined_label += incoming_label_in_combined;
                    new_combined_label += ",";
                    std::string outgoing_label_in_combined;
                    std::stringstream outgoing_label_in_combined_out;
                    outgoing_label_in_combined_out << label_map_label_outgoing;
                    outgoing_label_in_combined = outgoing_label_in_combined_out.str();
                    new_combined_label += outgoing_label_in_combined;
                    
                    relabel_map_lock.lock();
                    int label_map_label_combined = km->insert_relabel(new_combined_label);
                    relabel_map_lock.unlock();
                    
                    label_map_lock.lock();
                    km->insert_label(label_map_label_combined);
                    label_map_lock.unlock();
                    vertex.set_data(label_map_label_combined);
                    logstream(LOG_INFO) << "The value of label " << vertex.id() << " is: " << label_map_label_combined << std::endl;
                }
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

struct VertexRelabelDetection : public GraphChiProgram<VertexDataType, EdgeDataType> {
    
    //get the singleton kernelMaps
    KernelMaps* km = KernelMaps::get_instance();
    
    //locks for sync update
    std::mutex relabel_map_lock;
    std::mutex label_map_lock;
    /**
     *  Vertex update function.
     */
    void update(graphchi_vertex<VertexDataType, EdgeDataType> &vertex, graphchi_context &gcontext) {
        //this should not happen if data is clean - node id should be sequential and there should not be any skipped ids
        //we skip here for dirty data
        if (vertex.num_inedges() <= 0 && vertex.num_outedges() <= 0) {
            logstream(LOG_INFO) << "Isolated vertex "<<  vertex.id() <<" detected" << std::endl;
            return;
        }
        //swap phase in odd-numbered iterations
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
        } else {//update phase in even-numbered iterations
            if (gcontext.iteration == 0) {
                /* On first iteration, initialize vertex (and its edges). This is usually required, because
                 on each run, GraphChi will modify the data files. To start from scratch, it is easiest
                 do initialize the program in code. Alternatively, you can keep a copy of initial data files. */
                // for each vertex, set its label as its w3c type
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
                    int label_map_label = km->insert_relabel(vertex_label);
                    relabel_map_lock.unlock();
                    label_map_lock.lock();
                    km->insert_label(label_map_label);
                    label_map_lock.unlock();
                    vertex.set_data(label_map_label);
                    logstream(LOG_INFO) << "The value of label " << vertex.id() << " is: " << label_map_label << std::endl;
                } else {
                    logstream(LOG_FATAL) << "Invalid vertex_label in relabel_map. " << std::endl;
                    assert (vertex_label != "");
                }
            } else {//include edge type during relabeling in the second update phase iteration
                if (gcontext.iteration == 2) {
                    std::vector<std::pair<int, int>> incoming_pair_label_vec;
                    std::vector<std::pair<int, int>> outgoing_pair_label_vec;
                    for(int i=0; i < vertex.num_inedges(); i++) {
                        graphchi_edge<EdgeDataType> * in_edge = vertex.inedge(i);
                        int int_in_type = in_edge->get_data().old_src;
                        int int_in_edge_type = in_edge->get_data().edge;
                        std::pair<int, int> pair_label_in (int_in_type, int_in_edge_type);
                        incoming_pair_label_vec.push_back(pair_label_in);
                    }
                    for (int i=0; i < vertex.num_outedges(); i++) {
                        graphchi_edge<EdgeDataType> * out_edge = vertex.outedge(i);
                        int int_out_type = out_edge->get_data().old_dst;
                        int int_out_edge_type = out_edge->get_data().edge;
                        std::pair<int, int> pair_label_out (int_out_type, int_out_edge_type);
                        outgoing_pair_label_vec.push_back(pair_label_out);
                    }
                    struct {
                        bool operator()(std::pair<int, int> a, std::pair<int, int> b) {
                            return a.first < b.first;
                        }
                    } comparePair;
                    
                    std::sort(incoming_pair_label_vec.begin(), incoming_pair_label_vec.end(), comparePair);
                    std::sort(outgoing_pair_label_vec.begin(), outgoing_pair_label_vec.end(), comparePair);
                    
                    std::string new_incoming_label = "";
                    std::string first_incoming_num_string;
                    std::stringstream first_incoming_out;
                    first_incoming_out << vertex.get_data();
                    first_incoming_num_string = first_incoming_out.str();
                    new_incoming_label += first_incoming_num_string;
                    new_incoming_label += ",";
                    
                    for (std::vector<std::pair<int, int>>::iterator it = incoming_pair_label_vec.begin(); it != incoming_pair_label_vec.end(); ++it) {
                        std::string incoming_num_string_first;
                        std::stringstream incoming_out_first;
                        incoming_out_first << it->first;
                        incoming_num_string_first = incoming_out_first.str();
                        new_incoming_label += incoming_num_string_first + " ";
                        std::string incoming_num_string_second;
                        std::stringstream incoming_out_second;
                        incoming_out_second << it->second;
                        incoming_num_string_second = incoming_out_second.str();
                        new_incoming_label += incoming_num_string_second + " ";
                    }
                    
                    std::string new_outgoing_label = "";
                    std::string first_outgoing_num_string;
                    std::stringstream first_outgoing_out;
                    first_outgoing_out << vertex.get_data();
                    first_outgoing_num_string = first_outgoing_out.str();
                    new_outgoing_label += first_outgoing_num_string;
                    new_outgoing_label += ",";
                    
                    for (std::vector<std::pair<int, int>>::iterator it = outgoing_pair_label_vec.begin(); it != outgoing_pair_label_vec.end(); ++it) {
                        std::string outgoing_num_string_first;
                        std::stringstream outgoing_out_first;
                        outgoing_out_first << it->first;
                        outgoing_num_string_first = outgoing_out_first.str();
                        new_incoming_label += outgoing_num_string_first + " ";
                        std::string outgoing_num_string_second;
                        std::stringstream outgoing_out_second;
                        outgoing_out_second << it->second;
                        outgoing_num_string_second = outgoing_out_second.str();
                        new_incoming_label += outgoing_num_string_second + " ";
                    }
                    
                    relabel_map_lock.lock();
                    int label_map_label_incoming = km->insert_relabel(new_incoming_label);
                    int label_map_label_outgoing = km->insert_relabel(new_outgoing_label);
                    relabel_map_lock.unlock();
                    
                    std::string new_combined_label = ""; //label_map_label_incoming,label_map_label_outgoing
                    std::string incoming_label_in_combined;
                    std::stringstream incoming_label_in_combined_out;
                    incoming_label_in_combined_out << label_map_label_incoming;
                    incoming_label_in_combined = incoming_label_in_combined_out.str();
                    new_combined_label += incoming_label_in_combined;
                    new_combined_label += ",";
                    std::string outgoing_label_in_combined;
                    std::stringstream outgoing_label_in_combined_out;
                    outgoing_label_in_combined_out << label_map_label_outgoing;
                    outgoing_label_in_combined = outgoing_label_in_combined_out.str();
                    new_combined_label += outgoing_label_in_combined;
                    
                    relabel_map_lock.lock();
                    int label_map_label_combined = km->insert_relabel(new_combined_label);
                    relabel_map_lock.unlock();
                    
                    label_map_lock.lock();
                    km->insert_label(label_map_label_combined);
                    label_map_lock.unlock();
                    vertex.set_data(label_map_label_combined);
                    logstream(LOG_INFO) << "The value of label " << vertex.id() << " is: " << label_map_label_combined << std::endl;
                } else {//only takes incoming and outgoing vertex labels
                    std::vector<int> incoming_label_vec;
                    std::vector<int> outgoing_label_vec;
                    for(int i=0; i < vertex.num_inedges(); i++) {
                        graphchi_edge<EdgeDataType> * in_edge = vertex.inedge(i);
                        int int_in_type = in_edge->get_data().old_src;
                        incoming_label_vec.push_back(int_in_type);
                        logstream(LOG_INFO) << "Vertex " << vertex.id() << " getting " << int_in_type << " from in edges" << std::endl;
                    }
                    for (int i=0; i < vertex.num_outedges(); i++) {
                        graphchi_edge<EdgeDataType> * out_edge = vertex.outedge(i);
                        int int_out_type = out_edge->get_data().old_dst;
                        outgoing_label_vec.push_back(int_out_type);
                        logstream(LOG_INFO) << "Vertex " << vertex.id() << " getting " << int_out_type << " from out edges" << std::endl;
                    }
                    std::sort(incoming_label_vec.begin(), incoming_label_vec.end());
                    std::sort(outgoing_label_vec.begin(), outgoing_label_vec.end());
                    
                    int self_label = vertex.get_data();
                    incoming_label_vec.push_back(self_label);
                    outgoing_label_vec.push_back(self_label);
                    
                    std::string new_incoming_label = "";
                    std::string first_incoming_num_string;
                    std::stringstream first_incoming_out;
                    first_incoming_out << *(incoming_label_vec.end() - 1);
                    first_incoming_num_string = first_incoming_out.str();
                    new_incoming_label += first_incoming_num_string;
                    new_incoming_label += ",";
                    
                    for (std::vector<int>::iterator it = incoming_label_vec.begin(); it != incoming_label_vec.end() - 1; ++it) {
                        std::string incoming_num_string;
                        std::stringstream incoming_out;
                        incoming_out << *it;
                        incoming_num_string = incoming_out.str();
                        new_incoming_label += incoming_num_string + " ";
                    }
                    
                    std::string new_outgoing_label = "";
                    std::string first_outgoing_num_string;
                    std::stringstream first_outgoing_out;
                    first_outgoing_out << *(outgoing_label_vec.end() - 1);
                    first_outgoing_num_string = first_outgoing_out.str();
                    new_outgoing_label += first_outgoing_num_string;
                    new_outgoing_label += ",";
                    
                    for (std::vector<int>::iterator it = outgoing_label_vec.begin(); it != outgoing_label_vec.end() - 1; ++it) {
                        std::string outgoing_num_string;
                        std::stringstream outgoing_out;
                        outgoing_out << *it;
                        outgoing_num_string = outgoing_out.str();
                        new_outgoing_label += outgoing_num_string + " ";
                    }
                    
                    relabel_map_lock.lock();
                    int label_map_label_incoming = km->insert_relabel(new_incoming_label);
                    int label_map_label_outgoing = km->insert_relabel(new_outgoing_label);
                    relabel_map_lock.unlock();
                    
                    std::string new_combined_label = ""; //label_map_label_incoming,label_map_label_outgoing
                    std::string incoming_label_in_combined;
                    std::stringstream incoming_label_in_combined_out;
                    incoming_label_in_combined_out << label_map_label_incoming;
                    incoming_label_in_combined = incoming_label_in_combined_out.str();
                    new_combined_label += incoming_label_in_combined;
                    new_combined_label += ",";
                    std::string outgoing_label_in_combined;
                    std::stringstream outgoing_label_in_combined_out;
                    outgoing_label_in_combined_out << label_map_label_outgoing;
                    outgoing_label_in_combined = outgoing_label_in_combined_out.str();
                    new_combined_label += outgoing_label_in_combined;
                    
                    relabel_map_lock.lock();
                    int label_map_label_combined = km->insert_relabel(new_combined_label);
                    relabel_map_lock.unlock();
                    
                    label_map_lock.lock();
                    km->insert_label(label_map_label_combined);
                    label_map_lock.unlock();
                    vertex.set_data(label_map_label_combined);
                    logstream(LOG_INFO) << "The value of label " << vertex.id() << " is: " << label_map_label_combined << std::endl;
                }
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