#include "datetime.h"
#include <iomanip>
#include <time.h>
#include <format>
#include "str.h"

using namespace std;
using namespace std::chrono;

namespace grey::common::datetime {
   std::string to_iso_8601(std::chrono::time_point<std::chrono::system_clock> t) {
      // convert to time_t which will represent the number of
      // seconds since the UNIX epoch, UTC 00:00:00 Thursday, 1st. January 1970
      time_t epoch_seconds = std::chrono::system_clock::to_time_t(t);

      // Format this as date time to seconds resolution
      // e.g. 2016-08-30T08:18:51
      std::stringstream stream;

      struct tm buf;
#if WIN32
      gmtime_s(&buf, &epoch_seconds);
#else
      buf = *gmtime(&epoch_seconds);
#endif

      stream << std::put_time(&buf, "%FT%T");

      // If we now convert back to a time_point we will get the time truncated
      // to whole seconds 
      auto truncated = std::chrono::system_clock::from_time_t(epoch_seconds);

      // Now we subtract this seconds count from the original time to
      // get the number of extra microseconds..
      auto delta_us = std::chrono::duration_cast<std::chrono::microseconds>(t - truncated).count();

      // And append this to the output stream as fractional seconds
      // e.g. 2016-08-30T08:18:51.867479
      stream << "." << std::fixed << std::setw(6) << std::setfill('0') << delta_us;

      return stream.str();
   }

   std::string human_readable_duration(std::chrono::seconds seconds, bool short_format) {

       size_t idays{0}, ihours{0}, iminutes{0}, iseconds{0};

       // get numbers above

       auto rem = seconds;
       auto d_days = duration_cast<days>(rem);
       if(d_days.count() > 0) {
           idays = d_days.count();
           rem -= d_days;
       }

       auto d_hours = duration_cast<hours>(rem);
       if(d_hours.count() > 0) {
           ihours = d_hours.count();
           rem -= d_hours;
       }

       auto d_minutes = duration_cast<minutes>(rem);
       if(d_minutes.count() > 0) {
           iminutes = d_minutes.count();
           rem -= d_minutes;
       }

       if(rem.count() > 0) {
           iseconds = rem.count();
       }

       // now format it
       if(short_format) {
           string s;
           if(idays) {
               s += std::to_string(idays);
               s += "d";
           }
           if(ihours) {
               if(!s.empty()) s += " ";
               s += std::to_string(ihours);
               s += "h";
           }
           if(iminutes) {
               if(!s.empty()) s += " ";
               s += std::to_string(iminutes);
               s += "m";
           }
           if(iseconds) {
               if(!s.empty()) s += " ";
               s += std::to_string(iseconds);
               s += "s";
           }
           return s;
       } else {
           string s;
           if(idays) {
               s += str::humanise(idays, "day", "days");
           }
           if(ihours) {
               if(!s.empty()) s += " ";
               s += str::humanise(ihours, "hour", "hours");
           }
           if(iminutes) {
               if(!s.empty()) s += " ";
               s += str::humanise(iminutes, "minute", "minutes");
           }
           if(iseconds) {
               if(!s.empty()) s += " ";
               s += str::humanise(iseconds, "second", "seconds");
           }
           return s;
       }

   }

   measure::measure() {
       start_time = std::chrono::steady_clock::now();
   }

   measure::~measure() {
   }

   long long measure::take() {
      end_time = std::chrono::steady_clock::now();
      duration = end_time - start_time;
      return duration_cast<milliseconds>(duration).count();
   }
}