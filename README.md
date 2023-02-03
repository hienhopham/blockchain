
1. Install needed pakages

sudo apt install g++ python3 python3-dev pkg-config sqlite3 cmake ninja-build git python3-setuptools clang-format libc6-dev libc6-dev-i386 libclang-dev llvm-dev automake python3-pip

pip3 install PyBindGen pygccxml cxxfilt

2. Download ns3

git clone https://gitlab.com/nsnam/ns-3-allinone.git

cd ns-3-allinone
./download.py -n ns-3.36

3. Build

./ns3 configure --enable-examples --enable-tests --enable-python-bindings
./ns3 build
./ns3 run hello-simulator

Note:
    Run without build
    ./ns3 run --no-build examples/wireless/mixed-wired-wireless.py

    Enable log infor
    export 'NS_LOG=Blockchain=info:RsuNode=info:TopologyHelper=info'

    Print comand arguments
    ./ns3 run --no-build "scratch/blockchain/main.cc --PrintHelp"

4. Install VSC
5. Install C/C++ plugin
6. Go to c_cpp_properties.json file, add build path (absolute) to the "includePath" configuration. For ex: "/home/hienpham/repos/ns-3-allinone/ns-3.36/build/include/"
