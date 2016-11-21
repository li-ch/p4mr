#include <cctype>
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <iterator>
#include <chrono>


using namespace std;

typedef std::chrono::high_resolution_clock::time_point T_type;


//#define DATA_CHUNK_SIZE 16 // data chunks in bytes (Hadoop uses 64MB default)
#define MIN_CHUNK_SIZE 16 // chunk must be at least 16-byte size.

void 
writeTimespanToFile(const T_type& t_start, const T_type& t_end, 
                    const char* const filename, const char* const chunkSize, const char* const dataSize);


vector<string> splitString(const string& line) // splits a string into separate words (uses a spcace to split)
{
    istringstream buffer(line);
    vector<string> ret((istream_iterator< string >(buffer)), 
                                 istream_iterator< string >());
    return ret;
  
}


bool punctValue(const int ch)
{

  return (ispunct(ch) != 0);
}


// read data into memory and then start processing in memory
vector<string> readIntoMemory(const char* const FILE_NAME)
{
  string line;
  vector<string> words;
  
  ifstream input_file(FILE_NAME, ios_base::in); // open the data file
 
  if(input_file.is_open())
  {
    while(getline(input_file, line, '\n'))
    {
      line.erase(remove_if(line.begin (), line.end (), punctValue), line.end()); // get rid of punctuation signs
      vector<string> line_words = splitString(line); // get a vector of words on this line
      words.insert(words.end(), line_words.begin(), line_words.end()); // append to the word vector
    }
 
    input_file.close();
  }
  
   return words; // return loaded words into memory
}


// No need to write to a file. Store everything in vectors
void partitionData(const size_t DATA_CHUNK_SIZE, vector<string>& file_data)
{ 
 
  size_t size = 0; // the size of the current chunk (in bytes)
  const size_t SIZE_OF_NULL_CHAR = sizeof('\0'); // we need this since strings in C++ are char arrays with a null char

  vector< vector<string> > chunks; // a vector for storing chunks in memory
  chunks.push_back(vector<string>());


  for(; file_data.begin() != file_data.end(); )
  { 
    if((size + file_data.begin()->size()) > DATA_CHUNK_SIZE)
    {
      size = 0;
      chunks.push_back(vector<string>()); // new chunk
      continue; // this chunk is full
    }

    // read a word into the chunk 
    size += (file_data.begin()->size() + SIZE_OF_NULL_CHAR);
    chunks.back().push_back(std::move(*file_data.begin())); // use the move constructor == much faster
    file_data.erase(file_data.begin()); // erase the first element
  }// for

   /*chunks vector contains chunks that can be now processed, e.g., send over the network and so on*/ 
}



int main (int argc, char** argv)
{

  if(argc == 5) // first argument: chunk_size;  second argument: data_size string; third argument: datafile; fourth argument: timespan file. 
  {
    stringstream chunkReader(argv[1]);
   
    size_t chunk_size;

    chunkReader >> chunk_size;     

    if(chunk_size < MIN_CHUNK_SIZE)
    {
      printf("Your entered chunk size is too small. Please try one more time (min chunk size = %d bytes).\n", MIN_CHUNK_SIZE);
      return 0;
    }


    // Read the input file into memory
    vector<string> data_file(readIntoMemory(argv[3]));

    if(data_file.empty())
    {
     printf("Your entered file cannot be opened or is empty.\n"); 
    }


    /*** Start recording time ***/
    T_type t_start = chrono::high_resolution_clock::now();

    partitionData(chunk_size, data_file); // run a script for partition a big dataset into smaller ones.

    T_type t_end = chrono::high_resolution_clock::now();

    writeTimespanToFile(t_start, t_end, argv[4], argv[1], argv[2]);
  }
  else 
  {
    printf("Please input the chunk size, an input file for reading (.txt) and an output file for storing time (.txt)\n");
  }

  return 0;
}


// this function writes a timespan to a file
void 
writeTimespanToFile(const T_type& t_start, const T_type& t_end, 
             const char* const filename, const char* const chunkSize, const char* const datasetSize)
{

    chrono::duration<double> time_span =
        chrono::duration_cast<chrono::duration<double>> (t_end - t_start);
    stringstream timespanStream;
    timespanStream << time_span.count() << "\n"; // converts time into a tring

    string writeString(datasetSize);
    writeString.append(",")
    .append(chunkSize).append(",")
    .append(timespanStream.str()); // create a line for the text file

    ofstream file(filename, ios_base::app); // append times
    file.write(writeString.c_str(), writeString.size()); // write to the file

    file.close();
}
