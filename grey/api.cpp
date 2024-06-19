#include "api.h"
#include "backends/win32dx11.h"

using namespace std;
using namespace grey;

X_API void run(app_init_data* aid, const char* manifest)
{
   grey::backends::win32dx11 dx11("API Test");

   dx11.run();
}
