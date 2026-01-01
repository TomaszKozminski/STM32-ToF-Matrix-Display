#ifndef _IMAGE_H_
#define _IMAGE_H_

#include "ImageBackground.h"
#include "ImageMovingItem.h"

typedef struct{
    ImageBackground * Background;
    ImageMovingItem * MovingItem;
}Image;

#endif /* _IMAGE_H_ */