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

Unfortunately, you have to change the macro in the code (`#define VERSION X`, `#define TAKEEDGELABEL X`, `#define METRIC X`), rebuild, and re-run at this point.

_TODO: The application is not optimized. Many algorithms can be changed to make it more efficient. There are many redundant computations for now. Smaller locks should be used instead as well._

#### Make file in `myapps` directory:

This `Makefile` is to make jsonparser application.

Just run `make` to build.

#### Run jsonparser:

The usage is:

`./jsonparser` _`input_file_path`_ _`output_file_path`_

Note that each line in `input_file_path` should be a `JSON` object.
