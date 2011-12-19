//
//  regiontreeTests.m
//  regiontreeTests
//
//  Created by Kiel Gillard on 15/12/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#import "regiontreeTests.h"
#import "RectTree.h"
#import "RectTreePrivate.h"

@implementation regiontreeTests

- (void)setUp
{
    [super setUp];
    
    // Set-up code here.
}

- (void)tearDown
{
    // Tear-down code here.
    
    [super tearDown];
}

- (void)testZeroOriginRectDivision
{
    CGRect r = CGRectMake(0.0, 0.0, 1.0, 1.0);
    CGRect buffer[MAX_QUADRANTS];
    
    KRGRectTreeSubdivideRect(r, buffer);
    
    CGRect expected[MAX_QUADRANTS] = {
        CGRectMake(0.0, 0.0, 0.5, 0.5),
        CGRectMake(0.0, 0.5, 0.5, 0.5),
        CGRectMake(0.5, 0.5, 0.5, 0.5),
        CGRectMake(0.5, 0.0, 0.5, 0.5)
    };
    
    for (unsigned i = 0; i < MAX_QUADRANTS; i++) {
        STAssertTrue(CGRectEqualToRect(buffer[i], expected[i]), @"Rects at %u not equal", i);
    }
}

- (void)testNonZeroOriginRectDivision
{
    CGRect r = CGRectMake(-1.0, -1.0, 1.0, 1.0);
    CGRect buffer[MAX_QUADRANTS];
    
    KRGRectTreeSubdivideRect(r, buffer);
    
    CGRect expected[MAX_QUADRANTS] = {
        CGRectMake(-1.0, -1.0, 0.5, 0.5),
        CGRectMake(-1.0, -0.5, 0.5, 0.5),
        CGRectMake(-0.5, -0.5, 0.5, 0.5),
        CGRectMake(-0.5, -1.0, 0.5, 0.5)
    };
    
    for (unsigned i = 0; i < MAX_QUADRANTS; i++) {
        STAssertTrue(CGRectEqualToRect(buffer[i], expected[i]), @"Rects at %u not equal", i);
    }
}

- (void)testAppendRectInSingleQuadrant
{
    CGRect r = CGRectMake(0.0, 0.0, 1.0, 1.0);
    KRGRectTreeRef tree = KRGRectTreeCreate(r, 1);
    
    r = CGRectMake(0.6, 0.6, 0.3, 0.3);
    KRGRectTreeRectAppend(tree, r);
    
    STAssertTrue(tree->quadrants[2]->rects != NULL, @"Rect should have been stored in quadrant 3");
    
    KRGRectTreeRelease(tree);
}

- (void)testAppendRectInSingleQuadrantDeep
{
    CGRect r = CGRectMake(0.0, 0.0, 1.0, 1.0);
    KRGRectTreeRef tree = KRGRectTreeCreate(r, 2);
    
    r = CGRectMake(0.8, 0.8, 0.1, 0.1);
    KRGRectTreeRectAppend(tree, r);
    
    STAssertTrue(tree->quadrants[2]->quadrants[2]->rects != NULL, @"Rect should have been stored in quadrant 3");
    
    KRGRectTreeRelease(tree);
}

- (void)testHitTestInSingleQuadrantDeep
{
    CGRect r = CGRectMake(0.0, 0.0, 1.0, 1.0);
    KRGRectTreeRef tree = KRGRectTreeCreate(r, 1);
    
    r = CGRectMake(0.8, 0.8, 0.1, 0.1);
    KRGRectTreeRectAppend(tree, r);
    
    CGRect hitRect = KRGRectTreeHitTest(tree, CGPointMake(0.85, 0.85));
    STAssertTrue(CGRectEqualToRect(hitRect, r), @"Didn't find right rect in hit test");
    
    KRGRectTreeRelease(tree);
}

- (void)testDeepQuadrants
{
    CGRect r = CGRectMake(0.0, 0.0, 1.0, 1.0);
    KRGRectTreeRef tree = KRGRectTreeCreate(r, 2); /* divide surface to 2^3 units */
    
    CGRect s = tree->region;
    STAssertTrue(CGRectEqualToRect(r, s), @"Rects should be equal.");
    
    CGRect expected[] = {
        CGRectMake(0.0, 0.0, 0.5, 0.5),
            CGRectMake(0.00, 0.00, 0.25, 0.25),
            CGRectMake(0.00, 0.25, 0.25, 0.25),
            CGRectMake(0.25, 0.25, 0.25, 0.25),
            CGRectMake(0.25, 0.00, 0.25, 0.25),
        CGRectMake(0.0, 0.5, 0.5, 0.5),
            CGRectMake(0.00, 0.50, 0.25, 0.25),
            CGRectMake(0.00, 0.75, 0.25, 0.25),
            CGRectMake(0.25, 0.75, 0.25, 0.25),
            CGRectMake(0.25, 0.50, 0.25, 0.25),
        CGRectMake(0.5, 0.5, 0.5, 0.5),
            CGRectMake(0.50, 0.50, 0.25, 0.25),
            CGRectMake(0.50, 0.75, 0.25, 0.25),
            CGRectMake(0.75, 0.75, 0.25, 0.25),
            CGRectMake(0.75, 0.50, 0.25, 0.25),
        CGRectMake(0.5, 0.0, 0.5, 0.5),
            CGRectMake(0.50, 0.00, 0.25, 0.25),
            CGRectMake(0.50, 0.25, 0.25, 0.25),
            CGRectMake(0.75, 0.25, 0.25, 0.25),
            CGRectMake(0.75, 0.00, 0.25, 0.25)
    };
    
    //for each quadrant of the first layer,
    for (unsigned i = 0, idx = 0; i < MAX_QUADRANTS; i++) {
        
        //get and test it
        KRGRectTreeRef quadrant = tree->quadrants[i];
        STAssertTrue(CGRectEqualToRect(quadrant->region, expected[idx++]), @"Rects at %u not equal", i);
        
        //for each subquadrant,
        for (unsigned j = 0; j < MAX_QUADRANTS; j++) {
            KRGRectTreeRef subquadrant = quadrant->quadrants[j];
            STAssertTrue(CGRectEqualToRect(subquadrant->region, expected[idx++]), @"Rects at %u not equal", i);
        }
    }
    
    KRGRectTreeRelease(tree);
}

- (void)testExample
{
    CGRect r = CGRectMake(0.0, 0.0, 100.0, 100.0);
    KRGRectTreeRef tree = KRGRectTreeCreate(r, 1);
    
    CGRect s = tree->region;
    STAssertTrue(CGRectEqualToRect(r, s), @"Rects should be equal.");
    
    CGRect expected[MAX_QUADRANTS] = {
        CGRectMake(00.0, 00.0, 50.0, 50.0),
        CGRectMake(00.0, 50.0, 50.0, 50.0),
        CGRectMake(50.0, 50.0, 50.0, 50.0),
        CGRectMake(50.0, 00.0, 50.0, 50.0)
    };
    
    for (unsigned i = 0; i < MAX_QUADRANTS; i++) {
        STAssertTrue(CGRectEqualToRect(tree->quadrants[i]->region, expected[i]), @"Rects at %u not equal", i);
    }
    
    KRGRectTreeRelease(tree);
}

@end
