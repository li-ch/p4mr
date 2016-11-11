#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <string>
#include <cstring>
#include <vector>
#include <iterator>
#include <thread>
#include <ctime>
#include <chrono>
#include <map>
#include <set>

#include <dirent.h>

using namespace std;

typedef vector<string>::const_iterator VectorIter;

struct tokens: ctype<char> 
{
    tokens(): ctype<char>(get_table()) {}

    static ctype_base::mask const* get_table()
    {
        typedef ctype<char> cctype;
        static const cctype::mask *const_rc= cctype::classic_table();

        static cctype::mask rc[cctype::table_size];
        memcpy(rc, const_rc, cctype::table_size * sizeof(cctype::mask));

        rc[','] = ctype_base::space; 
        rc['.'] = ctype_base::space; 
        rc[' '] = ctype_base::space; 
        return &rc[0];
    }
};

vector<string> split(string const &input) {
    stringstream ss(input);
    ss.imbue(locale(locale(), new tokens()));
    istream_iterator<string> begin(ss);
    istream_iterator<string> end;
    vector<string> vstrings(begin, end);
    return vstrings;
}

vector<string> list_dir(const char* v) {
    DIR *dir;
    struct dirent *ent;
    vector<string> file_names;
    string PWD = string(".");
    string PRE = string("..");

    if ((dir = opendir (v)) != NULL) {
          /* print all the files and directories within directory */
          string fname;
          while ((ent = readdir (dir)) != NULL) {
              string dir_str = v;
              fname = ent->d_name;
              if (fname.compare(PWD) && fname.compare(PRE)) {
                file_names.push_back (dir_str.append(fname));
              } 
          }
            closedir (dir);
    } else {
          /* could not open directory */
          perror ("could not open directory");
    }

    return file_names;
}

void map_reduce(VectorIter start, VectorIter end) {
    string line;
    vector<string> words;
    map<string, int> maps;

    for (VectorIter f = start; f != end; ++f) {
        ifstream myfile (*f);
        if (myfile.is_open())
        {
            while ( getline (myfile,line) )
            {
                words = split(line);
                for (VectorIter w = words.begin(); w != words.end(); ++w) {
                    // Display words
                    // cout << *w << '\n';
                    maps[*w] += 1;     
                }
            }
            myfile.close();
            /* Display results
             *
            typedef map<string,int>::const_iterator MapIter;
            for (MapIter m = maps.begin(); m != maps.end(); ++m) {
                cout << m->first << ": " << m->second << '\n';
            }
            */
        }

        else cout << "Unable to open file"; 
    }
}

int main () {
    chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();
    vector<string> files;
    files = list_dir("./data/");
    unsigned long const num_files = files.size();

    /* threads group */
    unsigned long const min_per_thread = 2;
    unsigned long const max_threads =
        (num_files + min_per_thread - 1) / min_per_thread;
    unsigned long const hardware_threads = 
        thread::hardware_concurrency();
    unsigned long const num_threads = 
        min(hardware_threads != 0 ? hardware_threads:2, max_threads);

    unsigned long const block_size = num_files / num_threads;

    cout << "\tNumber of threads: " << num_threads << '\n';

    vector<thread> threads(num_threads -1);
    VectorIter first = files.begin(); 
 
    VectorIter last  = files.end(); 
    VectorIter block_start = first; 
    for (unsigned long i=0; i < (num_threads -1); ++i) {
        VectorIter block_end = block_start;
        advance(block_end, block_size);
        threads[i] = thread(
                map_reduce,
                block_start,
                block_end);
        block_start = block_end;
    }
    map_reduce(block_start, last);
    for_each( threads.begin(), threads.end(), mem_fn(&thread::join) );

    chrono::high_resolution_clock::time_point t2 =
        chrono::high_resolution_clock::now();
    chrono::duration<double> time_span =
        chrono::duration_cast<chrono::duration<double>> (t2 - t1);
    cout << "\tTime spent: " << time_span.count() << " seconds." << endl;

    return 0;
}

