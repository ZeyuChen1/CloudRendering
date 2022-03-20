//
//  main.m
//  CloudRendering
//
//  Created by Zeyu Chen on 2022-03-19.
//

#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION


//#include <Metal/Metal.hpp>


#import <Cocoa/Cocoa.h>
#import "AppDelegate.h"

int main(int argc, const char * argv[]) {
    NSApplication* app = [NSApplication sharedApplication];
    AppDelegate* appDelegate = [[AppDelegate alloc] init];
    [app setDelegate: appDelegate];
    [app run];
    
    return 0;
}
