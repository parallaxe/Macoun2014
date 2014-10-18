#import <Foundation/Foundation.h>

@interface Settings : NSObject
@property(strong, nonatomic) NSString *name;
@property(strong, nonatomic) NSURL *url;
@property(nonatomic) BOOL isValid;
@end

@implementation Settings
@end

@interface Controller : NSObject <NSURLConnectionDelegate>
@property(strong, nonatomic) Settings *settings;
@property(strong, nonatomic) NSURLConnection *connection;
@property(strong, nonatomic) id<NSURLConnectionDelegate> delegate;

- (void)connect;
- (void)disconnect;
@end

@implementation Controller
- (void)connect {
  NSLog(@"entering connect");
  if (!self.settings.url) {
    NSLog(@"error: no URL to connect");
  }
  NSURLRequest *request = [NSURLRequest requestWithURL:self.settings.url];
  self.connection =
      [NSURLConnection connectionWithRequest:request delegate:self];
  [self.connection start];
  NSLog(@"returning from connect");
}

- (void)disconnect {
  [self.connection cancel];
  self.connection = nil;
}

- (void)connection:(NSURLConnection *)connection
    didFailWithError:(NSError *)error {
}

- (void)connection:(NSURLConnection *)connection
    didReceiveResponse:(NSURLResponse *)response {
}

- (void)connection:(NSURLConnection *)connection didReceiveData:(NSData *)data {
}

- (void)connectionDidFinishLoading:(NSURLConnection *)connection {
}

@end