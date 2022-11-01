// common cross-platform image loading functions
// this is really a wrapper around stb_image quickly hacked up to follow RAII rather than provide a library-independent implementation
#include <string>
#include <memory>
#include <vector>

namespace grey::img
{
   class raw_img
   {
   public:
      raw_img(unsigned char* data, size_t x, size_t y) :
         data{ data }, x{ x }, y{ y } {}
      ~raw_img();

      size_t x;   // width
      size_t y;   // height

      unsigned char* get_data() const { return data; }
      
      operator bool() const { return data != nullptr; }

   private:
      unsigned char* data;
   };

   raw_img load_image_from_file(const std::string& path);

   raw_img load_image_from_memory(unsigned char* buffer, unsigned int len);
}