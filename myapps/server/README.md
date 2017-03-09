### This folder contains datasets from our Rudy server experiment

You can run this experiment yourself and generate more data as you like

Specifically, on one machine designated as your server:

* You can run: `./server.sh` with the following conditions:

** You must have `Ruby` installed on your server. If you are using `Fedora` and your system does not have `Ruby` readily installed, you can do this by giving this command: `sudo yum install ruby`

** The IP address of the server in the `server.sh` is hard-coded for our particular server. We also picked port `2345`. You need to change the IP address and/or port to your server's IP address and a port of your liking.

* Before you run the server script, make sure you have `camflow` installed on the server and have it monitor the server script by running `sudo camflow --track-file server.sh propagate`.

We also wrote a client script that makes requests to the server several times. In another/other machine(s), you can run `./client.sh`. You can also just use your browser and make requests to the server.

`public` folder contains the server's content.