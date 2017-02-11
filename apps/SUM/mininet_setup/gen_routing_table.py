#!/usr/bin/python
#coding=utf-8

import os
#import json
import networkx as nx

from subprocess import call
from read_topo import read_topo, get_port_map
from utils import str_list_2_file, read_template, \
                replace_by_kw, read_json

AST_FILE = "ast.json"
PLACEMENT_FILE = "placement.json"
P4SRC_DIR = "../p4srcs"
COMMANDS_DIR = "commands"
DEFAULT_COLLECTION_ID = '0'

def get_dependency( ast ):
    dpdncy_graph = {}
    for s in ast:
        dpdncy_graph.setdefault(s["label"], [])
    for s in ast:
        for p in s["params"]:
            if p["property"] == "data_label":
                dpdncy_graph[ s["label"] ].append( p["label"])
    return dpdncy_graph


def find_label_by_index( ast, index ):
    for s in ast:
        if s['index'] == index:
            return s['label']
    return None

def find_index_by_label( ast, label ):
    for s in ast:
        if s['label'] == label:
            return s['index']
    return None

def find_next_appID( index ):
    ast = read_json(AST_FILE)
    dpdncy_graph = get_dependency(ast)
    label = find_label_by_index(ast, index)
    for k,v in dpdncy_graph.iteritems():
        if label in v:
            return find_index_by_label(ast, k)
    return None

def get_operations( ast ):
    operations = {}
    for s in ast:
        if s["func"] == "sum":
            operations[s["index"]] = s["func"]
    return operations

def fix_placement( operations ):
    """Assign each function (sum)/application_id to one switch
    """
    nb_hosts, nb_switches, links, hosts_set, switches_set = read_topo()
    assigned = []
    placement_dict = {}
    for opt_id in operations:
        switch = switches_set.pop()
        if switch not in assigned:
            placement_dict[ opt_id ] = switch
            assigned.append(switch)
    return placement_dict

def get_roots( dpdncy_graph ):
    roots = set( dpdncy_graph.keys() )
    non_roots = set()
    for n in dpdncy_graph:
        for non_root in dpdncy_graph[n]:
            non_roots.add(non_root)
    return list( roots.difference(non_roots) )

def setup_routing( placement_json ):
    """return routing table based on App_ID and then switch_ID
    """
    nb_hosts, nb_switches, links, hosts_set, switches_set = read_topo()
    port_map = get_port_map()

    G = nx.Graph()
    G_switches = nx.Graph()
    for a, b in links:
        G.add_edge(a, b)
        if 's' in a and 's' in b:
            G_switches.add_edge(a, b)
    shortest_paths = nx.shortest_path(G)
    switches_shortest_paths = nx.shortest_path(G_switches)
    # eg: shortest_path = shortest_paths[src][dst]

    routing_table = {}
    for App_ID in placement_json:
        routing_table.setdefault(App_ID, {})
        dst = placement_json[App_ID]
        for src in switches_set:
            if src != dst:
                path = switches_shortest_paths[src][dst]
                neighbor = path[1]
                routing_table[App_ID][src] = port_map[src][neighbor]
    # pick a receiver host
    # here for TESTing we use the last one
    receiver_host = 'h' + str(nb_hosts)
    routing_table.setdefault(DEFAULT_COLLECTION_ID, {})
    for src in switches_set:
        path = shortest_paths[src][receiver_host]
        neighbor = path[1]
        routing_table[DEFAULT_COLLECTION_ID][src] = port_map[src][neighbor]

    return routing_table


def write_p4(switch, func="SUM", app_ID=0, child = None):
    template = read_template(func)
    if child is not None:
        template = replace_by_kw(template, "define NEXT_APP", "0x00", str(child))
    if app_ID != 0:
        template = replace_by_kw(template, "define ASSIGNED_ID", "0x00", str(app_ID))
    p4src_path = P4SRC_DIR + os.sep + "SUM_" + switch + ".p4"
    str_list_2_file(template, p4src_path)

def write_commands(switch, routing_table):
    cmd_file_name = COMMANDS_DIR + os.sep + "commands_" + switch + ".txt"
    # table_set_default update_table _drop
    # table_add update_table my_update
    commands = """
    table_set_default update_table my_update
    table_set_default collection_table my_collection
    """.split('\n')
    for app_ID in routing_table:
        dst_port = routing_table[app_ID].get(switch, None)
        next_app = find_next_appID( app_ID )
        if dst_port is not None:
            # if current switch is not matched to app_ID, forward to dedicated switch
            commands.append("table_add forward_table reset_port %s => %s"
                            % (app_ID, dst_port) )
        elif next_app is not None:
            # if current switch is matched to app_ID, forward result to next_apps
            dst_port = routing_table[next_app].get(switch, None)
            commands.append("table_add forward_table reset_port %s => %s"
                            % (app_ID, dst_port) )
        else:
            # forward to receiver host
            commands.append("table_add forward_table reset_port %s => %s"
                            % (app_ID, routing_table[DEFAULT_COLLECTION_ID][switch]) )
    str_list_2_file(commands, cmd_file_name)

def gen_p4srcs(dpdncy_graph, placement, routing_table):
    # mkdir
    call(["mkdir","-p",P4SRC_DIR])
    call(["mkdir","-p",COMMANDS_DIR])
    # get switches list
    nb_hosts, nb_switches, links, hosts_set, switches_set = read_topo()
    for s in switches_set:
        if s not in placement.values():
            write_p4(switch=s, func="SUM", app_ID=0, child=None)
            write_commands(s, routing_table)
        else:
            for cur_app in placement:
                if s == placement[cur_app]:
                    next_App = find_next_appID(cur_app)
                    break
            write_p4(switch=s, func="SUM", app_ID=cur_app, child=next_App)
            write_commands(s, routing_table)

if __name__ == "__main__":
    # tpaths = get_paths( AST_FILE )
    # print tpaths
    print "Dependency:\n=========="
    dpndcy = get_dependency( read_json(AST_FILE) )
    print dpndcy
    opts = get_operations( read_json(AST_FILE) )
    print "\nPlacement:\n=========="
    plm = fix_placement( opts )
    print plm
    print "\nRouting Table:\n==============="
    table = setup_routing( plm )
    for app in table:
        print " App_"+app, ":", table[app]
    print "Next app of 4 is ", find_next_appID('4')

    # TEST scripts generation
    gen_p4srcs(dpndcy, plm, table)
