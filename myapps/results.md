## Experiment Results

### Commands and its resulting Clusters, centroid and radius, and decisions (whether the monitoring instances are good or bad)
#### Ruby Server Experiment

* Command
```
bin/myapps/main ngraphs 12 nmonitor 2 file0 myapps/server/edgeList1.txt file1 myapps/server/edgeList2.txt file2 myapps/server/edgeList3.txt file3 myapps/server/edgeList4.txt file4 myapps/server/edgeList5.txt file5 myapps/server/edgeList6.txt file6 myapps/server/edgeList7.txt file7 myapps/server/edgeList8.txt file8 myapps/server/edgeList9.txt file9 myapps/server/edgeList_bad.txt file10 myapps/server/edgeList_bad.txt file11 myapps/server/edgeList4.txt niters 4
```

1. KLD 10 Instances for Learning (1 Bad Instance) and 2 Instances during Detection (1 Bad and 1 Good)

Clusters:
```
Cluster: 0 1 2 3 4 5 6 7 8 
Cluster: 9 
Final number of clusters: 1
```
Centroid and radius:
```
Final size of centroids: 1
Final size of radii: 1
Max radius of each cluster: 1.85487 
```
Decisions:
```
Distances of monitored instance: 3.14887 
This monitored instance is outside the radius of any cluster...

Distances of monitored instance: 0.762998 
This monitored instance is normal...
```

2. Hellinger Distance

Clusters:
```
Cluster: 0 1 2 3 4 5 6 7 8 9 
Final number of clusters: 1
```
Centroid and radius:
```
Final size of centroids: 1
Final size of radii: 1
Max radius of each cluster: 0.401805 
```
Decisions:
```
Distances of monitored instance: 0.401805 
This monitored instance is normal...

Distances of monitored instance: 0.269798 
This monitored instance is normal...
```

3. Euclidean Distance

Clusters:
```
Cluster: 8 
Cluster: 5 
Cluster: 9 
Cluster: 
Cluster: 
Cluster: 1 2 6 
Cluster: 0 3 4 7 
Cluster: 
Cluster: 
Final number of clusters: 2
```
Centroid and radius:
```
Final size of centroids: 2
Final size of radii: 2
Max radius of each cluster: 13.784 17.4929 
```
Decisions:
```
Distances of monitored instance: 177.815 182.779 
This monitored instance is outside the radius of any cluster... Recluster now...

Distances of monitored instance: 76.642 17.4929 
This monitored instance is normal...
```
