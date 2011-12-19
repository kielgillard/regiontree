//
//  Node.h
//  regiontree
//
//  Created by Kiel Gillard on 15/12/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#include <CoreGraphics/CoreGraphics.h>

#ifndef regiontree_RectTree_h
#define regiontree_RectTree_h

typedef struct KRGRectTree* KRGRectTreeRef;

CF_EXTERN_C_BEGIN

/**
 * Objects allow clients to perform hit testing by represent a region
 * subdivided into four quadrants.
 * @param depth How many times a region is subdivided.
 * @param rect The size of the initial region at division 1 (top level region).
 * @result An object representing a region divisible into depth number of quadrants.
 */
CF_EXPORT KRGRectTreeRef KRGRectTreeCreate(CGRect region, unsigned depth);

/**
 * Destroy the motherflipper.
 * @param tree Tree to free.
 */
CF_EXPORT void KRGRectTreeRelease(KRGRectTreeRef tree);

/**
 * Organises the given rectangle into the appropriate quadrant for 
 * future, efficient hit testing.
 * @param tree Sturcture which to organise this rectangle.
 * @param rect Rectangle to organise into the tree.
 * @result false indicates the rectangle couldn't be contained in the tree.
 */
CF_EXPORT bool KRGRectTreeRectAppend(KRGRectTreeRef tree, CGRect rect);

/**
 * @result Rectangle organised by the given tree which contains the point
 * or CGRectNull if the point does not lie in a rectangle organised by the
 * given tree.
 */
CF_EXPORT CGRect KRGRectTreeHitTest(KRGRectTreeRef tree, CGPoint point);

CF_EXTERN_C_END

#endif
