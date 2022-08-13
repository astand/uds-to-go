# UDS TO GO
## Lightwieght and simple to use UDS/DoCAN stack for MCU and other platforms

## Fast start and test
Repo has a test application based on socketCAN system which can be build and run to test and play with ISO-TP
To build and run test app (src/example/main.cpp) you have to have:
- Ubuntu PC (I didn't test it on any other Linux distributives)
- **can-utils** installed on you PC (optional, just to use **isotpdump**)
- **cmake** and C++ development toolkit (c++11 standard mandatory)

### Step 1
Install can-utils and build tools if necessary
### Step 2
Open terminal and go to the source code directory. Make directory for repo
``` 
mkdir uds-to-go
```
### Step 2
Clone repository from github
```
git clone https://github.com/astand/uds-to-go.git uds-to-go
```
### Step 3
Go to the repo directory and run build. There are also predefined shell scripts to run 
two instances of ISO-TP with test parameters. To use them they have to be executable
```
cd uds-to-go
cmake -S src -B build
cmake --build build
chmod u+x start-client.sh && chmod u+x start-serv.sh
```
### Step 4
ISO test app requiers alive socketCAN interface. Test scripts use interface 'vcan0', to start it up run the next commnad:
```
sudo ip link add name vcan0 type vcan
sudo ip link set dev vcan0 up
```
> To get vcan0 down use command:
> _sudo ip link delete vcan0_

### Step 5
Open three terminals and run next commands in each of them:
```
./start-client.sh
```
```
./start-serv.sh
```
```
isotpdump vcan0 -s 0x700 -d 0x701 -t d
```
After that you can send ISO-TP payloads with different length from server to client ang vice versa. 
You can also inspect CAN log in the ***isotpdump*** terminal

_Example of using server-client communication with isotpdump logging_
![Drag Racing](docs/test-run-example.png)
