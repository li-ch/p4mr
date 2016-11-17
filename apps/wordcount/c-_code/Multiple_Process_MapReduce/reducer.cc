#include <iostream>
#include <fstream>
#include <sstream>


using namespace std;


void reduce (const int index, const char* filename)
{
  ifstream input_file(filename, ios_base::in); // open the input file (words must be sorted in some order)
  
  ostringstream file_index, counter_string;
  file_index << index; // convert an int to a string 
  ofstream output_file(string("reducer_").append(file_index.str()).append(".txt").c_str(), ios_base::out); // the output file for aggregation/reducing
   
  string currWord(""); // current word for checking
  getline(input_file, currWord, '\n'); // read a line for the first word
  string line(""); // a line for reading
  int counter = 1; // for reducing

  while(!(input_file.eof())) // read the file until it ends
  {
  
     getline(input_file, line, '\n'); // read a line
   
     if(currWord.compare(line)) // means the words do no match ==>  
       // write the current word with its counter and reset the value
     {   
       counter_string << ", " << counter << '\n'; // convert an int into a string       

       string writeLine(currWord);
       writeLine.append(counter_string.str()); 
       output_file.write(writeLine.c_str(), writeLine.size()); // write to the file
       
       counter = 1; // reset the counter
       // clear ostringstream
       counter_string.clear();
       counter_string.str(""); 
       currWord = line; // reset the value of curr word
     }    
     else // means the same word - just increment the counter
     {
       counter++;
     }   
    
     line.clear(); // clear the current content of the line
  }  
   
  // Reading and reducing are done, close the files
  input_file.close();
  output_file.close();
}


int main (int argc, char** argv)
{
  
  if(argc > 2)
  {
     int m_index;
     istringstream reducer_SS(argv[1]); // read numbers that are required by the program (index)
 
     (reducer_SS >> m_index); // read the index for this reducer
  
     // start processing the input file
     reduce(m_index, argv[2]);
  }  
  else
  {
    std::cerr << "Please pass a text file for reading." << std::endl;
  }

  return 0;
}
