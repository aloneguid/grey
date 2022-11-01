#pragma once


#if defined(__WIN32__) || defined(_WIN32)
#if defined(X_EXPORT)
#define X_API __declspec(dllexport)
#else
#define X_API __declspec(dllimport)
#endif
#endif

struct app_init_data
{
   const char* window_title;
};

extern "C"
{
   X_API void run(app_init_data* aid, char* manifest);

   //todo: set_content(const char* id, const char* manifest);
   //todo: get_content

   //void set_property(const char* id, const char* property_name, const char* value);

}