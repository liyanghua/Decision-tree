#ifndef _NAIVE_BAYES_CLASSIFIER_HPP
#define _NAIVE_BAYES_CLASSIFIER_HPP
#include "inst_bag.hpp"
#include "classifier.hpp"

class NaiveBayesClassifier : public Classifier{
    private:
        InstanceBag* bag;

        vector<double> label_prob;
       
        // first index: label
        // second index: attribute name index
        // values: probability for each attribute values 
        map<int, map<int, vector<double> > > feature_prob;
    public:
        NaiveBayesClassifier() : bag(new InstanceBag()) {}
        ~NaiveBayesClassifier() {
            delete bag;
        }
        void train(const string& file);
        int classify(Instance& inst);


        InstanceBag* get_bag() {
            return bag;
        }
};

//= The implementation of Naive Bayes
//

void NaiveBayesClassifier::train(const string& file) {
    assert(bag != NULL);
    bag->load_data(file);

    // compute the label prob and feature prob
    const map<int, int>& label_counts = bag->get_label_counts();
    typedef map<int, int>::const_iterator c_it;
    c_it it(label_counts.begin());
    double total = static_cast<double>(bag->size());
    
    label_prob.resize(label_counts.size());
    for(; it != label_counts.end(); ++it) {
        double prob = (double)it->second / total;
        label_prob[it->first] = prob;

        cout << "The label prob for the label:" << it->first << ", is:" << prob << endl;
    }

    // compute the feature prob for each label
    const vector<AttrName>& attr_name_vec = bag->get_attr_name_vec();
    map<int, int> counts;
    // this is not efficient way. Possible improvements can be done when
    // we load the data. we can compute the probibility for each feature
    typedef map<int, int>::iterator m_i;
    for (size_t i=0; i<attr_name_vec.size(); ++i) { // for each attr name
        for (size_t j=0; j<attr_name_vec[i].vals.size(); ++j) { // for each attr values
            counts.clear();
            bag->get_inst_num(counts, i, j);
            m_i iter(counts.begin());
            for(; iter != counts.end(); ++iter) {
                // P[f(i) | C(i)] = 
                // the count of the instance contains f(i) with the label C(i) / the count of the instance with the label C(i)
                double prob = (double)iter->second / label_counts.at(iter->first);
                feature_prob[iter->first][i].push_back(prob);
            }
                   
        }
    }
}

// P(C(i) | D) = P(C(i)) + sum{P(f[j] | c[i])} for j in (0, N) 
// where N denotes the feature index for the instance to be predicted
int NaiveBayesClassifier::classify(Instance& inst) {
    const vector<int>& attr_val_vec = inst.get_attr_val_vec();

    // positive case
    double c1 = -log2(label_prob[0]);

    for(size_t i=0; i<attr_val_vec.size(); ++i) { // for each feature
        c1 += -log2(feature_prob[0][i][attr_val_vec[i]]);
    }

    // negative case
    double c2 = -log2(label_prob[1]);
    for(size_t i=0; i<attr_val_vec.size(); ++i) { // for each feature
        c2 += -log2(feature_prob[1][i][attr_val_vec[i]]);
    }


    if (c1 >= c2) {
        return 0;
    }
    else {
        return 1;
    }
}

#endif
