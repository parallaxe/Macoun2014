//
//  SecondViewController.m
//  oclint_example
//
//  Created by Hendrik von Prince on 18/09/14.
//  Copyright (c) 2014 Hendrik von Prince. All rights reserved.
//

#import "SecondViewController.h"

@interface SecondViewController ()

@end

@implementation SecondViewController
            
- (void)viewDidLoad {
  [super viewDidLoad];
  // Do any additional setup after loading the view, typically from a nib.
}

- (void)didReceiveMemoryWarning {
  [super didReceiveMemoryWarning];
  // Dispose of any resources that can be recreated.
}

-(void)viewWillAppear:(BOOL)animated {
  [super viewWillAppear:animated];
  
  if(animated) {
    return;
  }
  [self log:@"test"];
  
}

-(void)viewWillDisappear:(BOOL)animated {
  [super viewWillDisappear:animated];
}

-(NSString *)log:(NSString *)message {
  NSLog(@"log: %@", message);
  return message;
}

@end
