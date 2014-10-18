#import "tokenexample.h"

#define METHOD_RESULT @"hi"

@implementation Foo
// this is a very important method
-(id)bar { 
#ifdef DEBUG
  NSLog(@"debug message");
#endif
  return METHOD_RESULT;
}
@end

