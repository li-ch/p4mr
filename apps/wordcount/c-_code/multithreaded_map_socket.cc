// Compile by "~$(pwd):g++ -o multithreaded_mr -std=c++11 multithreaded_map_socket.cc -lpthread"
// "~$(pwd):./multithreaded_mr DATA_FOLDER #NUM_THREADS"
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



// The below includes are only for socket programming
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVERPORT 30001
#define SERVER_IP_ADDRESS "127.0.0.1"


using namespace std;

typedef vector<string>::const_iterator VectorIter;
typedef map<string,int>::const_iterator MapIter;
typedef chrono::high_resolution_clock::time_point T_type;

mutex my_mapper_lock;



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


void error(const char *msg)
{
    perror(msg);
    exit(1);
}


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
    unsigned long const num_mappers = num_threads;
    unsigned long const num_reducers = num_threads;
    vector<vector<string> > reduce_bins(num_reducers);


    /*** Load all data into memory, saying, a <vector> ***/
    vector<string> data = load_data(files);
    unsigned long const total_num_words = data.size();


    /*** Start recording time ***/
    T_type t1 = chrono::high_resolution_clock::now();
    T_type t_map, t_send, t_final;
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
    for_each( mapper_threads.begin(),
              mapper_threads.end(),
              mem_fn(&thread::join) );
    t_map = print_time(t1, "Map");
    
    /*** sending everything through a UDP socket ***/
    
    int sockfd;
    struct sockaddr_in serv_addr;
    int num_bytes;
    

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    
    if (sockfd < 0) 
      error("ERROR opening socket");

    memset((char *) &serv_addr, 0, sizeof(serv_addr));
 
    serv_addr.sin_family = AF_INET;
    inet_pton(AF_INET, SERVER_IP_ADDRESS, &(serv_addr.sin_addr)); 
    serv_addr.sin_port = htons(SERVERPORT);
 
    const socklen_t serverlen = sizeof(serv_addr);
   
    // iterate through all bins and send word by word to the network (case one)
    for(const auto& bin : reduce_bins) 
    {
    
     for(const auto& word : bin)
     {
      if((num_bytes = sendto(sockfd, word.c_str(), strlen(word.c_str()), 0,
               (struct sockaddr*) &serv_addr, serverlen)) < 0)
           error("sendto()"); 
     }// for
    }//for   
 
    close(sockfd);    

    t_send = print_time(t_map, "Send Time");


    /*** summary ***/
    t_final = print_time(t1, "Total Running");

    return 0;
}
