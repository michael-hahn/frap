# Fault-detection through Runtime Analysis of Provenance

[![DOI](https://zenodo.org/badge/82714093.svg)](https://zenodo.org/badge/latestdoi/82714093)

#### How to run vertexlabel_static: 

Make vertexlabel_static application by running the following command:

At `graph-chi` directory, run:

`make myapps/vertexlabel_static`

and then use the following command to run the application:

`bin/myapps/vertexlabel_static ngraphs <num_of_graphs> file0 <file_name> file1 <file_name> ...  niters <iteration_time>`

The application can take arbitrary number of graphs now. After providing the size in `<num_of_graphs>`, be sure to have option starting from `0` (i.e. `file0`), and all the way to the number `file``num_of_graphs - 1`.

The sequence of input files corresponds to the sequence of normalized kernel value of each graph.

Currently, three versions are available:

* Directionless (`#define VERSION 0` ): Each vertex takes the labels of all neighboring vertices, regardless of whether they are incoming-edge neighbors or outgoing-edge neighbors; edge labels are ignored in this version.

* Direction-aware (`#define VERSION 1`): Each vertex takes the labels of all incoming-edge neighboring vertices and generates a new label based on them. It then takes the labels of all outgoing-edge neighboring vertices and generates a new label based on them. Finally, it takes the two new labels and generates its final new label for future iterations. Edge labels are ignored in this version.

* Simple edge-aware (`#define TAKEEDGELABEL 1`): This macro can be set in addition to the previous two versions. When the macro is set to `1`, both version `0` and `1` will gather edge labels during the *_first_* time their neighboring vertex labels are being gathered. That is, edge labels are only gathered once during the first hop of neighbor discovery. Gathering edge labels after the first hop does not make sense, neither does relabeling edge labels. Note that although richer information about the graph is obtained, this is a simplified version since when sorting the labels for relabeling, edge labels and their corresponding vertex labels are not guaranteed physical proximity in the label array. For example, if a vertex labeled `0` has a neighbor node labeled `2` with an edge labeled `7` and another neighbor node labeled `3` with an edge labeled `1`, then we will have generate (assuming `VERSION 0`) the label array `[1, 2, 3, 7]` for relabeling purpose. However, it has more physical meaning if we can have the array `[2, 7, 3, 1]`(which will be implemented in another version). 

* Edge-aware (`#define VERSION 2`): This is the version that when sorting the array of labels, edge labels follow their corresponding vertex labels. This version treats incoming-edge and outgoing-edge differently (like version `1`).

Two metrics are available as well:

* Normalized sum of multiplication (`#define METRIC 0`): We illustrate this metric with an example. Consider three count arrays A:[1, 2, 3] B:[2, 0, 1] C:[30, 20, 10]. To obtain a normalized kernel value of A, we first calculate the dot product of A and B (which is 5), A and C (which is 100), A and A (which is 14), B and B (which is 5), and C and C (which is 1400). We notate the dot product of X and Y: D(X, Y). Then we calculate the normalized dot product: ND(X, Y) for X != Y. For instance, we calculate ND(A, B) = D(A, B)/(D(A, A) * D(B, B)). The final value for A is the average of ND(A, B) and ND(A, C).

* Geometric distance (`#define METRIC 1`): For each instance, we calculate the average geometric distance between the instance and the rest of the instances. 

Now we use fancy statistical analysis to detect anomaly. We have implemented 3 different distance measures and k-mean clustering:

* Kullback-Leibler distance: symmetric Kullback-Leibler divergence with back-off probability (`#define KULLBACKLEIBLER 0`).

* Hellinger distance: a symmetric Bhattacharyya distance that obeys triangle inequality. (`#define HELLINGER 1`).

* Euclidean distance: same as geometric distance. (`#define EUCLIDEAN 2`).

All above distance measures are used to perform K-mean clustering.

Unfortunately, you have to change the macro in the code (`#define VERSION X`, `#define TAKEEDGELABEL X`, `#define METRIC X`), rebuild, and re-run at this point.

_TODO: The application is not optimized. Many algorithms can be changed to make it more efficient. There are many redundant computations for now. Smaller locks should be used instead as well._

#### Run the better version

Exactly the same way as running vertexlabel_static!

At `graph-chi` directory, run:

`make myapps/main`

and then use the following command to run the application:

`bin/myapps/main ngraphs <num_of_graphs> nmonitor <num_of_graphs> file0 <file_name> file1 <file_name> ...  niters <iteration_time>`

* `ngraphs` should be total number of graphs, including the graphs from the learning stage and the ones from the detecting stage

* `nmonitor` should be the number of provenance DAGs being monitored only

* files being monitored should be at the end of the list of files. For example, if we have 3 learning DAGs and 1 monitored one, the command should be:
`bin/myapps/main ngraphs 4 nmonitor 1 file0 <file_name> file1 <file_name> file2 <file_name> file3 <file_name>`

##### Experiment Results 

We run the following command:

`bin/myapps/main ngraphs 19 nmonitor 1 file0 myapps/server/edgeList1.txt file1 myapps/server/edgeList2.txt file2 myapps/server/edgeList3.txt file3 myapps/server/edgeList4.txt file4 myapps/server/edgeList5.txt file5 myapps/server/edgeList6.txt file6 myapps/server/edgeList7.txt file7 myapps/server/edgeList8.txt file8 myapps/server/edgeList9.txt file9 myapps/server/edgeList_bad.txt file10 myapps/dataset1/edgeList1.txt file11 myapps/dataset1/edgeList2.txt file12 myapps/dataset1/edgeList3.txt file13 myapps/dataset1/edgeList4.txt file14 myapps/dataset1/edgeList5.txt file15 myapps/dataset1/edgeList6.txt file16 myapps/dataset1/edgeList7.txt file17 myapps/dataset1/edgeList_bad.txt file18 myapps/dataset1/edgeList8.txt niters 4`

(Use Kullback-Leibler distance metric)

* Explanation: file 0 - 8 are one normal program behavior; file 10 - 16 are another normal program behavior; file 9 and 17 are two different abnormal program behavior and file 18 is the file to be monitored

* The results of `kmeans_prior` (by clustering pairwise distances as prior):
```
Prior Cluster: 0-1 0-2 0-3 0-4 0-5 0-7 0-8 1-2 1-3 1-4 1-6 1-7 1-8 2-3 2-4 2-6 2-7 2-8 3-4 3-5 3-7 3-8 4-5 4-6 4-7 4-8 5-7 5-8 7-8 10-11 10-12 10-13 10-14 10-15 10-16 11-12 11-13 11-14 11-15 11-16 12-13 12-14 12-15 12-16 13-14 13-15 13-16 14-15 14-16 15-16 
Prior Cluster: 0-10 0-11 0-12 0-13 0-14 0-15 0-16 1-10 1-11 1-12 1-13 1-14 1-15 1-16 2-10 2-11 2-12 2-13 2-14 2-15 2-16 3-10 3-11 3-12 3-13 3-14 3-15 3-16 4-10 4-11 4-12 4-13 4-14 4-15 4-16 5-10 5-11 5-12 5-13 5-14 5-15 5-16 6-10 6-11 6-12 6-13 6-14 6-15 6-16 7-10 7-11 7-12 7-13 7-14 7-15 7-16 8-10 8-11 8-12 8-13 8-14 8-15 8-16 9-10 9-11 9-12 9-13 9-14 9-15 9-16 9-17 10-17 11-17 13-17 15-17 
Prior Cluster: 0-6 0-9 1-5 1-9 2-5 2-9 3-6 3-9 4-9 5-6 6-7 6-8 6-9 7-9 8-9 
Prior Cluster: 
Prior Cluster: 
Prior Cluster: 0-17 1-17 2-17 3-17 4-17 5-17 6-17 7-17 8-17 
Prior Cluster: 
Prior Cluster: 
Prior Cluster: 
Prior Cluster: 
Prior Cluster: 
Prior Cluster: 5-9 12-17 14-17 16-17 
Prior Cluster: 
Prior Cluster: 
Prior Cluster: 
Prior Cluster: 
Prior Cluster: 
Prior Cluster:  
```

We find 5 populated clusters. We can set 
`K = 5`
when clustering feature vectors (calling function `kmeans`). 

* The results of `kmeans` (by clustering feature vectors/instances using K = 5):
```
Cluster: 0 1 2 3 4 5 6 7 8 
Cluster: 10 11 12 13 14 15 16 
Cluster: 9 
Cluster: 17 
Cluster: 
```
Therefore, our prior algorithm (i.e., clustering KL distances) overestimate the number of clusters.

FRAP identifies 9 and 17 as bad instances to learn and only generate 2 clusters as its profile. It also records the centroid and the radius of each cluster for later detecting stage.

The profile has the following information:
```
Final number of clusters:2
Final size of centroids:2
Final size of distances:2
Max distance of each cluster: 1.97949 0.714156
```
During the detecting stage, FRAP analyzes the DAG of file 18, generating its count array using the relablling map from the learning stage, and comparing the count array with the two centroids of the clusters. It calculates the distance between this feature vector and the two centroid. The results are:

`Distances of monitored instance: 4.86445 0.525388`

Since file 18 is actually a well behaved instance that displays the second normal program behavior, it can be cluster to the second cluster; therefore, FRAP concludes:

`This monitored instance is normal...`

* We remove the empty clutser, record the centroid of each cluster and its radius of the cluster

* Our clustering and analysis algorithm can find abnormal program run: 9 and 17

#### Make file in `myapps` directory:

This `Makefile` is to make jsonparser application.

Just run `make` to build.

#### Run jsonparser:

The usage is:

`./jsonparser` _`input_file_path`_ _`output_file_path`_

Note that each line in `input_file_path` should be a `JSON` object.
