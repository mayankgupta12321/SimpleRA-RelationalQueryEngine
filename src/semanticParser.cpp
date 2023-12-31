#include"global.h"

bool semanticParse(){
    logger.log("semanticParse");
    switch(parsedQuery.queryType){
        case CLEAR: return semanticParseCLEAR();
        case CROSS: return semanticParseCROSS();
        case DISTINCT: return semanticParseDISTINCT();
        case EXPORT_MATRIX: return semanticParseEXPORT_MATRIX();
        case EXPORT: return semanticParseEXPORT();
        case INDEX: return semanticParseINDEX();
        case JOIN: return semanticParseJOIN();
        case LIST: return semanticParseLIST();
        case LOAD_MATRIX: return semanticParseLOAD_MATRIX();
        case LOAD: return semanticParseLOAD();
        case PRINT_MATRIX: return semanticParsePRINT_MATRIX();
        case PRINT: return semanticParsePRINT();
        case PROJECTION: return semanticParsePROJECTION();
        case RENAME_MATRIX: return semanticParseRENAME_MATRIX();
        case RENAME: return semanticParseRENAME();
        case SELECTION: return semanticParseSELECTION();
        case SORT: return semanticParseSORT();
        case SOURCE: return semanticParseSOURCE();
        case TRANSPOSE_MATRIX: return semanticParseTRANSPOSE_MATRIX();
        case CHECKSYMMETRY: return semanticParseCHECKSYMMETRY();
        case COMPUTE: return semanticParseCOMPUTE();
        case ORDER: return semanticParseORDER();
        case GROUP: return semanticParseGROUP();
        default: cout<<"SEMANTIC ERROR"<<endl;
    }

    return false;
}