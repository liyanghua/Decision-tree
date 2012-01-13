#ifndef _DTREE_H
#define _DTREE_H

#include "type_def.hpp"
#include "inst_bag.hpp"

struct DTreeNode {
    DTreeNode * parent;
    vector<DTreeNode*> children;

    // The test condition, store the index of the attribute values
    // in the instance bag
    vector<int> cond_vec;
    int attr_name_index;
    int label_index;


    DTreeNode() : parent(NULL), attr_name_index(-1), label_index(-1) {
        children.clear();
        cond_vec.clear();
    }

    void dump() {
        if (is_leaf()) {
            cout << "leaf node with label index=" << label_index << endl;
        }
        else {
            cout << "internal node with attr_name_index:" << attr_name_index << endl;
            cout << "and cond_vec: " << endl;
            dump_vector(cond_vec);
        }
    }

    bool is_leaf() {
        return (label_index != -1);
    }
};


class DTree {
    private:
        DTreeNode* root;
        // split strage, IG or IG-ratio
        bool using_ig_ratio;

    public:
        DTree() : root(new DTreeNode()), using_ig_ratio(false) {
        }
        ~DTree() {
			release(root);
			delete root;
			root = NULL;
        }

        DTreeNode* get_root() {
            return root;
        }
		
		void release(DTreeNode* p) {
		    if (p != NULL) {
			    vector<DTreeNode*> & children = p->children;
				for(size_t i=0; i<children.size(); ++i) {
				    if (children[i] != NULL) {
					    release(children[i]);
						children[i] = NULL;
					}
				}
			}
		}

        void build(InstanceBag* inst_bag) {
            assert(inst_bag != NULL);

            vector<int> inst_index;
            inst_index.reserve(inst_bag->size());
            for(size_t i=0; i<inst_bag->size(); ++i) {
                inst_index.push_back(i);
            } 
            
            map<int, int> allowed_attr; 
            for(int i=0; i<inst_bag->get_attr_name_num(); ++i) {
                allowed_attr[i] = i;
            } 

            build_tree(inst_bag, root, inst_index, allowed_attr);
        }

    private:

        // inst_index used to index the instance in instance bag
        void build_tree(InstanceBag* inst_bag, DTreeNode* node, 
                const vector<int>& inst_index, 
                const map<int, int>& allowed_attr) {
            assert(inst_bag != NULL);
#ifdef debug
            cout << "inst_index:" << endl;
            dump_vector(inst_index);

            cout << "allowed_attr:" << endl;
            dump_map(allowed_attr);
#endif

            // compute the entropy at this node
            int label = -1;
            double E = compute_entropy(inst_bag, inst_index, label);
            if (E == 0) {
#ifdef debug
                cout << "Entropy is 0, label this node as leaf, and the label is: " << label << endl;
#endif
                assert(label != -1);
                node->label_index = label;
                node->attr_name_index = -1;
                return;
            }

            if (allowed_attr.size() == 0) {
                // we used all the attributes, but we still dest not get pure subset
                // use the major votes 
                assert(label != -1);
                node->label_index = label;
                node->attr_name_index = -1;
                return;

            }

            
            const vector<AttrName> & attr_name_vec = inst_bag->get_attr_name_vec();
            // choose the best attribute name for the node, i.e., select the minimal entropy
            double min = 999; // set it to a bigger one 
            int best_attr_name_index = -1;

            map<int,int>::const_iterator attr_it(allowed_attr.begin());

            for (; attr_it != allowed_attr.end(); ++attr_it) {
                int i = attr_it->first;
                const AttrName& m = attr_name_vec[i];
                double IG_R = 0;
                double P_E = 0; // partial entropy
                // for each value of the name
                for (size_t j=0; j<m.vals.size(); ++j) {
                    pair<int, double> p = compute_stats(inst_bag, inst_index, i, j);
                    double prob = p.first / (double)inst_index.size();
                    P_E += p.second * prob;
                    IG_R += (- prob) * log2(prob);
                }

                if (using_ig_ratio) {
                    P_E /= IG_R;
                }
                if (min > P_E) {
                    min = P_E;
                    best_attr_name_index = i;
                }
            }

            node->attr_name_index = best_attr_name_index;

            cout << "Select the " << best_attr_name_index << " th attr: " << inst_bag->get_attr_name(best_attr_name_index) << endl;
            // remove the best attr index from the allowed attr map
            map<int, int> new_allowed_attr;
            modify_allowed_attr(allowed_attr, new_allowed_attr, best_attr_name_index);
            // Recursively build each branch
            const AttrName& m = attr_name_vec[best_attr_name_index];
            int node_size = m.vals.size();

            node->children.reserve(node_size);
            node->cond_vec.reserve(node_size);
            for (size_t k=0; k<m.vals.size(); k++) { // for each attr val
                vector<int> new_inst_index;
                modify_inst_vec(inst_bag, inst_index, new_inst_index, best_attr_name_index, k);
                DTreeNode* new_node = new DTreeNode();
                new_node->parent = node;
                node->children.push_back(new_node);
                node->cond_vec.push_back(k);
                build_tree(inst_bag, node->children[k], new_inst_index, new_allowed_attr);
            }
            
        }

        void modify_allowed_attr(const map<int, int>& old_attr, map<int, int>& new_attr, int rm_index) {
            map<int, int>::const_iterator it(old_attr.begin());
            for(; it != old_attr.end(); ++it) {
                if (it->first != rm_index) {
                    new_attr[it->first] = it->second;
                }
            }
        }

        void modify_inst_vec(InstanceBag* inst_bag, const vector<int>& old_inst, vector<int>& new_inst, int i, int j) {
            for (size_t x=0; x<old_inst.size(); ++x) {
                Instance& inst = (*inst_bag)[old_inst[x]];
                if (inst.contains(i, j)) {
                    new_inst.push_back(old_inst[x]);
                }
            }
        }

        pair<int, double> compute_stats(InstanceBag* inst_bag, const vector<int>& inst_index, int attr_name_index, int attr_val_index) {
            map<int, int> label_counts;
            int total = 0;
            for (size_t i=0; i<inst_index.size(); ++i) {
                Instance& inst = (*inst_bag)[inst_index[i]];
                if (inst.contains(attr_name_index, attr_val_index)) {
                    label_counts[inst.get_label_index()] += 1;
                    total += 1;
                }
            }
            
            double E = entropy(label_counts, total);
            pair<int, double> p;
            p.first = total;
            p.second = E;

            return p;
        }


        double entropy(map<int, int>& label_counts, const int total) {
            double E = 0;

            map<int, int>::iterator it(label_counts.begin());
            for (; it != label_counts.end(); ++it) {
                double p = (double) it->second / (double) total;
                E += (-p) * log2(p);
            }

            return E;
        }


        double compute_entropy(InstanceBag* inst_bag, const vector<int>& inst_index, int& major_label) {
            double E = 0;
            int total = inst_index.size();

            map<int, int> label_counts;
            for (size_t i=0; i<inst_index.size(); ++i) {
                label_counts[(*inst_bag)[inst_index[i]].get_label_index()] += 1;
            }
            E = entropy(label_counts, total);
            major_label = get_major_votes(label_counts);

            return E;
        }

        int get_major_votes(const map<int, int>& label_counts) {
            int l = -1;
            int c = 0;

            for (map<int, int>::const_iterator it(label_counts.begin()); it != label_counts.end(); ++it) {
                if (c < it->second) {
                    c = it->second;
                    l = it->first; // the label index
                }
            }

            return l;
        }
};

#endif
