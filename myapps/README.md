#### How to run vertexlabel_static: 

Make vertexlabel_static application by running the following command:

At `graph-chi` directory, run:

`make myapps/vertexlabel_static`

and then use the following command to run the application:

`bin/myapps/vertexlabel_static file <file_name> file2 <file_name> niters <iteration_time>`

Currently, two versions available:

* Directionless (`#define VERSION 0` ): Each vertex takes the labels of all neighboring vertices, regardless of whether they are incoming-edge neighbors or outgoing-edge neighbors; edge labels are ignored in this version.

* Direction-aware (`#define VERSION 1`): Each vertex takes the labels of all incoming-edge neighboring vertices and generates a new label based on them. It then takes the labels of all outgoing-edge neighboring vertices and generates a new label based on them. Finally, it takes the two new labels and generates its final new label for future iterations. Edge labels are ignored in this version.

Unfortunately, you have to change the macro in the code (`#define VERSION X`), rebuild, and re-run at this point.

#### Make file in `myapps` directory:

This `Makefile` is to make jsonparser application.

Just run `make` to build.

#### Run jsonparser:

The usage is:

`./jsonparser` _`input_file_path`_ _`output_file_path`_

Note that each line in `input_file_path` should be a `JSON` object.
