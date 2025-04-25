# Socket Test
## How to Run
Step 1: Build the files

`make clean && make build -j`

Step 2: Install Dependencies

`sudo apt-get install zlib1g-dev`

Step 3: Run the build.

Open one terminal and run the server

`cd server`

`./server <Port_Number>`

Open another terminal and run the client

`cd client`

`./client <host_by_name> <Port_Number>`
