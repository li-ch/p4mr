// Compile by "~$(pwd):g++ -std=c++11 multithreaded_mapreduce.cc -lpthread"
#include <algorithm>
#include <iostream>
#include <iterator>
#include <cstring>
#include <fstream>
#include <sstream>
#include <utility>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <mutex>
#include <ctime>
#include <map>
#include <set>

#include <dirent.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define SERVER "127.0.0.1"
#define BUFLEN 512
#define PORT 9999

using namespace std;

typedef vector<string>::const_iterator VectorIter;
typedef map<string,int>::const_iterator MapIter;
typedef vector<vector<pair<string, int> > >::const_iterator ResIter;
typedef vector<pair<string, int> >::const_iterator VecPairIter;
typedef chrono::high_resolution_clock::time_point T_type;

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
    for (const auto & f : file_names) {
        vector<string> file_words;
        file_words = read_file(f);
        all_words.insert(all_words.end(), file_words.begin(), file_words.end() );
    }
    return all_words;
}

void mapper(VectorIter start,
        VectorIter end,
        vector<vector<string> > & intermediate_bins,
        unsigned long const num_bins,
        vector<mutex> & my_mapper_locks) {
    for (VectorIter i = start; i != end; ++i) {
        string word = *i;
        size_t hashed_val = hash<string>{} (word);
        int bin_id = hashed_val % num_bins;

        // insert word into corresponding reduce bin
        // my_mapper_locks[bin_id].lock();
        intermediate_bins[bin_id].push_back( word );
        // my_mapper_locks[bin_id].unlock();
    }
}

void shuffle_to_reducers(vector<vector<vector<string> > > const & intermediate_bins,
        vector<string > & reduce_bin,
        unsigned long const bin_id) {
    for (const auto & inter: intermediate_bins) {
        for (const auto & word: inter[bin_id]) {
            reduce_bin.push_back( word );
        }
    }
}

void reducer(vector<vector<string> > const & reduce_bins,
        vector<vector<pair<string, int> > > & result_bins,
        unsigned long const bin_id) {
    VectorIter start = reduce_bins[bin_id].begin();
    VectorIter end   = reduce_bins[bin_id].end();
    string cur_word;
    int cur_count = 0;
    for (VectorIter i = start; i != end; ++i ) {
        if (!cur_count) {
            cur_word = *i;
            cur_count = 1;
        } else {
            if ( !cur_word.compare(*i) )
                cur_count += 1;
            else {
                /* update (word, count) in result_bins
                 * This process may not require lock since each reducer bin
                 * contains different set of words
                 *
                 */
                // my_reducer_lock.lock();
                pair<string, int> cur_pair
                    = make_pair (cur_word, cur_count);
                result_bins[bin_id].push_back (cur_pair);
                // my_reducer_lock.unlock();

                cur_word = *i;
                cur_count = 1;
            }
        }
    }

    pair<string, int> cur_pair
        = make_pair (cur_word, cur_count);
    result_bins[bin_id].push_back (cur_pair);
}


void udp_send(vector<vector<string> > const & reduce_bins) {
    struct sockaddr_in si_server;
    int s, i, slen = sizeof(si_server);
    char buf[BUFLEN];

    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1) {
        perror("socket");
        exit(1);
    }

    memset((char *) &si_server, 0, sizeof(si_server));
    si_server.sin_family = AF_INET;
    si_server.sin_port = htons(PORT);

    if (inet_aton(SERVER, &si_server.sin_addr)==0) {
        perror("inet_aton() failed\n");
        exit(1);
    }

    for (const auto& bin: reduce_bins) {
        for (const auto& bin_word: bin) {
            if (sendto(s,
                       bin_word.c_str(),
                       bin_word.size(),
                       0,
                       (struct sockaddr*) &si_server,
                       slen)==-1) {
                perror("sendto()");
                exit(1);
            }
        }
    }
}

void display_vector(vector<string> const & this_vec) {
    cout << '\t' << '\t';
    for (const auto& v: this_vec) {
        cout << v << ' ';
    }
    cout << '\n';
}

void display_reduce_bins(vector<vector<string> > const & reduce_bins) {
    for (const auto& bin: reduce_bins) {
        cout << '\t' << '\t';
        for (const auto& v: bin) {
            cout << v << ' ';
        }
        cout << '\n';
    }
    cout << '\n';
}

void display_reduce_bins_bytes(vector<vector<string> > const & reduce_bins) {
    cout << '\t' << "Bins Size (Bytes):" << ' ';
    for (const auto& bin: reduce_bins) {
        int cur_size = 0;
        for (const auto& bin_word: bin) {
            cur_size += bin_word.length();
        }
        cout <<  cur_size << ' ';
    }
    cout << '\n';
}

