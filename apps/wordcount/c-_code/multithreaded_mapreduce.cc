#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <string>
#include <cstring>
#include <vector>
#include <iterator>
#include <thread>
#include <mutex>
#include <ctime>
#include <chrono>
#include <map>
#include <set>

#include <dirent.h>

using namespace std;

#define NUM_MAPPERS    3
#define NUM_REDUCERS   3
typedef vector<string>::const_iterator VectorIter;
typedef map<string,int>::const_iterator MapIter;

mutex my_mapper_lock;
mutex my_reducer_lock;

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

vector<string> read_file(string const & file_name) {
    string line;
    vector<string> words;
    ifstream myfile (file_name);
    if (myfile.is_open()) {
        while ( getline (myfile, line) ) {
            vector<string> line_words;
            line_words = split(line);
            words.insert( words.end(), line_words.begin(), line_words.end() );
        }
        myfile.close();
    }
    else cout << "Unable to open file"; 

    return words;
}

vector<string> load_data(vector<string> const & file_names) {
    vector<string> all_words;
    VectorIter start = file_names.begin();
    VectorIter end   = file_names.end();
    for (VectorIter f = start; f != end; ++f) {
        vector<string> file_words;
        file_words = read_file(*f);
        all_words.insert(all_words.end(), file_words.begin(), file_words.end() );
    }
    return all_words;
}

void mapper(VectorIter start,
        VectorIter end,
        vector<vector<string> > & reduce_bins,
        unsigned long const num_bins) {
    for (VectorIter i = start; i != end; ++i ) {
        string word = *i;
        size_t hashed_val = hash<string>{} (word);
        int bin_id = hashed_val % num_bins;

        // insert word into corresponding reduce bin
        my_mapper_lock.lock();
        reduce_bins[bin_id].push_back( word );
        my_mapper_lock.unlock();
    }
}

void reducer(vector<vector<string> > const & reduce_bins,
        map<string, int> & result,
        unsigned long const bin_id) {
    VectorIter start = reduce_bins[bin_id].begin();
    VectorIter end   = reduce_bins[bin_id].end();
    for (VectorIter i = start; i != end; ++i ) {
        string word = *i;

        // update (word, count) in result
        // This process may not require lock since each reducer bin
        // contains different set of words
        // my_reducer_lock.lock();
        result[word] += 1;
        // my_reducer_lock.unlock();
    }
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
                    /* Display words
                     *
                    // cout << *w << '\n';
                    */
                    maps[*w] += 1;     
                }
            }
            myfile.close();
            /* Display results
             *
            for (MapIter m = maps.begin(); m != maps.end(); ++m) {
                cout << m->first << ": " << m->second << '\n';
            }
            */
        }
        else cout << "Unable to open file"; 
    }
}

void display_vector( vector<string> const & this_vec) {
    VectorIter start = this_vec.begin();
    VectorIter end   = this_vec.end();
    cout << '\t' << '\t';
    for (VectorIter i = start; i != end; ++i) {
        cout << *i << ' ';
    }
    cout << '\n';
}

void display_map( map<string, int> const & this_map) {
    MapIter start = this_map.begin();
    MapIter end   = this_map.end();
    cout << '\n' << '\t' << "  MR Results:" << '\n';
    for (MapIter i = start; i != end; ++i) {
        cout << '\t' << '\t';
        cout << i->first << ": " << i->second << '\n';
    }
    cout << '\n';
}

int main () {
    vector<string> files;
    files = list_dir("./data/");
    unsigned long const num_files = files.size();
    unsigned long const num_mappers = NUM_MAPPERS;
    unsigned long const num_reducers = NUM_REDUCERS;
    vector<vector<string> > reduce_bins(num_reducers);
    map<string, int> result;

    /*** Load all data into memory, saying, a <vector> ***/
    vector<string> data = load_data(files);
    unsigned long const total_num_words = data.size();

    chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();
    /*** mappers threads ***/
    unsigned long const hardware_threads = 
        thread::hardware_concurrency();
    unsigned long const num_mapper_threads = 
        min(hardware_threads != 0 ? hardware_threads:2, num_mappers);

    unsigned long const block_size = total_num_words / num_mapper_threads;

    cout << "\tNumber of mapper threads: " << num_mapper_threads << '\n';

    vector<thread> mapper_threads(num_mapper_threads -1);
    VectorIter first = data.begin(); 
 
    VectorIter last  = data.end(); 
    VectorIter block_start = first; 
    for (unsigned long i=0; i < (num_mapper_threads -1); ++i) {
        VectorIter block_end = block_start;
        advance(block_end, block_size);
        mapper_threads[i] = thread(
                mapper,
                block_start,
                block_end,
                ref(reduce_bins),
                num_reducers);
        block_start = block_end;
    }
    mapper(block_start, last, ref(reduce_bins), num_reducers);
    for_each( mapper_threads.begin(), mapper_threads.end(), mem_fn(&thread::join) );
    chrono::high_resolution_clock::time_point t_map =
        chrono::high_resolution_clock::now();
    chrono::duration<double> time_span =
        chrono::duration_cast<chrono::duration<double>> (t_map - t1);
    cout << "\tMap Time: " << time_span.count() << " seconds." << endl;

    /*** reducers threads ***/
    vector<thread> reducer_threads(num_reducers);
    cout << "\tNumber of reducer threads: " << num_reducers << '\n';
    for (unsigned long i=0; i < num_reducers; ++i) {
        reducer_threads[i] = thread(
                reducer,
                ref(reduce_bins),
                ref(result),
                i);
    }
    for_each( reducer_threads.begin(), reducer_threads.end(), mem_fn(&thread::join) );
    chrono::high_resolution_clock::time_point t_red =
        chrono::high_resolution_clock::now();
    time_span =
        chrono::duration_cast<chrono::duration<double>> (t_red - t_map);
    cout << "\tReduce Time: " << time_span.count() << " seconds." << endl;


    /*** summary ***/
    // display_map(result);
    chrono::high_resolution_clock::time_point t2 =
        chrono::high_resolution_clock::now();
    time_span =
        chrono::duration_cast<chrono::duration<double>> (t2 - t1);
    cout << "\tTotal Time spent: " << time_span.count() << " seconds." << endl;

    return 0;
}

