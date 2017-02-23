#### How to run vertexlabel_static: 

Make vertexlabel_static application by running the following command:

At `graph-chi` directory, run:

`make myapps/vertexlabel_static`

and then use the following command to run the application:

`bin/myapps/vertexlabel_static file <file_name> file2 <file_name> niters <iteration_time>`

#### Make file in `myapps` directory:

This `Makefile` is to make jsonparser application.

Just run `make` to build.

#### Run jsonparser:

The usage is:

`./jsonparser` _`input_file_path`_ _`output_file_path`_

Note that each line in `input_file_path` should be a `JSON` object.
