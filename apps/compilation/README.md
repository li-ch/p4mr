# Implementation of P4MR with compilation

### User Commands:
```
A := store<uint_64>("ip_h1:path_A");
B := store<uint_64>("ip_h2:path_B");
C := store<uint_64>("ip_h3:path_C");

D := SUM(A, B);
E := SUM(C, D);
```

### Topology

     h1 
       \
        \
  h2 --- S1 --- S2 --- h4
        /
       / 
     h3  

where, 'A' stores on h1, 'B' stores on h2 and 'C' stores on h3

### Packet Format
```
    Preamble (64) | opt_id (8) | int_data (64)
```

### Questions
    -   How to modify a metadata/register value
    -   How to send register value to one host
