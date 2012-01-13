#include<iostream>
#include<fstream>
#include<map>

#include "type_def.hpp"
#include "inst_bag.hpp"
#include "tree.hpp"
#include "dtree_classifier.hpp"
#include "nb_classifier.hpp"

#include "adaboost.hpp"

using namespace std;

void test_dtree() {
    DTreeClassifier c;
    c.train("dtree");

    InstanceBag * bag = c.get_bag();

    Stats s;
    for (size_t i=0; i<bag->size(); ++i) {
        Instance& inst = (*bag)[i];
        int label = c.classify(inst);
        int original_label = inst.get_label_index() ;
        s.inc(original_label, label);

    }

    cout << "Acc: " << s.get_accuracy() << endl;
    cout << "P: " << s.get_precision() << endl;
    cout << "Recall:" << s.get_recall() << endl;

    c.write_model("DTREE");
}

// naive bayes
void test_nb() {
    NaiveBayesClassifier nb;
    nb.train("dtree");
    InstanceBag * bag = nb.get_bag();
    Stats s;
    for (size_t i=0; i<bag->size(); ++i) {
        Instance& inst = (*bag)[i];
        int label = nb.classify(inst);
        int original_label = inst.get_label_index() ;
        s.inc(original_label, label);
    }

    cout << "Acc: " << s.get_accuracy() << endl;
    cout << "P: " << s.get_precision() << endl;
    cout << "Recall:" << s.get_recall() << endl;

}

void test_adaboost() {
    Adaboost boost;
    boost.train("dtree");
    InstanceBag * bag = boost.get_bag();
    Stats s;
    for (size_t i=0; i<bag->size(); ++i) {
        Instance& inst = (*bag)[i];
        int label = (boost.classify(inst) > 0.5) ? 1: 0;
        int original_label = inst.get_label_index() ;
        s.inc(original_label, label);
    }

    cout << "Acc: " << s.get_accuracy() << endl;
    cout << "P: " << s.get_precision() << endl;
    cout << "Recall:" << s.get_recall() << endl;

}

int main(int argc, char* argv[]) {
//    test_dtree();
//    test_nb();
    test_adaboost();
    return 0;
}
