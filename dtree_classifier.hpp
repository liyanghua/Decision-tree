#ifndef _DTREE_CLASSIFIER_
#define _DTREE_CLASSIFIER_

#include "tree.hpp"
#include "classifier.hpp"

const static char* MODEL = ".model";

class DTreeClassifier: public Classifier {
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
        // convert tree to rules
        void gen_rules(ofstream& out, DTreeNode* node, int level) {
            if (node != NULL) {
                if (node->is_leaf()) {
                    string label;
                    bag->get_label_string(node->label_index, label);
                    out << " THEN " + label << endl;

                }
                else {
                    if (node->parent == NULL) { //root
                        out << "IF " + bag->get_attr_name(node->attr_name_index) + " ";
                    }
                    else {
                        out << " && " << bag->get_attr_name(node->attr_name_index) + " ";
                    }

                    for (size_t i=0; i<node->children.size(); ++i) {
                        int cond_index = node->cond_vec[i];
                        out << " = " << bag->get_attr_val(node->attr_name_index, cond_index);
                        int l = level + 1;
                        gen_rules(out, node->children[i], l);
                    }
                }
            }
        }

        // traversal the tree from root to leaf
        int get_label(Instance& inst, DTreeNode* node) {
            if (node->is_leaf()) {
                return node->label_index;
            }

            int attr_name_index = node->attr_name_index;
            // find the match
            const vector<int>& attr_val_vec = inst.get_attr_val_vec();
            // get the attr value
            assert(attr_name_index >=0 && attr_name_index < (int)attr_val_vec.size());
            int attr_val_index = attr_val_vec[attr_name_index];

            for (size_t j = 0; j<node->cond_vec.size(); ++j) {
                int attr_val = node->cond_vec[j];
                if (attr_val_index == attr_val) {
                    return get_label(inst, node->children[j]);
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
    // TODO
}

void DTreeClassifier::write_model(const string& file) {
    ofstream out((file + MODEL).c_str());
    if (out.is_open()) {
        int level = 0;
        gen_rules(out, model->get_root(), level);
        out.close();
    }
}
#endif
