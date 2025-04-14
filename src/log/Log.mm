#if defined(__APPLE__)
#include <Foundation/Foundation.h>

void _ngenxx_log_apple(const char* log) {
  if (log) {
    NSLog(@"%@", [NSString stringWithCString:log encoding:NSUTF8StringEncoding]);
  }
}

#endif