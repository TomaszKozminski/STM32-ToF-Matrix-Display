#include "Font.h"
#include <ctype.h>
#include <stdatomic.h>
#include <string.h>


// struktura opisująca generator
typedef struct{
    charShape letters[26];
    charShape numbers[10];
    charShape space;
    charShape err;
    uint8_t shapeWidth;
    uint8_t shapeHeight;
}FontData;

/***********************************************************************************
 *                              DANE CZCIONKI
 * *********************************************************************************/

static const charShape sA = {
    .shape = {
        0b1110,
        0b1010,
        0b1110,
        0b1010,
        0b1010
    }  
};

static const charShape sB = {
    .shape = {
        0b1100,
        0b1010,
        0b1100,
        0b1010,
        0b1100
    }  
};

static const charShape sC = {
    .shape = {
        0b0100,
        0b1010,
        0b1000,
        0b1010,
        0b0100
    }  
};

static const charShape sD = {
    .shape = {
        0b1100,
        0b1010,
        0b1010,
        0b1010,
        0b1100
    }  
};

static const charShape sE = {
    .shape = {
        0b1110,
        0b1000,
        0b1100,
        0b1000,
        0b1110
    }  
};

static const charShape sF = {
    .shape = {
        0b1110,
        0b1000,
        0b1110,
        0b1000,
        0b1000
    }  
};

static const charShape sG = {
    .shape = {
        0b1110,
        0b1000,
        0b1011,
        0b1010,
        0b1110
    }  
};

static const charShape sH = {
    .shape = {
        0b1010,
        0b1010,
        0b1110,
        0b1010,
        0b1010
    }  
};

static const charShape sI = {
    .shape = {
        0b0100,
        0b0100,
        0b0100,
        0b0100,
        0b0100
    }  
};

static const charShape sJ = {
    .shape = {
        0b0010,
        0b0010,
        0b0010,
        0b1010,
        0b0100
    }  
};

static const charShape sK = {
    .shape = {
        0b1010,
        0b1010,
        0b1100,
        0b1010,
        0b1010
    }  
};

static const charShape sL = {
    .shape = {
        0b1000,
        0b1000,
        0b1000,
        0b1000,
        0b1110
    }  
};

static const charShape sM = {
    .shape = {
        0b1010,
        0b1110,
        0b1110,
        0b1010,
        0b1010
    }  
};

static const charShape sN = {
    .shape = {
        0b1001,
        0b1101,
        0b1111,
        0b1011,
        0b1001
    }  
};

static const charShape sO = {
    .shape = {
        0b0100,
        0b1010,
        0b1010,
        0b1010,
        0b0100
    }  
};

static const charShape sP = {
    .shape = {
        0b1100,
        0b1010,
        0b1100,
        0b1000,
        0b1000
    }   
};

static const charShape sQ = {
    .shape = {
        0b0100,
        0b1010,
        0b1010,
        0b1010,
        0b0111
    }  
};

static const charShape sR = {
    .shape = {
        0b1100,
        0b1010,
        0b1100,
        0b1010,
        0b1010
    }   
};

static const charShape sS = {
    .shape = {
        0b0110,
        0b1000,
        0b0100,
        0b0010,
        0b1100
    }   
};

static const charShape sT = {
    .shape = {
        0b1110,
        0b0100,
        0b0100,
        0b0100,
        0b0100
    }   
};

static const charShape sU = {
    .shape = {
        0b1010,
        0b1010,
        0b1010,
        0b1010,
        0b1110
    }  
};

static const charShape sV = {
    .shape = {
        0b1010,
        0b1010,
        0b1010,
        0b1010,
        0b0100
    }  
};

static const charShape sW = {
    .shape = {
        0b1010,
        0b1010,
        0b1110,
        0b1110,
        0b1010
    }  
};

static const charShape sX = {
    .shape = {
        0b1010,
        0b1010,
        0b0100,
        0b1010,
        0b1010
    }   
};

static const charShape sY = {
    .shape = {
        0b1010,
        0b1010,
        0b0100,
        0b0100,
        0b0100
    }   
};

static const charShape sZ = {
    .shape = {
        0b1110,
        0b0010,
        0b0100,
        0b1000,
        0b1110
    }   
};

static const charShape s0 = {
    .shape = {
        0b1110,
        0b1010,
        0b1010,
        0b1010,
        0b1110
    }   
};

static const charShape s1 = {
    .shape = {
        0b0110,
        0b0010,
        0b0010,
        0b0010,
        0b0010
    }   
};

static const charShape s2 = {
    .shape = {
        0b0100,
        0b1010,
        0b0010,
        0b0100,
        0b1110
    }   
};

static const charShape s3 = {
    .shape = {
        0b1110,
        0b0010,
        0b1110,
        0b0010,
        0b1110
    }   
};

static const charShape s4 = {
    .shape = {
        0b1010,
        0b1010,
        0b1110,
        0b0010,
        0b0010
    }   
};

static const charShape s5 = {
    .shape = {
        0b1110,
        0b1000,
        0b1110,
        0b0010,
        0b1110
    }   
};

static const charShape s6 = {
    .shape = {
        0b1110,
        0b1000,
        0b1110,
        0b1010,
        0b1110
    }   
};

static const charShape s7 = {
    .shape = {
        0b1110,
        0b0010,
        0b0010,
        0b0100,
        0b0100
    }   
};

static const charShape s8 = {
    .shape = {
        0b1110,
        0b1010,
        0b0100,
        0b1010,
        0b1110
    }   
};

static const charShape s9 = {
    .shape = {
        0b1110,
        0b1010,
        0b1110,
        0b0010,
        0b1110
    }   
};

static const charShape sSpace = {
    .shape = {
        0b0000,
        0b0000,
        0b0000,
        0b0000,
        0b0000
    }   
};

static const charShape sERR = {
    .shape = {
        0b0000,
        0b1110,
        0b1110,
        0b1110,
        0b0000
    }   
};

// generator BASIC
static const FontData basicFont = {
    .letters = {sA,sB,sC,sD,sE,sF,sG,sH,sI,sJ,sK,sL,sM,sN,sO,sP,sQ,sR,sS,sT,sU,sV,sW,sX,sY,sZ},
    .numbers = {s0,s1,s2,s3,s4,s5,s6,s7,s8,s9},
    .space = sSpace,
    .err = sERR,
    .shapeWidth = 4,        // width
    .shapeHeight = 5        // height
};

// biblioteka generatorów
static const FontData Fonts[] = {
    basicFont
};

void Font_GetCharShape(FONT_TYPE fontType, char character, charShape * retShape)
{
    FontData usedFont = Fonts[fontType];

    if(isalpha(character)){
        toupper(character);
        memcpy(retShape, &usedFont.letters[character - 'A'], sizeof(charShape));
    }else if(isdigit(character)){
        memcpy(retShape, &usedFont.numbers[character - '0'], sizeof(charShape));
    }else if(character == ' '){
        memcpy(retShape, &usedFont.space, sizeof(charShape));
    }else{
        memcpy(retShape, &usedFont.err, sizeof(charShape));
    }
}

void Font_GetShapeHeight(FONT_TYPE fontType, uint8_t * retHeight)
{
    *retHeight = Fonts[fontType].shapeHeight;
}

void Font_GetShapeWidth(FONT_TYPE fontType, uint8_t * retWidth)
{
    *retWidth = Fonts[fontType].shapeWidth;
}