#### This is a `tcpdump` experiment dataset directory.
You can run the script `tcpdumptest.sh` to generate more dataset to run the framework yourself. Certain requirements apply.

* The system must install `camflow` to track the program

* `tcpdump` should be of version `4.5.1` (or any version that contains the access violation vulnerability)

* Bad instance is created through giving bad input to `tcpdump`. We supply a good input `okay.pcap` and a bad input `crash.pcap` for the experiment. In our generated 10 datasets, one of which is generated using `crach.pcap` and the rest are using `okay.pcap`

* To run the script: `./tcpdumptest.sh <file_path>` (You need to specify where you want the result input file to be. That file will be the input to the framework)