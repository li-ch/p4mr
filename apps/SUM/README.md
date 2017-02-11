# Implementation of P4MR with compilation


### To Generate p4SRCs:
```
cd mininet_setup;
./gen_routing_table.py
```

### To Run:
```
cd mininet_setup;
./run_demo.sh
```
Then execute mininet commands to test, for example:
```
mininet> h6 ./receive.py h6-eth2 &      // listen on host6 iface h6-eth2
                                       // record pkts in myresults.txt
mininet> h1 ./send.py                 // send data pkts from host1
```
Remember to run ./cleanup after exiting from mininet console


### User Commands:
```
A := store<uint_64>("ip_h1:path_A");
B := store<uint_64>("ip_h2:path_B");
C := store<uint_64>("ip_h3:path_C");

D := SUM(A, B);
E := SUM(C, D);
```


### Topology (example, for the actual case check mininet_setup/topo.txt file)

      h1               h4
        \             /
         \           /
    h2 --- S1 --- S2 --- h5
         /           \
        /             \
      h3               h6

where, 'A' stores on h1, 'B' stores on h2 and 'C' stores on h3

### Packet Format
```
    Preamble (64) | app_id (8) | routing_id (8) | collection_id (8) | int_data (64)
```
where Preamble is to separate from other traffic,
      app_id is the application unit identifier,
      routing_id is used to route pkt, initialized by app_id,
      collection: 0 means data pkt, 1 means collection pkt,
      ini_data is fixed length integer input.

### TODO
combine with Compiler module
