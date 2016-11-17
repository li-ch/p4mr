#include <fstream>
#include <cctype>
#include <string>
#include <algorithm>
#include <vector>
#include <sstream>
#include <iterator>
#include <chrono>


using namespace std;

typedef std::chrono::high_resolution_clock::time_point T_type;


//#define DATA_CHUNK_SIZE 16 // data chunks in bytes (Hadoop uses 64MB default)
#define MIN_CHUNK_SIZE 16 // chunk must be at least 16-byte size.

void writeTimespanToFile(const T_type& t_start, const T_type& t_end, const char* const filename);

uint32_t getFileSize(ofstream& file)
{
  
  return ((uint32_t)file.tellp()); // returns file size
}

bool readRemain(const uint32_t DATA_CHUNK_SIZE, vector<string>& words, ofstream& file, const uint32_t curSize = 0)// has a side effect
{
  uint32_t position = 0, readBytes = 0;

  for(vector<string>::const_iterator itr = words.begin(); itr != words.end(); itr++)
  {
    if((curSize + readBytes + (*itr).size() + 1 /*'\n'*/)  > DATA_CHUNK_SIZE)
    { // don't read anymore
       break; 
    }
    
    stringstream sstream;
    sstream << (*itr) << '\n';
    string line =  sstream.str();
    file.write(line.c_str(), line.size()); // no need to read a whitespace
    
    // update variables 
    position++;
    readBytes += line.size(); 
  }// for loop

  if(position != words.size()) // means broke out of the loop
  { // clear the vector up to position
    vector<string>::iterator eraseItr = (words.begin() + position);
    words.erase(words.begin(), eraseItr); // clear the vector up to position

    return true;
  }
  else
  { 
    words.clear(); // clear the entire vector for next readings since it has been written on a file
    
    return false;
  }

}

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


void partitionData(const uint32_t DATA_CHUNK_SIZE, const char* const FILE_NAME)
{ 
 
  ifstream inputFile(FILE_NAME, ios_base::in); // open the data file

  int counter = 0; // a counter for chunk files
  vector<string> words(0); // a vector for storing words (see below how it is used)
 
  // read the file and keep subdividing it into smaller files (chunk files)
 
  while(!inputFile.eof())
  { 
    stringstream streamString;
    streamString << "chunk_" << counter << ".txt";
    const string title = streamString.str(); // a title for a new chunk file
    counter++; // increment the counter for next iteration


    ofstream file(title.c_str(), ios_base::out); // a new text file for writing  
    
    // if some words left from previous line, read them in the newly created file    
    if(!words.empty())
    { // read
      if(readRemain(DATA_CHUNK_SIZE, words, file)) // check if the new file is full
      {
        file.close();
        continue; // don't execute further since the new file is already full
      } 
    }
     
    string line = ""; // a line for reading

    while(getline(inputFile, line, '\n'))
    {
      line.erase(remove_if(line.begin (), line.end (), punctValue), line.end()); // get rid of punctuation signs
      vector<string> lineWords = splitString(line); // get a vector of words on this line
      line.clear(); // clear the string for a new reading

       
      // check if the file gets full; if yes, copy the remaining of the vector to the global vector; otherwise, do nothing.
      if(readRemain(DATA_CHUNK_SIZE, lineWords, file, getFileSize(file)))
      {
        words.insert(words.end(), lineWords.begin(), lineWords.end()); // append the remaining content to the global vector
        break; // break out of the most-inner while loop
      }       

    }// while  


    file.close(); // close the chunk file  
    
  }// end while 

  inputFile.close(); // close the data file
  
  while(!words.empty()) // read last words (empty buffer)
  {
    stringstream streamString;
    streamString << "chunk_" << counter << ".txt";
    const string title = streamString.str(); // a title for a new chunk file
    counter++; 

    ofstream file(title.c_str(), ios_base::out); // a new text file for writing  
    
    // if some words left from previous line, read them in the newly created file    
    readRemain(DATA_CHUNK_SIZE, words, file); // check if the new file is full
    file.close();    

  }// while  
}



int main (int argc, char** argv)
{

  if(argc == 4)
  {
    stringstream chunkReader(argv[1]);
   
    uint32_t chunk_size;

    chunkReader >> chunk_size;     

    if(chunk_size < MIN_CHUNK_SIZE)
    {
      printf("Your entered chunk size is too small. Please try one more time (min chunk size = %d bytes).\n", MIN_CHUNK_SIZE);
      return 0;
    }

    /*** Start recording time ***/
    T_type t_start = chrono::high_resolution_clock::now();

    partitionData(chunk_size, argv[2]); // run a script for partition a big dataset into smaller ones.

    T_type t_end = chrono::high_resolution_clock::now();

    writeTimespanToFile(t_start, t_end, argv[3]);
  }
  else 
  {
    printf("Please input the chunk size, an input file for reading (.txt) and an output file for storing time (.txt)\n");
  }

  return 0;
}


// this function writes a timespan to a file
void writeTimespanToFile(const T_type& t_start, const T_type& t_end, const char* const filename)
{

    chrono::duration<double> time_span =
        chrono::duration_cast<chrono::duration<double>> (t_end - t_start);
    stringstream timespanStream;
    timespanStream << time_span.count(); // converts time into a tring

    string writeString("Time: ");
    writeString.append(timespanStream.str()).append(" seconds\n"); // create a line for the text file

    ofstream file(filename, ios_base::app); // append times
    file.write(writeString.c_str(), writeString.size()); // write to the file

    file.close();
}

