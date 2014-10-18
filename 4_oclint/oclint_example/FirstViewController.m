//
//  FirstViewController.m
//  oclint_example
//
//  Created by Hendrik von Prince on 18/09/14.
//  Copyright (c) 2014 Hendrik von Prince. All rights reserved.
//

#import "FirstViewController.h"

@interface FirstViewController ()
-(BOOL)isEqual:(id)object;
@end

@implementation FirstViewController

-(BOOL)isEqual:(id)object {
  return false;
}

-(void)viewWillDisappear:(BOOL)animated {
  [super viewWillDisappear:animated];
}
            
- (void)viewDidLoad {

  int mode = 3;
  switch(mode) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
      NSLog(@"high cyclomatic complexity");
  }
  switch((int)self.view.bounds.size.width) {
    case 768:
      if(self.view.bounds.size.height> 300) {
      } else {
        if(self.view.bounds.size.height< 1024) {
        } else {
          for(int i= 0; i< 10; ++i) {
            switch(i) {
              case 0:
              case 1:
              case 2:
                if(i> 0) {
                } else {
                  for(int j= 0; j< 10; ++j) {
                    
                  }
                }
                break;
              default:
                return;
            }
          }
        }
      }
    default:
      return;
  }
}

-(void)viewWillAppear:(BOOL)animated {
  NSNumber *variableWithAVeryLongLongName= [NSNumber numberWithBool:YES];
  
  if(variableWithAVeryLongLongName.boolValue)
    goto fail;
    goto fail;

  NSLog(@"dead zone");
  
fail:
  NSLog(@"uh oh");
}

@end