void display_results(vector<vector<pair<string, int> > > const & results) {
    cout << '\n' << '\t' << "  MR Results:" << '\n';
    for (const auto& bin: results) {
        for (const auto& i: bin) {
            cout << '\t' << '\t';
            cout << i.first << ": " << i.second << '\n';
        }
    }
    cout << '\n';
}

T_type print_time (T_type t_prev, string const & info) {
    T_type t_now =
        chrono::high_resolution_clock::now();
    chrono::duration<double> time_span =
        chrono::duration_cast<chrono::duration<double>> (t_now - t_prev);
    cout << '\t' << info << " Time: " << time_span.count()
         << " seconds." << '\n' << endl;
    return t_now;
}


int main (int argc, char* argv[]) {
    /*** Check input correctness***/
    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " DATA_FOLDER"
             << " #NUM_Threads" << '\n';
        return 1;
    }
    istringstream ss(argv[2]);
    int num_threads;
    if (!(ss >> num_threads)) {
        cerr << "Invalid threads number: " << argv[2] << '\n';
        return 1;
    }

    vector<string> files;
    files = list_dir(argv[1]);
    unsigned long const num_files = files.size();
    const unsigned long num_mappers = num_threads;
    const unsigned long num_reducers = num_threads;
    vector<vector<vector<string> > > intermediate_bins(num_reducers, vector<vector<string> >(num_reducers));
    vector<vector<string> > reduce_bins(num_reducers);
    vector<vector<pair<string, int> > > result_bins(num_reducers);


    /*** Load all data into memory, saying, a <vector> ***/
    vector<string> data = load_data(files);
    unsigned long const total_num_words = data.size();


    /*** Start recording time ***/
    T_type t1 = chrono::high_resolution_clock::now();
    T_type t_map, t_sort, t_red, t_final, t_network;
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
    vector<mutex> my_mapper_locks(num_reducers);
    for (unsigned long i=0; i < (num_mapper_threads - 1); ++i) {
        VectorIter block_end = block_start;
        advance(block_end, block_size);
        mapper_threads[i] = thread(
                mapper,
                block_start,
                block_end,
                ref(intermediate_bins[i]),
                num_reducers,
                ref(my_mapper_locks));
        block_start = block_end;
    }
    mapper(block_start,
            last,
            ref(intermediate_bins[num_mapper_threads - 1]),
            num_reducers,
            ref(my_mapper_locks));
    for_each( mapper_threads.begin(),
              mapper_threads.end(),
              mem_fn(&thread::join) );

    /*** shuffling threads (ATTENTION: this part is included in MAP step) ***/
    data = vector<string> ();
    vector<thread> shuffle_threads(num_reducers);
    for (unsigned long i=0; i < num_reducers; ++i) {
        shuffle_threads[i] = thread(
                shuffle_to_reducers,
                ref(intermediate_bins),
                ref(reduce_bins[i]),
                i
                );
    }
    for_each( shuffle_threads.begin(),
            shuffle_threads.end(),
            mem_fn(&thread::join) );
    t_map = print_time(t1, "Map");


    /*** sorting threads ***/
    vector<thread> sort_threads(num_reducers);
    cout << "\tNumber of sorting threads: " << num_reducers << '\n';
    for (unsigned long i=0; i < num_reducers; ++i) {
        sort_threads[i] = thread(
                sort<vector<string>::iterator>,
                reduce_bins[i].begin(),
                reduce_bins[i].end()
                );
    }
    for_each( sort_threads.begin(),
            sort_threads.end(),
            mem_fn(&thread::join) );
    t_sort = print_time(t_map, "Sort");


    /*** reducers threads ***/
    vector<thread> reducer_threads(num_reducers);
    cout << "\tNumber of reducer threads: " << num_reducers << '\n';
    for (unsigned long i=0; i < num_reducers; ++i) {
        reducer_threads[i] = thread(
                reducer,
                ref(reduce_bins),
                ref(result_bins),
                i);
    }
    for_each( reducer_threads.begin(),
            reducer_threads.end(),
            mem_fn(&thread::join) );
    t_red = print_time(t_sort, "Reduce");


    /*** summary ***/
    t_final = print_time(t1, "Total Running");
    display_reduce_bins_bytes( ref(reduce_bins) );

    /*** Send intermediate data to localhost by UDP ***/
    T_type t_again = chrono::high_resolution_clock::now();
    udp_send(ref(reduce_bins));
    cout << endl;
    t_network = print_time(t_again, "In Network");

    return 0;
}
