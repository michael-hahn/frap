#### This is a `wget` experiment dataset directory.
You can run the script `wgettest_server.sh` on the server side to set up a malicious server. You can run `wgettest_client.sh` on the client side to generate dataset to run the framework yourself. Certain requirements apply.

* The client must call `wget` with version prior to `v1.18`

* The client must have `camflow` installed to track the program

* To run the server side script: `./wgettest_server.sh <IP_address>` (You need to specify the IP address of your server.) You also need root access for the server program to run.