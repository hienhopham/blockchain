# Blockchain with smart contracts
## 1. Install needed pakages
```sh
sudo apt install g++ python3 python3-dev pkg-config sqlite3 cmake ninja-build git python3-setuptools clang-format libc6-dev libc6-dev-i386 libclang-dev llvm-dev automake python3-pip
```

```sh
pip3 install PyBindGen pygccxml cxxfilt
```

## 2. Download ns3

```sh
git clone https://gitlab.com/nsnam/ns-3-allinone.git
```

```sh
cd ns-3-allinone
./download.py -n ns-3.36.1
```

## 3. Download rapidjson
Download from [rapidjson](https://github.com/Tencent/rapidjson/tree/master/include/rapidjson).
Copy the folder `rapidjson` under folder `ns-3.36`.

## 4. Build and run

```sh
./ns3 configure --enable-examples --enable-tests --enable-python-bindings
./ns3 build
./ns3 run "scratch/blockchain/main.cc"
```
Number of winner nodes and payments can be set through files from folder `auction`.

Transaction threshold for the smart contract condition can be set from the run command, ex:
```sh
./ns3 run "scratch/blockchain/main.cc -transThreshold=3"
```

**NOTE SOME HELPFUL COMMANDS**
| cmd | des |
| ------ | ------ |
| ./ns3 run --no-build examples/wireless/mixed-wired-wireless.py | Run without build |
| export 'NS_LOG=Blockchain=info:RsuNode=info:TopologyHelper=info' | Enable log infor|
| ./ns3 run "scratch/blockchain/main.cc --PrintHelp" | Print comand arguments |

## 5. Install VSC

## 6. Install C/C++ plugin
Go to `c_cpp_properties.json` file, add build path (absolute) to the **includePath** configuration. For ex: `"/home/hienpham/repos/ns-3-allinone/ns-3.36/build/include/"`

