#include<iostream>
#include<fstream>
#include<map>

#include "type_def.hpp"
#include "inst_bag.hpp"
#include "tree.hpp"
#include "dtree_classifier.hpp"

using namespace std;

int main(int argc, char* argv[]) {
    DTreeClassifier c;
    c.train("dtree");

    InstanceBag * bag = c.get_bag();

    cout << "number of inst:" << bag->size() << endl;

    for (size_t i=0; i<bag->size(); ++i) {
        Instance& inst = (*bag)[i];
        int label = c.classify(inst);

        cout << "classification label=" << label << endl;
        cout << "original label=" << inst.get_label_index() << endl;
    }

    return 0;
}
