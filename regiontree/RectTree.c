//
//  Node.c
//  regiontree
//
//  Created by Kiel Gillard on 15/12/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include <CoreFoundation/CoreFoundation.h>
#include <stdio.h>
#include "RectTree.h"
#include "RectTreePrivate.h"

static const CGPoint QuadrantPoints[MAX_QUADRANTS] = {
    { 0.0, 0.0 },
    { 0.0, 0.5 },
    { 0.5, 0.5 },
    { 0.5, 0.0 }
};

static const void * KRGRectCopy(CFAllocatorRef allocator, const void *value)
{
    CGRect *buffer = calloc(1, sizeof(CGRect));
    memcpy(buffer, value, sizeof(CGRect));
    return buffer;
}

static void KRGRectRelease(CFAllocatorRef allocator, const void *value)
{
    free((void *)value); 
}

static CFStringRef	KRGRectCopyDescription(const void *value)
{
    CGRect *r = (CGRect *)value;
    return CFStringCreateWithFormat(NULL, NULL, CFSTR("x = %.2f, y = %.2f, w = %.2f, h = %.2f"), 
                                    r->origin.x, r->origin.y, r->size.width, r->size.height);
}

static Boolean KRGRectEqualToRect(const void *value1, const void *value2)
{
    CGRect *r1 = (CGRect *)value1;
    CGRect *r2 = (CGRect *)value2;
    return CGRectEqualToRect(*r1, *r2);
}

static const CFArrayCallBacks kKRGRectCallbacks = {
    0,                                          // CFIndex version;
    KRGRectCopy,                                //CFArrayRetainCallBack retain;
    KRGRectRelease,                             //CFArrayReleaseCallBack release;
    KRGRectCopyDescription,                     //CFArrayCopyDescriptionCallBack copyDescription;
    KRGRectEqualToRect                          //CFArrayEqualCallBack equal;
};

static void KRGRectHitTest(const void *value, void *context)
{
    CGRect *r = (CGRect *)value;
    
    if (CGRectContainsPoint(*r, ((KRGRectHitTestContext *)context)->point)) {
        
        ((KRGRectHitTestContext *)context)->hitRect = r;
    }
}

static inline void KRGRectValidateRect(CGRect r)
{
    //reject null rects
    assert(CGRectIsNull(r) != true);
    
    //reject empty rects
    assert((CGRectGetWidth(r) * CGRectGetHeight(r)) > 0.0);
}

void KRGRectTreeSubdivideRect(CGRect rect, CGRect buffer[MAX_QUADRANTS]) {
    CGFloat ox = CGRectGetMinX(rect);
    CGFloat oy = CGRectGetMinY(rect);
    CGFloat w = CGRectGetWidth(rect);
    CGFloat h = CGRectGetHeight(rect);
    
    for (unsigned i = 0; i < MAX_QUADRANTS; i++) {
        buffer[i] = CGRectMake(ox + QuadrantPoints[i].x * w, oy + QuadrantPoints[i].y * h, w/2.0, h/2.0);
    }
}

void KRGRectCreateQuadrants(KRGRectTreeRef parent, unsigned level) {
    
    CGRect buffer[MAX_QUADRANTS];
    
    //subdivide this rect
    KRGRectTreeSubdivideRect(parent->region, buffer);
    
    //for each quadrant of the given parent's rect,
    for (unsigned i = 0; i < MAX_QUADRANTS; i++) {
        
        //create quadrant object for this rect and  relate it to the parent
        KRGRectTree *quadrant = calloc(1, sizeof(KRGRectTree));
        quadrant->region = buffer[i];
        parent->quadrants[i] = quadrant;
        
        //if this quadrant requires further refinement,
        if (level > 1) {
            
            //give this quadrant it's rect and subdivide it
            KRGRectCreateQuadrants(quadrant, (level - 1));
        }
    }
}

KRGRectTreeRef KRGRectTreeCreate(CGRect region, unsigned depth)
{
    KRGRectValidateRect(region);
    
    //validate depth
    depth = (depth > 0 ? depth : 1);
    
    //create the root of the tree
    KRGRectTree *result = calloc(1, sizeof(KRGRectTree));
    result->region = region;
    
    //generate the tree
    KRGRectCreateQuadrants(result, depth);
    
    return result;
}

void KRGRectTreeRelease(KRGRectTreeRef tree)
{
    if (tree) {
        
        for (int i = 0; i < MAX_QUADRANTS; i++) {
            KRGRectTreeRelease(tree->quadrants[i]);
        }
        
        CFMutableArrayRef rects = tree->rects;
        
        if (rects) {
            CFRelease(rects);
        }
        
        free(tree);
    }
}

KRGRectTreeRef KRGRectTreeGetQuadrantForRect(KRGRectTreeRef tree, CGRect rect)
{
    KRGRectTreeRef result;
    
    if (tree && CGRectContainsRect(tree->region, rect)) {
        
        result = tree;
        
        if (tree->quadrants[0]) {
            
            for (unsigned i = 0; i < MAX_QUADRANTS; i++) {
                
                if ((result = KRGRectTreeGetQuadrantForRect(tree->quadrants[i], rect))) {
                    break;
                }
            }
        }
        
    } else {
        
        result = NULL;
    }
    
    return result;
}

KRGRectTreeRef KRGRectTreeGetQuadrantForPoint(KRGRectTreeRef tree, CGPoint point, CGRect *outRect)
{
    KRGRectTreeRef result;
    
    if (tree && CGRectContainsPoint(tree->region, point)) {
        
        //check the quadrants to see if they can refine our hit testing
        for (unsigned i = 0; i < MAX_QUADRANTS; i++) {
            
            //is there a quadrant which can refine our hit testing?
            if ((result = KRGRectTreeGetQuadrantForPoint(tree->quadrants[i], point, outRect))) {
                
                //if so,
                if (result) {
                    
                    //break and attack!
                    break;
                }
            }
        }
        
        //if no quadrant could refine our hit testing,
        if (!result && tree->rects) {
            
            //find the rect which possibly contains our hit point
            KRGRectHitTestContext context;
            context.point = point;
            context.hitRect = NULL;
            CFArrayApplyFunction(tree->rects, CFRangeMake(0, CFArrayGetCount(tree->rects)), KRGRectHitTest, &context);
            
            //if we found a rect which contains the hit point,
            if (context.hitRect) {
                
                //pass it back up to the caller
                *outRect = *context.hitRect;
                result = tree;
            }
        }
        
    } else {
        
        result = NULL;
    }
    
    return result;
}

bool KRGRectTreeRectAppend(KRGRectTreeRef tree, CGRect rect)
{
    KRGRectValidateRect(rect);
    
    KRGRectTreeRef quadrant = KRGRectTreeGetQuadrantForRect(tree, rect);
    bool result;
    
    if (quadrant) {
        
        CFMutableArrayRef rects = quadrant->rects;
        
        if (!rects) {
            
            rects = CFArrayCreateMutable(NULL, 2, &kKRGRectCallbacks);
            quadrant->rects = rects;
        }
        
        CFArrayAppendValue(rects, &rect);
        
        result = true;
        
    } else {
        
        result = false;
    }
    
    return result;
}

CGRect KRGRectTreeHitTest(KRGRectTreeRef tree, CGPoint point)
{
    //reject malformed points
    if (isnan(point.x)) return CGRectNull;
    if (isnan(point.y)) return CGRectNull;
    
    CGRect result = CGRectNull;
    KRGRectTreeGetQuadrantForPoint(tree, point, &result);
    
    return result;
}
