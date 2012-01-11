#ifndef _TYPE_DEF_
#define _TYPE_DEF_

#include<vector>
#include<string>

using namespace std;

// attribute value
typedef string AttrValType;
typedef string Label;

enum AttrNameType {
    ATTR_NAME_DISCRETE = 0,
    ATTR_NAME_CONTINUOUS
};
// attribute name
struct AttrName {
    AttrNameType type;
    string name;
    // each name contains a list of values
    vector<AttrValType> vals;


    AttrName() : type(ATTR_NAME_DISCRETE), name("") {
        vals.clear();
    }

    AttrName(AttrNameType t, string n): type(t), name(n) {
        vals.clear();
    }
};

#endif
