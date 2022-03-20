//
//  RendererView.h
//  CloudRendering
//
//  Created by Zeyu Chen on 2022-03-19.
//

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

NS_ASSUME_NONNULL_BEGIN

@interface RendererView : NSView

- (id) init;
- (void) beginRendering;

@end

NS_ASSUME_NONNULL_END
