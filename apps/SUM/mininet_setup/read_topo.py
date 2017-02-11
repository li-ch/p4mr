#!/usr/bin/python

import networkx as nx

def read_topo():
    nb_hosts, nb_switches = 0, 0
    hosts_set, switches_set = set(), set()
    links = []
    with open("topo.txt", "r") as f:
        line = f.readline()[:-1]
        w, nb_switches = line.split()
        assert(w == "switches")
        line = f.readline()[:-1]
        w, nb_hosts = line.split()
        assert(w == "hosts")
        for line in f:
            if not f: break
            a, b = line.split()
            links.append( (a, b) )
            for target in [a, b]:
                assert(target[0] == "h" or target[0] == "s")
                if target[0] == "h": hosts_set.add(target)
                if target[0] == "s": switches_set.add(target)
        assert( int(nb_hosts) == len(hosts_set))
        assert( int(nb_switches) == len(switches_set))
    return int(nb_hosts), int(nb_switches), links, hosts_set, switches_set


def get_port_map():
    nb_hosts, nb_switches, links, hosts_set, switches_set = read_topo()
    port_map = {}

    for a, b in links:
        port_map.setdefault(a, {})
        port_map.setdefault(b, {})

        assert(b not in port_map[a])
        assert(a not in port_map[b])
        port_map[a][b] = len(port_map[a]) + 1
        port_map[b][a] = len(port_map[b]) + 1

    return port_map


def get_shortest_path(src, dst):
    nb_hosts, nb_switches, links, hosts_set, switches_set = read_topo()
    assert(src in hosts_set)
    assert(dst in hosts_set)
    port_map = get_port_map()

    G = nx.Graph()
    for a, b in links:
        G.add_edge(a, b)
    shortest_paths = nx.shortest_path(G)
    shortest_path = shortest_paths[src][dst]
    print "path is:", shortest_path

    port_list = []
    first = shortest_path[1]
    for h in shortest_path[2:]:
        port_list.append(port_map[first][h])
        first = h
    print "port list is:", port_list

    port_str = ""
    for p in port_list:
        port_str += chr(p)

    return shortest_path, port_list

if __name__ == "__main__":
    port_map = get_port_map()
    for target in sorted(port_map.keys()):
        print target, ": ", port_map[target]
