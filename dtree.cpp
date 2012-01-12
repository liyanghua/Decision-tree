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

    Stats s;
    for (size_t i=0; i<bag->size(); ++i) {
        Instance& inst = (*bag)[i];
        int label = c.classify(inst);
        int original_label = inst.get_label_index() ;

        // for binary classification, 0: pos, 1, neg in this example
        if (label == 0 && original_label == 0) { // true positive
            s.tp++;
        }
        if (label == 0 && original_label == 1) { // false positive
            s.fp++;
        }

        if (label == 1 && original_label == 0) { // false negative
            s.fn++;

        }

        if (label == 1 && original_label == 1) { // true negative
            s.tn++;
        }
    }

    cout << "Acc: " << s.get_accuracy() << endl;
    cout << "P: " << s.get_precision() << endl;
    cout << "Recall:" << s.get_recall() << endl;

    c.write_model("DTREE");

    return 0;
}
