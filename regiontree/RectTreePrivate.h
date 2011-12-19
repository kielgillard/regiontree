//
//  NodePrivate.h
//  regiontree
//
//  Created by Kiel Gillard on 15/12/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//

#ifndef regiontree_RectTreePrivate_h
#define regiontree_RectTreePrivate_h

#define MAX_QUADRANTS 4

typedef struct KRGRectTree {
    CGRect region;
    KRGRectTreeRef quadrants[MAX_QUADRANTS];
    CFMutableArrayRef rects;
} KRGRectTree;

void KRGRectTreeSubdivideRect(CGRect rect, CGRect buffer[MAX_QUADRANTS]);
void KRGRectCreateQuadrants(KRGRectTreeRef parent, unsigned level);
KRGRectTreeRef KRGRectTreeGetQuadrantForRect(KRGRectTreeRef tree, CGRect rect);
KRGRectTreeRef KRGRectTreeGetQuadrantForPoint(KRGRectTreeRef tree, CGPoint point, CGRect *outRect);

typedef struct {
    CGPoint point;
    CGRect *hitRect;
} KRGRectHitTestContext;

static void KRGRectHitTest(const void *value, void *context);

#endif
