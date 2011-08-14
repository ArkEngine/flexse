#include <string>
#include <vector>
using namespace std;

class nlp_processor
{
    private:
        nlp_processor(const nlp_processor&);
    public:
        nlp_processor();
        ~nlp_processor();
        void split(char* str, vector<string>& vstr);
};
