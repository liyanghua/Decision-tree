#ifndef _DTREE_CLASSIFIER_
#define _DTREE_CLASSIFIER_

#include "tree.hpp"

class DTreeClassifier {
    private:
        DTree* model;
        InstanceBag* bag;
    public:
        DTreeClassifier() : model(new DTree()), bag(new InstanceBag()) {}
        ~DTreeClassifier() {
            delete model;
            delete bag;
        }

        int classify(Instance& inst);
        void train(const string& file);
        
        void load_model(const string& file);
        void write_model(const string& file);

        InstanceBag* get_bag() {
            return bag;
        }
    private:
        int get_label(Instance& inst, DTreeNode* node) {
            if (node->is_leaf()) {
                return node->label_index;
            }
            // find the match
            const vector<int>& attr_val_vec = inst.get_attr_val_vec();
            for (size_t j = 0; j<node->cond_vec.size(); ++j) {
                int attr_val = node->cond_vec[j];
                cout << "check branch:" << attr_val << endl;
                for (size_t i=0; i<attr_val_vec.size(); ++i) {
                    if (attr_val_vec[i] == attr_val) {
                        return get_label(inst, node->children[j]);
                    }
                }
            }

            return -1;
        }
};

//============The implementation of the decision tree

int DTreeClassifier::classify(Instance& inst) {
    assert (model !=  NULL);

    DTreeNode* root = model->get_root();

    return get_label(inst, root);
}

void DTreeClassifier::train(const string& file) {
    bag->load_data(file);
    model->build(bag);
}

void DTreeClassifier::load_model(const string & file) {
}

void DTreeClassifier::write_model(const string& file) {
}
#endif
