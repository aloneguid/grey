#pragma once
#include <string>
#include <chrono>

namespace grey::common::datetime {
   std::string to_iso_8601(std::chrono::time_point<std::chrono::system_clock> t = std::chrono::system_clock::now());

   std::string human_readable_duration(std::chrono::seconds seconds, bool short_format = true);

   /// <summary>
   /// Really simple time measure utility.
   /// Starts measuring time when constructed. To 
   /// </summary>
   class measure {
   public:
      measure();
      ~measure();

      /// <summary>
      /// Stops measurement and returns number of milliseconds it took.
      /// </summary>
      long long take();


   private:
      std::chrono::steady_clock::time_point start_time;
      std::chrono::steady_clock::time_point end_time;
      std::chrono::nanoseconds duration;
   };
}