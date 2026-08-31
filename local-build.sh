yosys -p "read_verilog ldpc_encoder.v; hierarchy -top ldpc_encoder; write_cxxrtl ldpc_design.cpp"
g++ -O3 -std=c++14 -I $(yosys-config --datdir)/include main.cpp -o ldpc_sim
./ldpc_sim
