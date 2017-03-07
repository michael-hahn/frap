#### This is a `wget` experiment dataset directory.
You can run the script `wgettest_server.sh` on the server side to set up a malicious server. You can run `wgettest_client.sh` on the client side to generate dataset to run the framework yourself. Certain requirements apply.

* The client must call `wget` with version prior to `v1.18`. You must install older version of `wget` (for example, in our test case, we use `GNU wget 1.17.1`)if your system has a later version.

* The client must have `camflow` installed to track the program

* To run the server side script: `sudo ./wgettest_server.sh <IP_address>` (You need to specify the IP address of your server.) You also need root access for the server program to run. It is possible that the script is not executable. Run `chmod` to make it executable by you or your group.

* If your virtual IP address was not set correctly via Vagrant, run the following command line to fix the bug (run the command line in the VMs): `sudo service network restart`
