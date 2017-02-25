#### How to run vertexlabel_static: 

Make vertexlabel_static application by running the following command:

At `graph-chi` directory, run:

`make myapps/vertexlabel_static`

and then use the following command to run the application:

`bin/myapps/vertexlabel_static file <file_name> file2 <file_name> niters <iteration_time>`

Currently, three versions available:

* Directionless (`#define VERSION 0` ): Each vertex takes the labels of all neighboring vertices, regardless of whether they are incoming-edge neighbors or outgoing-edge neighbors; edge labels are ignored in this version.

* Direction-aware (`#define VERSION 1`): Each vertex takes the labels of all incoming-edge neighboring vertices and generates a new label based on them. It then takes the labels of all outgoing-edge neighboring vertices and generates a new label based on them. Finally, it takes the two new labels and generates its final new label for future iterations. Edge labels are ignored in this version.

* Simple edge-aware (`#define TAKEEDGELABEL 1`): This macro can be set in addition to the previous two versions. When the macro is set to `1`, both version `0` and `1` will gather edge labels during the *_first_* time their neighboring vertex labels are being gathered. That is, edge labels are only gathered once during the first hop of neighbor discovery. Gathering edge labels after the first hop does not make sense, neither does relabeling edge labels. Note that although richer information about the graph is obtained, this is a simplified version since when sorting the labels for relabeling, edge labels and their corresponding vertex labels are not guaranteed physical proximity in the label array. For example, if a vertex labeled `0` has a neighbor node labeled `2` with an edge labeled `7` and another neighbor node labeled `3` with an edge labeled `1`, then we will have generate (assuming `VERSION 0`) the label array `[1, 2, 3, 7]` for relabeling purpose. However, it has more physical meaning if we can have the array `[2, 7, 3, 1]`(which will be implemented in another version). 

Unfortunately, you have to change the macro in the code (`#define VERSION X`), rebuild, and re-run at this point.

#### Make file in `myapps` directory:

This `Makefile` is to make jsonparser application.

Just run `make` to build.

#### Run jsonparser:

The usage is:

`./jsonparser` _`input_file_path`_ _`output_file_path`_

Note that each line in `input_file_path` should be a `JSON` object.
