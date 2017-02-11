#!/usr/bin/python
#coding=utf-8

import os
import json

TEMPLATE_DIR = "../templates"

def read_json( json_file ):
    with open( json_file ) as data_file:
        data = json.load(data_file)
    return data

def file_2_str_list(filename):
    """Read file and store lines into list
    """
    str_list = []
    with open(filename, 'r') as fstream:
        for line in fstream:
            str_list.append(line)
    return str_list

def str_list_2_file(str_list, filename):
    """Write lines list to a file
    """
    with open(filename, 'w') as fstream:
        for line in str_list:
            if line == "" or line[-1] != '\n': line += '\n'
            fstream.write(line)
    print " Finish writing to file: " + filename

def sum_number_file(filename):
    """
    """
    mysum = 0
    with open(filename, 'r') as fstream:
        for line in fstream:
            mysum += sum([int(n) for n in line.strip().split(" ")])
    return mysum

def read_template(func="SUM"):
    """
    """
    template_path = TEMPLATE_DIR + os.sep + func + ".p4"
    template_lines = file_2_str_list(template_path)
    return template_lines

def test_template():
    templates = \
    """
    hello world
    of course
        test
    """
    print templates

def find_keyword_line(str_list, unique_kw):
    """ find the first line containing keyword
        return line index
    """
    for n in xrange(len(str_list)):
        if unique_kw in str_list[n]:
            return n
    return None

def replace_by_kw(str_list, unique_kw, to_replace, new_val):
    """ Replace a dedicated value field in the line
        if the line contains unique keyword
    """
    line_num = find_keyword_line(str_list, unique_kw)
    if line_num != None:
        str_list[line_num] = str_list[line_num].replace(to_replace, new_val)
    return str_list

def comment_line(str_list, unique_kw):
    """ Comment one line
        if the line exists with unique keyword
    """
    line_num = find_keyword_line(str_list, unique_kw)
    if line_num != None:
        str_list[line_num] = str_list[line_num] = "//" + str_list[line_num]
    return str_list

def add_lines(str_list, unique_kw, new_lines):
    """ Add new lines after the dedicated keyword line
        if the line exists with unique keyword
        return the new str_list
    """
    line_num = find_keyword_line(str_list, unique_kw)
    if line_num != None:
        return str_list[:line_num+1] + new_lines + str_list[line_num+1:]
    else:
        return None

if __name__ == "__main__":
    """
    contents = file_2_str_list("topo.txt")
    print contents
    str_list_2_file(contents, "test.txt")
    """
    test_template()
