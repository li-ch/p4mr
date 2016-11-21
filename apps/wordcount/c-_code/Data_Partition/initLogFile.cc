#include <fstream>
#include <string>


void initLogFile (const char* const filename)
{
  std::string log_line("dataset_size,chunk_size_bytes,partition_time_seconds\n");
  std::ofstream log_file(filename, std::fstream::ios_base::out); 
  log_file.write(log_line.c_str(), log_line.size());
  
  log_file.close();
  
}


int main (int argc, char** argv)
{
  // initialize the input file
  if(argc > 1)
  {
    initLogFile(argv[1]);
    return 0;
  }
  
  printf("Please enter the filename for time storage (.csv).");

  return 0;
}
