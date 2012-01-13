#ifndef _ADABOOST_HPP
#define _ADABOOST_HPP

#include "dtree_classifier.hpp"
#include "nb_classifier.hpp"
#include <cmath>


class Adaboost {
    private:
        vector<Classifier*> weak_classifier_vec;
        // instance weight 
        vector<double> weight;
        // learner weight used for predicted
        vector<double> learner_weight;

        InstanceBag* bag;

    private:
        int select_learner(InstanceBag* bag, double& loss);
        void update_weight(double w, int learner_index, InstanceBag* bag);
    public:
        void train(const string& file);
        double classify(Instance& inst);

        inline InstanceBag* get_bag() {
            return bag;
        }

        Adaboost();
        ~Adaboost();
};

////////================ simple implementation of Adboost

Adaboost::Adaboost() {
    DTreeClassifier* dtree = new DTreeClassifier();
    assert(dtree != NULL);

    NaiveBayesClassifier* nb = new NaiveBayesClassifier();
    assert(nb != NULL);

    weak_classifier_vec.push_back(dtree);
    weak_classifier_vec.push_back(nb);
}


Adaboost::~Adaboost() {
    for(size_t i=0; i<weak_classifier_vec.size(); ++i) {
        delete weak_classifier_vec[i];
    }
}



int Adaboost::select_learner(InstanceBag* bag, double& loss) {
    assert(bag != NULL);

    double min_loss =9999; // set it to a big one
    int min_learner_index = -1;
    for(size_t i=0; i<weak_classifier_vec.size(); ++i) { // for each learner
        double loss = 0;
        for (size_t j=0; j<bag->size(); ++j) { // for each training instance
            int predicted_label = weak_classifier_vec[i]->classify((*bag)[i]);
            int original_label = (*bag)[i].get_label_index();
            if (original_label != predicted_label) {
                loss += weight[j]; // 0/1 loss
            }
        }
        if (min_loss >= loss) {
            min_loss = loss;
            min_learner_index =i;
        }
    }

    loss = min_loss;

    return min_learner_index;
}


void Adaboost::update_weight(double w, int learner_index, InstanceBag* bag) {
    double e= 2.71828;

    for(size_t i=0; i<weight.size(); ++i) {
        Instance& inst = (*bag)[i];
        int label = weak_classifier_vec[learner_index]->classify(inst);
        int old_label = inst.get_label_index();
	if (label != old_label) { // give more weight to mis-classification item
	    weight[i] *= pow(e, w);
	}
    }    

    util::normalized(weight);
}

double Adaboost::classify(Instance& inst) {
    double res = -1;

    for(size_t i=0; i<weak_classifier_vec.size(); ++i) {
        int pred = weak_classifier_vec[i]->classify(inst);
        res = pred * learner_weight[i];
    }

    return res;
}

void Adaboost::train(const string& file) {
    for(size_t i=0; i<weak_classifier_vec.size(); ++i) {
        // train each model
        weak_classifier_vec[i]->train(file);
    }

    learner_weight.resize(weak_classifier_vec.size());

    // the core Adaboost Algorithm
    assert(weak_classifier_vec[0] != NULL);
    bag = weak_classifier_vec[0]->get_bag();
    assert(bag != NULL);

    size_t inst_num = bag->size();
    double beta = 1.0 / (double)inst_num;
    weight.resize(inst_num, beta);
    
    // max iterator number
    int T = 10;
    int t = 0;

    while (t < T) {
        // find the weak learner that minimize the loss
        double loss = 0;
        int learner_index = select_learner(bag, loss);
        cout << "The" << t << "th step select the " << learner_index << "th classifer. And the loss is: " << loss << endl;
        if (loss >= 0.5) { // the best learner is too weak
            break;
        }
        double a_t = 0.5 * log( double(1 + loss) / double(1 - loss));
        cout << "a_t =" << a_t << endl;

        // update the weight
        learner_weight[learner_index] = a_t;
        update_weight(a_t, learner_index, bag);
        ++t;
    }
}


#endif
