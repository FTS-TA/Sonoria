#include <sonoria/pool.h>
#include <sonoria/algorithmfactory.h> 

using namespace std;
using namespace sonoria;

void setExtractorDefaultOptions(Pool &options);
void setExtractorOptions(const std::string& filename, Pool& options);
void mergeValues(Pool& pool, Pool& options);
void outputToFile(Pool& pool, const string& outputFilename, Pool& options);

