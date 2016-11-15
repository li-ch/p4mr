#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <iterator>
#include <algorithm>
#include <string>
#include <functional>

using namespace std;


const string FILE_PREFIX = "mapper_";
const string FILE_POSTFIX = "_reducer_";
const string FILE_EXTENSION = ".txt";


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


void map(const uint32_t index, const uint32_t noOfReducers, const char* filename)
{
  
  ofstream reducer_files[noOfReducers]; // create an array of ofstream files for writing to the reducer files
  
  for(uint32_t f_index = 0; f_index < noOfReducers; f_index++) // read one by one to open files (C++x03 is used for getting an index)
  {
   
    // creates a file name that follows the naming convention: mapper_[mapper_index]_reducer_[reducer_index].txt
    string filename(FILE_PREFIX);
    filename.append(("" + index)).append(FILE_POSTFIX).append(("" + f_index)).append(FILE_EXTENSION);    

    (*(reducer_files + f_index)).open(filename.c_str(), ios_base::out); // open a file for writing
  }

  // read the input file and write to the output files
  ifstream inputFile(filename, ios_base::in);

  /*********************************** Input file processing begins here ********************************/ 
  string line(""); // create a line string for reading 
  hash<string> hashFunction; // a hash struct that provides hashing (refer to: http://www.cplusplus.com/reference/functional/hash/?kw=hash)
  
  while (!inputFile.eof())
  {
    getline(inputFile, line, '\n'); // read a line
    line.erase(remove_if(line.begin (), line.end (), punctValue), line.end()); // get rid of punctuation signs

    auto lineValues = splitString(line); // get words on the line
    
    for (const auto& word : lineValues) // read a word and write it to an appropriate file
    {
       auto reducerFileIndex = (hashFunction(word) % noOfReducers); // hash on a word
       (*(reducer_files + reducerFileIndex)).write((word + '\n').c_str(), (word.size() + 1)); // write a word
    }


    line.clear();
  }


  /****************************** Close all the open file ***********************************************/
  inputFile.close(); // close the input file
   
  // close the output files
  for(auto& outputFile : reducer_files)
  {
     outputFile.close(); // close an output file
  }
}


int main (int argc, char** argv)
{
  
  if(argc > 3) // arguments are the following: mapper_index  #of_reducers  input_file_name_for_mapper (.txt)
  {
     uint32_t m_index, m_noOfReducers;
     istringstream map_SS(argv[1]), reducer_SS(argv[2]); // read numbers that are required by the program 
 
     (map_SS >> m_index); // read the index for this map 
     (reducer_SS >> m_noOfReducers); // read the number of reducers
     // start processing the input file
     map(m_index, m_noOfReducers, argv[3]);
  }  
  else
  {
    std::cerr << "Please enter: index the_number_of_reducers input_text_file." << std::endl;
  }

  return 0;
}
